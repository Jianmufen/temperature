#ifndef __LCD_H
#define __LCD_H


#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"  
#include "main.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//SSD1306 OLED 驱动IC驱动代码
//驱动方式:8080并口/4线串口
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/1/14
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 


//OLED模式设置
//并行8080模式 （模块的BS1，BS2均接VCC）
		    						  
//---------------------------OLED端口定义--------------------------  					   
    
/* DC */
#define OLED_DC_H()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9,GPIO_PIN_SET)
#define OLED_DC_L()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9,GPIO_PIN_RESET)
     
/* Write*/
#define OLED_Write_H()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,GPIO_PIN_SET)
#define OLED_Write_L()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10,GPIO_PIN_RESET)
/* Read */
#define OLED_Read_H()      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11,GPIO_PIN_SET)
#define OLED_Read_L()      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11,GPIO_PIN_RESET)
/* CS */
#define OLED_CS_H()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8,GPIO_PIN_SET)
#define OLED_CS_L()        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8,GPIO_PIN_RESET)

#define DATAOUT(x) GPIOB->ODR=(GPIOB->ODR&0xff00)|(x&0x00FF); //输出
     
extern int8_t number;
extern int8_t number1;
extern int8_t number2;
extern int8_t number3;
extern int8_t number4;
extern int8_t number5;

extern int8_t a1,a2,a3,a4,a5,a6,a7,a8;
extern int8_t b1,b2,b3,b4,b5,b6,b7;
extern int8_t c1,c2,c3,c4,c5,c6,c7;
extern int8_t d4,e4;

extern char disp_buf[128];

void OLED_W_Command(unsigned char com);	
void OLED_W_Data(unsigned char dat); 		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char highlight);
void OLED_Refresh_Gram(void);
void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t);
void OLED_ShowString(unsigned char x,unsigned char y,const char *p,unsigned char highlight);
void OLED_China(unsigned char x,unsigned char y,unsigned char n, unsigned char highlight);
void OLED_China1(unsigned char x,unsigned char y, unsigned char highlight);
void OLED_OpenMenu(void);
void OLED_Huatu(uint8_t x,uint8_t y,uint8_t i);
#ifdef __cplusplus
}
#endif
#endif 

	 







 

