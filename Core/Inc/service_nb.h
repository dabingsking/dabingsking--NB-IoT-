/**
 * @file    service_nb.h
 * @brief   NB-IoT 服务层 - 巴法云 TCP 上报接口
 */
#ifndef SERVICE_NB_H
#define SERVICE_NB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_main.h"   /* SensorData_t */

typedef enum {
    NB_OK = 0,
    NB_ERR_NETWORK,   /* 网络未注册 */
    NB_ERR_CONNECT,   /* TCP 连接失败 */
    NB_ERR_SEND,      /* 数据发送失败 */
} NB_Status_t;

/**
 * @brief  完整执行一次巴法云上报：
 *         Init → CheckNetwork → TCPOpen → Auth → Publish → Close → PSM
 * @param  data  传感器采集结果指针
 * @return NB_OK 或对应错误码
 */
NB_Status_t NB_ReportData(const SensorData_t *data);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_NB_H */
