#ifndef _QOS_AGENT_H_
#define _QOS_AGENT_H_

/**
 * Copyright (c) 1998-2014 Tencent Inc. All Rights Reserved
 * 
 * This software is L5 API for c++,
 * If you need php/python/java version, it can be found here: http://cl5.oa.com
 *
 * @file qos_client.h
 * @brief L5 API interface 
 */

/**
 * If you have L5 api,L5Agent and L5 OSS problems, all docs and details are found here:
 * http://cl5.oa.com
 * and you could contact with @h_l5_helper.
 */

/**
 * @history
 *  2014.03.20 version 3.3.1
 *     1. 新增 ApiRouteResultUpdate, 由调用者指定延时单位，API规范以微秒上报到L5Agent,
 *        解决因SPP同步,SPP异步,L5API混合使用的延时单位不同，而导致分配不均衡不平滑.
 *     2. 解决 qos_client.h 在3.3.0与3.2.0接口声明不兼容
 *
 *  2013.11.25 version 3.3.0 
 *     1. 在原先支持sid的基础上，新增支持名字服务 sname,  支持OMG zkname (sid与sname 一一对应)
 *     2. 去掉第一次获取路由失败时输出stderr
 *     3. 解决批量上报延时,因累计而整数溢出的缺陷
 *
 *  2013.09.10 version 3.2.0
 *     1. 可配置静态权重   
 *     2. 解决在CGI模式下可能造成SID的第一台被调机过载
 *
 *  2013.06.17 version 3.1.0
 *     1. 增加l5api及l5agent版本号统计功能
 *     2. 修改l5api及l5agent的通讯地址为 127.0.0.1
 *
 *  2013.05.02 version 2.9.4
 *     1. 修复在同一应用中加载多次l5api静态库导致串包的BUG
 */
#include <netdb.h> 
#include <sys/socket.h> 
#include <stdio.h> 
#include <errno.h> 
#include <sys/time.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>

//using namespace std; // not use namespace for strick mix program

/**
 * L5API return code
 * @brief return code of l5api function
 * @enum _QOS_RTN  >=0 for success, <0 for errors
 */
enum _QOS_RTN
{
    QOS_RTN_OK	   		  = 0,  // success
    QOS_RTN_ACCEPT        = 1,	// success (forward compatiblility)
    QOS_RTN_LROUTE        = 2,  // success (forward compatiblility)
    QOS_RTN_TROUTE        = 3,  // success (forward compatiblility)
    QOS_RTN_STATIC_ROUTE  = 4,  // success (forward compatiblility)
    QOS_RTN_INITED        = 5,  // success (forward compatiblility)

    QOS_RTN_OVERLOAD	  = -10000,	 // sid overload, all ip:port of sid(modid,cmdid) is not available
    QOS_RTN_TIMEOUT       = -9999,	 // timeout
    QOS_RTN_SYSERR        = -9998,   // error
    QOS_RTN_SENDERR       = -9997,   // send error
    QOS_RTN_RECVERR       = -9996,   // recv error
    QOS_MSG_INCOMPLETE    = -9995,   // msg bad format (forward compatiblility)
    QOS_CMD_ERROR         = -9994,   // cmd invalid (forward compatiblility)
    QOS_MSG_CMD_ERROR     = -9993,   // msg cmd invalid (forward compatiblility)
    QOS_INIT_CALLERID_ERROR = -9992, // init callerid error
    QOS_RTN_PARAM_ERROR   = -9991    // parameter error
};

struct QOSREQUESTTMEXTtag;
//单次访问的基本信息
typedef struct QOSREQUESTtag
{
    int			    _flow;      //flow id
    int			    _modid;		//被调模块编码
    int			    _cmd;		//被调接口编码
    std::string		    _host_ip;	//被调主机IP
    unsigned short  _host_port;	//被调主机PORT
    int             _pre;       //pre value

    QOSREQUESTtag(const QOSREQUESTTMEXTtag &route);
    QOSREQUESTtag():_flow(0),_modid(0),_cmd(0),_host_port(0),_pre(0){}
}QOSREQUEST;

//新增带时延信息的接口
struct QOSREQUESTTMEXTtag
{
    int			    _flow;      //flow id
    int			    _modid;		//被调模块编码
    int			    _cmd;		//被调接口编码
    std::string		    _host_ip;	//被调主机IP
    unsigned short  _host_port; //被调主机PORT
    int             _pre;       //pre value 
    int             _delay;     //delay 

    QOSREQUESTTMEXTtag():_flow(0),_modid(0),_cmd(0),_host_port(0),_pre(0),_delay(0){}
    QOSREQUESTTMEXTtag(const QOSREQUEST & route);
};
typedef struct QOSREQUESTTMEXTtag QOSREQUEST_TMEXT;

typedef struct QOSREQUESTMTTCEXTtag
{
    int32_t			_modid;		//被调模块编码
    int32_t			_cmdid;     //被调接口编码
    int64_t			_key;		//被调接口编码
    int32_t			_funid;
    std::string		    _host_ip;	//被调主机IP
    unsigned short	_host_port;	//被调主机PORT
}QOSREQUEST_MTTCEXT;

typedef struct QOSREQUESTMTTC_TMEXTtag
{
    int32_t			_modid;		//被调模块编码
    int32_t			_cmdid;     //被调接口编码
    int64_t			_key;		//被调接口编码
    int32_t			_funid;
    std::string		    _host_ip;	//被调主机IP
    unsigned short  _host_port;	//被调主机PORT
    int32_t			_delay;     //路由访问时延
}QOSREQUEST_MTTCEXT_TM;

typedef struct ROUTEtag
{
    std::string _host_ip;
    unsigned short _host_port;
}QOSROUTE;

typedef struct QOSREQUESTROUTETABLEtag
{
    int			        _modid;	    //被调模块编码
    int			        _cmdid;		//被调接口编码
    std::vector<QOSROUTE>    _route_tb;

    QOSREQUESTROUTETABLEtag():_modid(0),_cmdid(0){}
}QOSREQUEST_RTB;

/**
 * @brief initialize route table for sid(modid,cmdid) to prepare faster cache
 * @note Release Version: 1.7.0 (forward compatiblility)
 * Args:
 *   @param modid:      modid needed to be inited
 *   @param cmdid:      cmdid needed to be inited
 *   @param time_out:   time limit to fetch route, [0.2, 1.0] seconds is recommended
 *   @param err_msg:    error messange when return value<0
 *
 * @return report success or fail
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled 
 */
int ApiInitRoute(int modid, int cmdid, float time_out, std::string& err_msg);

/**
 * @brief get route which return IP:PORT in qos_req._host_ip and qos_req._host_port,
 *        it is recommended that U should use the first function @ApiGetRoute
 * @note Release Version: 1.7.0 (forward compatiblility)
 * Args:
 *   @param qos_req:   request route of sid(qos_req._modid,qos_req._cmd),
 *                     return IP:PORT in qos_req._host_ip and qos_req._host_port
 *   @param time_out:  time limit to fetch route,the actrual time limit is 1s* time_out	
 *   @param err_msg:   error messange when return value<0
 *   @param tm_val:	   time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 *
 * @return report success or fail,
 *         return IP:PORT in qos_req._host_ip and qos_req._host_port when success
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled 
 */
int ApiGetRoute(QOSREQUEST& qos_req,float time_out,std::string& err_msg,struct timeval* tm_val=NULL);

/**
 *  @brief the old 4 function below only reserved for SNG, save it here for history.
 *  Do not remove for forward compatiblility
 */
int ApiGetRoute(QOSREQUEST_MTTCEXT& qos_req,float time_out,std::string& err_msg,struct timeval* tm_val=NULL);
int ApiGetRoute(QOSREQUEST_TMEXT& qos_req,float time_out,std::string& err_msg,struct timeval* tm_val=NULL);
int ApiGetRoute(QOSREQUEST_MTTCEXT_TM& qos_req,float time_out,std::string& err_msg,struct timeval* tm_val=NULL);
int ApiAntiParallelGetRoute(QOSREQUEST_MTTCEXT& qos_req,float time_out,std::string& err_msg,struct timeval* tm_val=NULL);

/**
 * @brief get route of sname which is in space of zkname,
 *        return IP:PORT in qos_req._host_ip and qos_req._host_port
 * @note Release Version: 3.3.1 (forward compatiblility)
 * @warning the function is recommended for zkname
 * Args:
 *   @param qos_req:   request route of sid(qos_req._modid,qos_req._cmd),
 *                     return IP:PORT in qos_req._host_ip and qos_req._host_port
 *   @param sname:     unique name of zkname space
 *   @param time_out:  time limit to fetch route,the actrual time limit is 1s* time_out	
 *   @param err_msg:   error messange when return value<0
 *   @param tm_val:	   time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 *
 * @return success or fail,
 *         return IP:PORT in qos_req._host_ip and qos_req._host_port when success
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled 
 */
int ApiGetRoute(QOSREQUEST& qos_req,std::string &sname,float time_out,std::string& err_msg,struct timeval* tm_val=NULL);

/**
 * @brief report result and delay of qos_req._host_ip which return by ApiGetRoute
 * @note Release Version: 3.3.1 (forward compatiblility)
 * Args:
 *   @param qos_req:	  qos_req(include last _modid,_cmd,_host_ip,_host_port) which return by ApiGetRoute
 *   @param ret:		  report status to L5_agent,0 for normal,<0 for abnormal
 *   @param usetime_type: time unit(enum QOS_TIME_TYPE), microseconds or milliseconds or seconds
 *   @param usetime:      used time
 *   @param err_msg:	  error messange when return value<0
 *   @param tm_val:	      time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 * @return report success or fail
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled 
 */
enum QOS_TIME_TYPE
{
    TIME_MICROSECOND = 0,
    TIME_MILLISECOND = 1,
    TIME_SECOND      = 2
};

int ApiRouteResultUpdate(QOSREQUEST& qos_req,int ret,int usetime_type, int usetime, std::string& err_msg,struct timeval* tm_val);

/**
 * @brief report result and delay of qos_req._host_ip which return by ApiGetRoute,
 *        it is recommended that U should use the first function @ApiRouteResultUpdate
 * @note Release Version: 1.7.0 (forward compatiblility)
 * @warning Please be careful.
 *          U must use the same time unit as other client services (eg. spp, l5api) in one sid(modid, cmdid).
 * Args:
 *   @param qos_req:	  qos_req(include last _modid,_cmd,_host_ip,_host_port) which return by ApiGetRoute
 *   @param ret:		  report status to L5_agent,0 for normal,<0 for abnormal
 *   @param usetime:      used time, microseconds or milliseconds or seconds
 *                        Please be careful.
 *                        U must use the same time unit as other client services (eg. spp, l5api) in one sid(modid, cmdid).
 *   @param err_msg:	  error messange when return value<0
 *   @param tm_val:	      time stamp transfered to api,in order to reduce gettimeofday,default as NULL
 * @return report success or fail
 *   @retval 0 for success
 *   @retval <0 for errors,while err_msg will be filled 
 */
int ApiRouteResultUpdate(QOSREQUEST& qos_req,int ret, int usetime_usec,std::string& err_msg,struct timeval* tm_val=NULL);

/**
 *  @brief the old 4 function below only reserved for SNG, save it here for history.
 *  Do not remove for forward compatiblility
 */
int ApiRouteResultUpdate(QOSREQUEST_MTTCEXT& qos_req,int ret, int usetime_usec,std::string& err_msg,struct timeval* tm_val=NULL);
int ApiRouteResultUpdate(QOSREQUEST_TMEXT& qos_req,int ret,int usetime_usec,std::string& err_msg,struct timeval* tm_val=NULL);
int ApiAntiParallelUpdate(QOSREQUEST_MTTCEXT& qos_req,int ret,int usetime_usec,std::string& err_msg,struct timeval* tm_val=NULL);
int ApiRouteResultUpdate(QOSREQUEST_MTTCEXT_TM& qos_req,int ret,int usetime_usec,std::string& err_msg,struct timeval* tm_val=NULL);

#ifdef OPEN_GHOST
/**
 *  @brief the old function below only reserved for SNG, save it here for history.
 *  Do not remove for forward compatiblility.
 */
int ApiGetRouteTable(QOSREQUEST_RTB &qos_req, std::string& err_msg);
#endif
#endif
