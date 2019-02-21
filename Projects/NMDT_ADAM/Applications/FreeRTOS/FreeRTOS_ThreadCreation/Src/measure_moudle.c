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
static uint8_t AD_ERROR = 0;							//1:������ADֵ����������Χ��393-740����0��������ADֵ����
static uint8_t SD_STATUS_OK = 0;					//0��SD����ʼ���ɹ���1��SD����ʼ��ʧ��
// ADת�����ֵ
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
	//SD����ʼ��
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

//	//��ȡSD������Ϣ
//	SD_CardInfo *p = malloc(sizeof(SD_CardInfo));
//	BSP_SD_GetCardInfo(p);
//	DEBUG("CardCapacity=%d	CardBlockSize=%d	RdBlockLen=%d\r\n", p->CardCapacity , p->CardBlockSize, p->Csd.RdBlockLen );
//	free(p);
	
	
	/************************************************SD���洢����ֵ***********************************************************/
/*		//��̨վ�š��¶ȶ���ֵ����ѯ���
	//��̨վ��
	memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
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
					device.station[5] = '\0';		//�������������0��ʹ��Ĭ�ϵ�00001��Ȼ���ٽ�00001д��SD����
					//snprintf(data_buf, sizeof(data_buf), "%05u", device.station);
					if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)device.station, 600000 * 512, 512, 1))			//��̨վ��д����600000����������
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
			//��ȡʧ��ʹ��Ĭ��̨վ��
			device.station[0] = '0';
			device.station[1] = '0';
			device.station[2] = '0';
  		device.station[3] = '0';
			device.station[4] = '1';
			device.station[5] = '\0';		//�������������0��ʹ��Ĭ�ϵ�00001��Ȼ���ٽ�00001д��SD����
			DEBUG("read device.station error=%s\r\n", device.station);
			BSP_SD_Init();
	}
	//����ѯ���
	memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
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
	//���¶ȶ���ֵ
	memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
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
	//��ȡ��������ֵ
	memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
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
	
	/*********************************************EEPROM�洢����ֵ*****************************************************************/
	//�洢̨վ��
	memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
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
				DEBUG("�ѵ���������\r\n");
		}
		else
		{
				device.station[0] = data_buf[0];
				device.station[1] = data_buf[1];
				device.station[2] = data_buf[2];
				device.station[3] = data_buf[3];
				device.station[4] = data_buf[4];
				//device.station[5] = '\0';
				DEBUG("��������\r\n");
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
	
	//�洢�¶ȶ���ֵ
	if(data_eeprom_read(TEMP_CORRECT_ADDR, (uint8_t *)&i , 4) == HAL_OK)
	{
			DEBUG("�����������¶ȶ���ֵ=%d\r\n", i);
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
					DEBUG("�Լ�����Ķ���ֵ\r\n");
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
	
	//�洢��������ʱ����
	//memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
	if(data_eeprom_read(DATA_TIME_ADDR, (uint8_t *)&device.data_time , 1) == HAL_OK)
	{
			DEBUG("read data_time=%d\r\n", device.data_time);
			//device.data_time = strtoul(data_buf, NULL, 10);
	}
	else
	{
				device.data_time = 1;
	}
	
	//�洢��������ֵ
	memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
	if(data_eeprom_read(FLOW_ADRR, (uint8_t *)&data_buf, 16) == HAL_OK)
	{
			DEBUG("�������ַ����еĺ�������ֵ=%s\r\n", data_buf);
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
	
	/*CPU��һ���ϵ��־*/
	first_power = 1;
  
	/*CPU�ڲ�ADC1��ʼ�� ������·���ѹ*/
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

/*����������*/
static void Measure_Thread(void const *argument)
{
	volatile uint32_t i = 0;
	
	/* get system time use struct tm */
  (void)get_sys_time_tm(&datetime);
	
  while(osSemaphoreWait(semaphore, 1)	==	osOK);					//������ʼ���ź���
	
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
        
        /* Temperature Measurement �¶Ȳ���*/
        TemperaturesMeasure(&Measure_Date, &Measure_Time);
				
				if(Measure_Time.Seconds==30)
        {
          /* OnBoard Measurement ��·���ѹ����*/
          OnBoardMeasure();
          
        }
        
				if(Measure_Time.Seconds == 0)
				{
					DEBUG("rain.RAIN_1M: %d	rain.RAIN_1H: %d	rain.RAIN_1D: %d\r\n", rain.RAIN_1M, rain.RAIN_1H, rain.RAIN_1D);
					//�������������浽����������
					rain.RAIN_M[Measure_Time.Minutes] =  rain.RAIN_1M;
					rain.RAIN_1M = 0;                                            /*������һ���ӵ�����*/
					
					//�������ƽ���¶�
					/*ȥ�������Сֵ��������¶�ƽ��ֵ*/
					temp.TEMP_1M = AverageWithoutMaxMin(temp.TEMP_T_2S, 30);
		
					/*CPU�ϵ��һ����ʹ�ò���ֵ����1����ƽ���¶�ֵ �տ����������СֵҲ��Ӱ��*/
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
		
					/*Сʱ�¶������Сֵ����	���������ϲ��ܲ�Сʱ�������¶�*/
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
					/*���¶ȴ�����δ����(��������ADֵ����������Χ) ��Сʱ�¶�������ֵ��ʼ��*/
					if(AD_ERROR == 1)
					{
								temp.TEMP_1H_MIN = +800;
								temp.TEMP_1H_MAX_TIME = 1;
								temp.TEMP_1H_MAX = -600;
								temp.TEMP_1H_MIN_TIME = 1;
					}
					
					/*����������*/
					memset(data_buf, 0, sizeof(data_buf));
					i = 0;
					i = OutputHourDataFill(data_buf, &datetime);
					DEBUG("data number=%d\r\n", i);
					
					//������������֮��ÿСʱ����ʼ����������飨�洢60������������ֵ��
					if(Measure_Time.Minutes == 0)
					{
							for(i = 0; i < 60; i++)
							{
									rain.RAIN_M[i] = 0;
							}
					}
					
					DEBUG("321\r\n");
					//������ѯ���ʱ�䷢�ͷ�������
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
							DEBUG("�������ݳɹ�\r\n");
					}
					
					/*����������������д��SD��������ҹ���ζ�*/
					i = 0;
					i = CalculateSector(&Measure_Date, &Measure_Time);
					DEBUG("1Sectors=%d\r\n", i);
					//û�в���SD��������SD������д���ݿ�������������������������дSD��֮ǰ���ж�SD���Ƿ����
					if(SD_STATUS_OK == 0)
					{
							if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, i * 512, 512, 1))			//һ��ֻдһ������
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
					
					
					/*��һ����֮�� �����һ��CPU�ϵ��־*/
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
//	float temp_10_correct = 0;			//��-10��10��֮����¶ȶ���ֵ
	
	i = time->Seconds / 2;
	DEBUG("Tempearture i = %d\r\n", i);
	/*ÿ�β���֮ǰ�����¶�ֵ��Ϊ����¶�-60���϶�*/
	temp.TEMP_T_10 = -60;
	
	/*�¶�AD����*/
	temp.TEMP_AD = AD7705_SingleMeasurement(RT_AD7705_CH , RT_AD7705_GAIN , RT_AD7705_BUFFER);
	DEBUG("temp.TEMP_AD:%d\r\n", temp.TEMP_AD);
	//�����¶ȴ�������ADֵӦ����393--740֮�䣬�ڴ˷�Χ֮���ADֵ�����쳣�����Ʃ�磺�¶ȴ�����δ���ϣ�
	//Ӱ�쵽Сʱ����¶�
	if((temp.TEMP_AD > 22000) || (temp.TEMP_AD < 12000))
	{
			AD_ERROR = 1;
	}
	else
	{
			AD_ERROR = 0;
	}
//	DEBUG("J = %d\r\n", j);
	/*ת��Ϊ��ѹֵ*/
	temp.TEMP_V  = (((float)temp.TEMP_AD)/((float)(MAX_AD_VALUE))) * REF_EXT_VOLTAGE / 16;      
	DEBUG("temp.TEMP_V:%f\r\n", temp.TEMP_V);
	
	/*ת��Ϊ����*/
	DEBUG("temp_flow=%.9f\r\n", temp.temp_flow);
	temp.TEMP_R  = (temp.TEMP_V / temp.temp_flow);
	DEBUG("temp.TEMP_R:%f\r\n", temp.TEMP_R);
	
	/*�����¶�ֵ*/
	temp.TEMP_T = CalculateTemperature(temp.TEMP_R);
	DEBUG("TEMP_T:%f\r\n", temp.TEMP_T);
	
	/*2018-03-02 09:20����Զ�̲鿴����ֵ*/
	if(device.usart_sampling )
	{
		printf("TEMP_AD=%d	TEMP_T=%f\r\n",	temp.TEMP_AD,	temp.TEMP_T);
	}
	
	/*�¶ȶ���ֵ*/
	DEBUG("temp.temp_correction_value=%f\r\n", temp.temp_correction_value);
	
//	//�жϲ������¶�ֵ�Ƿ���-10��10���϶�֮��
//	if((temp.TEMP_T <= 10) && (temp.TEMP_T >=0))
//	{
//			temp_10_correct	= temp.temp_correction_value / 10 ;
//			temp.TEMP_T = temp.TEMP_T - (temp_10_correct * temp.TEMP_T);
//			DEBUG("�¶ȴ���0С��10\r\n");
//	}
//	else if((temp.TEMP_T >= -10) && (temp.TEMP_T < 0))
//	{
//			temp_10_correct	= temp.temp_correction_value / 10 ;
//			temp.TEMP_T = temp.TEMP_T + (temp_10_correct * temp.TEMP_T);
//			DEBUG("�¶ȴ���-10С��0\r\n");
//	}
//	else
//	{
//			temp.TEMP_T -= temp.temp_correction_value;
//			DEBUG("�¶ȴ���10С��-10\r\n");
//	}
	
	
	temp.TEMP_T -= temp.temp_correction_value;
	DEBUG("����֮���TEMP_T:%f\r\n", temp.TEMP_T);
	
	/*����֮���¶�ֵ��10���ٽ�����������*/
	if(temp.TEMP_T >= 0)						//�¶�Ϊ������������+0.05
	{
			temp.TEMP_T += 0.05;
	}
	else														//�¶�Ϊ������������-0.05
	{
			temp.TEMP_T -= 0.05;
	}
	DEBUG("��������֮���TEMP_T:%f\r\n", temp.TEMP_T);
	temp.TEMP_T_10 = (int16_t )(temp.TEMP_T * 10);
	DEBUG("TEMP_T_10:%d\r\n", temp.TEMP_T_10);
	
		/*ȷ���¶�ֵ��Χ*/
	if(temp.TEMP_T_10 > 800)
	{
		temp.TEMP_T_10 = 800;
	}
	if(temp.TEMP_T_10 < -600)
	{
		temp.TEMP_T_10 = -600;
	}
	
	//һ������30���¶�ֵ���浽������
	temp.TEMP_T_2S[i] = temp.TEMP_T_10;
}
	
/*��·���ѹ����*/
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
  
  HAL_ADC_Start(&hadc);  /* Start ADC Conversion ��ʼ������·���ѹ*/
  
  /* Wait for regular group conversion to be completed. �ȵ�������ת�����*/
  if(HAL_ADC_PollForConversion(&hadc,1000)==HAL_OK)
  {
		/*��ȡ��·��AD����ֵ*/
    device.board_AD = HAL_ADC_GetValue(&hadc);
		DEBUG("board_AD=%d\r\n", device.board_AD);
  }
	
	
	/*��·���ѹ*/
	device.board_voltage    = (((float)device.board_AD) / ((float)MAX_ADC1_VALUE)) * REF_ADC1_VOLTAGE;
	DEBUG("board_voltage1=%f\r\n", device.board_voltage);
	
	device.board_voltage    = (device.board_voltage) * R1_R2;
	DEBUG("board_voltage2=%f\r\n", device.board_voltage);
	
	device.board_voltage_10 = (uint32_t) (device.board_voltage * 10 + 0.5);
	DEBUG("board_voltage_10=%d\r\n", device.board_voltage_10);
	
	
	/*���Ʋ�����ѹ���߻��߹���*/
	if(device.board_voltage_10 > VI_UPPER_LIMIT)
	{
		device.board_voltage_10 = 145;
	}
	if(device.board_voltage_10 < VI_LOWER_LIMIT)
	{
		device.board_voltage_10 = 55;
	}
}






