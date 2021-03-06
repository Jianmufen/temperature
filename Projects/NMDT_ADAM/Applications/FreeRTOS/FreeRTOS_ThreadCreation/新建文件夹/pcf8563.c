#include "pcf8563.h"
#include "usart_module.h"

//模拟IIC驱动PCF8563   PB10=SCL,PB11=SDA,PA0=INT  晶振为32.768KHz.
PCF8563_Time_Typedef PCF_DataStruct_Time;
unsigned char Time_Buffer[7];

//初始化IIC
HAL_StatusTypeDef IIC_Init(void)
{					     
	IIC_Start(); //I2C总线启动
	IIC_Send_Byte(0xA2);//发送写命令
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("111\r\n");
		return HAL_ERROR;//函数运行到这里  就结束了
	}
	IIC_Send_Byte(0x00);//选择地址0x00控制状态寄存器1，之后读写地址自动增1
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("222\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x00); //设置STOP=0，芯片时钟运行
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("333\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x13);//打开报警中断和定时器中断，脉冲方式
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("444\r\n");
		return HAL_ERROR;
	}
	IIC_Start(); //I2C总线启动
	IIC_Send_Byte(0xA2);//发送写命令
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("555\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x09);//选择地址0x09分钟报警寄存器
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("666\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x80);//关闭分钟报警
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("777\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x80);//关闭小时报警
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("888\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x80);//关闭日报警
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("999\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x80);//关闭星期报警
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("000\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x00);//禁用CLKOUT输出并设为高阻抗
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("123\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x82);//定时器有效，频率设为1Hz
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("456\r\n");
		return HAL_ERROR;
	}
	IIC_Send_Byte(0x01);//定时器有效，频率设为1Hz
	if(IIC_Wait_Ack()==HAL_ERROR)//等待I2C总线回复
	{
		DEBUG("789\r\n");
		return HAL_ERROR;
		
	}
	IIC_Stop(); //I2C总线停止
	return HAL_OK;
}


//配置SDA为输出模式
void SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*Configure GPIO pins : PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

//配置SDA为输入模式
void SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/*Configure GPIO pins : PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT ;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


void I2C_delay(void)  //PCF8563的I2C总线频率为400kHz，根据不同处理器设定适当等待数
{
	volatile unsigned char i=5;
	while(i)
	{
		i--;
	}
}

//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA_H;	  	  
	IIC_SCL_H;
	I2C_delay();
 	IIC_SDA_L;//START:when CLK is high,DATA change form high to low 
	I2C_delay();
	IIC_SCL_L;//钳住I2C总线，准备发送或接收数据 
	I2C_delay();
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();     //sda线输出
	IIC_SCL_H;
	IIC_SDA_L;
 	I2C_delay();
	IIC_SDA_H;
	I2C_delay();	
  IIC_SCL_L; 	
	I2C_delay();
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
HAL_StatusTypeDef IIC_Wait_Ack(void)
{
	SDA_IN();      //SDA设置为输入  
	IIC_SCL_L;
 	I2C_delay();
	IIC_SDA_H;//在标志位时钟脉冲出现时 SDA 线应保持低电平
 	I2C_delay();   
	IIC_SCL_H;
	I2C_delay();
	if(SSDA)//如果SDA的电平为低，就代表接收带PCF8563发出的应答信号
	{
		I2C_delay();  
		IIC_SCL_L;
		return HAL_ERROR;
	}
	I2C_delay();  
	IIC_SCL_L;//时钟输出0 	   
	return HAL_OK;;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	SDA_OUT();     //sda线输出
	IIC_SCL_L;
	IIC_SDA_L;
	I2C_delay();
	IIC_SCL_H;
	I2C_delay();
	IIC_SCL_L;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	SDA_OUT();     //sda线输出
	IIC_SCL_L;
	IIC_SDA_H;//
	I2C_delay();
	IIC_SCL_H;
	I2C_delay();
	IIC_SCL_L;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
unsigned char IIC_Send_Byte(unsigned char RTC_Cmd)
{                        
  unsigned char i=8;  
SDA_OUT();     //sda线输出  
  while(i--)
	{
		IIC_SCL_L;
		if(RTC_Cmd&0x80)
		{
			IIC_SDA_H;
		}
		else
		{
			IIC_SDA_L;
		}
		RTC_Cmd<<=1;
		I2C_delay();
		IIC_SCL_H;
		I2C_delay();
		IIC_SCL_L;
		I2C_delay();
	}
	IIC_SCL_L;
	return SSDA;//每接收到8位数据位，PCF8563产生一个应答信号，由SDA接收到
   	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
unsigned char IIC_Read_Byte(void)
{
	unsigned char  i=8;
	unsigned char Read_Byte=0;
	SDA_IN();//SDA设置为输入
  IIC_SDA_H;
	while(i--)
	{
		Read_Byte<<=1;
		IIC_SCL_L;
		I2C_delay();
		IIC_SCL_H;
		I2C_delay();
		if(SSDA)//如果SDA脚接收到高电平
		{
			Read_Byte|=0x01;
		}
	}
	IIC_SCL_L;
	return Read_Byte;
	
}



HAL_StatusTypeDef PCF8563_Set_Time(uint8_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second)
{
	//检查参数，屏蔽无效位
	if (year > 99)      year    = 0;  //恢复00年
	if (month > 12)     month   = 1;  //恢复1月
	if (day > 31)       day     = 1;  //恢复1日
	if (hour > 23)      hour    = 0;  //恢复0时
	if (minute > 59)    minute  = 0;  //恢复0分钟
	if (second > 59)    second  = 0;  //恢复0秒
	PCF_DataStruct_Time.RTC_Year   =  RTC_ByteToBcd2(year);
	PCF_DataStruct_Time.RTC_Month  =  RTC_ByteToBcd2(month);
	PCF_DataStruct_Time.RTC_Day    =  RTC_ByteToBcd2(day);
	PCF_DataStruct_Time.RTC_Hour   =  RTC_ByteToBcd2(hour);
	PCF_DataStruct_Time.RTC_Minute =  RTC_ByteToBcd2(minute);
	PCF_DataStruct_Time.RTC_Second =  RTC_ByteToBcd2(second);
	IIC_Start(); //I2C总线启动
	IIC_Send_Byte(0xA2);//发送写命令
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号1\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Send_Byte(0x02);//选择秒寄存器地址0x02
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号2\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Second);//写入秒  （0x02）
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号3\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Minute);//写入分  （0x03）
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号4\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Hour);//写入时  （0x04）
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号5\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Day);//写入天  （0x05）
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号6\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Send_Byte(3);//写入星期  （0x06）
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号7\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Month);//写入月  （0x07）
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号8\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Send_Byte(PCF_DataStruct_Time.RTC_Year);//写入年  （0x08）
	if(IIC_Wait_Ack()==HAL_ERROR)
  {
		//printf("没有等到应答信号9\r\n");
		return  HAL_ERROR;
	}//等待I2C总线回复
	
	IIC_Stop();
	return HAL_OK;
}

HAL_StatusTypeDef PCF8563_Read_Time(void)
{
	IIC_Start(); //I2C总线启动
	IIC_Send_Byte(0xA2);//发送写命令
	if(IIC_Wait_Ack()==HAL_ERROR)
	{
		//printf("没有等到应答信号10\r\n");
		return HAL_ERROR;
	}
	//IIC_Wait_Ack();
	IIC_Send_Byte(0x02);//发送起始地址
	if(IIC_Wait_Ack()==HAL_ERROR)
	{
		//printf("没有等到应答信号11\r\n");
		return HAL_ERROR;
	}
	IIC_Start();
	IIC_Send_Byte(0xA3);//发送读命令
	if(IIC_Wait_Ack()==HAL_ERROR)
	{
		//printf("没有等到应答信号12\r\n");
		return HAL_ERROR;
	}
	Time_Buffer[0]=IIC_Read_Byte();//读取秒
	IIC_Ack();//接收数据的时候要产生应答信号，让PCF8563接收到，确认已接收到数据
	
	Time_Buffer[1]=IIC_Read_Byte();//读取分钟
	IIC_Ack();
	
	Time_Buffer[2]=IIC_Read_Byte();//读取小时
	IIC_Ack();
	
	Time_Buffer[3]=IIC_Read_Byte();//读取天
	IIC_Ack();
	
	Time_Buffer[4]=IIC_Read_Byte();//读取星期
	IIC_Ack();
	
	Time_Buffer[5]=IIC_Read_Byte();//读取月
	IIC_Ack();
	
	Time_Buffer[6]=IIC_Read_Byte();//读取年
	IIC_NAck();
	IIC_Stop();
	
	//屏蔽参数
	Time_Buffer[6] &= 0xff;//年
	Time_Buffer[5] &= 0x1f;//月
	Time_Buffer[3] &= 0x3f;//日
	Time_Buffer[2] &= 0x3f;//时
	Time_Buffer[1] &= 0x7f;//分
	Time_Buffer[0] &= 0x7f;//秒
	//转换成十进制
	PCF_DataStruct_Time.RTC_Year   =  RTC_Bcd2ToByte(Time_Buffer[6]);//年
	PCF_DataStruct_Time.RTC_Month  =  RTC_Bcd2ToByte(Time_Buffer[5]);//月
	PCF_DataStruct_Time.RTC_Day    =  RTC_Bcd2ToByte(Time_Buffer[3]);//天
	PCF_DataStruct_Time.RTC_Hour   =  RTC_Bcd2ToByte(Time_Buffer[2]);//时
	PCF_DataStruct_Time.RTC_Minute =  RTC_Bcd2ToByte(Time_Buffer[1]);//分
	PCF_DataStruct_Time.RTC_Second =  RTC_Bcd2ToByte(Time_Buffer[0]);//秒
	return HAL_OK;
}

