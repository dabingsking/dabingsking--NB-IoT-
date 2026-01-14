#ifndef __ADC_H
#define __ADC_H

#define Window_Size     10
#define Sensor_EN_GPIO_Port GPIOB
#define IMU_EN GPIO_Pin_13
#define MQ_4_EN GPIO_Pin_11
#define WATER_EN GPIO_Pin_10

extern uint16_t AD_Value[3];
extern uint16_t Bat_Value[Window_Size];
extern uint16_t MQ4_Value;
extern uint16_t Water_Value;

// extern char *Yes;
// extern char *NO;
extern char *Warn;
extern char *Safety;

extern uint8_t Count;

extern uint8_t Battery_level;

void Sensor_GPIO_Init(void);
void ALLADC_Init(void);
void AD_GetValue(void);
void Get_ADC_value(void);

#endif
