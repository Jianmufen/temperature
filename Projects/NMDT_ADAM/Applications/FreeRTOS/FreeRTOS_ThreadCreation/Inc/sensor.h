#ifndef __SENSOR_H
#define __SENSOR_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l1xx_hal.h"
#include "main.h"	 
#include "time.h"		 
	 

	 
//#define QC_R 0  //������ȷ
//#define QC_L 1  //����ȱ��	 
//#define TEMP_CORRECTION_VALUE    (0)  /*PT100���¶ȶ���ֵ*/
	 
//#define st(x)      do { x } while (__LINE__ == -1)
   
/* length of array */
#define LENGTH_OF(x)   (sizeof(x)/sizeof(*(x)))

extern char  data_buf[512];
/*Сʱ���ݸ�ʽ*/	 
typedef struct
{
	char temperature[4];            /*�����¶� "+201=+20.1   -201=-20.1"*/
	char temperature_max[4];        /*1Сʱ����¶� "+201=+20.1   -201=-20.1"*/
	char temperature_max_time[4];   /*1Сʱ����¶�ʱ��ʱ�� "1211"*/
	char temperature_min[4];        /*1Сʱ����¶� "+201=+20.1   -201=-20.1"*/
	char temperature_min_time[4];   /*1Сʱ����¶�ʱ��ʱ�� "1211"*/
	char dummy_3[50];               /*����ʪ����Сʪ��ʱ��硰00000000009999999999999999999999999999999999999999��*/
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
	char dummy_4[37];               /*������ѹ�����ѹ�Ƿ������ѹʱ�������ܼ�������ܼ���ʱ�֡�0000000000000000000000000000000000000��*/
	char board_v[3];                /*�����Դ��ѹ��121=12.1V��*/
}DATA_1H;

	 
typedef struct   //ͨѶ֡���ݰ�ͷ   �������ݺ���ʷ����
{
  char start_flag[2];   //��ʼ��   "[D"
	char station_flag[5];  //վ�ţ���00001��
  char time_flag[14];   //ʱ�䣺������ʱ��20171007150000
  DATA_1H data_1h;
  char end_flag[2];   //������  "]"
}DATA_1HOUR;	
//extern DATA_1HOUR data_hour;

typedef struct  //�¶�����
{
	uint32_t  TEMP_AD;              /*�¶�AD����ֵ*/
	//int       TEMP_AD_2S[30];       /*һ������30���¶�AD����ֵ*/
	float     TEMP_V;               /*�����ĵ�ѹֵ*/
	//float     TEMP_V_2S[30];        /*30����ѹֵ*/ 
  float     TEMP_R;               /*�¶ȵ������ֵ*/
	//float     TEMP_R_2S[30];        /*һ������30���¶ȵ������ֵ*/
	float     TEMP_T;               /*�¶Ȳ���ֵ*/
	float     temp_correction_value;/*�¶ȶ���ֵ*/
	double 		temp_flow;						/*��������ֵ*/
	int16_t		TEMP_T_10;						/*�����¶ȵ�10��*/
	int16_t   TEMP_T_2S[30];          /*һ������30���¶Ȳ���ֵ*/
	//int       TEMP_1M_QC[30];    /*1������30���������ʿ�����*/
	int16_t   TEMP_1M;              /*һ������ƽ���¶�*/
	//int16_t   TEMP_1H[60];          /*1Сʱ��60������ƽ���¶Ȳ���ֵ*/
	//int       TEMP_1H_QC[60];    /*1Сʱ��60��ƽ���¶��ʿ��ʿ�����*/
	int16_t   TEMP_1H_MIN;          /*1Сʱ����С�¶�*/
	uint32_t  TEMP_1H_MIN_TIME;  /*1Сʱ����С�¶ȳ��ֵ�ʱ�䣺����*/
	int16_t   TEMP_1H_MAX;          /*1Сʱ������¶�*/
	uint32_t  TEMP_1H_MAX_TIME;  /*1Сʱ������¶ȳ��ֵ�ʱ�䣺����*/
	//int16_t   TEMP_1D_MIN;          /*1������С�¶�*/
	//uint32_t  TEMP_1D_MIN_TIME;  /*1������С�¶ȳ��ֵ�ʱ�䣺ʱ����*/
	//int16_t   TEMP_1D_MAX;          /*1��������¶�*/
	//uint32_t  TEMP_1D_MAX_TIME;  /*1��������¶ȳ��ֵ�ʱ�䣺ʱ����*/
}TEMP;
extern TEMP temp;


typedef struct  //��������
{
	uint32_t RAIN_1M;               /*���������ۼ�ֵ*/
	int      RAIN_M[60];              /*һСʱ��60����������*/
	uint32_t RAIN_1H;               /*Сʱ�����ۼ�ֵ*/
	//int      RAIN_H[24];            /*���24��Сʱ����*/
	uint32_t RAIN_1D;               /*1�������ۼ�ֵ*/
	//int      RAIN_D[31];            /*���31�����������*/
}RAIN;
extern RAIN rain;


typedef struct /*�豸״̬*/
{
	char  station[5];              /*̨վ��*/
//	TEMP      temp;                 /*�¶�����*/
//	RAIN      rain;                 /*��������*/
	uint32_t  board_AD;             /*��·�����AD*/
	float     board_voltage;        /*���ѹ*/
	//float     board_voltage_1H[60]; /*1Сʱ���60����·���ѹֵ*/
	uint32_t  board_voltage_10;     /*�����ĵ�·���ѹ��10��*/
	//uint32_t  AD7705_CH2_AD;           /*AD7705ͨ��2�������ʲô����׼����*/
	//float     AD7705_CH2_V;         /*AD7705ͨ��2�������ת���ɵĵ�ѹ*/
	uint8_t   data_time;            /*��ѯ����  �������ӷ�һ������ 0,1���Զ����ͣ�data_time>1:���Ǹ�data_time���ӷ�һ������*/
	uint8_t 			usart_sampling;				/*����Զ�̲鿴����ֵ*/
}Device_State;
extern Device_State device;


/*��������*/
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
