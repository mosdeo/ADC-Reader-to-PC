#include "Bit.h"

void ADC_Initial(void)
{
	/*
	1.開啟ADC硬體
	2.設定轉速
	3.選頻道
	4.被選中的頻道要設定成Input-Only Mode
	*/
	
	//***Step1~3***//
	ADCON=0xE0;
	//===ADCEN==SPEED1==SPEED2==ADCI==ADCS==CH2==CH1=CH0=
	//=====1======1=======1======0======0====0====0===0==
	
	//***Step4:有使用的通道Input-Only Mode, 不用的comment掉***//
	P1M0|=BIT[0];P1M1&=~BIT[0];
	P1M0|=BIT[1];P1M1&=~BIT[1];
	P1M0|=BIT[2];P1M1&=~BIT[2];
// 	P1M0|=BIT[3];P1M1&=~BIT[3];
// 	P1M0|=BIT[4];P1M1&=~BIT[4];
// 	P1M0|=BIT[5];P1M1&=~BIT[5];
// 	P1M0|=BIT[6];P1M1&=~BIT[6];
// 	P1M0|=BIT[7];P1M1&=~BIT[7];
}

unsigned int ADC_10BitGet(unsigned char Channel)
{
	ADCON&=0xF8; //清除通道選擇
	ADCON|=0x07&Channel; //設定這次取樣的頻道
	ADCON|=BIT3;	//ADCS=1,開始轉換
	while(!(ADCON&BIT4));	//等待ADCI=1(轉換完畢),這行會讓UART失效
	ADCON&=(~BIT4);	//然後清除ADCI
	
	return (ADCH<<2)&0x3FC + ADCL;
}