#include <REG_MG82F564.h>
#include <BIT.h>
#include <stdio.h>
#define clrwdt() WDTCR|=0x10
/*
三通道Analog讀取程式
使用MG82F564
ADC 為 P1
*/
unsigned char code HEX[16]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39
													 ,0x41,0x42,0x43,0x44,0x45,0x46};

struct AnalogValue{
	unsigned char ucBYTE[3];	 //以3個Byte存放000~3FF
	unsigned char HEX[3]; 		 //以3個Byte存放"000"~"3FF",供UART用
	unsigned short int DecVal; //以整數格式存放Dec,供MCU內部運算用
};

unsigned char i,j; 						   //for loop
unsigned char ucQUERY;				 //Rx SBUF
unsigned char ucUsedChannel=3; //由此設定依序要使用多少通道
char ucTxAllChannel[10]={0};
//char ucTxAllChannelForBCB[16]={0}; // ex: "$ADC,3FF,3FF,3FF"
							
struct AnalogValue ADCV_Ch[8]; //存放8通道ADC轉換結果的陣列

void TxAnalogByASCII(unsigned char Channel); //發送ADC結果到SBUF
void TxAllAnalogByASCII(void);
void TxAllAnalogByASCII_BCB(void);
void MakeStringForBCB(void);

// //==== 延遲函數 ================================
// void delay1ms(int x)
// {	int i,j;				// 宣告變數 
// 	for (i=1;i<x*12;i++)		// 執行x次,延遲X*1ms
// 		for (j=1;j<120;j++)
// 			;// 執行120次,延遲1ms
// }							// delay1ms()函數結束

// void putchar(char c)
// {
// 	if (c=='\n')
// 	{
// 		while(!TI)
// 			clrwdt();

// 		TI=0;
// 		SBUF=0x0d;
// 	}
// 	
// 	while(!TI)
// 			clrwdt();
// 	
// 	TI=0;
// 	SBUF=c;
// 	//return(c);
// }

main()
{
// 	SCON=0x50;TMOD=0x22;
// 	TH0=156;TL0=TH0; //for PWM,100us中斷一次
// 	TH1=253;TL1=TH1; //for UART
	
	IE=0x90; /* (中斷Enable)EA=1,(串列中斷)ES=1*/
	SCON=0x70; /*Serial Port mode2*/
	//===SM0==SM1==SM2==REN==TB8==RB8==TI==RI=
	//====0====1====1====1====0====0===0===0==
	TMOD=0x20; /*Timer mode2*/
	TH1=TL1=250; 
	PCON|=0x80;//SMOD=1;
	TR1=1; /*Timer1 啟動!*/
	TI=1; /*發射中斷啟動!*/
	ucQUERY=0;

	/*
	1.開啟ADC硬體
	2.設定轉速
	3.選頻道
	4.被選中的頻道要設定成Input-Only Mode
	5.以ADRJ設定ADC結果格式
	*/
	
	ADCON=0xE0; //步驟1~3
	//===ADCEN==SPEED1==SPEED2==ADCI==ADCS==CH2==CH1=CH0=
	//=====1======1=======1======0======0====0====0===0==
	
	//步驟4
	//有使用的通道Input-Only Mode
	for(i=0;i<ucUsedChannel;++i)
	{
		P1M0|=BIT[i];
		P1M1&=~BIT[i];
	}
	
	//步驟5
	//設定ADC結果格式by ADRJ(in AUXR0)
	AUXR0|=BIT2;//ADCJ=1;
	
	//Set LED
	for(i=0;i<8;++i)
	{
		P0M0|=BIT[i];
		P0M1|=BIT[i];
	}
	P0=0x00;
	
	while(1)
	{
		
		//切換頻道 & AD轉換 & 讀取 & 轉換資料格式
		for(i=0;i<ucUsedChannel;++i)
		{
			ADCON&=0xF8; //清除CH2,1,0
			ADCON|=i; //設定這次取樣的頻道
			ADCON|=BIT3;	//ADCS=1,開始轉換
			while(!(ADCON&BIT4));	//等待ADCI=1(轉換完畢),這行會讓UART失效
			ADCON&=(~BIT4);	//然後清除ADCI
			
			//取高中低3個Byte
			ADCV_Ch[i].ucBYTE[2]=ADCH;
			ADCV_Ch[i].ucBYTE[1]=ADCL>>4;
			ADCV_Ch[i].ucBYTE[0]=ADCL&0x0F;
			
			//將高中低3個Byte轉換為ASCII字元,可直接發送給SBUF
			ADCV_Ch[i].HEX[2]=HEX[ADCV_Ch[i].ucBYTE[2]];
			ADCV_Ch[i].HEX[1]=HEX[ADCV_Ch[i].ucBYTE[1]];
			ADCV_Ch[i].HEX[0]=HEX[ADCV_Ch[i].ucBYTE[0]];
		}
		
		//將所有頻道資訊依序放入同一字串 cTxAllChannel[10]={CH0,CH1,CH2,\n}
		for(i=0;i<ucUsedChannel;++i)
			for(j=2;j<=2&&j>=0;--j)
				ucTxAllChannel[(2-j)+3*i]=ADCV_Ch[i].HEX[j];
		
		//ucTxAllChannel[9]=0;
		
		//MakeStringForBCB();
	}
}


void serial_INT(void) interrupt 4
{	
	if(RI)
	{
		ucQUERY=SBUF;
		RI=0;
		
		switch(ucQUERY)
		{
			case'3'://PC請求類比通道
				TxAllAnalogByASCII(); //三通道一起
				break;
			
			case'B'://PC請求類比通道
				TxAllAnalogByASCII_BCB(); //三通道一起
				break;
			
			case'Q'://PC請求類比通道0
				TxAnalogByASCII(0);
				break;
			
			case'R'://PC請求類比通道1
				TxAnalogByASCII(1);
				break;
			
			case'S'://PC請求類比通道2
				TxAnalogByASCII(2);
				break;
			
			default:
				break;
		}	
		
		//RI=0;
	}
	

}

void TxAnalogByASCII(unsigned char ucChannel)
{/*==範圍:0~1023(03FF)*/

// 	char j;
// 	for(j=2;0<=j;j--)
// 	{
// 		putchar(ADCV_Ch[ucChannel].HEX[j]);//發射高2bit之ASCII
// 		SBUF=ADCV_Ch[ucChannel].HEX[j];
// 		while(!TI);//等待發射完畢
// 		TI=0;			//清除發射中斷旗標
// 	}
	
	printf("%c%c%c\n",
				ADCV_Ch[ucChannel].HEX[2],
				ADCV_Ch[ucChannel].HEX[1],
				ADCV_Ch[ucChannel].HEX[0]);
	
	
	ucQUERY=0; //清除PC送來的請求訊號
}

void TxAllAnalogByASCII(void)
{/*==範圍:0~1023(03FF)*/

// 	char j;
// 	for(j=2;0<=j;j--)
// 	{
// 		putchar(ADCV_Ch[ucChannel].HEX[j]);//發射高2bit之ASCII
// 		SBUF=ADCV_Ch[ucChannel].HEX[j];
// 		while(!TI);//等待發射完畢
// 		TI=0;			//清除發射中斷旗標
// 	}
	
// 	printf("%c%c%c%c%c%c%c%c%c\n",
// 				ADCV_Ch[0].HEX[2],
// 				ADCV_Ch[0].HEX[1],
// 				ADCV_Ch[0].HEX[0],
// 				
// 				ADCV_Ch[1].HEX[2],
// 				ADCV_Ch[1].HEX[1],
// 				ADCV_Ch[1].HEX[0],
// 				
// 				ADCV_Ch[2].HEX[2],
// 				ADCV_Ch[2].HEX[1],
// 				ADCV_Ch[2].HEX[0]
// 	);

	printf("%s\n\0",ucTxAllChannel);
	
	ucQUERY=0; //清除PC送來的請求訊號
}

void TxAllAnalogByASCII_BCB(void)
{/*==範圍:0~1023(03FF)*/

	//printf("$ADC,%s\n\0",ucTxAllChannel);
	
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
	
	ucQUERY=0; //清除PC送來的請求訊號
}

// void MakeStringForBCB(void)
// {
// 	sprintf(ucTxAllChannelForBCB,"$ADC,%c%c%c,%c%c%c,%c%c%c",
// 				ADCV_Ch[0].HEX[2],
// 				ADCV_Ch[0].HEX[1],
// 				ADCV_Ch[0].HEX[0],
// 				
// 				ADCV_Ch[1].HEX[2],
// 				ADCV_Ch[1].HEX[1],
// 				ADCV_Ch[1].HEX[0],
// 				
// 				ADCV_Ch[2].HEX[2],
// 				ADCV_Ch[2].HEX[1],
// 				ADCV_Ch[2].HEX[0]
// 	);
// }