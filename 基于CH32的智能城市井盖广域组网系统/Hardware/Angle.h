#ifndef __ANGLE_H
#define __ANGLE_H

typedef struct 
{
    float LastP;	            // 上次估算协方差 初始化值为0.02
    float Now_P;	            // 当前估算协方差 初始化值为0
    float out;		            // 卡尔曼滤波器输出 初始化值为0
    float Kg;			        // 卡尔曼增益 初始化值为0
    float Q;			        // 过程噪声协方差 初始化值为0.001
    float R;			        // 观测噪声协方差 初始化值为0.543
}KFP;							// Kalman Filter parameter

extern KFP KFP_accelX;
extern KFP KFP_accelY;
extern KFP KFP_accelZ;
extern KFP KFP_gyroX;
extern KFP KFP_gyroY;
extern KFP KFP_gyroZ;
extern float  Q_ANGLE_X, Q_ANGLE_Y, Q_ANGLE_Z; 

extern float pitch,roll,yaw;

float kalmanFilter(KFP *kfp,float input);
void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az);
void algorithm(float ax,float ay,float az,float gx,float gy,float gz);

#endif
