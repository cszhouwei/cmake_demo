#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
			 
#include <semaphore.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <errno.h>
#include "Attr_API.h"

#define ATTR_SHM_ID             24555//基础共享内存
#define ADVAN_SHM_ID            24551//进程端口共享内存
#define DEP_SHM_ID              24552//告警共享内存
#define DEP_SHM_LOCK_ID         34552

//特性id号，表示里面是个带上报ip的特性值
#define	ATTR_EXE_DATA				12345678 
//特性id号，表示里面是个带上报ip的特性字符串
#define	ATTR_EXE_STR				12345679

// modified by arrowliu, 2006-08-15
// 支持1000个特性在Makefile中控制
#ifndef MAX_ATTR_NODE 
#define MAX_ATTR_NODE 1000
#endif

// added by noradcai 2009-03-17
#define MAX_ATTR_NODE_FOR_OLD_SHM	100	//老版本的共享内存最多特性数
#define SIZE_FOR_SHM_CHECK		1		//检测共享内存时,shmget用到的size.定义为最小的size,即1.
// checkshm result
#define CHECK_SHM_ERROR		-1
#define CHECK_SHM_NO_EXIST	1
#define CHECK_SHM_SUCCESS	0

#define ATTR_SUB_ID             1

#define MAX_NETINFO_SIZE 256*1024
#define MAX_PROCINFO_SIZE 256*1024
#define MAX_USRINFO_SIZE 256*1024

#define DEP_ATTTIBUTE_BASE 600
#define MAX_DEP_SHMEM_LEN  2*1024*1024
#define MAX_DEP_VALUE_LEN  (MAX_DEP_SHMEM_LEN - sizeof(int))




typedef struct
{
	int iUse;
	int iAttrID;
	int iCurValue;
} AttrNode;

typedef struct
{
	AttrNode astNode[MAX_ATTR_NODE];
} AttrList;

static AttrList *pstAttr=NULL; //存放整形等固定内容的共享内存
static char* g_pBusiShareMem = NULL;
static int g_shmLockId = -1;


// added by noradcai, 2009-03-17
static int g_max_attr_node = MAX_ATTR_NODE;;
static int g_shm_size = 0;

//是否已经进行内存的检查.程序第一次调用Attr_API,Attr_API_Set或Get_Attr_Value时,需要CheckShm,
//以便指定g_max_attr_node和g_shm_size的值.以后的调用就不需要CheckShm了.
static int g_shm_check_flag = 0;

int getMaxAttrNode()
{
	return g_max_attr_node;
}
int getShmSize()
{
	return g_shm_size;
}

int needCheckShm()
{
	return g_shm_check_flag == 0 ? 1 : 0;

}

void setShmCheckFlag()
{
	g_shm_check_flag = 1;

}


// added by noradcai, 2009-03-17
// 根据agent运行环境中的共享内存大小指定g_max_attr_node及g_shm_size的值
int CheckShm()
{
	struct shmid_ds shmInfo;
	int shmID = -1;
	if((shmID = shmget(ATTR_SHM_ID, SIZE_FOR_SHM_CHECK, 0666 & (~IPC_CREAT))) < 0)
	{
		
		if(errno == ENOENT)
		{	
			//共享内存不存在,返回1,表示需要新建共享内存
			printf("share memory not exist, need create new share memory!\n");
			g_max_attr_node = MAX_ATTR_NODE;
			g_shm_size = sizeof(AttrNode) * g_max_attr_node;
			return CHECK_SHM_NO_EXIST;
		}
		else
		{
			perror("shmget failed!!");
			return CHECK_SHM_ERROR;
		}
	}

	if(shmctl(shmID, IPC_STAT, &shmInfo) < 0)
	{
		perror("shmctl failed!!");
		return CHECK_SHM_ERROR;
	}

//	printf("size:%d attach:%d\n", shmInfo.shm_segsz, shmInfo.shm_nattch);
	if(shmInfo.shm_segsz < sizeof(AttrList))
	{
		g_max_attr_node = MAX_ATTR_NODE_FOR_OLD_SHM;
		
	}
	else
	{
		g_max_attr_node = MAX_ATTR_NODE;
	}
	
	g_shm_size = sizeof(AttrNode) * g_max_attr_node;
	
//	printf("g_max_attr_node=%d\n",g_max_attr_node);
	return CHECK_SHM_SUCCESS;
}

static char* GetShm(int iKey, int iSize, int iFlag)
{
	int iShmID;
	char* sShm;
	char sErrMsg[50];

	if ((iShmID = shmget(iKey, iSize, iFlag)) < 0) 
	{
		sprintf(sErrMsg, "shmget %d %d", iKey, iSize);
		fprintf(stderr, "%s: %s", sErrMsg, strerror(errno));
		return NULL;
	}
	if ((sShm = shmat(iShmID, NULL ,0)) == (char *) -1)
	{
		fprintf(stderr, "shmat: %s", strerror(errno));
		return NULL;
	}
	return sShm;
}

static int GetShm2(void **pstShm, int iShmID, int iSize, int iFlag)
{
	char* sShm;

	if (!(sShm = GetShm(iShmID, iSize, iFlag & (~IPC_CREAT))))
	{
		if (!(iFlag & IPC_CREAT)) return -1;
		if (!(sShm = GetShm(iShmID, iSize, iFlag))) return -1;

		memset(sShm, 0, iSize);
	}
	*pstShm = sShm;
	return 0;
}

int SearchAttrID(AttrList *pstAttr,int attr,int *piPos)
{
	int i=0;
	*piPos=0;

	while(pstAttr->astNode[i].iUse)
	{
		if(pstAttr->astNode[i].iAttrID==attr)
		{
			*piPos=i;
			return 1;
		}
		++i;

		//if(i>MAX_ATTR_NODE)return -1;
		if(i>=getMaxAttrNode())
		{			
			return -1;
		}
	}

	//if(!*piPos) *piPos=i;
	*piPos = i;

	return 0;
}

int Get_Attr_Value(int attr,int *iValue)
{
	int AttrPos, iRet;

	// added by noradcai 2009-03-30 增加对共享内存的检查
	if(needCheckShm())
	{
		if(CheckShm()!=CHECK_SHM_SUCCESS)
		{
			fprintf(stderr, "check shm error!\n");
			return -1;
		}
		setShmCheckFlag();
	}

	if (!pstAttr&&GetShm2((void**)&pstAttr, ATTR_SHM_ID, getShmSize(), 0666) < 0)
	{
		fprintf(stderr, "can not get shm ...continue!\n");
		return -1;
	}

	iRet = SearchAttrID(pstAttr,attr,&AttrPos);

	if(iRet == 1)//返回0 attr不存在，返回1 attr存在
	{
		*iValue = pstAttr->astNode[AttrPos].iCurValue;
	}
	else 
	{
		return -1;
	}

	return 0;
}

int Attr_API(int attr,int iValue)
{
	int AttrPos, iRet;

	// added by noradcai 2009-03-30 增加对共享内存的检查
	if(needCheckShm())
	{
		if(CheckShm()!=CHECK_SHM_SUCCESS)
		{
			fprintf(stderr, "check shm error!\n");
			return -1;
		}
		setShmCheckFlag();
	}

	if (!pstAttr&&GetShm2((void**)&pstAttr, ATTR_SHM_ID, getShmSize(), 0666) < 0)
	{
		fprintf(stderr, "get share memory error\n");
		return -1;
	}

	iRet = SearchAttrID(pstAttr,attr,&AttrPos);

	if(!iRet)//返回0 attr不存在，返回1 attr存在
	{
		pstAttr->astNode[AttrPos].iUse=1;
		pstAttr->astNode[AttrPos].iAttrID=attr;
		pstAttr->astNode[AttrPos].iCurValue=iValue;
	}
	else if(iRet==-1)
	{
		fprintf(stderr, "%s %d: out of memory. maxAttrNode=%d\n", __FILE__, __LINE__,getMaxAttrNode());
		return -1;
	}
	else
	{
		pstAttr->astNode[AttrPos].iCurValue+=iValue;
	}

	return 0;
}

int Attr_API_Set(int attr,int iValue)
{
	int AttrPos, iRet;

	// added by noradcai 2009-03-30 增加对共享内存的检查
	if(needCheckShm())
	{
		if(CheckShm()!=CHECK_SHM_SUCCESS)
		{
			fprintf(stderr, "check shm error!\n");
			return -1;
		}
		setShmCheckFlag();
	}

	if (!pstAttr&&GetShm2((void**)&pstAttr, ATTR_SHM_ID, getShmSize(), 0666) < 0)
	{		
		fprintf(stderr, "get share memory error\n");
		return -1;
	}

	iRet = SearchAttrID(pstAttr,attr,&AttrPos);

	if(!iRet)//返回0 attr不存在，返回1 attr存在
	{
		pstAttr->astNode[AttrPos].iUse=1;
		pstAttr->astNode[AttrPos].iAttrID=attr;
		pstAttr->astNode[AttrPos].iCurValue=iValue;
	}
	else if(iRet==-1)
	{
		fprintf(stderr, "%s %d: out of memory. maxAttrNode=%d\n", __FILE__, __LINE__,getMaxAttrNode());
		return -1;
	}
	else
	{
		pstAttr->astNode[AttrPos].iCurValue=iValue;
	}

	return 0;
}

static int semlock()
{
	struct sembuf lock[1];
	int ret = 0;
	int lockid = semget(DEP_SHM_LOCK_ID, 1, 0);
	if (lockid == -1)
	{
		perror("get singal error\n");
		return -1;
	}

	lock[0].sem_num = 0;
	lock[0].sem_op  = -1;
	lock[0].sem_flg = SEM_UNDO;

	ret = semop(lockid, lock, 1);
	if (ret<0)
		perror("op signal error\n");

	return ret;
}

static int semunlock()
{
	int ret = 0;
	struct sembuf lock[1];
	int lockid = semget(DEP_SHM_LOCK_ID, 1, 0);
	if (lockid == -1)
		return -1;

	lock[0].sem_num = 0;
	lock[0].sem_op  = 1;
	lock[0].sem_flg = SEM_UNDO;

	ret = semop(lockid, lock, 1);
	return ret;
}

//初始化信号量，在agent启动时调用一次
int init_sem()
{
	if (GetShm2((void**)&g_pBusiShareMem, DEP_SHM_ID , MAX_DEP_SHMEM_LEN , 0666 | IPC_CREAT) < 0)
	{
		fprintf(stderr, "Can't create business share memory\n");
		return -1;
	}
	memset(g_pBusiShareMem , 0 , MAX_DEP_SHMEM_LEN);

	g_shmLockId = semget(DEP_SHM_LOCK_ID, 1, IPC_EXCL);
	if(g_shmLockId != -1)
	{
		semctl(g_shmLockId, 0, IPC_RMID);
	}

	g_shmLockId = semget(DEP_SHM_LOCK_ID, 1, 0666 | IPC_CREAT|IPC_EXCL);
	if(g_shmLockId < 0)
	{	
		perror("create signal failed\n");
		return -1;
	}
	if(semctl(g_shmLockId , 0 , SETVAL,1) < 0) 
	{
		perror("create signal failed\n");
		return -1;
	}

	return g_shmLockId;
}


//设置业务部门的数据到共享内存
int adv_attr_set(int attr_id , size_t len , char* pvalue)
{
	char *p = NULL;
	int *plen = NULL;
	if (attr_id < DEP_ATTTIBUTE_BASE ||
			len == 0 ||
			pvalue == NULL)
		return -1;

	//加锁
	if (semlock() < 0)
		return -1;

	if (!g_pBusiShareMem && GetShm2((void**)&g_pBusiShareMem, DEP_SHM_ID, MAX_DEP_SHMEM_LEN , 0666) < 0)
	{
		semunlock();
		return -1;// 修正无法获取共享内存问题,modified by arrowliu, 2006-03-29
	}

	//取可用长度
	plen = (int*)g_pBusiShareMem;
	if (plen == NULL)
	{
		semunlock();
		return -1;
	}

	if (*plen > MAX_DEP_VALUE_LEN || (MAX_DEP_VALUE_LEN - *plen) < len + 8)	// modified by arrowliu, 2006-08-10
	{
		semunlock();
		return -1;
	}

	//copy value
	p = g_pBusiShareMem;
	p += sizeof(int);//all the data length
	p += *plen;

	*plen += len + 2*sizeof(int);//increase length,一个整形是长度，一个是id

	//first 4byte is length of data
	*(int*)p = htonl(len);

	//second 4byte is attribute id of data
	p += sizeof(int);
	*(int*)p = htonl(attr_id);

	//other buffer for the data
	p += sizeof(int);
	memcpy(p , pvalue , len);

	semunlock();
	return 0;
}

int get_adv_memusedlen()
{
	int *plen = NULL;
	int len = 0;

	//加锁
	if (semlock() < 0)
		return -1;

	if (!g_pBusiShareMem && GetShm2((void**)&g_pBusiShareMem, DEP_SHM_ID, MAX_DEP_SHMEM_LEN , 0666) < 0)
	{
		semunlock();
		return -1;
	}

	//取已用长度
	plen = (int*)g_pBusiShareMem;
	if (plen == NULL)
	{
		semunlock();
		return -1;
	}

	if (*plen < 0 || *plen > (int)MAX_DEP_VALUE_LEN)
	{
		semunlock();
		return -1;
	}

	len = *plen;
	semunlock();  
	return len;
}

	//取可用部门共享内存大小
int get_adv_memlen()
{
	int *plen = NULL;
	int len = 0;

	//加锁
	if (semlock() < 0)
		return -1;

	if (!g_pBusiShareMem && GetShm2((void**)&g_pBusiShareMem, DEP_SHM_ID, MAX_DEP_SHMEM_LEN , 0666) < 0)
	{
		semunlock();
		return -1;
	}

	//取已用长度
	plen = (int*)g_pBusiShareMem;
	if (plen == NULL)
	{
		semunlock();
		return -1;
	}

	if (*plen < 0 || *plen > (int)MAX_DEP_VALUE_LEN)
	{
		semunlock();
		return -1;
	}

	len = *plen;
	semunlock();
	return MAX_DEP_VALUE_LEN - len;
}

//取部门共享内存的内容,注意pOut调用者分配，且大小一定要等于或大于len
int get_adv_mem(size_t offset , size_t len , char* pOut)
{
	int actual_size = len;
	int *plen = NULL;
	if (offset >= MAX_DEP_VALUE_LEN ||
			len < 1 ||
			pOut == NULL)
		return -1;

	memset(pOut , 0 , len);

	//加锁
	if (semlock() < 0)
		return -1;

	if (!g_pBusiShareMem && GetShm2((void**)&g_pBusiShareMem, DEP_SHM_ID, MAX_DEP_SHMEM_LEN , 0666) < 0)
	{
		semunlock();
		return -1;
	}

	//取已用长度
	plen = (int*)g_pBusiShareMem;
	if (plen == NULL)
	{
		semunlock();
		return -1;
	}

	if (actual_size > *plen)
		actual_size = *plen;

	memcpy(pOut , g_pBusiShareMem + sizeof(int) + offset ,actual_size);
	semunlock();
	return actual_size;
}

int free_adv_mem()
{
	int *plen = NULL;
	// 	int len = 0;

	//加锁
	if (semlock() < 0)
		return -1;

	if (!g_pBusiShareMem && GetShm2((void**)&g_pBusiShareMem, DEP_SHM_ID, MAX_DEP_SHMEM_LEN , 0666) < 0)
	{
		semunlock();
		return -1;
	}


	//取已用长度
	plen = (int*)g_pBusiShareMem;
	if (plen == NULL)
	{
		semunlock();
		return -1;
	}

	if (*plen < 0 || *plen > (int)MAX_DEP_VALUE_LEN)
	{
		semunlock();
		return -1;
	}

	memset(g_pBusiShareMem + sizeof(int) , 0 , *plen);

	*plen = 0;
	semunlock();
	return 0;
}

#define ULONG unsigned long 
#define setULong_H(_pos, value) {*((ULONG*)(_pos))=htonl(value);}

/**
 * 带IP上报数值型业务特性值
 * strIP: 字符串IP地址
 * iAttrID: 特性id
 * iValue: 特性值
 * 成功返回0，失败返回-1
*/
int setNumAttrWithIP(const char* strIP, int iAttrID, int iValue)
{
	char buf[12];
	char* p = NULL;
	long iIP = -1;
	struct in_addr inaddr;

	if (!strIP)
	{
		fprintf(stderr, "strIP is NULL, error!\n");
		return -1;
	}

	if (0==inet_aton(strIP, &inaddr))
	{
		fprintf(stderr, "invalid IP: %s\n", strIP);
		return -1;
	}

	iIP = inaddr.s_addr;

	if (iAttrID <=0)
	{
		fprintf(stderr, "invalid iAttrID: %d\n", iAttrID);
		return -1;
	}

	// 封装协议，网络字节序
	// 4个字节的IP, 4个字节id，4个字节value
	bzero(buf, sizeof(buf));
	p = buf;
	setULong_H(p, iIP);
	p+=4;

	setULong_H(p, iAttrID);
	p+=4;

	setULong_H(p, iValue);
	p+=4;

	return adv_attr_set(ATTR_EXE_DATA, sizeof(buf), buf);
}

/**
 * 带IP上报字节型业务特性
 * strIP: 字符串IP地址
 * iAttrID: 特性id
 * len: 字节串特性长度
 * pval: 字节串特性首地址
 * 成功返回0，失败返回-1
*/
int setStrAttrWithIP(const char* strIP, int iAttrID, size_t len , const char* pval)
{
	char buf[65536];
	char* p = NULL;
	long iIP = -1;
	struct in_addr inaddr;

	if (len > sizeof(buf))
	{
		fprintf(stderr, "len is %zd, longger than 65536!\n", len);
		return -1;
	}
	
	if (!strIP)
	{
		fprintf(stderr, "strIP is NULL, error!\n");
		return -1;
	}

	if (0==inet_aton(strIP, &inaddr))
	{
		fprintf(stderr, "invalid IP: %s\n", strIP);
		return -1;
	}

	iIP = inaddr.s_addr;

	if (iAttrID <=0)
	{
		fprintf(stderr, "invalid iAttrID: %d\n", iAttrID);
		return -1;
	}

	// 封装协议，网络字节序
	// 4个字节的IP, 4个字节id，4个字节长度，len个字节内容
	bzero(buf, sizeof(buf));
	p = buf;
	setULong_H(p, iIP);
	p+=4;

	setULong_H(p, iAttrID);
	p+=4;

	setULong_H(p, len);
	p+=4;

	memcpy(p, pval, len);
	p+=len;

	return adv_attr_set(ATTR_EXE_STR, p-buf, buf);
}

/**
 * 进程独占锁，只要进程存在，就保持锁，直到进程退出；
 * 如果信号量未创建，创建信号量，并初始化信号量为1，再加独占锁；
 * 如果信号量已创建，尝试加独占锁，加锁失败，表明已经运行了该程序，退出；
 * 由于最多只有一个进程能够获取独占锁，该程序能避免同一个程序的多个进程运行
 * semid: 信号量id
 * return 0成功，否则失败
*/
int ExclusiveLock(int semid)
{
	struct sembuf ops[1];	
	int ret = -1;

	int lockid = -1;
	lockid  = semget(semid, 1, 0666 | IPC_CREAT | IPC_EXCL );
	if(lockid != -1)
	{// 原来不存在，需要初始化为1
		if (semctl(lockid , 0 , SETVAL, 1) < 0)
		{
			perror("create signal failed");
			return -1;
		}
	}
	else
	{// 已经存在，获取该信号量
		lockid  = semget(semid, 1, 0666);
		if(lockid == -1)
		{
			perror("get signal failed");
			return -1;
		}
	}

	// 申请信号量锁
	ops[0].sem_num = 0;
	ops[0].sem_op = -1;
	ops[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

	ret = semop(lockid, ops, 1);
	if (ret==-1)
	{
		fprintf(stderr, "Get lock failed: %s.\n", strerror(errno));
		return (-1);
	}

	return 0;
}

