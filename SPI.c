#include "SPI.h"

//LSM9DS0 register definition
#define OUT_X_L_G 0x28
#define OUT_X_H_G 0x29
#define OUT_Y_L_G 0x2A
#define OUT_Y_H_M 0x2B
#define OUT_Z_L_G 0x2C
#define OUT_Z_H_M 0x2D


void init_SPI1(void)
{
	SPI_InitTypeDef SPI_InitStruct;
  GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	// set to full duplex mode, seperate MOSI and MISO lines
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 
	// transmit in master mode, NSS pin has to be always high
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master; 
  // one packet of data is 8 bits wide	
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; 
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_High; 
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;      
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	// SPI frequency is APB2 frequency / 4
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; 
	// data is transmitted MSB first
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI1, &SPI_InitStruct); 
	
  /* Connect pins to SPI SCK, MISO, MOSI to alternate function peripheral */
  GPIO_PinAFConfig(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_AF_SPI1);

  /* setup SCK, MISO, MOSI pins */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;     // set pins to alternate function
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;    // set GPIO speed
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;     // SPI requires push pull
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;     // enable pull up resistors
  GPIO_Init(GPIOB, &GPIO_InitStruct);         // init GPIOB
  
	select_SPI_Channel(NONE);

	// enable SPI1
	SPI_Cmd(SPI1, ENABLE); 
}

void select_SPI_Channel(uint32_t channel)
{
	switch(channel)
	{
		case NONE:
		{
			GPIO_SetBits(GPIOE, GPIO_Pin_6);
			GPIO_SetBits(GPIOE, GPIO_Pin_7);
			GPIO_SetBits(GPIOE, GPIO_Pin_8);
		}
			break;
		case DISP:
		{
			GPIO_SetBits(GPIOE, GPIO_Pin_6);
		}
			break;
		case XM:
		{
			GPIO_SetBits(GPIOE, GPIO_Pin_7);
		}
			break;
		case G:
		{
			GPIO_SetBits(GPIOE, GPIO_Pin_8);
		}
			break;
	}
}

// read register from SPI channel
uint8_t get_SPI_Data(uint8_t adress, uint8_t channel)
{
	//select channel
	select_SPI_Channel(channel); 
	//set adress to "write" mode by adding 1000 0000 (0x80 or 128)
  adress = 0x80 | adress; 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	//send adress
	SPI_I2S_SendData(SPI1, adress); 

	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	//Clear RXNE bit
	SPI_I2S_ReceiveData(SPI1); 

	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	//Dummy byte to generate clock
	SPI_I2S_SendData(SPI1, 0x00); 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));

	//deselect channel
	select_SPI_Channel(NONE); 
	//read data
	return  SPI_I2S_ReceiveData(SPI1); 
}

//write register to SPI channel
void set_SPI_Data(uint8_t adress, uint8_t channel, uint8_t data)
{
	//select chanel
	select_SPI_Channel(channel); 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	//send adress
	SPI_I2S_SendData(SPI1, adress); 

	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	//Clear RXNE bit
	SPI_I2S_ReceiveData(SPI1); 

	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
	//send data
	SPI_I2S_SendData(SPI1, data); 
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	//Clear RXNE bit
	SPI_I2S_ReceiveData(SPI1); 
	//deselect all
	select_SPI_Channel(NONE); 
}
