#include "Bit.h"

void ADC_Initial(void)
{
	/*
	1.�}��ADC�w��
	2.�]�w��t
	3.���W�D
	4.�Q�襤���W�D�n�]�w��Input-Only Mode
	*/
	
	//***Step1~3***//
	ADCON=0xE0;
	//===ADCEN==SPEED1==SPEED2==ADCI==ADCS==CH2==CH1=CH0=
	//=====1======1=======1======0======0====0====0===0==
	
	//***Step4:���ϥΪ��q�DInput-Only Mode, ���Ϊ�comment��***//
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
	ADCON&=0xF8; //�M���q�D���
	ADCON|=0x07&Channel; //�]�w�o�����˪��W�D
	ADCON|=BIT3;	//ADCS=1,�}�l�ഫ
	while(!(ADCON&BIT4));	//����ADCI=1(�ഫ����),�o��|��UART����
	ADCON&=(~BIT4);	//�M��M��ADCI
	
	return (ADCH<<2)&0x3FC + ADCL;
}