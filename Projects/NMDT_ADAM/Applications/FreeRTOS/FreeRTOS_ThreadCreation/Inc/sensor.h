#ifndef __SENSOR_H
#define __SENSOR_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l1xx_hal.h"
#include "main.h"	 
#include "time.h"		 
	 

	 
//#define QC_R 0  //数据正确
//#define QC_L 1  //数据缺测	 
//#define TEMP_CORRECTION_VALUE    (0)  /*PT100的温度订正值*/
	 
//#define st(x)      do { x } while (__LINE__ == -1)
   
/* length of array */
#define LENGTH_OF(x)   (sizeof(x)/sizeof(*(x)))

extern char  data_buf[512];
/*小时数据格式*/	 
typedef struct
{
	char temperature[4];            /*整点温度 "+201=+20.1   -201=-20.1"*/
	char temperature_max[4];        /*1小时最高温度 "+201=+20.1   -201=-20.1"*/
	char temperature_max_time[4];   /*1小时最高温度时间时分 "1211"*/
	char temperature_min[4];        /*1小时最低温度 "+201=+20.1   -201=-20.1"*/
	char temperature_min_time[4];   /*1小时最低温度时间时分 "1211"*/
	char dummy_3[50];               /*整点湿度最小湿度时间风“00000000009999999999999999999999999999999999999999”*/
	char rain_1[2];
	char rain_2[2];
	char rain_3[2];
	char rain_4[2];
	char rain_5[2];
	char rain_6[2];
	char rain_7[2];
	char rain_8[2];
	char rain_9[2];
	char rain_10[2];
	char rain_11[2];
	char rain_12[2];
	char rain_13[2];
	char rain_14[2];
	char rain_15[2];
	char rain_16[2];
	char rain_17[2];
	char rain_18[2];
	char rain_19[2];
	char rain_20[2];
	char rain_21[2];
	char rain_22[2];
	char rain_23[2];
	char rain_24[2];
	char rain_25[2];
	char rain_26[2];
	char rain_27[2];
	char rain_28[2];
	char rain_29[2];
	char rain_30[2];
	char rain_31[2];
	char rain_32[2];
	char rain_33[2];
	char rain_34[2];
	char rain_35[2];
	char rain_36[2];
	char rain_37[2];
	char rain_38[2];
	char rain_39[2];
	char rain_40[2];
	char rain_41[2];
	char rain_42[2];
	char rain_43[2];
	char rain_44[2];
	char rain_45[2];
	char rain_46[2];
	char rain_47[2];
	char rain_48[2];
	char rain_49[2];
	char rain_50[2];
	char rain_51[2];
	char rain_52[2];
	char rain_53[2];
	char rain_54[2];
	char rain_55[2];
	char rain_56[2];
	char rain_57[2];
	char rain_58[2];
	char rain_59[2];
	char rain_0[2];
	char dummy_4[37];               /*整点气压最高气压是分最低气压时分整点能见度最低能见度时分“0000000000000000000000000000000000000”*/
	char board_v[3];                /*整点电源电压“121=12.1V”*/
}DATA_1H;

	 
typedef struct   //通讯帧数据包头   分钟数据和历史数据
{
  char start_flag[2];   //起始符   "[D"
	char station_flag[5];  //站号：“00001”
  char time_flag[14];   //时间：年月日时分20171007150000
  DATA_1H data_1h;
  char end_flag[2];   //结束符  "]"
}DATA_1HOUR;	
//extern DATA_1HOUR data_hour;

typedef struct  //温度数据
{
	uint32_t  TEMP_AD;              /*温度AD采样值*/
	//int       TEMP_AD_2S[30];       /*一分钟内30个温度AD采样值*/
	float     TEMP_V;               /*测量的电压值*/
	//float     TEMP_V_2S[30];        /*30个电压值*/ 
  float     TEMP_R;               /*温度电阻采样值*/
	//float     TEMP_R_2S[30];        /*一分钟内30个温度电阻采样值*/
	float     TEMP_T;               /*温度采样值*/
	float     temp_correction_value;/*温度订正值*/
	double 		temp_flow;						/*恒流电流值*/
	int16_t		TEMP_T_10;						/*采样温度的10倍*/
	int16_t   TEMP_T_2S[30];          /*一分钟内30个温度采样值*/
	//int       TEMP_1M_QC[30];    /*1分钟内30个采样点质控数组*/
	int16_t   TEMP_1M;              /*一分钟内平均温度*/
	//int16_t   TEMP_1H[60];          /*1小时内60个分钟平均温度采样值*/
	//int       TEMP_1H_QC[60];    /*1小时内60个平均温度质控质控数组*/
	int16_t   TEMP_1H_MIN;          /*1小时内最小温度*/
	uint32_t  TEMP_1H_MIN_TIME;  /*1小时内最小温度出现的时间：分秒*/
	int16_t   TEMP_1H_MAX;          /*1小时内最大温度*/
	uint32_t  TEMP_1H_MAX_TIME;  /*1小时内最大温度出现的时间：分秒*/
	//int16_t   TEMP_1D_MIN;          /*1天内最小温度*/
	//uint32_t  TEMP_1D_MIN_TIME;  /*1天内最小温度出现的时间：时分秒*/
	//int16_t   TEMP_1D_MAX;          /*1天内最大温度*/
	//uint32_t  TEMP_1D_MAX_TIME;  /*1天内最大温度出现的时间：时分秒*/
}TEMP;
extern TEMP temp;


typedef struct  //雨量数据
{
	uint32_t RAIN_1M;               /*分钟雨量累计值*/
	int      RAIN_M[60];              /*一小时内60个分钟雨量*/
	uint32_t RAIN_1H;               /*小时雨量累计值*/
	//int      RAIN_H[24];            /*最近24个小时雨量*/
	uint32_t RAIN_1D;               /*1天雨量累计值*/
	//int      RAIN_D[31];            /*最近31天的雨量雨量*/
}RAIN;
extern RAIN rain;


typedef struct /*设备状态*/
{
	char  station[5];              /*台站号*/
//	TEMP      temp;                 /*温度数据*/
//	RAIN      rain;                 /*雨量数据*/
	uint32_t  board_AD;             /*电路板测量AD*/
	float     board_voltage;        /*板电压*/
	//float     board_voltage_1H[60]; /*1小时测的60个电路板电压值*/
	uint32_t  board_voltage_10;     /*测量的电路板电压的10倍*/
	//uint32_t  AD7705_CH2_AD;           /*AD7705通道2不清楚测什么，先准备着*/
	//float     AD7705_CH2_V;         /*AD7705通道2测量结果转换成的电压*/
	uint8_t   data_time;            /*轮询机制  隔几分钟发一条数据 0,1：自动发送，data_time>1:就是隔data_time分钟发一条数据*/
	uint8_t 			usart_sampling;				/*串口远程查看采样值*/
}Device_State;
extern Device_State device;


/*函数声明*/
int OutputHourDataFill(char *Data,const struct tm *Datetime);
uint8_t OutputHour1DataFill(const struct tm *Datetime);
int16_t AverageAll(const int16_t *DataBuffer,uint32_t Count);
int16_t AverageWithoutMaxMin(const int16_t *DataBuffer,uint32_t Count);
float TemperatureCorrection(float PreTemperature,float *CorrectionTable,uint32_t TableCount);
float CalculateTemperature(float resistance);
uint32_t CalculateSector(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);

#ifdef __cplusplus
}
#endif
#endif
