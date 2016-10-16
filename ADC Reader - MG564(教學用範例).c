#include <REG_MG82F564.h>
#include <BIT.h>
#include <stdio.h>
#define clrwdt() WDTCR|=0x10
/*
MG82FE564
��
"����ADCŪ��"�оǽd�ҵ{���A�HUART@19200bps�@����X���
ADC �� Port 1
Programmer: �L����@2013.8.9
*/

//���}�C�i�N 0~15 �� int �� char �ର ASCII
unsigned char code HEX[16]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39
													 ,0x41,0x42,0x43,0x44,0x45,0x46};

//�ΨӦs�� ADC ���G����Ƶ��c
struct AnalogValue{
	unsigned char ucBYTE[3];	 //�H3��Byte�s��000~3FF
	unsigned char HEX[3]; 		 //�H3��Byte�s��"000"~"3FF",��UART��
	unsigned short int DecVal; //�H��Ʈ榡�s��Dec,��MCU�����B���
};

unsigned char i,j; 						  //for loop
unsigned char ucQUERY;				 //Rx SBUF
unsigned char ucUsedChannel=3; //�Ѧ��]�w�̧ǭn�ϥΦh�ֳq�D

//Timer Flag
unsigned char Counter5ms=0;
bit Flag_20ms=0;
							
struct AnalogValue ADCV_Ch[8]; //�ŧi�s��8�q�DADC�ഫ���G���}�C���c

void TxAllAnalogByASCII(void);

main()
{
	
	IE=0x82; /* (���_Enable)EA=1,(Timer0���_)ET0=1*/
	//===EA========ET2==ES0=ET1==EX1=ET0==EX0==
	//====1====0====0====0====0====0===1===0===
	
	SCON=0x50; /*Serial Port mode2*/
	//===SM0==SM1==SM2==REN==TB8==RB8==TI==RI=
	//====0====1====0====1====0====0===0===0==
	
	TMOD=0x21;
	/*T1=mode2,T0=mode1*/
	
	TH1=TL1=250; 
	PCON|=0x80;//SMOD=1;
	TR0=TR1=1; /*Timer0,1 �Ұ�!*/
	TI=1; //�϶ǰe�i�H�ǰe
	
	//Initial string(���b�r��), �ѨM�F�Ĥ@�Ӧr���~�����D
	putchar(0);
	putchar(10); // '\n'
	
	//Dealy, ��CPU í�w���ɶ�
	for(i=0;i<50;i++)
		for(j=1;j!=0;j++);

	/*
	1.�}��ADC�w��
	2.�]�w��t
	3.���W�D
	4.�Q�襤���W�D�n�]�w��Input-Only Mode
	5.�HADRJ�]�wADC���G�榡
	*/
	
	ADCON=0x???; //�B�J1~3
	//===ADCEN==SPEED1==SPEED2==ADCI==ADCS==CH2==CH1=CH0=
	//=====?======?=======?======?======?====?====?===?==
	
	//�B�J4
	//���ϥΪ��q�DInput-Only Mode
	for(i=0;i<???;++i)
	{
		P1M0|=???
		P1M1&=???
	}
	
	//�B�J5
	//�]�wADC���G�榡by ADRJ(in AUXR0)
	AUXR0|=???;//ADCJ=?;
	
	
	while(1)
	{
		
		//�����W�D & AD�ഫ & Ū�� & �ഫ��Ʈ榡
		for(i=0;i<???;++i)
		{
			ADCON&=???; //�M��CH2,1,0
			ADCON|=???; //�]�w�o�����˪��W�D
			ADCON|=???;	//ADCS=1,�}�l�ഫ
			while(???);	//����ADCI=1(�ഫ����),�o��|��UART����
			ADCON&=???;	//�M��M��ADCI
			
			//�������C3��Byte
			ADCV_Ch[i].ucBYTE[2]=???
			ADCV_Ch[i].ucBYTE[1]=???
			ADCV_Ch[i].ucBYTE[0]=???
			
			//�N�����C3��Byte�ഫ��ASCII�r��,�i�����o�e��SBUF
			ADCV_Ch[i].HEX[2]=HEX[ADCV_Ch[i].ucBYTE[2]];
			ADCV_Ch[i].HEX[1]=HEX[ADCV_Ch[i].ucBYTE[1]];
			ADCV_Ch[i].HEX[0]=HEX[ADCV_Ch[i].ucBYTE[0]];
		}
		
		
		//if(Flag_20ms)
			TxAllAnalogByASCII();
	}
}

void Timer0_INT(void) interrupt 1
{//�w�ɹ�ADC sample & hold
	
	//Reload, 5ms ���_�@��
	TH0=(65536-9216)>>8;
	TL0=(65536-9216)%256;
	
	Counter5ms++;
	
	if(4==Counter5ms)
	{
		Flag_20ms=1;
		Counter5ms=0; //�k�s
	}
	else
		Flag_20ms=0;

	
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
