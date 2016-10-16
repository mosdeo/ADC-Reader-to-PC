#include <REG_MG82F564.h>
#include <BIT.h>
#include <stdio.h>
#define clrwdt() WDTCR|=0x10
/*
MG82FE564
之
"內建ADC讀取"教學範例程式，以UART@19200bps作為輸出顯示
ADC 為 Port 1
Programmer: 林高遠@2013.8.9
*/

//此陣列可將 0~15 的 int 或 char 轉為 ASCII
unsigned char code HEX[16]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39
													 ,0x41,0x42,0x43,0x44,0x45,0x46};

//用來存放 ADC 結果的資料結構
struct AnalogValue{
	unsigned char ucBYTE[3];	 //以3個Byte存放000~3FF
	unsigned char HEX[3]; 		 //以3個Byte存放"000"~"3FF",供UART用
	unsigned short int DecVal; //以整數格式存放Dec,供MCU內部運算用
};

unsigned char i,j; 						  //for loop
unsigned char ucQUERY;				 //Rx SBUF
unsigned char ucUsedChannel=3; //由此設定依序要使用多少通道

//Timer Flag
unsigned char Counter5ms=0;
bit Flag_20ms=0;
							
struct AnalogValue ADCV_Ch[8]; //宣告存放8通道ADC轉換結果的陣列結構

void TxAllAnalogByASCII(void);

main()
{
	
	IE=0x82; /* (中斷Enable)EA=1,(Timer0中斷)ET0=1*/
	//===EA========ET2==ES0=ET1==EX1=ET0==EX0==
	//====1====0====0====0====0====0===1===0===
	
	SCON=0x50; /*Serial Port mode2*/
	//===SM0==SM1==SM2==REN==TB8==RB8==TI==RI=
	//====0====1====0====1====0====0===0===0==
	
	TMOD=0x21;
	/*T1=mode2,T0=mode1*/
	
	TH1=TL1=250; 
	PCON|=0x80;//SMOD=1;
	TR0=TR1=1; /*Timer0,1 啟動!*/
	TI=1; //使傳送可以傳送
	
	//Initial string(防呆字串), 解決了第一個字錯誤的問題
	putchar(0);
	putchar(10); // '\n'
	
	//Dealy, 讓CPU 穩定的時間
	for(i=0;i<50;i++)
		for(j=1;j!=0;j++);

	/*
	1.開啟ADC硬體
	2.設定轉速
	3.選頻道
	4.被選中的頻道要設定成Input-Only Mode
	5.以ADRJ設定ADC結果格式
	*/
	
	ADCON=0x???; //步驟1~3
	//===ADCEN==SPEED1==SPEED2==ADCI==ADCS==CH2==CH1=CH0=
	//=====?======?=======?======?======?====?====?===?==
	
	//步驟4
	//有使用的通道Input-Only Mode
	for(i=0;i<???;++i)
	{
		P1M0|=???
		P1M1&=???
	}
	
	//步驟5
	//設定ADC結果格式by ADRJ(in AUXR0)
	AUXR0|=???;//ADCJ=?;
	
	
	while(1)
	{
		
		//切換頻道 & AD轉換 & 讀取 & 轉換資料格式
		for(i=0;i<???;++i)
		{
			ADCON&=???; //清除CH2,1,0
			ADCON|=???; //設定這次取樣的頻道
			ADCON|=???;	//ADCS=1,開始轉換
			while(???);	//等待ADCI=1(轉換完畢),這行會讓UART失效
			ADCON&=???;	//然後清除ADCI
			
			//取高中低3個Byte
			ADCV_Ch[i].ucBYTE[2]=???
			ADCV_Ch[i].ucBYTE[1]=???
			ADCV_Ch[i].ucBYTE[0]=???
			
			//將高中低3個Byte轉換為ASCII字元,可直接發送給SBUF
			ADCV_Ch[i].HEX[2]=HEX[ADCV_Ch[i].ucBYTE[2]];
			ADCV_Ch[i].HEX[1]=HEX[ADCV_Ch[i].ucBYTE[1]];
			ADCV_Ch[i].HEX[0]=HEX[ADCV_Ch[i].ucBYTE[0]];
		}
		
		
		//if(Flag_20ms)
			TxAllAnalogByASCII();
	}
}

void Timer0_INT(void) interrupt 1
{//定時對ADC sample & hold
	
	//Reload, 5ms 中斷一次
	TH0=(65536-9216)>>8;
	TL0=(65536-9216)%256;
	
	Counter5ms++;
	
	if(4==Counter5ms)
	{
		Flag_20ms=1;
		Counter5ms=0; //歸零
	}
	else
		Flag_20ms=0;

	
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
