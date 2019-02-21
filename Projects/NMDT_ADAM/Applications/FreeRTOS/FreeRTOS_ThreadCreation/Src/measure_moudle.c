/* Includes ------------------------------------------------------------------*/
#include "measure_moudle.h"
#include "cmsis_os.h"

#include "usart_module.h"
#include "sys_time_module.h"
#include "stm32_adafruit_sd.h"
#include "eeprom.h"

#define measureSTACK_SIZE   configMINIMAL_STACK_SIZE
#define measurePRIORITY     osPriorityNormal

  /* RTC Time*/
static RTC_TimeTypeDef Measure_Time;
static RTC_DateTypeDef Measure_Date; 
struct tm datetime;	

static int first_power = 0;
static uint8_t AD_ERROR = 0;							//1:测量的AD值超过正常范围（393-740）。0：测量的AD值正常
static uint8_t SD_STATUS_OK = 0;					//0：SD卡初始化成功。1：SD卡初始化失败
// AD转换结果值
__IO uint16_t ADC_ConvertedValue;
/* os relative */
static osThreadId MeasureThreadHandle;
static osSemaphoreId semaphore;
static osMutexId mutex;

/* Private function prototypes -----------------------------------------------*/
static void Measure_Thread(void const *argument);
static void TemperaturesMeasure(const RTC_DateTypeDef *date,const RTC_TimeTypeDef *time);
static void OnBoardMeasure(void);

/**
  * @brief  Init Measurement Module. 
  * @retval 0:success;-1:failed
  */
int32_t init_measure_module(void)
{
	uint32_t i = 0;
//	DEBUG("sizeof(u8)=%d\r\n", sizeof(uint8_t));
//	DEBUG("sizeof(u16)=%d\r\n", sizeof(uint16_t));
//	DEBUG("sizeof(u32)=%d\r\n", sizeof(uint32_t));
//	DEBUG("sizeof(dl)=%d\r\n", sizeof(double));
//	DEBUG("sizeof(f)=%d\r\n", sizeof(float));
	uint8_t status = 0;
	device.usart_sampling = 0;			/**/
	//SD卡初始化
	status = BSP_SD_Init();
	if(status == MSD_OK)
	{
			DEBUG("SD Init OK\r\n");
			SD_STATUS_OK = 0;
	}
	else if(status == MSD_ERROR)
	{
			DEBUG("SD Init Error\r\n");
			SD_STATUS_OK = 1;
			if(BSP_SD_Init() == MSD_OK)
			{
					SD_STATUS_OK = 0;
					DEBUG("SD Init Second OK\r\n");
			}
	}

//	//读取SD卡的信息
//	SD_CardInfo *p = malloc(sizeof(SD_CardInfo));
//	BSP_SD_GetCardInfo(p);
//	DEBUG("CardCapacity=%d	CardBlockSize=%d	RdBlockLen=%d\r\n", p->CardCapacity , p->CardBlockSize, p->Csd.RdBlockLen );
//	free(p);
	
	
	/************************************************SD卡存储记忆值***********************************************************/
/*		//读台站号、温度订正值、轮询间隔
	//读台站号
	memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
	if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)device.station, 600000 * 512, 6, 1))
	{
			status = strtoul(data_buf, NULL, 10);
			DEBUG("read device.station=%c%c%c%c%c\r\n", device.station[0], device.station[1], device.station[2], device.station[3], device.station[4]);
			if((device.station[0] == '0') && (device.station[1] == '0') && (device.station[2] == '0') && (device.station[3] == '0') && (device.station[4] == '0'))
			{
					device.station[0] = '0';
					device.station[1] = '0';
					device.station[2] = '0';
				  device.station[3] = '0';
					device.station[4] = '1';
					device.station[5] = '\0';		//如果读出来的是0，使用默认的00001，然后再将00001写到SD卡里
					//snprintf(data_buf, sizeof(data_buf), "%05u", device.station);
					if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)device.station, 600000 * 512, 512, 1))			//将台站号写到第600000个扇区号内
						{
								DEBUG("first set station ok\r\n");
						}
						else
						{
								DEBUG("first set station error\r\n");
								BSP_SD_Init();
						}
			}
			else
			{
					DEBUG("first read device.station=%c%c%c%c%c\r\n",device.station[0], device.station[1], device.station[2], device.station[3], device.station[4]);
			}
	}
	else
	{
			//读取失败使用默认台站号
			device.station[0] = '0';
			device.station[1] = '0';
			device.station[2] = '0';
  		device.station[3] = '0';
			device.station[4] = '1';
			device.station[5] = '\0';		//如果读出来的是0，使用默认的00001，然后再将00001写到SD卡里
			DEBUG("read device.station error=%s\r\n", device.station);
			BSP_SD_Init();
	}
	//读轮询间隔
	memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
	if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)data_buf, 600020 * 512, 512, 1))
	{
			device.data_time = strtoul(data_buf, NULL, 10);
			DEBUG("read device.data_time=%d\r\n", device.data_time);
	}
	else
	{
			device.data_time = 1;
			DEBUG("first read device.data_time error\r\n");
			BSP_SD_Init();
	}
	//读温度订正值
	memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
	if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)data_buf, 600010 * 512, 512, 1))
	{
			temp.temp_correction_value = atof(data_buf) / 10;
			DEBUG("read temp.temp_correction_value=%f\r\n", temp.temp_correction_value);
	}
	else
	{
			temp.temp_correction_value = 0;
			DEBUG("first temp.temp_correction_value error\r\n");
			BSP_SD_Init();
	}
	//读取恒流电流值
	memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
	if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)data_buf, 600030 * 512, 512, 1))
	{
			temp.temp_flow = atof(data_buf);
			if(temp.temp_flow == 0)
			{
					temp.temp_flow	= 0.0002;
			}
			DEBUG("read temp.temp_flow=%.9f\r\n", temp.temp_flow);
	}
	else
	{
			temp.temp_flow	= 0.0002;
			DEBUG("first temp.temp_flow error\r\n");
			BSP_SD_Init();
	}
*/	
	
	/*********************************************EEPROM存储记忆值*****************************************************************/
	//存储台站号
	memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
	if(data_eeprom_read(STATION_ADDR, (uint8_t *)data_buf, 8) == HAL_OK)
	{
		DEBUG("read station1=%s\r\n", data_buf);
		DEBUG("read station[0]=%d\r\n", data_buf[0]);
		DEBUG("read station[1]=%d\r\n", data_buf[1]);
		DEBUG("read station[2]=%d\r\n", data_buf[2]);
		DEBUG("read station[3]=%d\r\n", data_buf[3]);
		DEBUG("read station[4]=%d\r\n", data_buf[4]);
		if((data_buf[0] == '\0') && (data_buf[1] == '\0') && (data_buf[2] == '\0') && (data_buf[3] == '\0') && (data_buf[4] == '\0'))
		{
				device.station[0] = '0';	
				device.station[1] = '0';
				device.station[2] = '0';
				device.station[3] = '0';
				device.station[4] = '1';
				//device.station[5] = '\0';
				DEBUG("难道在这里了\r\n");
		}
		else
		{
				device.station[0] = data_buf[0];
				device.station[1] = data_buf[1];
				device.station[2] = data_buf[2];
				device.station[3] = data_buf[3];
				device.station[4] = data_buf[4];
				//device.station[5] = '\0';
				DEBUG("在这里了\r\n");
		}
	}
	else
	{
				device.station[0] = '0';	
				device.station[1] = '0';
				device.station[2] = '0';
				device.station[3] = '0';
				device.station[4] = '1';
				//device.station[5] = '\0';
	}
	
	//存储温度订正值
	if(data_eeprom_read(TEMP_CORRECT_ADDR, (uint8_t *)&i , 4) == HAL_OK)
	{
			DEBUG("读出的整形温度订正值=%d\r\n", i);
			if(i < 100)
			{
					temp.temp_correction_value = ((float)i) / 10;
			}
			else if((i > 100) && (i < 200))
			{
					temp.temp_correction_value = 0 -  ((float)(i - 100)) / 10;
			}
			else if((i > 200) && (i < 10000))
			{
					temp.temp_correction_value = ((float)i / 10000);
					DEBUG("自己定义的订正值\r\n");
			}
			else if(i > 10000)
			{
					temp.temp_correction_value = 0 - ((float)(i - 100000) / 10000);
			}
			DEBUG("read temp_correction_value=%f\r\n", temp.temp_correction_value);
	}
	else
	{
				temp.temp_correction_value  = 0;
	}
	
	//存储发送数据时间间隔
	//memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
	if(data_eeprom_read(DATA_TIME_ADDR, (uint8_t *)&device.data_time , 1) == HAL_OK)
	{
			DEBUG("read data_time=%d\r\n", device.data_time);
			//device.data_time = strtoul(data_buf, NULL, 10);
	}
	else
	{
				device.data_time = 1;
	}
	
	//存储恒流电流值
	memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
	if(data_eeprom_read(FLOW_ADRR, (uint8_t *)&data_buf, 16) == HAL_OK)
	{
			DEBUG("读出的字符串中的恒流电流值=%s\r\n", data_buf);
			temp.temp_flow = atof(data_buf);
			DEBUG("read string temp_flow=%.9f\r\n", temp.temp_flow);
			if(temp.temp_flow == 0)
			{
					temp.temp_flow = 0.0002;
					DEBUG("temp_flow is default value\r\n");
			}
	}
	else
	{
				temp.temp_flow = 0.0002;
	}
	
	/*CPU第一次上电标志*/
	first_power = 1;
  
	/*CPU内部ADC1初始化 测量电路板电压*/
	ADC_Init();
	
  /* Init AD7705 */
  if(AD7705_Init())
  {
    DEBUG("AD7705 OK!\r\n");
  }
  else
  {
    DEBUG("AD7705 Err!\r\n");	
  }
	
	/* Define used semaphore */
  osSemaphoreDef(SEM);
  /* Create the semaphore used by the two threads */
  semaphore=osSemaphoreCreate(osSemaphore(SEM), 1);
  if(semaphore == NULL)
  {
    DEBUG("Create Semaphore failed!\r\n");
    return -1;
  }
	
	 /* Create the mutex */
  osMutexDef(Mutex);
  mutex=osMutexCreate(osMutex(Mutex));
  if(mutex == NULL)
  {
    DEBUG("Create Mutex failed!\r\n");
    return -1;
  }
	
	/* Create a thread to update system date and time */
  osThreadDef(Measure, Measure_Thread, measurePRIORITY, 0, measureSTACK_SIZE);
  MeasureThreadHandle=osThreadCreate(osThread(Measure), NULL);
  if(MeasureThreadHandle == NULL)
  {
    DEBUG("Create Measure Thread failed!\r\n");
    return -1;
  }
  
  
  return 0;
}

/**
  * @brief  Start a measurement. 
  * @retval 0:success;-1:failed
  */
int32_t start_measure(void)
{
  /* Release the semaphore */
  if(semaphore==NULL)
  {
    return -1;
  }
  
  if(osSemaphoreRelease(semaphore)!=osOK)
  {
    return -1;
  }
  
  return 0;
}

/*测量任务函数*/
static void Measure_Thread(void const *argument)
{
	volatile uint32_t i = 0;
	
	/* get system time use struct tm */
  (void)get_sys_time_tm(&datetime);
	
  while(osSemaphoreWait(semaphore, 1)	==	osOK);					//消耗起始的信号量
	
  while(1)
  {
    /* Try to obtain the semaphore */
    if(osSemaphoreWait(semaphore,osWaitForever) == osOK)
    {
      /* get current system time */
      (void)get_sys_time(&Measure_Date, &Measure_Time);
			
			/* struct tm */
      datetime.tm_year 	=	Measure_Date.Year+2000;
      datetime.tm_mon		=	Measure_Date.Month;
      datetime.tm_mday	=	Measure_Date.Date;
      datetime.tm_hour	=	Measure_Time.Hours;
      datetime.tm_min		=	Measure_Time.Minutes;
      datetime.tm_sec		=	Measure_Time.Seconds;
			
			 /* Wait until a Mutex becomes available */
      if(osMutexWait(mutex,1000)==osOK)
      {
        
        /* Temperature Measurement 温度测量*/
        TemperaturesMeasure(&Measure_Date, &Measure_Time);
				
				if(Measure_Time.Seconds==30)
        {
          /* OnBoard Measurement 电路板电压测量*/
          OnBoardMeasure();
          
        }
        
				if(Measure_Time.Seconds == 0)
				{
					DEBUG("rain.RAIN_1M: %d	rain.RAIN_1H: %d	rain.RAIN_1D: %d\r\n", rain.RAIN_1M, rain.RAIN_1H, rain.RAIN_1D);
					//将分钟雨量保存到雨量数组里
					rain.RAIN_M[Measure_Time.Minutes] =  rain.RAIN_1M;
					rain.RAIN_1M = 0;                                            /*清零上一分钟的雨量*/
					
					//计算分钟平均温度
					/*去掉最大最小值计算分钟温度平均值*/
					temp.TEMP_1M = AverageWithoutMaxMin(temp.TEMP_T_2S, 30);
		
					/*CPU上电第一分钟使用测量值代替1分钟平均温度值 刚开机对最大最小值也有影响*/
					if(first_power)
					{
							temp.TEMP_1M 					= temp.TEMP_T_10;
							temp.TEMP_1H_MIN			= temp.TEMP_1M;
							temp.TEMP_1H_MIN_TIME = Measure_Time.Hours *100 + Measure_Time.Minutes ;
							DEBUG("TEMP_1H_MIN:%d   TEMP_1H_MIN_TIME:%d\r\n", temp.TEMP_1H_MIN , temp.TEMP_1H_MIN_TIME);
							temp.TEMP_1H_MAX			= temp.TEMP_1M;
							temp.TEMP_1H_MAX_TIME = Measure_Time.Hours *100 +Measure_Time.Minutes ;
							DEBUG("TEMP_1H_MIN:%d   TEMP_1H_MIN_TIME:%d\r\n", temp.TEMP_1H_MAX , temp.TEMP_1H_MAX_TIME);
				
					}
					DEBUG("TEMP_1M:%d\r\n", temp.TEMP_1M);
		
					/*小时温度最大最小值计算	传感器插上才能测小时最大最低温度*/
					if(((temp.TEMP_1M > temp.TEMP_1H_MAX) || (Measure_Time.Minutes == 1)) && (AD_ERROR == 0))
					{
								temp.TEMP_1H_MAX      = temp.TEMP_1M;
								temp.TEMP_1H_MAX_TIME = Measure_Time.Hours *100 +Measure_Time.Minutes ;
								DEBUG("TEMP_1H_MIN:%d   TEMP_1H_MIN_TIME:%d\r\n", temp.TEMP_1H_MAX , temp.TEMP_1H_MAX_TIME);
					}
					if(((temp.TEMP_1M < temp.TEMP_1H_MIN) || (Measure_Time.Minutes == 1)) && (AD_ERROR == 0))
					{
								temp.TEMP_1H_MIN      = temp.TEMP_1M;
								temp.TEMP_1H_MIN_TIME = Measure_Time.Hours *100 + Measure_Time.Minutes ;
								DEBUG("TEMP_1H_MIN:%d   TEMP_1H_MIN_TIME:%d\r\n", temp.TEMP_1H_MIN , temp.TEMP_1H_MIN_TIME);
					}
					/*当温度传感器未插上(即测量的AD值超出正常范围) 令小时温度最大最低值初始化*/
					if(AD_ERROR == 1)
					{
								temp.TEMP_1H_MIN = +800;
								temp.TEMP_1H_MAX_TIME = 1;
								temp.TEMP_1H_MAX = -600;
								temp.TEMP_1H_MIN_TIME = 1;
					}
					
					/*填充分钟数据*/
					memset(data_buf, 0, sizeof(data_buf));
					i = 0;
					i = OutputHourDataFill(data_buf, &datetime);
					DEBUG("data number=%d\r\n", i);
					
					//填充完分钟数据之后，每小时的起始清空雨量数组（存储60个分钟雨量的值）
					if(Measure_Time.Minutes == 0)
					{
							for(i = 0; i < 60; i++)
							{
									rain.RAIN_M[i] = 0;
							}
					}
					
					DEBUG("321\r\n");
					//根据轮询间隔时间发送分钟数据
					if(((Measure_Time.Minutes % device.data_time ) == 0) && (device.data_time != 0))
					{
							//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFF);
							//HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 3, 0xff);
							DEBUG("123\r\n");
							data_buf[252] = '}';
							data_buf[253] = '\r';
							data_buf[254] = '\n';
							data_buf[255] = '\0';
							printf("%s", data_buf );			//
							DEBUG("发送数据成功\r\n");
					}
					
					/*填充完分钟数据立即写到SD卡，以免夜长梦多*/
					i = 0;
					i = CalculateSector(&Measure_Date, &Measure_Time);
					DEBUG("1Sectors=%d\r\n", i);
					//没有插入SD卡导致向SD卡里面写数据卡死（测量任务函数崩溃），故写SD卡之前先判断SD卡是否插入
					if(SD_STATUS_OK == 0)
					{
							if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, i * 512, 512, 1))			//一次只写一个扇区
							{
									DEBUG("write data to SD Card OK\r\n");
									//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFF);
							}
							else
							{
									DEBUG("write data to SD Card error\r\n");
									BSP_SD_Init();
							}
					}
					
					
					/*第一分钟之后 清除第一次CPU上电标志*/
					first_power = 0;
				}
        /* Release mutex */
        osMutexRelease(mutex);
				
			}
		}
	}
}

/**
  * @brief  Temperature Measurement 
  * @param  Temperature Measure Structure
  * @retval None
  */
static void TemperaturesMeasure(const RTC_DateTypeDef *date,const RTC_TimeTypeDef *time)
{
	uint8_t i = 0;
//	float temp_10_correct = 0;			//在-10到10度之间的温度订正值
	
	i = time->Seconds / 2;
	DEBUG("Tempearture i = %d\r\n", i);
	/*每次测量之前都将温度值设为最低温度-60摄氏度*/
	temp.TEMP_T_10 = -60;
	
	/*温度AD测量*/
	temp.TEMP_AD = AD7705_SingleMeasurement(RT_AD7705_CH , RT_AD7705_GAIN , RT_AD7705_BUFFER);
	DEBUG("temp.TEMP_AD:%d\r\n", temp.TEMP_AD);
	//测量温度传感器的AD值应该在393--740之间，在此范围之外的AD值都是异常情况（譬如：温度传感器未插上）
	//影响到小时最大温度
	if((temp.TEMP_AD > 22000) || (temp.TEMP_AD < 12000))
	{
			AD_ERROR = 1;
	}
	else
	{
			AD_ERROR = 0;
	}
//	DEBUG("J = %d\r\n", j);
	/*转换为电压值*/
	temp.TEMP_V  = (((float)temp.TEMP_AD)/((float)(MAX_AD_VALUE))) * REF_EXT_VOLTAGE / 16;      
	DEBUG("temp.TEMP_V:%f\r\n", temp.TEMP_V);
	
	/*转换为电阻*/
	DEBUG("temp_flow=%.9f\r\n", temp.temp_flow);
	temp.TEMP_R  = (temp.TEMP_V / temp.temp_flow);
	DEBUG("temp.TEMP_R:%f\r\n", temp.TEMP_R);
	
	/*计算温度值*/
	temp.TEMP_T = CalculateTemperature(temp.TEMP_R);
	DEBUG("TEMP_T:%f\r\n", temp.TEMP_T);
	
	/*2018-03-02 09:20串口远程查看采样值*/
	if(device.usart_sampling )
	{
		printf("TEMP_AD=%d	TEMP_T=%f\r\n",	temp.TEMP_AD,	temp.TEMP_T);
	}
	
	/*温度订正值*/
	DEBUG("temp.temp_correction_value=%f\r\n", temp.temp_correction_value);
	
//	//判断测量的温度值是否在-10到10摄氏度之间
//	if((temp.TEMP_T <= 10) && (temp.TEMP_T >=0))
//	{
//			temp_10_correct	= temp.temp_correction_value / 10 ;
//			temp.TEMP_T = temp.TEMP_T - (temp_10_correct * temp.TEMP_T);
//			DEBUG("温度大于0小于10\r\n");
//	}
//	else if((temp.TEMP_T >= -10) && (temp.TEMP_T < 0))
//	{
//			temp_10_correct	= temp.temp_correction_value / 10 ;
//			temp.TEMP_T = temp.TEMP_T + (temp_10_correct * temp.TEMP_T);
//			DEBUG("温度大于-10小于0\r\n");
//	}
//	else
//	{
//			temp.TEMP_T -= temp.temp_correction_value;
//			DEBUG("温度大于10小于-10\r\n");
//	}
	
	
	temp.TEMP_T -= temp.temp_correction_value;
	DEBUG("订正之后的TEMP_T:%f\r\n", temp.TEMP_T);
	
	/*订正之后温度值的10倍再进行四舍五入*/
	if(temp.TEMP_T >= 0)						//温度为正四舍五入是+0.05
	{
			temp.TEMP_T += 0.05;
	}
	else														//温度为负四舍五入是-0.05
	{
			temp.TEMP_T -= 0.05;
	}
	DEBUG("四舍五入之后的TEMP_T:%f\r\n", temp.TEMP_T);
	temp.TEMP_T_10 = (int16_t )(temp.TEMP_T * 10);
	DEBUG("TEMP_T_10:%d\r\n", temp.TEMP_T_10);
	
		/*确定温度值范围*/
	if(temp.TEMP_T_10 > 800)
	{
		temp.TEMP_T_10 = 800;
	}
	if(temp.TEMP_T_10 < -600)
	{
		temp.TEMP_T_10 = -600;
	}
	
	//一分钟内30个温度值保存到数组里
	temp.TEMP_T_2S[i] = temp.TEMP_T_10;
}
	
/*电路板电压测量*/
static void OnBoardMeasure(void)
{
	 /* use STM32 ADC Channel 1 to measure VDD */
  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  ADC_ChannelConfTypeDef sConfig={0};
  
  /* channel : ADC_CHANNEL_1 */
  sConfig.Channel=ADC_CHANNEL_9;
  sConfig.Rank=ADC_REGULAR_RANK_1;
  sConfig.SamplingTime=ADC_SAMPLETIME_96CYCLES;
  HAL_ADC_ConfigChannel(&hadc,&sConfig);
  
  HAL_ADC_Start(&hadc);  /* Start ADC Conversion 开始测量电路板电压*/
  
  /* Wait for regular group conversion to be completed. 等到规则组转换完成*/
  if(HAL_ADC_PollForConversion(&hadc,1000)==HAL_OK)
  {
		/*读取电路板AD测量值*/
    device.board_AD = HAL_ADC_GetValue(&hadc);
		DEBUG("board_AD=%d\r\n", device.board_AD);
  }
	
	
	/*电路板电压*/
	device.board_voltage    = (((float)device.board_AD) / ((float)MAX_ADC1_VALUE)) * REF_ADC1_VOLTAGE;
	DEBUG("board_voltage1=%f\r\n", device.board_voltage);
	
	device.board_voltage    = (device.board_voltage) * R1_R2;
	DEBUG("board_voltage2=%f\r\n", device.board_voltage);
	
	device.board_voltage_10 = (uint32_t) (device.board_voltage * 10 + 0.5);
	DEBUG("board_voltage_10=%d\r\n", device.board_voltage_10);
	
	
	/*限制测量电压过高或者过低*/
	if(device.board_voltage_10 > VI_UPPER_LIMIT)
	{
		device.board_voltage_10 = 145;
	}
	if(device.board_voltage_10 < VI_LOWER_LIMIT)
	{
		device.board_voltage_10 = 55;
	}
}






