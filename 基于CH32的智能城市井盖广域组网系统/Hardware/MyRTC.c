#include "sys.h"

/*
参数  : 无
函数体: RTC初始化
返回值: 无
*/
void MyRTC_Init(void)
{
    //1.开启PWR和BKP的时钟，使能BKP和RTC的访问
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR,ENABLE);
	RCC_PB1PeriphClockCmd(RCC_PB1Periph_BKP,ENABLE);

    PWR_BackupAccessCmd(ENABLE);                            // 备份寄存器使能访问，使能BKP和RTC的访问
    //借助BKP的特性来防止重复初始化，启动一次后，BKP_DR1的值就为0xA5A5，只要有备用电源，则BKP_DR1就一直为0xA5A5，在复位就不会再初始化，只会调用等待函数
    //读备份寄存器
    //2.开启LSI的时钟，并等待LSI时钟初始化完成
    RCC_LSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)!=SET);         // SET是真的意思，所以标志位等于真的时候就退出循环
    //3.选择RTCCLK的时钟源
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);                 // 时钟源选择了LSI
    RCC_RTCCLKCmd(ENABLE);                                  // 使能时钟
    //4.调用两个等待函数
    RTC_WaitForSynchro();                                   // 等待同步函数
    RTC_WaitForLastTask();                                  // 等待上一次操作完成
    //5.配置预分频器
    RTC_SetPrescaler(38000-1);                              // 分频后的频率要为1Hz，所以LSI要分40000，0也是一个值，所以要减1
    RTC_WaitForLastTask();                                  // 等待函数，因为每次写寄存器都要在前一次写操作结束后进行，所以每写一次就等待，保障下一次写操作正常
    //6.设置初始时间
    RTC_SetCounter(0);                                      // 设置初始时间位00：00：00
    RTC_WaitForLastTask();                                  // 等待上一次操作完成
    //7.写入备份寄存器
    BKP_WriteBackupRegister(BKP_DR1,0xA5A5);                // 代表初始化完成，下次就不用初始化了
}

/*
参数  : 无
函数体: 设置RTC的闹钟值
返回值: 无
*/
void RTC_SetClock(void)
{
    // 等待上一次的RTC操作完成
    RTC_WaitForLastTask();

    // 获取当前RTC的计数值
    Rtc_Clock=RTC_GetCounter();
    RTC_WaitForLastTask();                                      // 等待上一次操作完成

    // 设置闹钟在10s后触发
    Rtc_Clock+=20;

    // 设置RTC的闹钟
    RTC_SetAlarm(Rtc_Clock);
    RTC_WaitForLastTask();                                      // 等待上一次操作完成

    // 使能RTC闹钟
    RTC_ITConfig(RTC_IT_ALR,ENABLE);
    RTC_WaitForLastTask();                                      // 等待上一次操作完成

    // 中断线映射到EXTI_Line17
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line17; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;      // 上升沿触发（根据需求调整）
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);

    // 配置NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;         // 闹钟中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   // 比RTC全局中断的优先级高
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
}

void Enter_StopMode(void) 
{
    // 清除RTC闹钟标志（若已设置）
    RTC_ClearFlag(RTC_FLAG_ALR);
    
    // 选择电压调节器模式（低功耗或正常）
    PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
}
