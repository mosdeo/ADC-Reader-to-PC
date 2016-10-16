#include <REG_MG82FL524-564.H>
#include <stdio.h>
#include "ADC.h"
#include "UART.h"
#define clrwdt() WDTCR|=0x10
/*
三通道Analog讀取程式
使用MG82FE564
ADC 為 Port 1
Programmer: 林高遠@2014.12.27
*/
unsigned char code HEX[16]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39
													 ,0x41,0x42,0x43,0x44,0x45,0x46};

struct AnalogValue{
	unsigned char ucBYTE[3];	 //以3個Byte存放000~3FF
	unsigned char HEX[3]; 		 //以3個Byte存放"000"~"3FF",供UART用
	int DecVal; //以整數格式存放Dec,供MCU內部運算用
};

unsigned char i,j; 						  //for loop
unsigned char ucQUERY;				 //Rx SBUF
unsigned char ucUsedChannel=3; //由此設定依序要使用多少通道

//Timer Flag
volatile bit Timer0_Flag_20ms=0;
	
xdata struct AnalogValue ADCV_Ch[8]; //存放8通道ADC轉換結果的陣列
void TxAllAnalogByASCII(void);

int main()
{
	IE=0x82; /* (中斷Enable)EA=1,(Timer0中斷)ET0=1*/
	//===EA========ET2==ES0=ET1==EX1=ET0==EX0==
	//====1====0====0====0====0====0===1===0===
	
	TMOD|=0x02;	//T0=mode1 for Timer0_INT_Counter
	TH0=TL0=(65535-225); //225/1843200 = 1/8192
	TR0=1; /*Timer0 啟動!*/
	
	ADC_Initial();
	UART_Initial();
	//Initial string(防呆字串), 解決了第一個字錯誤的問題
	putchar(0);
	putchar(10); // '\n'
	
	//Dealy, 讓CPU 穩定的時間
	for(i=0;i<50;i++)
		for(j=1;j!=0;j++);
	
	while(1)
	{
		if(Timer0_Flag_20ms)
		{
			Timer0_Flag_20ms=0;
			
			//切換頻道 & AD轉換 & 讀取 & 轉換資料格式
			for(i=0;i<ucUsedChannel;++i)
			{
				ADCV_Ch[i].DecVal = ADC_10BitGet(i);
				
				//將高中低3個Byte轉換為ASCII字元,可直接發送給SBUF
				ADCV_Ch[i].HEX[2]=HEX[ADCV_Ch[i].DecVal/256];
				ADCV_Ch[i].HEX[1]=HEX[(ADCV_Ch[i].DecVal%256)/16];
				ADCV_Ch[i].HEX[0]=HEX[ADCV_Ch[i].DecVal%16];
			}
			
			TxAllAnalogByASCII();
		}
	}
}

void Timer0_INT(void) interrupt 1
{
	static unsigned int Timer0_INT_Counter=0;	
	
	Timer0_INT_Counter++;
	if(0==Timer0_INT_Counter%163)Timer0_Flag_20ms=1;
}

void TxAllAnalogByASCII(void)
{/*==範圍:0~1023(03FF)*/
	
	printf("$ADC,%c%c%c,%c%c%c,%c%c%c\n\0",
				ADCV_Ch[0].HEX[2],
				ADCV_Ch[0].HEX[1],
				ADCV_Ch[0].HEX[0],
				
				ADCV_Ch[1].HEX[2],
				ADCV_Ch[1].HEX[1],
				ADCV_Ch[1].HEX[0],
				
				ADCV_Ch[2].HEX[2],
				ADCV_Ch[2].HEX[1],
				ADCV_Ch[2].HEX[0]
				);
}