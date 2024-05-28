#include "reg51.h"
#include <stdlib.h>
#include <string.h>
#include <intrins.h>
//--------------------------------
sbit RS=P3^0;
sbit RW=P3^1;
sbit E=P3^2;
sbit DQ=P3^7;
//------------------------------------

sbit ADCS = P2 ^ 0;  //AD��Ƭѡ 
sbit ADDI = P2 ^ 5;
sbit ADDO = P2 ^ 5;
sbit ADCLK = P2 ^ 4;  //AD��CLK 
sbit gear0 = P1^1;//���ڲ���ֻ�� P1^1 ~ P1^4������������
sbit gear1 = P1^2;
sbit gear2 = P1^3;
sbit on = P1^4;
sbit heat = P1^7;//�����ź�����˿�





//-----------------------------------
unsigned char dispbitcode[8] = { 0xF7,0xFB,0xFD,0xFE,0xEF,0xDF,0xBF,0x7F }; //λɨ��
unsigned char dispcode[11] = { 0xC0,0xF9,0xA4,0xbB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF };  //���һ����ȫ������ֹ����һЩ���� 
//��ѡ�� ������
unsigned char dispbuf[4];
unsigned int pressure_;
unsigned char getdata;
//----------------------------------------------------














int readtemp=0;
unsigned char str[]={"0123456789"};
unsigned char s[]={"Temperature:"};

void delay_18B20(unsigned int i)//��ʱ����
{
	for(;i>0;i--);
}

void Init_DS18B20(void) 	//��ʼ��
{
  unsigned char x=0;
  DQ = 1;          //DQ���ߣ�
  delay_18B20(8);  //������ʱ
  DQ = 0;          //DQ���ͣ�    ���������ͣ�����ʱ��Ϊ480us��960us֮��
  delay_18B20(80); //��ʱ����480us
  DQ = 1;          //�������ߣ�����������
  delay_18B20(14);
  x=DQ;            //��x=0��ʼ���ɹ�����x=1��ʼ��ʧ��
  delay_18B20(20);//�ȴ�DS18B20��Ӧ������ʼ���ɹ�������15-60us֮�����һ���͵�ƽ�źţ����źŻ����60us��240us
}                 //֮��DS18B20�������ͷ����ߣ����ߵ�ƽ�ᱻ����

unsigned char ReadOneChar(void)//��ʱ���Ϊ����0��ʱ��Ͷ���1��ʱ���������̡�
  {
    unsigned char i=0;
    unsigned char dat = 0;
    for (i=8;i>0;i--) 
    {
      DQ = 0; // �������ߣ���Ҫ��ȡDS18B20������ʱ��������Ҫ���������ͣ�������1us��ʱ�䣬
      dat>>=1;//ÿ��ȡ��λ��������λ
      DQ = 1; //�������ߣ�Ȼ���������ߣ���ʱ�辡���ȡ��
      if(DQ)
      dat|=0x80;
      delay_18B20(4);//�����͵���ȡ����״̬��ʱ�䲻�ܳ���15us��
     }
     return(dat);
  }
	
void WriteOneChar(unsigned char dat)//дʱ���Ϊд��0��ʱ���д��1��ʱ���������̡�DS18B20д��0��ʱ���д��1��ʱ���Ҫ��ͬ��
	
{
  unsigned char i=0;
  for (i=8; i>0; i--)
  {
   DQ = 0;           
   DQ = dat&0x01;
   if(DQ){delay_18B20(1);DQ=1;}//��Ҫд��0��ʱ��������Ҫ����������60?s���Ա�֤DS18B20�ܹ���15?s��45?s֮�䣬��ȷ�ز���I/O�����ϵġ�0����ƽ
	 else{delay_18B20(5);DQ = 1;}//��Ҫд��1��ʱ�������߱�����֮����15?s֮�ھ͵��ͷŵ����ߡ�
   dat>>=1;
   }
}

unsigned char ReadTemperature(void)//���¶�
{							 
 unsigned char a=0,b=0;
 unsigned int temp=0;
 Init_DS18B20();
 WriteOneChar(0xCC);  // ���������кŲ���
 WriteOneChar(0x44);  // �����¶�ת��
 delay_18B20(100);    // 
 Init_DS18B20();
 WriteOneChar(0xCC);  //���������кŲ���
 WriteOneChar(0xBE);  //��ȡ�¶ȼĴ���
 delay_18B20(100);
 a=ReadOneChar();     //���¶ȵ�λ
 b=ReadOneChar();     //���¶ȸ�λ
 temp=((b*256+a)>>4); //��ǰ�ɼ��¶ȳ�16�õ�ʵ���¶�
 return(temp);
}

void delay(unsigned int n)//��ʱ����
{
  unsigned int i=0,j=0;
	for(i=0;i<n;i++)
	{
	  for(j=0;j<120;j++);
	}
}

void writedat(unsigned char dat)//д���ݺ�����LCD1602�ĺ���
{
  RS=1;  //  RS:����/����ѡ���
	RW=0;  //  R/W :��/дѡ��� 
	E=0;   //  ʹ�ܶ�:�½�����Ч 
	P1=dat;
	delay(5);
	E=1;
	E=0;
}

void writecom(unsigned char com)//д����� 
{
  RS=0;  //  RS:����/����ѡ��� 
	RW=0;  //  R/W :��/дѡ���  
	E=0;   //ʹ�ܶ�:�½�����Ч 
	P1=com;
	delay(5);
	E=1;
	E=0; 
}

void initlcd()//��ʼ��LCD1602
{
  writecom(0x38);  //0x38������16��2��ʾ
	writecom(0x0c);  //0x0C�����ÿ���ʾ������ʾ���
	writecom(0x06);  //0x06��дһ���ַ����ַָ���1
	writecom(0x01);  //0x01����ʾ��0������ָ����0
}

int displayTemp()//��ʾ����
{
	char a;
	unsigned int temp0=0,temp1=0,temp2=0,i=0;
	temp0=readtemp/100;
	temp1=(readtemp%100)/10;
	temp2=readtemp%10;
	writecom(0x80);  //0x80��LCD��һ�е���ʼ��ַ
	delay(5);    	// ��ʱ
	while(s[i]!='\0')
	{
	  writedat(s[i]);
		delay(5);
		i++;
	}
	a=6;
	writecom(0x80+0x40+5);  //0x80+0x40��LCD��2�е���ʼ��ַ
	delay(5);
	writedat(str[temp0]);
	delay(5);           //��ʱ
	writedat(str[temp1]);
	delay(5);            //��ʱ	
	writedat(str[temp2]);
	delay(5);
	writedat(0xDF);
	delay(5);           //��ʱ
	writedat('C');
	delay(5); 	//��ʱ
	
	return str[temp0]+str[temp1]+str[temp2];
	
}





//----------------------------------------------------	
//----------------------------------------------------	
//----------------------------------------------------	
//----------------------------------------------------	
//----------------------------------------------------	
//----------------------------------------------------	
//----------------------------------------------------	
void delay_1ms(void)
{
	unsigned char x, y;
	x = 3;
	while (x--)
	{
		y = 40;
		while (y--);
	}
}
void displayPressure(void)
{
	char k;
	dispbuf[3] = pressure_ / 1000;  //���λ 
			dispbuf[2] = (pressure_ % 1000) / 100;
			dispbuf[1] = ((pressure_ % 1000) % 100) / 10;
			dispbuf[0] = ((pressure_ % 1000) % 100) % 10;  //���λ
	for (k = 0; k < 4; k++)
	{
		P1 = ~dispbitcode[k];
		P0 = ~dispcode[dispbuf[k]];
		if (k == 1)
			P0 = P0 + 0x80;
		delay_1ms();
	}
}

unsigned int ADC0832(unsigned char channel)  //ADת�������ؽ��
{
	unsigned char i = 0;
	unsigned char j;
	unsigned int dat = 0;
	unsigned char ndat = 0;

	if (channel == 0) channel = 2;
	if (channel == 1) channel = 3;
	ADDI = 1;
	_nop_();
	_nop_();
	ADCS = 0;
	_nop_();
	_nop_();
	ADCLK = 1;
	_nop_();
	_nop_();
	ADCLK = 0;
	_nop_();
	_nop_();
	ADCLK = 1;
	ADDI = channel & 0x1;
	_nop_();
	_nop_();
	ADCLK = 0;
	_nop_();
	_nop_();
	ADCLK = 1;
	ADDI = (channel >> 1) & 0x1;
	_nop_();
	_nop_();
	ADCLK = 0;
	ADDI = 1;
	_nop_();
	_nop_();
	dat = 0;
	for (i = 0; i < 8; i++)
	{
		dat |= ADDO;
		ADCLK = 1;
		_nop_();
		_nop_();
		ADCLK = 0;
		_nop_();
		_nop_();
		dat <<= 1;
		if (i == 7) dat |= ADDO;
	}
	for (i = 0; i < 8; i++)
	{
		j = 0;
		j = j | ADDO;
		ADCLK = 1;
		_nop_();
		_nop_();
		ADCLK = 0;
		_nop_();
		_nop_();
		j = j << 7;
		ndat = ndat | j;
		if (i < 7) ndat >>= 1;
	}
	ADCS = 1;
	ADCLK = 0;
	ADDO = 1;
	dat <<= 8;
	dat |= ndat;
	return(dat);
}



//----------------------------------------------------------

void main()
{
	float voltage;
	int pressure0,tperature0,tperature1,tperature2,Gear,a;
	//int *a;
	pressure0 = 1000;//������ѹǿ�������ֵ��Ҫ��ʼ����,���ֵ��Ҫ�ο�����Դ�����Сѹǿ
	tperature0 = 40;//3���¶ȵ�
	tperature1 = 45;
	tperature2 = 50;
	initlcd();
	
	
	while(1)
	{
		
	 readtemp=ReadTemperature();//�¶ȵ�ʮ������	
	 //readtemp = (float)readtemp;
	 //readtemp -=10;
			
	 //displayTemp();
		
		
		getdata = ADC0832(0);   //ʹ��CH0 
		if (14 < getdata < 243)   //ѹ��������Χ 
		{
			int vary = getdata;
			/*ѹ����������ѹ��ת��Ϊ��ѹ����AD�������ѹֵ������ģת�������ǵ�ѹֵ����ֱ����ζ��ѹ���Ĵ�С
			����5V������ζ��5kPa,��˻���Ҫһ��ת��*/ 
			voltage = ((10.0 / 23.0)*vary) + 9.3; //press���ǵ�ѹ 
			pressure_ = (int)(voltage * 10);//����pressure ������ϳ������������������,���������������ֵ���ʵ����ֵ��ƫ��
			//��ʾbuff��һ����λ ,���pressure ��ʵ��ֵ��ʮ��,����ѹ����ֵ��ʱ��Ҫ����ֵ����ʮ��,��Ȼ���������ж�
			 
			//displayPressure();			
		}
		P1 = 0x00;
		if(on)
		{
			tperature0 = 40;//3���¶ȵ�
			tperature1 = 45;
			tperature2 = 50;
			
			if(pressure_>pressure0)
			{
				if(gear0==1&&(readtemp<tperature0))
				{
					heat = 1;
					delay(50);
				}else if(gear0==1&&(readtemp>tperature0))
				{
					heat = 0;
					delay(50);
				}
				
				if(gear1==1&&(readtemp<tperature1))
				{
					heat = 1;
					delay(50);
				}else if(gear1==1&&(readtemp>tperature1))
				{
					heat = 0;
					delay(60);
				}
				if(gear2==1&&(readtemp<tperature2))
				{
					heat = 1;
					delay(50);
				}else if(gear2==1&&(readtemp>tperature2))
				{
					heat = 0;
				}
				delay(50);
				



				
			}
			
		}
		else
		{
			heat = 0;
		}
		
		
		delay(10);//����һ����ʱ,�ӻ��ж�ʱ��,��ֹ���ȶ϶�����
		
		
		
		
//		if(pressure_>1030&&readtemp<45)
//		{
//			heat = 1;
//		}
//		else
//		{
//			heat = 0;
//		}
		
		
		
					
					
					
					  
		
//			if(0&&readtemp>26)
//			{
//				P0 = 0x55;
//			}
//			else
//			{
//				P0 = 0x00;
//			}
		
		
		
	} 
}

