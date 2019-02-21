/* Includes ------------------------------------------------------------------*/
#include "usart_module.h"
#include "cmsis_os.h"

#include "sys_time_module.h"
#include "measure_moudle.h"
#include "string.h"
#include "stdlib.h"
#include "stm32_adafruit_sd.h"
#include "eeprom.h"

/* Private define ------------------------------------------------------------*/
#define UART_RX_BUF_SIZE  (512) 	
#define storagePRIORITY     osPriorityNormal
#define storageSTACK_SIZE   configMINIMAL_STACK_SIZE

/* RTC Timeͨ����������ʱ��*/
static RTC_TimeTypeDef Usart_Time;
static RTC_DateTypeDef Usart_Date;


		
static char rx1_buffer[UART_RX_BUF_SIZE]={0};  /* USART1 receiving buffer */
static uint32_t rx1_count=0;     /* receiving counter */
static uint8_t cr1_begin=false;        /* '\r'  received */ 
static uint8_t rx1_cplt=false;   /* received a frame of data ending with '\r'and'\n' */

/* os relative */
static osThreadId Usart1ThreadHandle;
static osSemaphoreId semaphore_usart1;
static void Usart1_Thread(void const *argument);

/**
  * @brief  Init Storage Module. 
  * @retval 0:success;-1:failed
  */
int32_t init_usart_module(void)
{
	USART1_UART_Init(9600);
	
	printf("Hello World!\r\n");
	
	/* Define used semaphore ��������1���ź���*/
  osSemaphoreDef(SEM_USART1);
  /* Create the semaphore used by the two threads */
  semaphore_usart1=osSemaphoreCreate(osSemaphore(SEM_USART1), 1);
  if(semaphore_usart1 == NULL)
  {
    DEBUG("Create Semaphore_USART1 failed!");
    return -1;
  }
 
	
	/* Create a thread to read historical data��������1����洢�������� */
  osThreadDef(Usart1, Usart1_Thread, storagePRIORITY, 0, storageSTACK_SIZE);
  Usart1ThreadHandle=osThreadCreate(osThread(Usart1), NULL);
  if(Usart1ThreadHandle == NULL)
  {
    DEBUG("Create Usart1 Thread failed!");
    return -1;
  }
	
  return 0;
	
	
}


/*����1������������*/
static void Usart1_Thread(void const *argument)
{
	/*ʱ��ṹ��*/
	//struct tm datetime_u={0};
	
  //int down_year=0,down_month=0,down_day=0,down_hour=0,down_minutes=0,down_number=0,down_second=0;
	volatile uint32_t i = 0;
	volatile uint8_t down_number = 0;
	
	if(init_sys_time_module()<0)
  {
    DEBUG("init sys_time module failed!\r\n");
	  //HAL_NVIC_SystemReset();  /*����CPU*/
  }
  else
  {
    DEBUG("init sys_time module ok!\r\n");
  }
	
	if(init_measure_module()<0)
  {
    DEBUG("init measure module failed!\r\n");
		//HAL_NVIC_SystemReset();  /*����CPU*/
  }
  else
  {
    DEBUG("init measure module ok!\r\n");
  }
	
	while(osSemaphoreWait(semaphore_usart1, 1)	==	osOK);			//CPU��������ʱ�򣬴����ź�����ʱ�򣬾��п��õ��ź����������������ĵ����ź������ȴ����ź�����ʱ�価���̣ܶ�����д1����һ��ʱ������
	
	while(1)
	{
		
		if(osSemaphoreWait(semaphore_usart1,osWaitForever)==osOK)
		{
//			//�õ����ڽ��յ�����ʱ��ʱ��
//			set_sys_time(&Usart_Date, &Usart_Time);
			if(strlen(rx1_buffer)==29)
			{
				/*����վ��"<D00000000000000000001012345> ����վ��Ϊ12345"*/ 
				if((memcmp("<D0000",rx1_buffer,6) == 0)&&(rx1_buffer[21] == 49))
				{
						printf("T\r\n");
						device.station[0] = rx1_buffer[23];
						device.station[1] = rx1_buffer[24];
						device.station[2] = rx1_buffer[25];
					  device.station[3] = rx1_buffer[26];
						device.station[4] = rx1_buffer[27];
						//device.station[5] = '\0';
						DEBUG("station:%c%c%c%c%c\r\n", device.station[0], device.station[1], device.station[2], device.station[3], device.station[4]);
					
					/************************************��̨վ�Ŵ洢��SD ����*********************************************/
//						memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
//						data_buf[0] = rx1_buffer[23];								//��������̨վ���ַ���ֵ���ַ�������
//						data_buf[1] = rx1_buffer[24];
//						data_buf[2] = rx1_buffer[25];
//						data_buf[3] = rx1_buffer[26];
//						data_buf[4] = rx1_buffer[27];
////						DEBUG("�ַ������������̨վ�ţ�%c%c%c%c%c\r\n", data_buf[0], data_buf[1], data_buf[2], data_buf[3], data_buf[4]);
//						if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)device.station, 600000 * 512, 512, 1))			//��̨վ��д����600000����������
//						{
//								DEBUG("set station ok\r\n");
//						}
//						else
//						{
//								DEBUG("set station error\r\n");
//						}
						/************************************��̨վ�Ŵ洢��EEPROM��*********************************************/
						if(data_eeprom_write(STATION_ADDR, (uint8_t *)device.station, 8) == HAL_OK)
						{
									DEBUG("save station ok\r\n");
						}
						else 
						{
									DEBUG("save station error\r\n");
						}
				

				}
				/*��ʱ<D00002018011914425510000000> ����ʱ��Ϊ2018-01-19 14:42:55*/
				else if((memcmp("<D0010",rx1_buffer,6) == 0)&&(rx1_buffer[20] == 49))
				{
						printf("T\r\n");
						Usart_Time.Seconds     =   (rx1_buffer[18]-48)*10+rx1_buffer[19]-48;
						Usart_Time.Minutes     =   (rx1_buffer[16]-48)*10+rx1_buffer[17]-48;
						Usart_Time.Hours       =   (rx1_buffer[14]-48)*10+rx1_buffer[15]-48;
						Usart_Date.Date        =   (rx1_buffer[12]-48)*10+rx1_buffer[13]-48;
						Usart_Date.Month       =   (rx1_buffer[10]-48)*10+rx1_buffer[11]-48;
						Usart_Date.Year        =   (rx1_buffer[8]-48)*10+rx1_buffer[9]-48;
						if(PCF8563_Set_Time(Usart_Date.Year, Usart_Date.Month, Usart_Date.Date, Usart_Time.Hours, Usart_Time.Minutes, Usart_Time.Seconds) != HAL_OK)
						{
									DEBUG("Set Time Fail");
						}
						else
						{
									sync_time();
									DEBUG("Set Time Success");
						}	
				 }
				/*��Ҫ��������	<D10772018011914425500000000>��2018-01-19 14:42:55��ʼ��Ҫ77����������*/
				else if(memcmp("<D10",rx1_buffer,4) == 0)
				{
					printf("T\r\n");
					/*��Ҫ�Ļ�׼ʱ�估������������*/
					Usart_Date.Month     = (rx1_buffer[10]-48)*10+rx1_buffer[11]-48;
					Usart_Date.Date      = (rx1_buffer[12]-48)*10+rx1_buffer[13]-48;
					Usart_Time.Hours     = (rx1_buffer[14]-48)*10+rx1_buffer[15]-48;
					Usart_Time.Minutes 	 = (rx1_buffer[16]-48)*10+rx1_buffer[17]-48;
					//down_year    = (rx1_buffer[8]-48)*10+rx1_buffer[9]-48;
					//down_second  = 0;
					/*������*/
					//��������������Чλ
					//if (down_year > 99)      down_year    = 0;  //�ָ�00��
					if (Usart_Date.Month  > 12)     Usart_Date.Month   = 1;  //�ָ�1��
					if (Usart_Date.Date > 31)       Usart_Date.Date     = 1;  //�ָ�1��
					if (Usart_Time.Hours > 23)      Usart_Time.Minutes    = 0;  //�ָ�0ʱ
					if (Usart_Time.Minutes > 59)    Usart_Time.Minutes  = 0;  //�ָ�0����
					/*��Ҫ�������ݵ�����*/
					down_number = (rx1_buffer[4] -48)*10 + rx1_buffer[5] - 48;
					DEBUG("Minute Data Numbers=%d\r\n", down_number);
					/*�����Ҫ���ݵ���ʼ������*/
					i = 0;
					i = CalculateSector(&Usart_Date, &Usart_Time);
					DEBUG("SD Card Minute Data Start Sectors=%d\r\n", i);
					while(down_number--)
					{
							memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
							if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)data_buf, i * 512, 512, 1))
							{
									//���������Ƿ��Ƿ�������
									if((memcmp("[D",data_buf,2) == 0) && (data_buf[251] == ']'))
									{
											data_buf[252] = '}';
											data_buf[253] = '\r';
											data_buf[254] = '\n';
											data_buf[255] = '\0';
											//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFF);
											printf("%s", data_buf);
											osDelay(100);
									}
									else
									{
											//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFF);
											osDelay(100);
									}
							}
							else
							{
									BSP_SD_Init();
							}
							i++;			//�����ż�1 
					}
				}
				/*��ҪСʱ����<D12772018011914425500000000>��2018-01-19 14:42:55��ʼ��Ҫ77����������*/
				else if(memcmp("<D12",rx1_buffer,4) == 0)
				{
					printf("T\r\n");
					/*��Ҫ�Ļ�׼ʱ�估������������*/
					Usart_Date.Month    = (rx1_buffer[10]-48)*10+rx1_buffer[11]-48;
					Usart_Date.Date     = (rx1_buffer[12]-48)*10+rx1_buffer[13]-48;
					Usart_Time.Hours    = (rx1_buffer[14]-48)*10+rx1_buffer[15]-48;
					Usart_Time.Minutes  = 0;					//�������ݣ������Ϊ0
					/*������*/
					//��������������Чλ
					//if (down_year > 99)      down_year    = 0;  //�ָ�00��
					if (Usart_Date.Month > 12)     Usart_Date.Month   = 1;  //�ָ�1��
					if (Usart_Date.Date > 31)       Usart_Date.Date     = 1;  //�ָ�1��
					if (Usart_Time.Hours > 23)      Usart_Time.Hours    = 0;  //�ָ�0ʱ
					/*��ҪСʱ���ݵ�����*/
					down_number = (rx1_buffer[4] -48)*10 + rx1_buffer[5] - 48;
					DEBUG("Hour Data Numbers=%d\r\n", down_number);
					//�����ʼ��Ҫ���������ݶ�Ӧ��������
					i = 0;
					i = CalculateSector(&Usart_Date, &Usart_Time);
					DEBUG("SD Card Hour Data Start Sectors=%d\r\n", i);
					while(down_number--)
						{
							memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
							if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)data_buf, i * 512, 512, 1) && (data_buf[17] == '0') && (data_buf[18] == '0'))
							{
									//���������Ƿ���Сʱ����
									if((memcmp("[D",data_buf,2) == 0) && (data_buf[251] == ']'))
									{
											data_buf[252] = '}';
											data_buf[253] = '\r';
											data_buf[254] = '\n';
											data_buf[255] = '\0';
											//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFF);
											printf("%s", data_buf);
											osDelay(100);
									}
									else
									{
											//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFF);
											osDelay(100);
									}
									
							}
							else
							{
									DEBUG("read  data fail\r\n");
							}
							i += 60;			//ÿһ���������ݵ����������60 
					}
				}
				/*��ȡ��ǰ��������  <D00000000000000000000000000>*/
				else if((memcmp("<D0000",rx1_buffer,6) == 0)&&(rx1_buffer[22] == 48)&&(rx1_buffer[21] == 48)&&(rx1_buffer[20] == 48))
				{
					printf("T\r\n");
					get_sys_time(&Usart_Date,&Usart_Time);
					/* struct tm */
					datetime.tm_year  = Usart_Date.Year+2000;
					datetime.tm_mon   = Usart_Date.Month;
					datetime.tm_mday  = Usart_Date.Date;
					datetime.tm_hour  = Usart_Time.Hours;
					datetime.tm_min   = Usart_Time.Minutes;
					datetime.tm_sec   = Usart_Time.Seconds;
					/*����������*/
					i = OutputHourDataFill((char *)data_buf, &datetime);
					//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFFFF);
					data_buf[252] = '}';
					data_buf[253] = '\r';
					data_buf[254] = '\n';
					data_buf[255] = '\0';
					printf("%s", data_buf);

				}
				/*�¶ȶ��� <D0200-110000000000000000000>*/
				else if(memcmp("<D0200",rx1_buffer,6) == 0)
				{
						printf("T\r\n");
						if(rx1_buffer[6] == '+')
						{
								temp.temp_correction_value = (float)(rx1_buffer[7] - 48) + ((float)(rx1_buffer[8] - 48)) / 10;
								i = (uint32_t)(temp.temp_correction_value * 10);
								DEBUG("�¶ȶ���ֵ������=%d\r\n", i);
						}
						else if(rx1_buffer[6] == '-')
						{
								temp.temp_correction_value = 0 -((float)(rx1_buffer[7] - 48) + ((float)(rx1_buffer[8] - 48)) / 10);
								i = (uint32_t)(temp.temp_correction_value * 10) + 100;
								DEBUG("�¶ȶ���ֵ������=%d\r\n", i);
						}
						DEBUG("temp.temp_correction_value=%f\r\n", temp.temp_correction_value);
//						//���¶ȶ���ֵд����600010��������
//						memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
//						data_buf[0] = rx1_buffer[6];
//						data_buf[1] = rx1_buffer[7];
//						data_buf[2] = rx1_buffer[8];
//						DEBUG("�ַ�������������¶ȶ���ֵ��%c%c%c\r\n", data_buf[0], data_buf[1], data_buf[2]);
//						if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, 600010 * 512, 512, 1))			//һ��ֻдһ������
//						{
//								DEBUG("set temp_correction_value ok\r\n");
//						}
//						else
//						{
//								DEBUG("set temp_correction_value error\r\n");
//						}
						
						if(data_eeprom_write(TEMP_CORRECT_ADDR, (uint8_t *)&i, 4) == HAL_OK)
						{
									DEBUG("save temp_correction_value ok\r\n");
						}
						else 
						{
									DEBUG("save temp_correction_value error\r\n");
						}
				}
				/*�������ݵ���ѯ���� <D01010000000000000000000000>*/
				else if(memcmp("<D01",rx1_buffer,4) == 0)
				{
						printf("T\r\n");
						device.data_time = (rx1_buffer[4]-48)*10 + (rx1_buffer[5]-48);
						DEBUG("data_time:%d\r\n",device.data_time);
						if(device.data_time > 60)
						{
								DEBUG("data_time error\r\n");
								printf("COMMAND ERROR!\r\n");
						}
						else
						{
//								memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
//								data_buf[0] = rx1_buffer[4];
//								data_buf[1] = rx1_buffer[5];
//								DEBUG("�ַ��������������ѯ���ʱ�䣺%c%c\r\n", data_buf[0], data_buf[1]);
//								//����ѯ���д����600020��������
//								if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, 600020 * 512, 512, 1))			//һ��ֻдһ������
//								{
//										DEBUG("set data_time ok\r\n");
//								}
//								else
//								{
//										DEBUG("set data_time error\r\n");
//								}
							
								if(data_eeprom_write(DATA_TIME_ADDR, (uint8_t *)&device.data_time , 1) == HAL_OK)
								{
											DEBUG("save data_time ok\r\n");
								}
								else 
								{
											DEBUG("save data_time error\r\n");
								}
						}
				}
			}
//			else if(strlen(rx1_buffer) == 8)			 /*����SD��*/
//			{	
//					printf("T\r\n");
//					i = strtoul(rx1_buffer, NULL, 10);
//					OutputHourDataFill(data_buf, &datetime);
//					if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, i * 512, 512, 1))			//һ��ֻдһ������
//					{
//							memset(data_buf, 0, sizeof(data_buf));
//							if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)data_buf, i * 512, 512, 1))
//							{
//									HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFF);
//							}
//					}
//			}
			else if((strlen(rx1_buffer) == 9) && (strcasecmp("<restart>", rx1_buffer) == 0))
			{
					HAL_NVIC_SystemReset();						//����CPU
			}
			else if((strlen(rx1_buffer) == 9) && (strcasecmp("<version>", rx1_buffer) == 0))
			{
					printf("version:V1.2");
			}
			else if((strlen(rx1_buffer) == 12) && (memcmp("<TEMP", rx1_buffer, 5) == 0))		//���ú�������ֵ ���<TEMP200123>
			{
					temp.temp_flow = atof(rx1_buffer + 5) / 1000000000;
					DEBUG("set temp_flow=%.9f\r\n", temp.temp_flow);
					
//					memset(data_buf, 0, sizeof(data_buf));			//�������ַ�������
//					snprintf(data_buf, sizeof(data_buf), "%.9f", temp.temp_flow);
//					if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, 600030 * 512, 512, 1))			//һ��ֻдһ������
//						{
//										DEBUG("save temp_flow ok\r\n");
//						}
//					else
//						{
//										DEBUG("save temp_flow error\r\n");
//										BSP_SD_Init();
//						}
					//i = (uint32_t)(temp.temp_flow * 1000000000);
					snprintf(data_buf, sizeof(data_buf), "%.9f", temp.temp_flow);
					DEBUG("�ַ����еĺ�������=%s\r\n", data_buf);
					if(data_eeprom_write(FLOW_ADRR, (uint8_t *)&data_buf, 16) == HAL_OK)
						{
										DEBUG("save FLOW_ADRR ok\r\n");
						}
					else 
						{
										DEBUG("save FLOW_ADRR error\r\n");
						}
					
			}
			else if((memcmp("<correction", rx1_buffer, 11) == 0) && (strlen(rx1_buffer) == 19)) //�����룺<correction+0.1439>
			{
					printf("T\r\n");
					temp.temp_correction_value = atof(rx1_buffer + 11);
					DEBUG("correction=%f\r\n", temp.temp_correction_value);
					
					if(temp.temp_correction_value >= 0)
					{
							i = temp.temp_correction_value * 10000;
					}
					else if(temp.temp_correction_value < 0)
					{
							i = abs(temp.temp_correction_value * 10000) + 100000;
					}
					DEBUG("correction I=%d\r\n", i);
					if(data_eeprom_write(TEMP_CORRECT_ADDR, (uint8_t *)&i, 4) == HAL_OK)
						{
									DEBUG("save temp_correction_value ok\r\n");
						}
						else 
						{
									DEBUG("save temp_correction_value error\r\n");
						}
			}
			else if((strlen(rx1_buffer) == 12) && (strcasecmp("<correction>", rx1_buffer) == 0))
			{
						printf("T\r\n");
						printf("temp_correction_value=%f\r\n", temp.temp_correction_value);
			}
			else if((strlen(rx1_buffer) == 6) && (strcasecmp("<time>", rx1_buffer) == 0))
			{
						get_sys_time(&Usart_Date, &Usart_Time);
						printf("time:20%02d-%02d-%02d %02d:%02d:%02d\r\n", Usart_Date.Year , Usart_Date.Month , Usart_Date.Date , Usart_Time.Hours , Usart_Time.Minutes , Usart_Time.Seconds );
			}
			else if((strlen(rx1_buffer) == 7) && (strcasecmp("<usart>", rx1_buffer) == 0))			/*2018-03-02 09:20���Զ�̲鿴����ֵ*/
			{
					device.usart_sampling = 1;
			}
			else if((strlen(rx1_buffer) == 8) && (strcasecmp("<usart0>", rx1_buffer) == 0))			/*2018-03-02 09:20���Զ�̲鿴����ֵ*/
			{
					device.usart_sampling = 0;
			}
			else
			{
					printf("F\r\n");
					//printf("%s\r\n", rx1_buffer);
			}
			
			rx1_count	=	0;
			rx1_cplt	=	false;                                              /* clear the flag of receive */
			memset(rx1_buffer,0,sizeof(rx1_buffer));                      /* clear the register of receive */
		}
	}
}



/**����1�жϺ���*/
void USART1_IRQHandler(void)
{
  UART_HandleTypeDef *huart=&huart1;
  uint32_t tmp_flag = 0, tmp_it_source = 0;

  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_PE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_PE);  
  /* UART parity error interrupt occurred ------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  { 
    huart->ErrorCode |= HAL_UART_ERROR_PE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_FE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_ERR);
  /* UART frame error interrupt occurred -------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_FE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_NE);
  /* UART noise error interrupt occurred -------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_NE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_ORE);
  /* UART Over-Run interrupt occurred ----------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    huart->ErrorCode |= HAL_UART_ERROR_ORE;
  }
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
  /* UART in mode Receiver ---------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_Receive_IT(huart);*/
    uint8_t data=0;
  
    data=huart->Instance->DR;  /* the byte just received  */
    
		
		if(!rx1_cplt)
    {
      if(data=='<')
      {
        cr1_begin=true;
        rx1_count=0;
        rx1_buffer[rx1_count]=data;
        rx1_count++; 
      }
     
      else if(cr1_begin==true)
      {
        rx1_buffer[rx1_count]=data;
        rx1_count++; 
        if((rx1_count == (UART_RX_BUF_SIZE-1)) && (data != '>'))  /* rx buffer full */
        {
          /* Set transmission flag: trasfer complete*/
          rx1_cplt=true;
					rx1_count=0;
        }
        
        if((data=='>') && (rx1_count < (UART_RX_BUF_SIZE-1)))
        {
          rx1_cplt=true;
          cr1_begin=false;
        }
      }
      else
      {
        rx1_count=0;
      }
    }

		
    
  	 /* received a data frame ���ݽ�����ɾ��ͷŻ�����*/
    if(rx1_cplt == true)
    {
      if(semaphore_usart1 != NULL)
      {
         /* Release mutex */
        osSemaphoreRelease(semaphore_usart1);
				DEBUG("USART1 receive semaphore_usart1\r\n");
      }
    }

   
    }
  
  
  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_TXE);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TXE);
  /* UART in mode Transmitter ------------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_Transmit_IT(huart);*/
  }

  tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_TC);
  tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC);
  /* UART in mode Transmitter end --------------------------------------------*/
  if((tmp_flag != RESET) && (tmp_it_source != RESET))
  {
    /*UART_EndTransmit_IT(huart);*/
  }  

  if(huart->ErrorCode != HAL_UART_ERROR_NONE)
  {
    /* Clear all the error flag at once */
    __HAL_UART_CLEAR_PEFLAG(huart);
    
    /* Set the UART state ready to be able to start again the process */
    huart->State = HAL_UART_STATE_READY;
    
    HAL_UART_ErrorCallback(huart);
  } 
}




