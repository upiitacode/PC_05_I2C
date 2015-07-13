#include "stm32f30x.h"                  // Device header
#include "serial_stdio.h"
#include "retarget_stm32f3.h"
#include <string.h>
/*Led PB13, Button PC13*/

void delay_ms(int delay_time);

void i2c_init_slave(void);

Serial_t USART1_Serial={USART1_getChar,USART1_sendChar};
Serial_t USART2_Serial={USART2_getChar,USART2_sendChar};

char mybf[80];/*Input buffer*/
char wordBuffer[80];

int rDataCounter=0;
int rxi2cData[3];
int rx_index=0;
int evnt_counter=0;

int main(){
	int lastCounter=0;
	USART2_init(9600);
	serial_puts(USART2_Serial,"\nI2C Slave ready\n");
	i2c_init_slave();
	while(1){
		if(rDataCounter!=lastCounter){
			serial_printf(USART2_Serial,"data= %d\n",rDataCounter);
			serial_printf(USART2_Serial,"d[0]=%d\n",rxi2cData[0]);
			serial_printf(USART2_Serial,"d[1]=%d\n",rxi2cData[1]);
			serial_printf(USART2_Serial,"d[2]=%d\n",rxi2cData[2]);
			lastCounter=rDataCounter;
		}
	}
}


void delay_ms(int delay_time){
	for(int i=0; i<delay_time; i++);
}

void i2c_init_slave(void){
	
	/* RCC Configuration */
	/*I2C1 Peripheral clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_I2C1EN, ENABLE);
	/*GPIOB clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOBEN, ENABLE);
	/* GPIO Configuration */
	
	/*Configure I2C1 SCL(PB8) and SDA(PB9) */
	GPIO_InitTypeDef  myGPIO;
	GPIO_StructInit(&myGPIO);
	myGPIO.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
	myGPIO.GPIO_Mode = GPIO_Mode_AF;
	myGPIO.GPIO_Speed = GPIO_Speed_50MHz;
	myGPIO.GPIO_OType = GPIO_OType_OD;
	myGPIO.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &myGPIO);
	/* Connect PB8 to I2C1_SCL */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_4);
	/* Connect PB9 to I2C1_SDA */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_4);
	
	#define PRESC 3
	#define SCLL 0x13
	#define SCLH 0xF
	#define SDADEL 0x2
	#define	SCLDEL 0x04 
	
	I2C_Cmd(I2C1,DISABLE);
	I2C_InitTypeDef  I2C_InitStructure;
	I2C_StructInit(&I2C_InitStructure);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_AnalogFilter=I2C_AnalogFilter_Disable;
	I2C_InitStructure.I2C_DigitalFilter=0x04;
	I2C_InitStructure.I2C_Timing= (PRESC<<28)|(SCLDEL<<20)|(SDADEL<<16)|(SCLH<<8)|(SCLL<<0);
	I2C_InitStructure.I2C_OwnAddress1 = 0x62<<1;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_StretchClockCmd(I2C1,DISABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
	I2C_ITConfig(I2C1,I2C_IT_RXNE,ENABLE);
	I2C_ITConfig(I2C1,I2C_IT_ADDR,ENABLE);
	NVIC_EnableIRQ(I2C1_EV_IRQn);
	I2C_Cmd(I2C1,ENABLE);
}


void I2C1_EV_IRQHandler(void){
	if(I2C_GetITStatus(I2C1,I2C_IT_ADDR)){
		I2C_ClearITPendingBit(I2C1,I2C_IT_ADDR);
		rx_index=0;
	}
	if(I2C_GetITStatus(I2C1,I2C_IT_RXNE)){
		rxi2cData[rx_index%3]=I2C_ReceiveData(I2C1);
		rDataCounter++;
		rx_index++;
	}
	evnt_counter++;
}

