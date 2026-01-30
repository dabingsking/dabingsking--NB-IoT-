


#include <stdbool.h> 
#include "hal_lis3dh.h"
#include "hal_iic_emu.h"
#include "lis3dh_driver.h" 
#include "stm32f10x.h" 
#include "stdint.h"


bool  USE_IIC_LIS3DH			= false;

void spi_cs_low()
{ 
	GPIO_ResetBits( GPIOA,   GPIO_Pin_4);
}

void spi_cs_high()
{ 
	GPIO_SetBits( GPIOA,   GPIO_Pin_4);
}

uint8_t spi_access(uint8_t data)
{ 
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	SPI_I2S_SendData(SPI1, data);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	return SPI_I2S_ReceiveData(SPI1);
}


int hal_lis3dh_interface_init()
{ 
	if( USE_IIC_LIS3DH)
	{
		HAL_IIC_EMU_Init();
		HAL_IIC_EMU_SetSlaveAddr(0x32);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO    , ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure;

		/*  LIS3DH CS 鎺ラ珮鐢靛钩---------------------------------*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4    ;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_SetBits( GPIOA,   GPIO_Pin_4);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6    ;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_SetBits( GPIOA,   GPIO_Pin_6);


		/* Configure GPIO For LIS3DH Int input --------------------------------*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6  | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOB, &GPIO_InitStructure);





		return 0;
	}
	else
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO    , ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

		GPIO_InitTypeDef GPIO_InitStructure;

		/* Configure SPIy pins: SCK, MISO and MOSI ---------------------------------*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5  | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* Configure SPIy pins: CS ---------------------------------*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4    ;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_SetBits( GPIOA,   GPIO_Pin_4);



		/* Configure GPIO For LIS3DH Int input --------------------------------*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6  | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOB, &GPIO_InitStructure);


		SPI_InitTypeDef   SPI_InitStructure;

		/* SPIy Config -------------------------------------------------------------*/
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
		SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
		SPI_InitStructure.SPI_CRCPolynomial = 7;
		SPI_Init(SPI1, &SPI_InitStructure);

		SPI_Cmd(SPI1, ENABLE);

		return 0;
	}
}





int hal_lis3dh_init(bool iic_mode_sel)
{  
	uint8_t value;

	USE_IIC_LIS3DH = iic_mode_sel;

	hal_lis3dh_interface_init();

	//读LIS3DH寄存器，确认LIS3DH通信成功
	LIS3DH_ReadReg(LIS3DH_WHO_AM_I, &value) ;
	if(value != 0x33)
	{
		return -1;
	}

	//复位LIS3DH内部寄存器
	LIS3DH_RebootMemory();
	for(volatile uint32_t i=0;i<1000000;i++);

	//设置LIS3DH采样率
	LIS3DH_SetODR(LIS3DH_ODR_400Hz) ;
	//LIS3DH工作模式
	LIS3DH_SetMode(LIS3DH_NORMAL);
	LIS3DH_SetFullScale(LIS3DH_FULLSCALE_8 );
	LIS3DH_SetHPFMode(LIS3DH_HPM_NORMAL_MODE_RES);
	LIS3DH_SetBDU(MEMS_ENABLE);
	LIS3DH_SetAxis(LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE);


	//    LIS3DH_FIFOModeEnable(LIS3DH_FIFO_STREAM_MODE);
	//////    LIS3DH_SetTriggerInt(LIS3DH_TRIG_INT1);
	//    LIS3DH_SetWaterMark(16);



	/**
	 * enable BDU function
	 */
	LIS3DH_SetBDU(MEMS_ENABLE);

	/**
	 * click function configuration
	 */
	LIS3DH_SetClickCFG( LIS3DH_XS_ENABLE );//| LIS3DH_XS_ENABLE);
	LIS3DH_SetClickLIMIT(0x33) ;//127ms
	LIS3DH_SetClickTHS(20);
	LIS3DH_SetClickLATENCY(0xff);   //637ms
	LIS3DH_SetClickWINDOW(0xff);    //637ms


	/**
	 * high pass filter configuration
	 */
	//    LIS3DH_SetHPFMode(LIS3DH_HPM_NORMAL_MODE  ) ;
	//    LIS3DH_SetHPFCutOFF(LIS3DH_HPFCF_3  ) ;
	//    LIS3DH_SetFilterDataSel(MEMS_ENABLE  ) ;


	LIS3DH_SetInt1Pin(
			LIS3DH_CLICK_ON_PIN_INT1_ENABLE |
			LIS3DH_I1_INT1_ON_PIN_INT1_DISABLE |
			LIS3DH_I1_INT2_ON_PIN_INT1_DISABLE |
			LIS3DH_I1_DRDY1_ON_INT1_DISABLE |
			LIS3DH_I1_DRDY2_ON_INT1_DISABLE |
			LIS3DH_WTM_ON_INT1_DISABLE |
			LIS3DH_INT1_OVERRUN_DISABLE
	);

	LIS3DH_SetInt2Pin(
			LIS3DH_CLICK_ON_PIN_INT2_ENABLE |
			LIS3DH_I2_INT1_ON_PIN_INT2_DISABLE |
			LIS3DH_I2_INT2_ON_PIN_INT2_DISABLE |
			LIS3DH_I2_BOOT_ON_INT2_DISABLE |
			LIS3DH_INT_ACTIVE_HIGH );


	//    LIS3DH_Int1LatchEnable(MEMS_ENABLE);


	return 0;
}

bool hal_lis3dh_get_int1_status(void)
{


	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6);


}

bool hal_lis3dh_get_int2_status(void)
{
	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7);
}






/**
 *
 * @param x
 * @param y
 * @param z
 * @return
 */
int hal_lis3dh_get_xyz(short *x,short *y,short *z)
{
	AxesRaw_t aux_raw;

	LIS3DH_GetAccAxesRaw( &aux_raw );

	*x = aux_raw.AXIS_X;
	*y = aux_raw.AXIS_Y;
	*z = aux_raw.AXIS_Z;

	return 0;
}



int hal_lis3dh_spi_write
(
		uint8_t reg_addr,
		uint8_t const *data,
		uint8_t length
)
{
	spi_cs_low();

	spi_access( reg_addr & (~(0x01<<7)) );

	for(uint16_t i=0;i<length;i++)
	{
		spi_access( *data++ );
	}

	spi_cs_high();


	return 0;
}


int hal_lis3dh_spi_read
(
		uint8_t reg_addr,
		uint8_t *data,
		uint8_t length
)
{
	spi_cs_low();

	spi_access( reg_addr | (0x01<<7));
	for(uint16_t i=0;i<length;i++)
	{
		*data = spi_access(0x00);
	}

	spi_cs_high();

	return 0;
}



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/*******************************************************************************
 * Function Name		: LIS3DH_ReadReg
 * Description		: Generic Reading function. It must be fullfilled with either
 *			: I2C or SPI reading functions
 * Input			: Register Address
 * Output		: Data REad
 * Return		: None
 *******************************************************************************/
u8_t LIS3DH_ReadReg(u8_t Reg, u8_t* Data) {
	if (USE_IIC_LIS3DH)
	{
		HAL_IIC_EMU_Read  (  Reg,  Data,  1);
		return 1;
	}
	else
	{
		hal_lis3dh_spi_read
		(
				Reg,
				Data,
				1
		);

		//To be completed with either I2c or SPI reading function
		//i.e. *Data = SPI_Mems_Read_Reg( Reg );
		return 1;
	}
}


/*******************************************************************************
 * Function Name		: LIS3DH_WriteReg
 * Description		: Generic Writing function. It must be fullfilled with either
 *			: I2C or SPI writing function
 * Input			: Register Address, Data to be written
 * Output		: None
 * Return		: None
 *******************************************************************************/
u8_t LIS3DH_WriteReg(u8_t WriteAddr, u8_t Data) {
	if (USE_IIC_LIS3DH)
	{
		HAL_IIC_EMU_Write(  WriteAddr,   &Data,  1);
		return 1;
	}
	else
	{
		hal_lis3dh_spi_write
		(
				WriteAddr,
				&Data,
				1
		);

		//To be completed with either I2c or SPI writing function
		//i.e. SPI_Mems_Write_Reg(WriteAddr, Data);
		return 1;
	}

}

