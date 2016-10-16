#include <REG_MG82F564.h>
#include <BIT.h>
#include <stdio.h>
#define clrwdt() WDTCR|=0x10
/*
�T�q�DAnalogŪ���{��
�ϥ�MG82F564
ADC �� P1
*/
unsigned char code HEX[16]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39
													 ,0x41,0x42,0x43,0x44,0x45,0x46};

struct AnalogValue{
	unsigned char ucBYTE[3];	 //�H3��Byte�s��000~3FF
	unsigned char HEX[3]; 		 //�H3��Byte�s��"000"~"3FF",��UART��
	unsigned short int DecVal; //�H��Ʈ榡�s��Dec,��MCU�����B���
};

unsigned char i,j; 						   //for loop
unsigned char ucQUERY;				 //Rx SBUF
unsigned char ucUsedChannel=3; //�Ѧ��]�w�̧ǭn�ϥΦh�ֳq�D
char ucTxAllChannel[10]={0};
//char ucTxAllChannelForBCB[16]={0}; // ex: "$ADC,3FF,3FF,3FF"
							
struct AnalogValue ADCV_Ch[8]; //�s��8�q�DADC�ഫ���G���}�C

void TxAnalogByASCII(unsigned char Channel); //�o�eADC���G��SBUF
void TxAllAnalogByASCII(void);
void TxAllAnalogByASCII_BCB(void);
void MakeStringForBCB(void);

// //==== ������ ================================
// void delay1ms(int x)
// {	int i,j;				// �ŧi�ܼ� 
// 	for (i=1;i<x*12;i++)		// ����x��,����X*1ms
// 		for (j=1;j<120;j++)
// 			;// ����120��,����1ms
// }							// delay1ms()��Ƶ���

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
// 	TH0=156;TL0=TH0; //for PWM,100us���_�@��
// 	TH1=253;TL1=TH1; //for UART
	
	IE=0x90; /* (���_Enable)EA=1,(��C���_)ES=1*/
	SCON=0x70; /*Serial Port mode2*/
	//===SM0==SM1==SM2==REN==TB8==RB8==TI==RI=
	//====0====1====1====1====0====0===0===0==
	TMOD=0x20; /*Timer mode2*/
	TH1=TL1=250; 
	PCON|=0x80;//SMOD=1;
	TR1=1; /*Timer1 �Ұ�!*/
	TI=1; /*�o�g���_�Ұ�!*/
	ucQUERY=0;

	/*
	1.�}��ADC�w��
	2.�]�w��t
	3.���W�D
	4.�Q�襤���W�D�n�]�w��Input-Only Mode
	5.�HADRJ�]�wADC���G�榡
	*/
	
	ADCON=0xE0; //�B�J1~3
	//===ADCEN==SPEED1==SPEED2==ADCI==ADCS==CH2==CH1=CH0=
	//=====1======1=======1======0======0====0====0===0==
	
	//�B�J4
	//���ϥΪ��q�DInput-Only Mode
	for(i=0;i<ucUsedChannel;++i)
	{
		P1M0|=BIT[i];
		P1M1&=~BIT[i];
	}
	
	//�B�J5
	//�]�wADC���G�榡by ADRJ(in AUXR0)
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
		
		//�����W�D & AD�ഫ & Ū�� & �ഫ��Ʈ榡
		for(i=0;i<ucUsedChannel;++i)
		{
			ADCON&=0xF8; //�M��CH2,1,0
			ADCON|=i; //�]�w�o�����˪��W�D
			ADCON|=BIT3;	//ADCS=1,�}�l�ഫ
			while(!(ADCON&BIT4));	//����ADCI=1(�ഫ����),�o��|��UART����
			ADCON&=(~BIT4);	//�M��M��ADCI
			
			//�������C3��Byte
			ADCV_Ch[i].ucBYTE[2]=ADCH;
			ADCV_Ch[i].ucBYTE[1]=ADCL>>4;
			ADCV_Ch[i].ucBYTE[0]=ADCL&0x0F;
			
			//�N�����C3��Byte�ഫ��ASCII�r��,�i�����o�e��SBUF
			ADCV_Ch[i].HEX[2]=HEX[ADCV_Ch[i].ucBYTE[2]];
			ADCV_Ch[i].HEX[1]=HEX[ADCV_Ch[i].ucBYTE[1]];
			ADCV_Ch[i].HEX[0]=HEX[ADCV_Ch[i].ucBYTE[0]];
		}
		
		//�N�Ҧ��W�D��T�̧ǩ�J�P�@�r�� cTxAllChannel[10]={CH0,CH1,CH2,\n}
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
			case'3'://PC�ШD����q�D
				TxAllAnalogByASCII(); //�T�q�D�@�_
				break;
			
			case'B'://PC�ШD����q�D
				TxAllAnalogByASCII_BCB(); //�T�q�D�@�_
				break;
			
			case'Q'://PC�ШD����q�D0
				TxAnalogByASCII(0);
				break;
			
			case'R'://PC�ШD����q�D1
				TxAnalogByASCII(1);
				break;
			
			case'S'://PC�ШD����q�D2
				TxAnalogByASCII(2);
				break;
			
			default:
				break;
		}	
		
		//RI=0;
	}
	

}

void TxAnalogByASCII(unsigned char ucChannel)
{/*==�d��:0~1023(03FF)*/

// 	char j;
// 	for(j=2;0<=j;j--)
// 	{
// 		putchar(ADCV_Ch[ucChannel].HEX[j]);//�o�g��2bit��ASCII
// 		SBUF=ADCV_Ch[ucChannel].HEX[j];
// 		while(!TI);//���ݵo�g����
// 		TI=0;			//�M���o�g���_�X��
// 	}
	
	printf("%c%c%c\n",
				ADCV_Ch[ucChannel].HEX[2],
				ADCV_Ch[ucChannel].HEX[1],
				ADCV_Ch[ucChannel].HEX[0]);
	
	
	ucQUERY=0; //�M��PC�e�Ӫ��ШD�T��
}

void TxAllAnalogByASCII(void)
{/*==�d��:0~1023(03FF)*/

// 	char j;
// 	for(j=2;0<=j;j--)
// 	{
// 		putchar(ADCV_Ch[ucChannel].HEX[j]);//�o�g��2bit��ASCII
// 		SBUF=ADCV_Ch[ucChannel].HEX[j];
// 		while(!TI);//���ݵo�g����
// 		TI=0;			//�M���o�g���_�X��
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
	
	ucQUERY=0; //�M��PC�e�Ӫ��ШD�T��
}

void TxAllAnalogByASCII_BCB(void)
{/*==�d��:0~1023(03FF)*/

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
	
	ucQUERY=0; //�M��PC�e�Ӫ��ШD�T��
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