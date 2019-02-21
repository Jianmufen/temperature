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

/* RTC Time通过串口设置时间*/
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
	
	/* Define used semaphore 创建串口1的信号量*/
  osSemaphoreDef(SEM_USART1);
  /* Create the semaphore used by the two threads */
  semaphore_usart1=osSemaphoreCreate(osSemaphore(SEM_USART1), 1);
  if(semaphore_usart1 == NULL)
  {
    DEBUG("Create Semaphore_USART1 failed!");
    return -1;
  }
 
	
	/* Create a thread to read historical data创建串口1处理存储数据任务 */
  osThreadDef(Usart1, Usart1_Thread, storagePRIORITY, 0, storageSTACK_SIZE);
  Usart1ThreadHandle=osThreadCreate(osThread(Usart1), NULL);
  if(Usart1ThreadHandle == NULL)
  {
    DEBUG("Create Usart1 Thread failed!");
    return -1;
  }
	
  return 0;
	
	
}


/*串口1处理数据任务*/
static void Usart1_Thread(void const *argument)
{
	/*时间结构体*/
	//struct tm datetime_u={0};
	
  //int down_year=0,down_month=0,down_day=0,down_hour=0,down_minutes=0,down_number=0,down_second=0;
	volatile uint32_t i = 0;
	volatile uint8_t down_number = 0;
	
	if(init_sys_time_module()<0)
  {
    DEBUG("init sys_time module failed!\r\n");
	  //HAL_NVIC_SystemReset();  /*重启CPU*/
  }
  else
  {
    DEBUG("init sys_time module ok!\r\n");
  }
	
	if(init_measure_module()<0)
  {
    DEBUG("init measure module failed!\r\n");
		//HAL_NVIC_SystemReset();  /*重启CPU*/
  }
  else
  {
    DEBUG("init measure module ok!\r\n");
  }
	
	while(osSemaphoreWait(semaphore_usart1, 1)	==	osOK);			//CPU刚启动的时候，创建信号量的时候，就有可用的信号量，在这里先消耗掉该信号量。等待该信号量的时间尽可能短，这里写1代表一个时钟周期
	
	while(1)
	{
		
		if(osSemaphoreWait(semaphore_usart1,osWaitForever)==osOK)
		{
//			//得到串口接收到命令时的时间
//			set_sys_time(&Usart_Date, &Usart_Time);
			if(strlen(rx1_buffer)==29)
			{
				/*重设站号"<D00000000000000000001012345> 设置站号为12345"*/ 
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
					
					/************************************将台站号存储在SD 卡里*********************************************/
//						memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
//						data_buf[0] = rx1_buffer[23];								//将读到的台站号字符赋值给字符串数组
//						data_buf[1] = rx1_buffer[24];
//						data_buf[2] = rx1_buffer[25];
//						data_buf[3] = rx1_buffer[26];
//						data_buf[4] = rx1_buffer[27];
////						DEBUG("字符串数组里面的台站号：%c%c%c%c%c\r\n", data_buf[0], data_buf[1], data_buf[2], data_buf[3], data_buf[4]);
//						if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)device.station, 600000 * 512, 512, 1))			//将台站号写到第600000个扇区号内
//						{
//								DEBUG("set station ok\r\n");
//						}
//						else
//						{
//								DEBUG("set station error\r\n");
//						}
						/************************************将台站号存储在EEPROM里*********************************************/
						if(data_eeprom_write(STATION_ADDR, (uint8_t *)device.station, 8) == HAL_OK)
						{
									DEBUG("save station ok\r\n");
						}
						else 
						{
									DEBUG("save station error\r\n");
						}
				

				}
				/*对时<D00002018011914425510000000> 设置时间为2018-01-19 14:42:55*/
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
				/*补要分钟数据	<D10772018011914425500000000>从2018-01-19 14:42:55开始补要77条分钟数据*/
				else if(memcmp("<D10",rx1_buffer,4) == 0)
				{
					printf("T\r\n");
					/*补要的基准时间及分钟数据条数*/
					Usart_Date.Month     = (rx1_buffer[10]-48)*10+rx1_buffer[11]-48;
					Usart_Date.Date      = (rx1_buffer[12]-48)*10+rx1_buffer[13]-48;
					Usart_Time.Hours     = (rx1_buffer[14]-48)*10+rx1_buffer[15]-48;
					Usart_Time.Minutes 	 = (rx1_buffer[16]-48)*10+rx1_buffer[17]-48;
					//down_year    = (rx1_buffer[8]-48)*10+rx1_buffer[9]-48;
					//down_second  = 0;
					/*检查参数*/
					//检查参数，屏蔽无效位
					//if (down_year > 99)      down_year    = 0;  //恢复00年
					if (Usart_Date.Month  > 12)     Usart_Date.Month   = 1;  //恢复1月
					if (Usart_Date.Date > 31)       Usart_Date.Date     = 1;  //恢复1日
					if (Usart_Time.Hours > 23)      Usart_Time.Minutes    = 0;  //恢复0时
					if (Usart_Time.Minutes > 59)    Usart_Time.Minutes  = 0;  //恢复0分钟
					/*补要分钟数据的条数*/
					down_number = (rx1_buffer[4] -48)*10 + rx1_buffer[5] - 48;
					DEBUG("Minute Data Numbers=%d\r\n", down_number);
					/*算出补要数据的起始扇区号*/
					i = 0;
					i = CalculateSector(&Usart_Date, &Usart_Time);
					DEBUG("SD Card Minute Data Start Sectors=%d\r\n", i);
					while(down_number--)
					{
							memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
							if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)data_buf, i * 512, 512, 1))
							{
									//检查读到的是否是分钟数据
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
							i++;			//扇区号加1 
					}
				}
				/*补要小时数据<D12772018011914425500000000>从2018-01-19 14:42:55开始补要77条整点数据*/
				else if(memcmp("<D12",rx1_buffer,4) == 0)
				{
					printf("T\r\n");
					/*补要的基准时间及分钟数据条数*/
					Usart_Date.Month    = (rx1_buffer[10]-48)*10+rx1_buffer[11]-48;
					Usart_Date.Date     = (rx1_buffer[12]-48)*10+rx1_buffer[13]-48;
					Usart_Time.Hours    = (rx1_buffer[14]-48)*10+rx1_buffer[15]-48;
					Usart_Time.Minutes  = 0;					//整点数据，则分钟为0
					/*检查参数*/
					//检查参数，屏蔽无效位
					//if (down_year > 99)      down_year    = 0;  //恢复00年
					if (Usart_Date.Month > 12)     Usart_Date.Month   = 1;  //恢复1月
					if (Usart_Date.Date > 31)       Usart_Date.Date     = 1;  //恢复1日
					if (Usart_Time.Hours > 23)      Usart_Time.Hours    = 0;  //恢复0时
					/*补要小时数据的条数*/
					down_number = (rx1_buffer[4] -48)*10 + rx1_buffer[5] - 48;
					DEBUG("Hour Data Numbers=%d\r\n", down_number);
					//算出起始补要的整点数据对应的扇区号
					i = 0;
					i = CalculateSector(&Usart_Date, &Usart_Time);
					DEBUG("SD Card Hour Data Start Sectors=%d\r\n", i);
					while(down_number--)
						{
							memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
							if(MSD_OK == BSP_SD_ReadBlocks((uint32_t *)data_buf, i * 512, 512, 1) && (data_buf[17] == '0') && (data_buf[18] == '0'))
							{
									//检查读到的是否是小时数据
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
							i += 60;			//每一个整点数据的扇区号相差60 
					}
				}
				/*读取当前分钟数据  <D00000000000000000000000000>*/
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
					/*填充分钟数据*/
					i = OutputHourDataFill((char *)data_buf, &datetime);
					//HAL_UART_Transmit(&huart1, (uint8_t *)data_buf, sizeof(data_buf), 0xFFFF);
					data_buf[252] = '}';
					data_buf[253] = '\r';
					data_buf[254] = '\n';
					data_buf[255] = '\0';
					printf("%s", data_buf);

				}
				/*温度订正 <D0200-110000000000000000000>*/
				else if(memcmp("<D0200",rx1_buffer,6) == 0)
				{
						printf("T\r\n");
						if(rx1_buffer[6] == '+')
						{
								temp.temp_correction_value = (float)(rx1_buffer[7] - 48) + ((float)(rx1_buffer[8] - 48)) / 10;
								i = (uint32_t)(temp.temp_correction_value * 10);
								DEBUG("温度订正值的整形=%d\r\n", i);
						}
						else if(rx1_buffer[6] == '-')
						{
								temp.temp_correction_value = 0 -((float)(rx1_buffer[7] - 48) + ((float)(rx1_buffer[8] - 48)) / 10);
								i = (uint32_t)(temp.temp_correction_value * 10) + 100;
								DEBUG("温度订正值的整形=%d\r\n", i);
						}
						DEBUG("temp.temp_correction_value=%f\r\n", temp.temp_correction_value);
//						//将温度订正值写到第600010个扇区号
//						memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
//						data_buf[0] = rx1_buffer[6];
//						data_buf[1] = rx1_buffer[7];
//						data_buf[2] = rx1_buffer[8];
//						DEBUG("字符串数组里面的温度订正值：%c%c%c\r\n", data_buf[0], data_buf[1], data_buf[2]);
//						if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, 600010 * 512, 512, 1))			//一次只写一个扇区
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
				/*分钟数据的轮询机制 <D01010000000000000000000000>*/
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
//								memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
//								data_buf[0] = rx1_buffer[4];
//								data_buf[1] = rx1_buffer[5];
//								DEBUG("字符串数组里面的轮询间隔时间：%c%c\r\n", data_buf[0], data_buf[1]);
//								//将轮询间隔写到第600020个扇区中
//								if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, 600020 * 512, 512, 1))			//一次只写一个扇区
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
//			else if(strlen(rx1_buffer) == 8)			 /*调试SD卡*/
//			{	
//					printf("T\r\n");
//					i = strtoul(rx1_buffer, NULL, 10);
//					OutputHourDataFill(data_buf, &datetime);
//					if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, i * 512, 512, 1))			//一次只写一个扇区
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
					HAL_NVIC_SystemReset();						//重启CPU
			}
			else if((strlen(rx1_buffer) == 9) && (strcasecmp("<version>", rx1_buffer) == 0))
			{
					printf("version:V1.2");
			}
			else if((strlen(rx1_buffer) == 12) && (memcmp("<TEMP", rx1_buffer, 5) == 0))		//设置恒流电流值 命令：<TEMP200123>
			{
					temp.temp_flow = atof(rx1_buffer + 5) / 1000000000;
					DEBUG("set temp_flow=%.9f\r\n", temp.temp_flow);
					
//					memset(data_buf, 0, sizeof(data_buf));			//先清零字符串数组
//					snprintf(data_buf, sizeof(data_buf), "%.9f", temp.temp_flow);
//					if(MSD_OK == BSP_SD_WriteBlocks((uint32_t *)data_buf, 600030 * 512, 512, 1))			//一次只写一个扇区
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
					DEBUG("字符串中的恒流电流=%s\r\n", data_buf);
					if(data_eeprom_write(FLOW_ADRR, (uint8_t *)&data_buf, 16) == HAL_OK)
						{
										DEBUG("save FLOW_ADRR ok\r\n");
						}
					else 
						{
										DEBUG("save FLOW_ADRR error\r\n");
						}
					
			}
			else if((memcmp("<correction", rx1_buffer, 11) == 0) && (strlen(rx1_buffer) == 19)) //命令入：<correction+0.1439>
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
			else if((strlen(rx1_buffer) == 7) && (strcasecmp("<usart>", rx1_buffer) == 0))			/*2018-03-02 09:20添加远程查看采样值*/
			{
					device.usart_sampling = 1;
			}
			else if((strlen(rx1_buffer) == 8) && (strcasecmp("<usart0>", rx1_buffer) == 0))			/*2018-03-02 09:20添加远程查看采样值*/
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



/**串口1中断函数*/
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

		
    
  	 /* received a data frame 数据接收完成就释放互斥量*/
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




