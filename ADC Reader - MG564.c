#include <REG_MG82FL524-564.H>
#include <stdio.h>
#include "ADC.h"
#include "UART.h"
#define clrwdt() WDTCR|=0x10
/*
�T�q�DAnalogŪ���{��
�ϥ�MG82FE564
ADC �� Port 1
Programmer: �L����@2014.12.27
*/
unsigned char code HEX[16]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39
													 ,0x41,0x42,0x43,0x44,0x45,0x46};

struct AnalogValue{
	unsigned char ucBYTE[3];	 //�H3��Byte�s��000~3FF
	unsigned char HEX[3]; 		 //�H3��Byte�s��"000"~"3FF",��UART��
	int DecVal; //�H��Ʈ榡�s��Dec,��MCU�����B���
};

unsigned char i,j; 						  //for loop
unsigned char ucQUERY;				 //Rx SBUF
unsigned char ucUsedChannel=3; //�Ѧ��]�w�̧ǭn�ϥΦh�ֳq�D

//Timer Flag
volatile bit Timer0_Flag_20ms=0;
	
xdata struct AnalogValue ADCV_Ch[8]; //�s��8�q�DADC�ഫ���G���}�C
void TxAllAnalogByASCII(void);

int main()
{
	IE=0x82; /* (���_Enable)EA=1,(Timer0���_)ET0=1*/
	//===EA========ET2==ES0=ET1==EX1=ET0==EX0==
	//====1====0====0====0====0====0===1===0===
	
	TMOD|=0x02;	//T0=mode1 for Timer0_INT_Counter
	TH0=TL0=(65535-225); //225/1843200 = 1/8192
	TR0=1; /*Timer0 �Ұ�!*/
	
	ADC_Initial();
	UART_Initial();
	//Initial string(���b�r��), �ѨM�F�Ĥ@�Ӧr���~�����D
	putchar(0);
	putchar(10); // '\n'
	
	//Dealy, ��CPU í�w���ɶ�
	for(i=0;i<50;i++)
		for(j=1;j!=0;j++);
	
	while(1)
	{
		if(Timer0_Flag_20ms)
		{
			Timer0_Flag_20ms=0;
			
			//�����W�D & AD�ഫ & Ū�� & �ഫ��Ʈ榡
			for(i=0;i<ucUsedChannel;++i)
			{
				ADCV_Ch[i].DecVal = ADC_10BitGet(i);
				
				//�N�����C3��Byte�ഫ��ASCII�r��,�i�����o�e��SBUF
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
{/*==�d��:0~1023(03FF)*/
	
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