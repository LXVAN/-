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

sbit ADCS = P2 ^ 0;  //AD的片选 
sbit ADDI = P2 ^ 5;
sbit ADDO = P2 ^ 5;
sbit ADCLK = P2 ^ 4;  //AD的CLK 
sbit gear0 = P1^1;//现在测试只有 P1^1 ~ P1^4可以正常工作
sbit gear1 = P1^2;
sbit gear2 = P1^3;
sbit on = P1^4;
sbit heat = P1^7;//加热信号输出端口





//-----------------------------------
unsigned char dispbitcode[8] = { 0xF7,0xFB,0xFD,0xFE,0xEF,0xDF,0xBF,0x7F }; //位扫描
unsigned char dispcode[11] = { 0xC0,0xF9,0xA4,0xbB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF };  //最后一个是全亮，防止出现一些问题 
//段选码 共阳极
unsigned char dispbuf[4];
unsigned int pressure_;
unsigned char getdata;
//----------------------------------------------------














int readtemp=0;
unsigned char str[]={"0123456789"};
unsigned char s[]={"Temperature:"};

void delay_18B20(unsigned int i)//延时函数
{
	for(;i>0;i--);
}

void Init_DS18B20(void) 	//初始化
{
  unsigned char x=0;
  DQ = 1;          //DQ拉高，
  delay_18B20(8);  //稍作延时
  DQ = 0;          //DQ拉低，    将总线拉低，持续时间为480us到960us之间
  delay_18B20(80); //延时大于480us
  DQ = 1;          //拉高总线，将总线拉高
  delay_18B20(14);
  x=DQ;            //若x=0初始化成功，若x=1初始化失败
  delay_18B20(20);//等待DS18B20的应答，若初始化成功，会在15-60us之后产生一个低电平信号，该信号会持续60us到240us
}                 //之后DS18B20会主动释放总线，总线电平会被拉高

unsigned char ReadOneChar(void)//读时序分为读“0”时序和读“1”时序两个过程。
  {
    unsigned char i=0;
    unsigned char dat = 0;
    for (i=8;i>0;i--) 
    {
      DQ = 0; // 拉低总线，当要读取DS18B20的数据时，我们需要将总线拉低，并保持1us的时间，
      dat>>=1;//每读取移位向右移移位
      DQ = 1; //拉高总线，然后将总线拉高，此时需尽快读取。
      if(DQ)
      dat|=0x80;
      delay_18B20(4);//从拉低到读取引脚状态的时间不能超过15us。
     }
     return(dat);
  }
	
void WriteOneChar(unsigned char dat)//写时序分为写“0”时序和写“1”时序两个过程。DS18B20写“0”时序和写“1”时序的要求不同，
	
{
  unsigned char i=0;
  for (i=8; i>0; i--)
  {
   DQ = 0;           
   DQ = dat&0x01;
   if(DQ){delay_18B20(1);DQ=1;}//当要写“0”时，单总线要被拉低至少60?s，以保证DS18B20能够在15?s到45?s之间，正确地采样I/O总线上的“0”电平
	 else{delay_18B20(5);DQ = 1;}//当要写“1”时，单总线被拉低之后，在15?s之内就得释放单总线。
   dat>>=1;
   }
}

unsigned char ReadTemperature(void)//读温度
{							 
 unsigned char a=0,b=0;
 unsigned int temp=0;
 Init_DS18B20();
 WriteOneChar(0xCC);  // 跳过读序列号操作
 WriteOneChar(0x44);  // 启动温度转换
 delay_18B20(100);    // 
 Init_DS18B20();
 WriteOneChar(0xCC);  //跳过读序列号操作
 WriteOneChar(0xBE);  //读取温度寄存器
 delay_18B20(100);
 a=ReadOneChar();     //读温度低位
 b=ReadOneChar();     //读温度高位
 temp=((b*256+a)>>4); //当前采集温度除16得到实际温度
 return(temp);
}

void delay(unsigned int n)//延时函数
{
  unsigned int i=0,j=0;
	for(i=0;i<n;i++)
	{
	  for(j=0;j<120;j++);
	}
}

void writedat(unsigned char dat)//写数据函数，LCD1602的函数
{
  RS=1;  //  RS:数据/命令选择端
	RW=0;  //  R/W :读/写选择端 
	E=0;   //  使能端:下降沿有效 
	P1=dat;
	delay(5);
	E=1;
	E=0;
}

void writecom(unsigned char com)//写命令函数 
{
  RS=0;  //  RS:数据/命令选择端 
	RW=0;  //  R/W :读/写选择端  
	E=0;   //使能端:下降沿有效 
	P1=com;
	delay(5);
	E=1;
	E=0; 
}

void initlcd()//初始化LCD1602
{
  writecom(0x38);  //0x38；设置16×2显示
	writecom(0x0c);  //0x0C：设置开显示，不显示光标
	writecom(0x06);  //0x06：写一个字符后地址指针加1
	writecom(0x01);  //0x01：显示清0，数据指针清0
}

int displayTemp()//显示函数
{
	char a;
	unsigned int temp0=0,temp1=0,temp2=0,i=0;
	temp0=readtemp/100;
	temp1=(readtemp%100)/10;
	temp2=readtemp%10;
	writecom(0x80);  //0x80：LCD第一行的起始地址
	delay(5);    	// 延时
	while(s[i]!='\0')
	{
	  writedat(s[i]);
		delay(5);
		i++;
	}
	a=6;
	writecom(0x80+0x40+5);  //0x80+0x40：LCD第2行的起始地址
	delay(5);
	writedat(str[temp0]);
	delay(5);           //延时
	writedat(str[temp1]);
	delay(5);            //延时	
	writedat(str[temp2]);
	delay(5);
	writedat(0xDF);
	delay(5);           //延时
	writedat('C');
	delay(5); 	//延时
	
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
	dispbuf[3] = pressure_ / 1000;  //最高位 
			dispbuf[2] = (pressure_ % 1000) / 100;
			dispbuf[1] = ((pressure_ % 1000) % 100) / 10;
			dispbuf[0] = ((pressure_ % 1000) % 100) % 10;  //最低位
	for (k = 0; k < 4; k++)
	{
		P1 = ~dispbitcode[k];
		P0 = ~dispcode[dispbuf[k]];
		if (k == 1)
			P0 = P0 + 0x80;
		delay_1ms();
	}
}

unsigned int ADC0832(unsigned char channel)  //AD转换，返回结果
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
	pressure0 = 1000;//当测量压强大于这个值就要开始加热,这个值需要参考人体对床的最小压强
	tperature0 = 40;//3个温度档
	tperature1 = 45;
	tperature2 = 50;
	initlcd();
	
	
	while(1)
	{
		
	 readtemp=ReadTemperature();//温度的十进制数	
	 //readtemp = (float)readtemp;
	 //readtemp -=10;
			
	 //displayTemp();
		
		
		getdata = ADC0832(0);   //使用CH0 
		if (14 < getdata < 243)   //压力测量范围 
		{
			int vary = getdata;
			/*压力传感器将压力转换为电压量，AD将这个电压值进行数模转换，但是电压值并不直接意味着压力的大小
			例如5V并不意味着5kPa,因此还需要一步转换*/ 
			voltage = ((10.0 / 23.0)*vary) + 9.3; //press即是电压 
			pressure_ = (int)(voltage * 10);//由于pressure 是用拟合出来的曲线运算出来的,所以运算出来的数值会和实际数值有偏差
			//显示buff，一共四位 ,这个pressure 比实际值大十倍,设置压力阈值的时候要把阈值乘以十倍,不然不能做出判断
			 
			//displayPressure();			
		}
		P1 = 0x00;
		if(on)
		{
			tperature0 = 40;//3个温度档
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
		
		
		delay(10);//最后加一个延时,延缓判断时间,防止加热断断续续
		
		
		
		
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

