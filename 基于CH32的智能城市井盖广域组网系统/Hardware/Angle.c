#include "sys.h"
#include <math.h>

#define Kp 10.0f                  // 这里的KpKi是用于调整加速度计修正陀螺仪的速度
#define Ki 0.000f
#define dt   0.005
float pitch,roll,yaw;
 
// 算根号倒数-----快一些
static float invSqrt(float x)
{
	float halfx=0.5f*x;
	float y=x;
	long i=*(long*)&y;
	i=0x5f3759df-(i>>1);
	y=*(float*)&i;
	y=y*(1.5f-(halfx*y*y));
	return y;
}

KFP KFP_accelX={0.02,0,0,0,0.001,0.543};
KFP KFP_accelY={0.02,0,0,0,0.001,0.543};
KFP KFP_accelZ={0.02,0,0,0,0.001,0.543};
KFP KFP_gyroX ={0.02,0,0,0,0.001,0.543};
KFP KFP_gyroY ={0.02,0,0,0,0.001,0.543};
KFP KFP_gyroZ ={0.02,0,0,0,0.001,0.543};

/**
 *卡尔曼滤波器
 *@param KFP *kfp 卡尔曼结构体参数
 *   		 float input 需要滤波的参数的测量值（即传感器的采集值）
 *@return 滤波后的参数（最优值）
 */
float kalmanFilter(KFP *kfp,float input)
{
	//预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
	kfp->Now_P = kfp->LastP + kfp->Q;

	//卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
	kfp->Kg = kfp->Now_P / (kfp->Now_P + kfp->R);

	//更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
	kfp->out = kfp->out + kfp->Kg * (input -kfp->out);//因为这一次的预测值就是上一次的输出值

	//更新协方差方程: 本次的系统协方差付给 kfp->LastP 威下一次运算准备。
	kfp->LastP = (1-kfp->Kg) * kfp->Now_P;

	return kfp->out;
}
#define halfT 0.01f             					// 采样周期的一半，用于求解四元数微分方程时计算角增量
 
float q0 = 1, q1 = 0, q2 = 0, q3 = 0;     			// 初始位置姿态角为：0、0、0，对应四元数为：1、0、0、0
 
float exInt = 0, eyInt = 0, ezInt = 0;    
//重力加速度在三轴上的分量与用当前姿态计算得来的重力在三轴上的分量的误差的积分
 
float  Q_ANGLE_X= 0, Q_ANGLE_Y = 0, Q_ANGLE_Z = 0;   
 
// 互补滤波函数
// 输入参数：g表陀螺仪角速度(弧度/s)，a表加计（m/s2或g都可以，会归一化）
void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
{
 
	float q0temp,q1temp,q2temp,q3temp;							// 四元数暂存变量，求解微分方程时要用
	float norm; 												// 矢量的模或四元数的范数
	float vx, vy, vz;											// 当前姿态计算得来的重力在三轴上的分量
	float ex, ey, ez;											// 当前加计测得的重力加速度在三轴上的分量
	//与用当前姿态计算得来的重力在三轴上的分量的误差
	float q0q0 = q0*q0;
	float q0q1 = q0*q1;
	float q0q2 = q0*q2;
	float q1q1 = q1*q1;
	float q1q3 = q1*q3;
	float q2q2 = q2*q2;
	float q2q3 = q2*q3;
	float q3q3 = q3*q3;
	if(ax*ay*az==0)												// 加计处于自由落体状态时不进行姿态解算，因为会产生分母无穷大的情况
		return;
	norm = sqrt(ax*ax + ay*ay + az*az);							// 单位化加速度计，
	ax = ax / norm;												// 这样变更了量程也不需要修改KP参数，因为这里归一化了
	ay = ay / norm;
	az = az / norm;
	// 用当前姿态计算出重力在三个轴上的分量，重力在n系下是[0,0,g]，乘以转换矩阵就转到b系
	// 参考坐标n系转化到载体坐标b系，用四元数表示的方向余弦矩阵第三行即是
	vx = 2*(q1q3 - q0q2);
	vy = 2*(q0q1 + q2q3);
	vz = q0q0 - q1q1 - q2q2 + q3q3 ;
	// 计算测得的重力与计算得重力间的误差，这个误差是通过向量外积（也就是叉乘）求出来的
	ex = (ay*vz - az*vy) ;
	ey = (az*vx - ax*vz) ;
	ez = (ax*vy - ay*vx) ;
 
	exInt = exInt + ex * Ki;  // 对误差进行积分
	eyInt = eyInt + ey * Ki;
	ezInt = ezInt + ez * Ki;
	
	gx = gx + Kp*ex + exInt;  // 将误差PI（比例和积分项）补偿到陀螺仪角速度
	gy = gy + Kp*ey + eyInt;
	gz = gz + Kp*ez + ezInt;  // 没有磁力计的话就无法修正偏航角
	
	// 下面进行姿态的更新，也就是四元数微分方程的求解
	q0temp=q0;
	q1temp=q1;
	q2temp=q2;
	q3temp=q3;
	// 采用一阶毕卡解法
	q0 = q0temp + (-q1temp*gx - q2temp*gy -q3temp*gz)*halfT;
	q1 = q1temp + (q0temp*gx + q2temp*gz -q3temp*gy)*halfT;
	q2 = q2temp + (q0temp*gy - q1temp*gz +q3temp*gx)*halfT;
	q3 = q3temp + (q0temp*gz + q1temp*gy -q2temp*gx)*halfT;
	
	// 单位化四元数在空间旋转时不会拉伸，仅有旋转角度，这类似线性代数里的正交变换
	norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
	q0 = q0 / norm;
	q1 = q1 / norm;
	q2 = q2 / norm;
	q3 = q3 / norm;
	
	// 四元数到欧拉角的转换
	// 其中YAW航向角由于加速度计对其没有修正作用，因此此处直接用陀螺仪积分代替
	Q_ANGLE_Z = Q_ANGLE_Z + gz*halfT*2*57.3f; // yaw
	// Q_ANGLE_Z = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;
	Q_ANGLE_Y = asin(-2 * q1 * q3 + 2 * q0* q2)*57.3f; // pitch
	Q_ANGLE_X = atan2(2 * q2 * q3 + 2 * q0 * q1,-2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3f; // roll
}

// a加速度 g角速度  a——>g   g->dps
void algorithm(float ax,float ay,float az,float gx,float gy,float gz)
{
	
	float recip;						// 平方根倒数
	float Vx,Vy,Vz;
	float ex,ey,ez;
	float accex=0,accey=0,accez=0;
	float g1,g2,g3,g4,g5;
	gx=gx*0.0174f;						// 将dps转换为弧度
	gy=gy*0.0174f;
	gz=gz*0.0174f;
	
	recip=invSqrt(ax*ax+ay*ay+az*az);
	// 提取姿态矩阵的重力分量
	
	Vx=2*(q1*q3-q0*q2);
	Vy=2*(q0*q1+q2*q3);
	Vz=1-2*q1*q1-2*q2*q2;
	// 求姿态误差
	
	ex=ay*Vz-az*Vy;
	ey=az*Vx-ax*Vz;
	ez=ax*Vy-ay*Vx;
	// 积分误差
	
  accex=accex+ex*Ki*dt;
	accey=accey+ey*Ki*dt;
	accez=accez+ez*Ki*dt;
	// 互补滤波，使用pid进行误差修正
	gx=gx+Kp*ex+accex;
	gy=gy+Kp*ey+accey;
	gz=gz+Kp*ez+accez;
	// 解四元数微分方程如下
	gx=gx*(0.5f*dt);
	gy=gy*(0.5f*dt);
	gz=gz*(0.5f*dt);
	q0=q0+(-q1*gx-q2*gy-q3*gz);
	q1=q1+(q0*gx+q2*gz-q3*gy);
	q2=q2+(q0*gy-q1*gz+q3*gx);
	q3=q3+(q0*gz+q1*gy-q2*gx);
	// 归一化
	recip=invSqrt(q0*q0+q1*q1+q2*q2+q3*q3);
	q0=q0*recip;
	q1=q1*recip;
	q2=q2*recip;
	q3=q3*recip;
	// 计算姿态角
  
	g1=2.0f*(q1*q3-q0*q2);
	g2=2.0f*(q0*q1+q2*q3);
	g3=q0*q0-q1*q1-q2*q2+q3*q3;
	g4=2.0f*(q1*q2+q0*q3);
	g5=q0*q0+q1*q1-q2*q2-q3*q3;
	
  	pitch=-asinf(g1)*57.3f;
	roll=atanf(g2/g3)*57.3f;
	yaw=atanf(g4/g5)*57.3f;
}
