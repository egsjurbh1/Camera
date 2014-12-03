/** \brief 系统全局参数配置
 *
 * \author lq
 * \update 141202
 * \return
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include "system.h"
#include "readconfig.h"

/**< 系统全局参数初始化 */
int SystemInit(SystemPara *sys)
{
    FileBuf buf[CONFIGNUM];
    int ret,location;

     /* 读配置文件 */
    ret = readfile("config.ini", buf);
    if(ret) {
        printf("readfile error.\n");
        return -1;
    }
    /* 参数配置 */
    ret = prasebuffer( "nodenum", &location, buf);
    if(!ret)
        cfg->Camera.nodenum = NODENUM;  //读默认参数
    else
        cfg->Camera.nodenum = atoi(buf[location].keyvalue);

    ret = prasebuffer( "pvalue", &location, buf);
    if(!ret)
        cfg->Camera.pvalue = PVALUE;  //读默认参数
    else
        cfg->Camera.pvalue = atof(buf[location].keyvalue);

    ret = prasebuffer( "strengthincre", &location, buf);
    if(!ret)
        cfg->Camera.strengthincre = STRENGTHINCRE;  //读默认参数
    else
        cfg->Camera.strengthincre = atof(buf[location].keyvalue);

    ret = prasebuffer( "strengthinit", &location, buf);
    if(!ret)
        cfg->Camera.strengthinit = STRENGTHINIT;  //读默认参数
    else
        cfg->Camera.strengthinit = atof(buf[location].keyvalue);

    ret = prasebuffer( "strengthratio", &location, buf);
    if(!ret)
        cfg->Camera.strengthratio = STRENGTHRATIO;  //读默认参数
    else
        cfg->Camera.strengthratio = atof(buf[location].keyvalue);

    ret = prasebuffer( "tvalue", &location, buf);
    if(!ret)
        cfg->Camera.tvalue = TVALUE;  //读默认参数
    else
        cfg->Camera.tvalue = atof(buf[location].keyvalue);

    ret = prasebuffer( "objectnum", &location, buf);
    if(!ret)
        cfg->Object.objectnum = OBJECTNUM;  //读默认参数
    else
        cfg->Object.objectnum = atoi(buf[location].keyvalue);


    sys->writecycle_cmmu = 15;          //统计通信代价的写入周期（秒）
    sys->writecycle_instantc = 1;       //瞬时通信写入周期（秒）
    sys->writecycle_strength = 3;       //节点强度写入周期（秒）
    /* Map */
    sys->Map.crossnum = 2;              //路口数量
    sys->Map.crossid_init = 901;        //路口ID起始编号
    sys->Map.x_max = 100;               //地图默认x轴最大坐标
    sys->Map.y_max = 100;               //地图默认y轴最大坐标
    /* Object */
    sys->Object.objectnum = 4;          //目标数量
    sys->Object.objectid_init = 601;    //目标ID起始编号
    sys->Object.speed[0] = 5;           //目标速度集，坐标/秒
    sys->Object.speed[1] = 2;
    /* Node */
    sys->Node.nodenum = 6;              //节点数量
    sys->Node.nodeid_init = 101;        //节点ID起始编号
    sys->Node.feedbackcost = 0.1;       //反馈消息通信权重
    sys->Node.taskcost = 0.9;           //任务消息通信权重，和为1
    sys->Node.strengthinit = 1.3;       //strength初值
    sys->Node.strengthincre = 1.0;      //strength增量
    sys->Node.strengthratio = 0.1;      //strength系数
    sys->Node.pvalue = 0.01;            //公式参数概率ω
    sys->Node.tvalue = 1.0;             //公式参数阈值
    sys->Node.maxvalue = 5.0;           //strength最大值
    sys->Node.minvalue = 0.0;           //strength最小值
    sys->Node.maxnum = 30;              //节点最大跟踪目标数
    sys->Node.maxload = 0.95;           //节点负载阈值
    sys->Node.fovhalfwidth = 5;         //视域半宽
    sys->Node.fovlength = 10;           //视域长度
    sys->Node.commu_mode = MULTICAST;   //通信方式

    printf("System Parameters Initialize Success.\n");
    return 0;
}








