/* Includes ------------------------------------------------------------------*/
#include "sensor.h"
#include "usart_module.h"

/*结构体变量声明声明*/
TEMP temp;
RAIN rain;
Device_State device;

char  data_buf[512];

static const int MONTH_DAY[13]= {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/******************************************************************************************
 *传感器测量计算部分
 ********************************************************************************************/

/*Private function prototypes 私有函数原型*/
static int32_t search_for_termperature_interval_index(float Temperature,int32_t *Bottom,int32_t *Top);
/* Temperature */
//小气候PT100温度计算表
static const float RTD_TAB_PT100[15]=
{
	76.33,80.31,84.27,88.22,92.16,96.09,100.00,103.90,       /*-60℃~+10℃*/		
	107.79,111.67,115.54,119.40,123.24,127.07,130.89	       /*+20℃~+80℃*/
};

//温度值对照表
static const float TEMP_COMPARISON_TABLE[15]=
{
  -60,-50,-40,-30,-20,-10,0,10,       /* -60C - +10C */		
  20,30,40,50,60,70,80	              /* +20C - +80C */
};

//计算PT100温度值
float CalculateTemperature(float resistance)
{
	float fTem;
	float fL;
	float fH;
	int iTem;
	uint8_t cbottom,ctop;
	uint8_t i;
        
	if(resistance<RTD_TAB_PT100[0])
	{
		return -60.0;
	}
	if(resistance>RTD_TAB_PT100[14])
	{
		return 80.0;
	}
	cbottom=0;
	ctop=14;
	for(i=7;(ctop-cbottom)!=1;)
	{
		if(resistance<RTD_TAB_PT100[i])
		{
			ctop=i;
			i=(ctop+cbottom)/2;
		}
		else if(resistance>RTD_TAB_PT100[i])
		{
			cbottom=i;
			i=(ctop+cbottom)/2;
		}
		else
		{
			iTem=(int)i*10-60;
			fTem=(float)iTem;
			return fTem;
		}
	}
	iTem=(int)i*10-60;
	fL=RTD_TAB_PT100[cbottom];
	fH=RTD_TAB_PT100[ctop];
	fTem=(((resistance-fL)*10)/(fH-fL))+iTem ;
	return fTem;
}



/********************************************************************************************
 *订正测量的数据
 ********************************************************************************************/

/*根据订正表订正温度值*/
/**
  * @brief  search for termperature value interval's index.
  * @param  temperature: temperature value
  * @param  bottom: bottom index of the range
  * @param  top: top index of the range
  * @retval 0:within range;-1:out of range
  */
static int32_t search_for_termperature_interval_index(float Temperature,int32_t *Bottom,int32_t *Top)
{
  int32_t bottom=0,top=0,index=0;
  
  bottom=0;
  top=LENGTH_OF(TEMP_COMPARISON_TABLE)-1;
  
  /* out of range */
  if((Temperature < TEMP_COMPARISON_TABLE[bottom]) || (Temperature > TEMP_COMPARISON_TABLE[top]))
  {
    return -1;
  }
  
  /* binary search */
  for(index=(bottom+top)/2;(top-bottom)!=1;)
  {
    if(Temperature < TEMP_COMPARISON_TABLE[index])
    {
      top=index;
      index=(bottom+top)/2;
    }
    else if(Temperature > TEMP_COMPARISON_TABLE[index])
    {
      bottom=index;
      index=(bottom+top)/2;
    }
    else
    {
      bottom=index;
      top=index;
      break;
    }
  }
  
  /* set the index */
  *Bottom=bottom;
  *Top=top;
  
  return 0;
}

/**
  * @brief  Temperature Correction.
  * @param  PreTemperature: temperature value before correction
  * @param  CorrectionTable: pointer to corretion table
  * @param  TableCount: count of corretion table 
  * @retval temperature after correction.
  */
float TemperatureCorrection(float PreTemperature,float *CorrectionTable,uint32_t TableCount)
{
  int32_t bottom=0,top=0;
  float correction=0,vL=0,vH=0;
  
  /* search for value interval's index */
  if(search_for_termperature_interval_index(PreTemperature,&bottom,&top)<0)  /* out of range */
  {
    return PreTemperature;
  }
  
  if(top > (TableCount-1))
  {
    return PreTemperature;
  }
  
  if(bottom == top)
  {
    correction=CorrectionTable[top];
    
    return (PreTemperature+correction);
  }
  
  /* Calculate correction value */
  vL=TEMP_COMPARISON_TABLE[bottom];
  vH=TEMP_COMPARISON_TABLE[top];
  correction=(PreTemperature-vL)/(vH-vL);  /* ratio */
  vL=CorrectionTable[bottom];
  vH=CorrectionTable[top];
  correction=correction*(vH-vL) + vL;      /* correction value */
  
  return (PreTemperature+correction);
}

/*********************************************************************
 * 计算平均值，去除最大值和最小值
 */
/**
  * @brief  calculating average without max and min
  * @param  
  * @retval None
  */
int16_t AverageWithoutMaxMin(const int16_t *DataBuffer,uint32_t Count)
{
  float sum_aver=0;
  int16_t average=0;
  int16_t max=DataBuffer[0],min=DataBuffer[0];
  uint32_t i=0,valid_count=Count;
  
  if(Count==0)
  {
    return DataBuffer[0];
  }
  
  for(i=0;i<Count;i++)
  {
    if(DataBuffer[i]>max)
    {
      max=DataBuffer[i];
    }
    if(DataBuffer[i]<min)
    {
      min=DataBuffer[i];
    }
    
    
    sum_aver+=DataBuffer[i];
  }
  

  if(valid_count>2)
  {
    sum_aver-=min;
    sum_aver-=max;  //去除最大值和最小值
    valid_count-=2;
  }
  
  sum_aver/=valid_count;
  
  /* round-off */
  if(sum_aver>=0)
  {
    average=(int16_t)(sum_aver+0.5);
  }
  else
  {
    average=(int16_t)(sum_aver-0.5);
  }

  
  return average;
}


/*********************************************************************
 * 计算平均值，包含所有值
 */
/**
  * @brief  calculating average
  * @param  
  * @retval None
  */
int16_t AverageAll(const int16_t *DataBuffer,uint32_t Count)
{
  float sum_aver=0;
  int16_t average=0;
  uint32_t i=0;
  
  if(Count<2)
  {
    return DataBuffer[0];
  }
  
  for(i=0;i<Count;i++)
  {
    sum_aver+=DataBuffer[i];
  }
  
  
  sum_aver/=Count;
  
  /* round-off */
  if(sum_aver>=0)
  {
    average=(int16_t)(sum_aver+0.5);
  }
  else
  {
    average=(int16_t)(sum_aver-0.5);
  }

  
  return average;
}



/*填充小时数据*/
int OutputHourDataFill(char *Data,const struct tm *Datetime)
{
	DATA_1HOUR *output_data = (DATA_1HOUR *)Data;
	int count = 0;
	
	if(Data==NULL)
  {
    return 0;
  }
	
	count =  snprintf(output_data->start_flag,sizeof(output_data->start_flag) + 1,"[D");
	count += snprintf(output_data->station_flag,sizeof(output_data->station_flag)+ 1,"%c%c%c%c%c",device.station[0], device.station[1], device.station[2], device.station[3], device.station[4]);
	if(Datetime)
	{
		count += snprintf(output_data->time_flag,sizeof(output_data->time_flag) + 1,"%04u%02u%02u%02u%02u%02u",
			                Datetime->tm_year,Datetime->tm_mon,Datetime->tm_mday,
                      Datetime->tm_hour,Datetime->tm_min,Datetime->tm_sec);
	}
	else
	{
		count += snprintf(output_data->time_flag,sizeof(output_data->time_flag) + 1,"%04u%02u%02u%02u%02u%02u",
             2000,0,0,0,0,0);
	}
	//分钟温度
	if(temp.TEMP_1M < 0)
	{
			count +=  snprintf(output_data->data_1h.temperature,sizeof(output_data->data_1h.temperature) + 1,"-%03u", abs(temp.TEMP_1M));
	}
	else if(temp.TEMP_1M >= 0)
	{
			count +=  snprintf(output_data->data_1h.temperature,sizeof(output_data->data_1h.temperature) + 1,"+%03u", abs(temp.TEMP_1M));
	}
	//小时最大温度
	if(temp.TEMP_1H_MAX < 0)
	{
			count +=  snprintf(output_data->data_1h.temperature_max,sizeof(output_data->data_1h.temperature_max) + 1,"-%03u", abs(temp.TEMP_1H_MAX));
	}
	else if(temp.TEMP_1H_MAX >= 0)
	{
			count +=  snprintf(output_data->data_1h.temperature_max,sizeof(output_data->data_1h.temperature_max) + 1,"+%03u", abs(temp.TEMP_1H_MAX));
	}
	count +=  snprintf(output_data->data_1h.temperature_max_time ,sizeof(output_data->data_1h.temperature_max_time) + 1,"%04u", temp.TEMP_1H_MAX_TIME); 
		//小时最低温度
	if(temp.TEMP_1H_MIN < 0)
	{
			count +=  snprintf(output_data->data_1h.temperature_min,sizeof(output_data->data_1h.temperature_min) + 1,"-%03u", abs(temp.TEMP_1H_MIN));
	}
	else if(temp.TEMP_1H_MIN >= 0)
	{
			count +=  snprintf(output_data->data_1h.temperature_min,sizeof(output_data->data_1h.temperature_min) + 1,"+%03u", abs(temp.TEMP_1H_MIN));
	}
	count +=  snprintf(output_data->data_1h.temperature_min_time ,sizeof(output_data->data_1h.temperature_min_time) + 1,"%04u", temp.TEMP_1H_MIN_TIME); 
	count +=  snprintf(output_data->data_1h.dummy_3 ,sizeof(output_data->data_1h.dummy_3) + 1,"00000000000000000000000000000000000000000000000000"); 
	count +=  snprintf(output_data->data_1h.rain_1  ,sizeof(output_data->data_1h.rain_1) + 1, "%02u", rain.RAIN_M[1]); 
	count +=  snprintf(output_data->data_1h.rain_2  ,sizeof(output_data->data_1h.rain_2) + 1, "%02u", rain.RAIN_M[2]); 
	count +=  snprintf(output_data->data_1h.rain_3  ,sizeof(output_data->data_1h.rain_3) + 1, "%02u", rain.RAIN_M[3]); 
	count +=  snprintf(output_data->data_1h.rain_4  ,sizeof(output_data->data_1h.rain_4) + 1, "%02u", rain.RAIN_M[4]); 
	count +=  snprintf(output_data->data_1h.rain_5  ,sizeof(output_data->data_1h.rain_5) + 1, "%02u", rain.RAIN_M[5]); 
	count +=  snprintf(output_data->data_1h.rain_6  ,sizeof(output_data->data_1h.rain_6) + 1, "%02u", rain.RAIN_M[6]); 
	count +=  snprintf(output_data->data_1h.rain_7  ,sizeof(output_data->data_1h.rain_7) + 1, "%02u", rain.RAIN_M[7]); 
	count +=  snprintf(output_data->data_1h.rain_8  ,sizeof(output_data->data_1h.rain_8) + 1, "%02u", rain.RAIN_M[8]); 
	count +=  snprintf(output_data->data_1h.rain_9  ,sizeof(output_data->data_1h.rain_9) + 1, "%02u", rain.RAIN_M[9]); 
	count +=  snprintf(output_data->data_1h.rain_10  ,sizeof(output_data->data_1h.rain_10) + 1, "%02u", rain.RAIN_M[10]); 
	count +=  snprintf(output_data->data_1h.rain_11  ,sizeof(output_data->data_1h.rain_11) + 1, "%02u", rain.RAIN_M[11]); 
	count +=  snprintf(output_data->data_1h.rain_12  ,sizeof(output_data->data_1h.rain_12) + 1, "%02u", rain.RAIN_M[12]); 
	count +=  snprintf(output_data->data_1h.rain_13  ,sizeof(output_data->data_1h.rain_13) + 1, "%02u", rain.RAIN_M[13]); 
	count +=  snprintf(output_data->data_1h.rain_14  ,sizeof(output_data->data_1h.rain_14) + 1, "%02u", rain.RAIN_M[14]); 
	count +=  snprintf(output_data->data_1h.rain_15  ,sizeof(output_data->data_1h.rain_15) + 1, "%02u", rain.RAIN_M[15]); 
	count +=  snprintf(output_data->data_1h.rain_16  ,sizeof(output_data->data_1h.rain_16) + 1, "%02u", rain.RAIN_M[16]); 
	count +=  snprintf(output_data->data_1h.rain_17  ,sizeof(output_data->data_1h.rain_17) + 1, "%02u", rain.RAIN_M[17]); 
	count +=  snprintf(output_data->data_1h.rain_18  ,sizeof(output_data->data_1h.rain_18) + 1, "%02u", rain.RAIN_M[18]); 
	count +=  snprintf(output_data->data_1h.rain_19  ,sizeof(output_data->data_1h.rain_19) + 1, "%02u", rain.RAIN_M[19]); 
	count +=  snprintf(output_data->data_1h.rain_20  ,sizeof(output_data->data_1h.rain_20) + 1, "%02u", rain.RAIN_M[20]); 
	count +=  snprintf(output_data->data_1h.rain_21  ,sizeof(output_data->data_1h.rain_21) + 1, "%02u", rain.RAIN_M[21]); 
	count +=  snprintf(output_data->data_1h.rain_22  ,sizeof(output_data->data_1h.rain_22) + 1, "%02u", rain.RAIN_M[22]); 
	count +=  snprintf(output_data->data_1h.rain_23  ,sizeof(output_data->data_1h.rain_23) + 1, "%02u", rain.RAIN_M[23]); 
	count +=  snprintf(output_data->data_1h.rain_24  ,sizeof(output_data->data_1h.rain_24) + 1, "%02u", rain.RAIN_M[24]); 
	count +=  snprintf(output_data->data_1h.rain_25  ,sizeof(output_data->data_1h.rain_25) + 1, "%02u", rain.RAIN_M[25]); 
	count +=  snprintf(output_data->data_1h.rain_26  ,sizeof(output_data->data_1h.rain_26) + 1, "%02u", rain.RAIN_M[26]); 
	count +=  snprintf(output_data->data_1h.rain_27  ,sizeof(output_data->data_1h.rain_27) + 1, "%02u", rain.RAIN_M[27]); 
	count +=  snprintf(output_data->data_1h.rain_28  ,sizeof(output_data->data_1h.rain_28) + 1, "%02u", rain.RAIN_M[28]); 
	count +=  snprintf(output_data->data_1h.rain_29  ,sizeof(output_data->data_1h.rain_29) + 1, "%02u", rain.RAIN_M[29]); 
	count +=  snprintf(output_data->data_1h.rain_30  ,sizeof(output_data->data_1h.rain_30) + 1, "%02u", rain.RAIN_M[30]); 
	count +=  snprintf(output_data->data_1h.rain_31  ,sizeof(output_data->data_1h.rain_31) + 1, "%02u", rain.RAIN_M[31]); 
	count +=  snprintf(output_data->data_1h.rain_32  ,sizeof(output_data->data_1h.rain_32) + 1, "%02u", rain.RAIN_M[32]); 
	count +=  snprintf(output_data->data_1h.rain_33  ,sizeof(output_data->data_1h.rain_33) + 1, "%02u", rain.RAIN_M[33]); 
	count +=  snprintf(output_data->data_1h.rain_34  ,sizeof(output_data->data_1h.rain_34) + 1, "%02u", rain.RAIN_M[34]); 
	count +=  snprintf(output_data->data_1h.rain_35  ,sizeof(output_data->data_1h.rain_35) + 1, "%02u", rain.RAIN_M[35]); 
	count +=  snprintf(output_data->data_1h.rain_36  ,sizeof(output_data->data_1h.rain_36) + 1, "%02u", rain.RAIN_M[36]); 
	count +=  snprintf(output_data->data_1h.rain_37  ,sizeof(output_data->data_1h.rain_37) + 1, "%02u", rain.RAIN_M[37]); 
	count +=  snprintf(output_data->data_1h.rain_38  ,sizeof(output_data->data_1h.rain_38) + 1, "%02u", rain.RAIN_M[38]); 
	count +=  snprintf(output_data->data_1h.rain_39  ,sizeof(output_data->data_1h.rain_39) + 1, "%02u", rain.RAIN_M[39]); 
	count +=  snprintf(output_data->data_1h.rain_40  ,sizeof(output_data->data_1h.rain_40) + 1, "%02u", rain.RAIN_M[40]); 
	count +=  snprintf(output_data->data_1h.rain_41  ,sizeof(output_data->data_1h.rain_41) + 1, "%02u", rain.RAIN_M[41]); 
	count +=  snprintf(output_data->data_1h.rain_42  ,sizeof(output_data->data_1h.rain_42) + 1, "%02u", rain.RAIN_M[42]); 
	count +=  snprintf(output_data->data_1h.rain_43  ,sizeof(output_data->data_1h.rain_43) + 1, "%02u", rain.RAIN_M[43]); 
	count +=  snprintf(output_data->data_1h.rain_44  ,sizeof(output_data->data_1h.rain_44) + 1, "%02u", rain.RAIN_M[44]); 
	count +=  snprintf(output_data->data_1h.rain_45  ,sizeof(output_data->data_1h.rain_45) + 1, "%02u", rain.RAIN_M[45]); 
	count +=  snprintf(output_data->data_1h.rain_46  ,sizeof(output_data->data_1h.rain_46) + 1, "%02u", rain.RAIN_M[46]); 
	count +=  snprintf(output_data->data_1h.rain_47  ,sizeof(output_data->data_1h.rain_47) + 1, "%02u", rain.RAIN_M[47]); 
	count +=  snprintf(output_data->data_1h.rain_48  ,sizeof(output_data->data_1h.rain_48) + 1, "%02u", rain.RAIN_M[48]); 
	count +=  snprintf(output_data->data_1h.rain_49  ,sizeof(output_data->data_1h.rain_49) + 1, "%02u", rain.RAIN_M[49]); 
	count +=  snprintf(output_data->data_1h.rain_50  ,sizeof(output_data->data_1h.rain_50) + 1, "%02u", rain.RAIN_M[50]); 
	count +=  snprintf(output_data->data_1h.rain_51  ,sizeof(output_data->data_1h.rain_51) + 1, "%02u", rain.RAIN_M[51]); 
	count +=  snprintf(output_data->data_1h.rain_52  ,sizeof(output_data->data_1h.rain_52) + 1, "%02u", rain.RAIN_M[52]); 
	count +=  snprintf(output_data->data_1h.rain_53  ,sizeof(output_data->data_1h.rain_53) + 1, "%02u", rain.RAIN_M[53]); 
	count +=  snprintf(output_data->data_1h.rain_54  ,sizeof(output_data->data_1h.rain_54) + 1, "%02u", rain.RAIN_M[54]); 
	count +=  snprintf(output_data->data_1h.rain_55  ,sizeof(output_data->data_1h.rain_55) + 1, "%02u", rain.RAIN_M[55]); 
	count +=  snprintf(output_data->data_1h.rain_56  ,sizeof(output_data->data_1h.rain_56) + 1, "%02u", rain.RAIN_M[56]); 
	count +=  snprintf(output_data->data_1h.rain_57  ,sizeof(output_data->data_1h.rain_57) + 1, "%02u", rain.RAIN_M[57]); 
	count +=  snprintf(output_data->data_1h.rain_58  ,sizeof(output_data->data_1h.rain_58) + 1, "%02u", rain.RAIN_M[58]); 
	count +=  snprintf(output_data->data_1h.rain_59  ,sizeof(output_data->data_1h.rain_59) + 1, "%02u", rain.RAIN_M[59]); 
	count +=  snprintf(output_data->data_1h.rain_0  ,sizeof(output_data->data_1h.rain_0) + 1, "%02u", rain.RAIN_M[0]); 
	
	count +=  snprintf(output_data->data_1h.dummy_4 ,sizeof(output_data->data_1h.dummy_4) + 1,"0000000000000000000000000000000000000");
	count +=  snprintf(output_data->data_1h.board_v ,sizeof(output_data->data_1h.board_v) + 1,"%03u",(uint8_t)(device.board_voltage *10) ); 
	count +=  snprintf(output_data->end_flag,sizeof(output_data->end_flag),"]");
	
	DEBUG("count=%d\r\n", count);
	return (count);
}



/*说明：计算出一年的当前月日时分所对应的扇区号。譬如：1月1日0时0分=0（扇区） 1月1日0时1分=1（扇区） 1月1日0时2分=0（扇区）
 *
 *
 *
 *
 *
 */
uint32_t CalculateSector(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
		//扇区号
		uint32_t sectors = 0;
		uint8_t yue = 0, ri = 0, shi = 0, fen = 0;
		if(date)
		{
				
				yue  = date->Month ;
				ri   = date->Date ;
		}
		if(time)
		{
				shi = time->Hours ;
				fen = time->Minutes ;
		}
		if(yue> 12)		yue = 1;
		if(ri> 31)		ri  = 1;
		if(shi> 23)		shi = 0;
		if(fen> 59)		fen = 0;
		
		DEBUG("%d-%d %d:%d\r\n", yue, ri, shi, fen);
		DEBUG("sectors=%d\r\n", sectors);
		//当月的前几个月的扇区号 （譬如1月前面没有月，2月前面的1月，3月的前面1、2月等等）
		yue -= 1;
		DEBUG("yue=%d\r\n", yue);
		while(yue--)
		{
				DEBUG("当月的天数=%d\r\n", MONTH_DAY[yue+1]);
				sectors += MONTH_DAY[yue+1] * 24 * 60 ;	//一天24小时，一小时60分钟
				DEBUG("YUE\r\n");
				DEBUG("sectors=%d\r\n", sectors);
		}
		
		//当月的前几天的扇区号
		ri -= 1;
		DEBUG("ri=%d\r\n", ri);
		sectors += ri * 24 * 60;									//一天24小时，一小时60分钟
		DEBUG("sectors=%d\r\n", sectors);
		
		//当天的前几个小时的扇区号
		//shi -= 1;
		DEBUG("shi=%d\r\n", shi);
		sectors += shi * 60;											//一小时60分钟
		DEBUG("sectors=%d\r\n", sectors);
		
		//当前小时的分钟数的扇区号
		DEBUG("fen=%d\r\n", fen);
		sectors += (fen + 1);
		DEBUG("sectors=%d\r\n", sectors);
		return sectors;
}	




