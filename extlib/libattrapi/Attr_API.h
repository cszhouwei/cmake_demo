#ifndef ATTR_API_H
#define ATTR_API_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * 向部门共享内存写数值型特性值
 * 如果指定特性已被设置值,新值覆盖原值
 * attr: 特性ID
 * iValue: 特性值
 * return: -1表示失败;0表示成功
 */
int Attr_API_Set(int attr,int iValue);

/**
 * 获取数值型特性的值
 * 如果指定特性已被设置值,新值覆盖原值
 * attr: 特性ID
 * iValue: 特性值存储地址
 * return: -1表示失败;0表示成功 
 */
int Get_Attr_Value(int attr,int *iValue); 

/**
 * 不推荐使用
 * 向部门共享内存写数值型特性值
 * 如果指定特性已被设置值,新值被累加到原值上
 * attr: 特性ID
 * iValue: 累加值
 */
int Attr_API(int attr,int iValue);

/**
 * 取未用部门共享内存大小
 * 返回值－1表示失败;其他表示未用共享内存大小
 */ 
int get_adv_memlen();

/**
 * 取已用部门共享内存大小
 * 返回值－1表示失败;其他表示已用用共享内存大小
 */  
int get_adv_memusedlen();

/**
 * 向部门共享内存写数据,让agent发送给网管服务器处理
 * attr_id:数据的属性id，从600开始，小于600为非法
 * len数据长度，需要小于共享内存的可用大小，共享内存初始可用大小是2Mk － sizeof(int)
 * pvalue:部门实际业务数据，非空。
 * 返回值0表示成功，其他失败
 * 调用注意：可以连续多次调用设置不同数据，数据将被依次往后排列，直到2M
 */ 
int adv_attr_set(int attr_id , size_t len , char* pvalue);

/**
 * 取部门共享内存的内容,注意pOut调用者分配，且大小一定要等于或大于len
 * offset:偏移量，表示从部门共享内存开始offset长度开始取值
 * len：取的内容的长度，如果大于共享内存大小，取共享内存最大长度
 * pOut:输出内容buffer，由调用者分配，注意大小一定要等于或大于len
 */ 
int get_adv_mem(size_t offset , size_t len , char* pOut);

/**
 * 带IP上报数值型业务特性性值
 * strIP: 字符串IP地址
 * iAttrID: 特性id
 * iValue: 特性值
 * 成功返回0，失败返回-1
 */ 
int setNumAttrWithIP(const char* strIP, int iAttrID, int iValue);

/**
 * 带IP上报字节型业务特性
 * strIP: 字符串IP地址
 * iAttrID: 特性id
 * len: 字节串特性长度
 * pval: 字节串特性首地址
 * 成功返回0，失败返回-1
*/
int setStrAttrWithIP(const char* strIP, int iAttrID, size_t len , const char* pval);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // ATTR_API_H

