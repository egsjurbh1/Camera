#ifndef _SYSTEM_H
#define _SYSTEM_H

#define sleep(x) Sleep(1000 * x)    //Sleep毫秒 sleep秒 不同系统可能不含sleep

#define MULTICAST       1       //任务发布模式：组播
#define BROADCAST       2       //任务发布模式：广播
#define NONEID          0       //无编号
#define BUFFER          10      //buffer
#define CONFIGNUM       100     //配置参数数量

#define TRACK_MODE      1       //跟踪目标模式
#define TASK_MODE       2       //任务分配模式

#define METHOD_TQTA     1       //TQTA方法
#define METHOD_QTA      2       //QTA方法
#define METHOD_TA       3       //TA方法

/* Map参数结构体 */
typedef struct MapParametersStruct
{
    int x_max;              //地图默认x轴最大坐标
    int y_max;              //地图默认y轴最大坐标
    int crossnum;           //路口数量
    int crossid_init;       //路口ID起始编号
}MapPara;

/* Object参数结构体 */
typedef struct ObjectParametersStruct
{
    int objectnum;                  //目标数量
    int objectid_init;              //目标ID起始编号
    int speed[BUFFER];              //目标速度
}ObjectPara;

/* Node参数结构体 */
typedef struct NodeParametersStruct
{
    int nodenum;            //节点数量
    int nodeid_init;        //节点ID起始编号
    int maxnum;             //节点最大跟踪目标数
    float maxload;          //节点负载阈值
    int commu_mode;         //节点通信方式
    int fovlength;          //视域长度
    int fovhalfwidth;       //视域半宽
    float strengthinit;     //strength初值
    float strengthincre;    //strength增量
    float strengthratio;    //strength系数
    float pvalue;           //公式参数概率ω
    float tvalue;           //公式参数阈值
    float maxvalue;         //strength最大值
    float minvalue;         //strength最小值
    float taskcost;         //任务消息通信权重
    float feedbackcost;     //反馈消息通信权重
}NodePara;

/* Task参数结构体 */
typedef struct TaskParametersStruct
{
    int tasknum;                  //目标数量
    int taskid_init;              //目标ID起始编号
    int method_type;              //任务分配方法
    float qos_para;               //QoS参数
}TaskPara;

/* 系统参数结构体 */
typedef struct SystemParametersStruct
{
    int system_mode;            //系统运行模式
    int writecycle_cmmu;        //统计通信代价的写入周期（秒）
    int writecycle_strength;    //节点强度写入周期（秒）
    int writecycle_instantc;    //瞬时通信写入周期（秒）
    MapPara Map;
    ObjectPara Object;
    NodePara Node;
    TaskPara Task;
}SystemPara;

extern int SystemInit( SystemPara *sys);                 //系统参数初始化

#endif // _SYSTEM_H


