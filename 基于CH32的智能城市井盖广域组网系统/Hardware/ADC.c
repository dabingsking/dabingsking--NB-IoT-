#include "sys.h"

//定义分压电路参数
#define ADC_Ref 3.3f          // ADC的参考电压
#define ADC_Fenbianlv 4095    // ADC的分辨率，12位ADC

//电池特性参数
#define Bat_Min_V 3.0f        // 电池最小电压
#define Bat_Max_V 4.2f        // 电池最大电压

#define Window_Size     10    // 滑动均值滤波的窗口大小，可根据纹波调整

#define ALPHA_FAST 0.995f     // 动态加权一阶低通滤波的快速变化时的滤波系数
#define ALPHA_SLOW 0.005f     // 动态加权一阶低通滤波的缓慢变化时的滤波系数
#define THRESHOLD 5           // 动态加权一阶低通滤波的变化速率阈值

uint16_t AD_Value[3]={0,0,0};               // ADC1三通道采集的值
uint16_t Bat_Value[Window_Size]={0};        // 电量采集值
uint16_t MQ4_Value=0;                       // MQ-4的采集值
uint16_t Water_Value=0;                     // 水位采集值
// char *Yes="Yes";
// char *NO="NO";
char *Warn="Alert!";
char *Safety="Safety";
uint8_t Index=0;                            // 滑动均值滤波的索引值
uint32_t Sum_Bat_Value=0;                   // 电量总值

uint8_t Battery_level=0;                    // 电池电量百分比

uint8_t Once=1;                             // 只执行一次

uint16_t Last_value = 0;                    // 旧值
float Alpha = ALPHA_SLOW;                   // 动态变化的变化速率

float V_Bat=0;


void Sensor_GPIO_Init(void)
{
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB,ENABLE);
    GPIO_InitTypeDef GPIO_InitStruture = {0};
    GPIO_InitStruture.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_InitStruture.GPIO_Pin=IMU_EN | MQ_4_EN | WATER_EN;
    GPIO_InitStruture.GPIO_Speed=GPIO_Speed_2MHz;
    GPIO_Init(Sensor_EN_GPIO_Port,&GPIO_InitStruture);
    GPIO_ResetBits(Sensor_EN_GPIO_Port, IMU_EN | MQ_4_EN | WATER_EN);
}

/*
参数  : 无
函数体: 所有ADC的初始化函数
返回值: 无
*/
void ALLADC_Init(void)
{
    //1.开启时钟
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA,ENABLE);          // MQ4在PA1-ADC1通道1   BAT在PA0-ADC1通道0  电池电压ADC采集口
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB,ENABLE);          // WATER在PB1-ADC1通道9
    //2.配置GPIO
    GPIO_InitTypeDef GPIO_InitStruture = {0};
    GPIO_InitStruture.GPIO_Mode=GPIO_Mode_AIN;
    GPIO_InitStruture.GPIO_Pin=GPIO_Pin_0;
    GPIO_Init(GPIOA,&GPIO_InitStruture);                        // BAT在PA0-ADC0

    GPIO_InitStruture.GPIO_Mode=GPIO_Mode_AIN;
    GPIO_InitStruture.GPIO_Pin=GPIO_Pin_1;
    GPIO_Init(GPIOA,&GPIO_InitStruture);                        // MQ4在PA1-ADC1

    GPIO_InitStruture.GPIO_Mode=GPIO_Mode_AIN;
    GPIO_InitStruture.GPIO_Pin=GPIO_Pin_1;
    GPIO_Init(GPIOB,&GPIO_InitStruture);                        // WATER在PB1-ADC9

    GPIO_InitStruture.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_InitStruture.GPIO_Pin=GPIO_Pin_3;
    GPIO_InitStruture.GPIO_Speed=GPIO_Speed_2MHz;
    GPIO_Init(GPIOB,&GPIO_InitStruture);                        // 控制采集电量的端口PB3

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_ADC1,ENABLE);           // ADC1时钟
    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);            // DMA时钟

    RCC_ADCCLKConfig(RCC_PCLK2_Div8);                           // ADC1的时钟8分频

    //3.配置ADC通道和采样时间
    ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_CyclesMode7);
    ADC_RegularChannelConfig(ADC1,ADC_Channel_1,2,ADC_SampleTime_CyclesMode7);
    ADC_RegularChannelConfig(ADC1,ADC_Channel_9,3,ADC_SampleTime_CyclesMode7);

    //4.配置ADC
    ADC_InitTypeDef ADC_InitStruture;
    ADC_InitStruture.ADC_ContinuousConvMode=DISABLE;                    // 连续转换模式 DISABLE表示单次转换，ENABLE表示连续转换
    ADC_InitStruture.ADC_ScanConvMode=ENABLE;                           // 扫描转换模式 DISABLE表示非扫描模式(只采集单个ADC))，ENABLE表示扫描模式(采集多个ADC)
    ADC_InitStruture.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;    // 定义用于启动常规通道模数转换的外部触发器（此处无外部触发）
    ADC_InitStruture.ADC_Mode=ADC_Mode_Independent;                     // ADC模式，（将ADC配置为独立或双模式）（此处为独立模式）
    ADC_InitStruture.ADC_DataAlign=ADC_DataAlign_Right;                 // 指定ADC数据对齐是左对齐还是右对齐（此处为右对齐）
    ADC_InitStruture.ADC_NbrOfChannel=3;                                // 指定要转换的ADC通道数，使用常规通道组的序列器（可选范围1-16）
    //配置 ADC 输出缓冲器 输出缓冲器是 ADC 模块中的一个组件，用于存储转换后的数据并将其提供给微控制器的其他部分使用。
    ADC_InitStruture.ADC_OutputBuffer=ADC_OutputBuffer_Disable;         // 在 ADC 转换完成后，保持转换结果不变，直到下一次转换完成。
    //可编程增益放大器 允许在 ADC 转换前对输入信号进行放大，从而提高小信号的测量精度。
    ADC_InitStruture.ADC_Pga=1;                                         // 增益为1（不放大，直接输入）
    ADC_Init(ADC1,&ADC_InitStruture);
    //5.DMA配置
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)&ADC1->RDATAR;             // 外设站点的起始地址
    DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;     // 外设站点的数据宽度,DR寄存器是16位，所以用半字
    DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;                // 外设站点是否自增,不自增，始终转运同一位置的数据到对应位置
    DMA_InitStructure.DMA_MemoryBaseAddr=(uint32_t)AD_Value;                      // 存储器站点的起始地址
    DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;             // 内存数据大小也为半字，跟外设数据大小相同
    DMA_InitStructure.DMA_MemoryInc=DMA_MemoryInc_Enable;                         // 存储器站点是否自增,选择自增
    DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralSRC;                              // 传输方向.此处把外设站点设为SRC，源点，即由外设站点到存储器
    DMA_InitStructure.DMA_BufferSize=3;                                           // 缓存区大小，即传输计数器
    DMA_InitStructure.DMA_Mode=DMA_Mode_Normal;                                   // Normal;传输模式，即是否使用自动重装,转运数组为存储器到存储器，不采用自动重装 
    DMA_InitStructure.DMA_M2M=DMA_M2M_Disable;                                    // 选择是否是存储器到存储器，即选择硬件还是软件触发
    DMA_InitStructure.DMA_Priority=DMA_Priority_High;                             // 优先级，按参数要求给个优先级即可
    DMA_Init(DMA1_Channel1,&DMA_InitStructure);                                   // 把结构体的指定的参数配置到DMA1的通道1里面去
    //6.启动DMA
    DMA_Cmd(DMA1_Channel1,ENABLE);                                                // 使能DMA
    //7.启动ADC   
    ADC_DMACmd(ADC1,ENABLE);                                                      // 使能ADC的DMA请求
    ADC_Cmd(ADC1,ENABLE);                                                         // 使能ADC
    ADC_FIFO_Cmd(ADC1, ENABLE);    
    ADC_BufferCmd(ADC1, DISABLE); //disable buffer
    //8.校准
    ADC_ResetCalibration(ADC1);                                                   // 重置所选ADC校准寄存器
    while(ADC_GetResetCalibrationStatus(ADC1)==SET);                              // 获取所选ADC重置校准寄存器状态
    ADC_StartCalibration(ADC1);                                                   // 启动所选ADC校准过程
    while(ADC_GetCalibrationStatus(ADC1)==SET);                                   // 是否校准完毕    
}

/*
参数  : 无
函数体: 获取所有传感器的值，并通过DMA传输
返回值: 无
*/
void AD_GetValue(void)
{
    DMA_Cmd(DMA1_Channel1, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel1, 3);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while(DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET);
    DMA_ClearFlag(DMA1_FLAG_TC1);
}

/*
参数  : 新获取的传感器的值
函数体: 动态加权一阶低通滤波（自适应滤波）
返回值: 无
*/
uint16_t Adaptive_filter(uint16_t new_value) 
{
    // 计算变化速率
    int16_t delta = abs(new_value - Last_value);
    
    // 根据变化速率动态调整滤波系数
    Alpha = (delta > THRESHOLD) ? ALPHA_FAST : ALPHA_SLOW;
    
    // 一阶低通滤波
    Last_value = (uint16_t)(Alpha * new_value + (1.0f - Alpha) * Last_value);
    return Last_value;
}

/*
参数  : 无
函数体: 获取所有ADC的值，并进行滤波，处理数据
返回值: 无
*/
void Get_ADC_value(void)
{
    // 采样样本,初始化滤波缓冲区,所以只执行一次
    if(Once==1)
    {
        for(int i=0;i<Window_Size;i++)
        {
            AD_GetValue();
            Bat_Value[i]=AD_Value[0];
            Sum_Bat_Value+=Bat_Value[i];
        }
        Once=0;   
    }

    // 获取新ADC值
    AD_GetValue();

    // 通过动态加权一阶低通滤波算法获取水位的ADC值
    Water_Value=Adaptive_filter(AD_Value[2]);
    MQ4_Value=Adaptive_filter(AD_Value[1]);

    // 更新总和：减去最早值，加上新值
    Sum_Bat_Value=Sum_Bat_Value-Bat_Value[Index]+AD_Value[0];
     // 存储新值到缓冲区
    Bat_Value[Index]=AD_Value[0];
    // 更新索引（循环缓冲区）
    Index=(Index+1)%Window_Size;
     // 得到平均值
    Arverge_Bat=Sum_Bat_Value/Window_Size;

    // P沟道MOS管，低电平导通，决定是否获取电池余量
    if(V_Bat_Sw==1)  GPIO_ResetBits(GPIOB,GPIO_Pin_3);
    else GPIO_SetBits(GPIOB,GPIO_Pin_3);

    // 算取MQ-4浓度
    MQ4=(MQ4_Value/4095.0f)*100;

    // 算取电池电量
    V_Bat=(float)Arverge_Bat*ADC_Ref/ADC_Fenbianlv;
    V_Bat=V_Bat*2;

    if(V_Bat>4.2f) Battery_level=100;       // 限制电压范围
    else if(V_Bat>=4.0f)                    // 根据电压区间使用不同的计算方法
    {
        // 高电压区域，电压变化快，灵敏度高
        Battery_level = 100 - 120 * pow(4.2f - V_Bat, 0.5f);
    }
    else if(V_Bat>=3.6f)
    {
        // 中间区域，电压变化平缓
        Battery_level = 40 + 50 * (V_Bat - 3.6f) / 0.4f;
    }
    else if(V_Bat>=3.0f)
    {
        // 低电压区域，电压快速下降
        Battery_level = 40 * pow((V_Bat - 3.0f) / 0.6f, 2);
    }
    else Battery_level=0;
}
