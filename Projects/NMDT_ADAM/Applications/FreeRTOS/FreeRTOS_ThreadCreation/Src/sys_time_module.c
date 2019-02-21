/* Includes ------------------------------------------------------------------*/
#include "sys_time_module.h"
#include "cmsis_os.h"

#include "usart_module.h"
#include "measure_moudle.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define systimeSTACK_SIZE   configMINIMAL_STACK_SIZE
#define systimePRIORITY     osPriorityHigh

/* RTC Time*/
static RTC_TimeTypeDef sys_time;
static RTC_DateTypeDef sys_date;

/* os relative */
static osThreadId SysTimeThreadHandle;
static osSemaphoreId semaphore;
static osMutexId mutex;
/* Private function prototypes -----------------------------------------------*/
static void SysTime_Thread(void const *argument);

/**
  * @brief  Init System Time Module. 
  * @retval 0:success;-1:failed
  */
int32_t init_sys_time_module(void)
{
	//初始化RTC
	MX_RTC_Init();
	
	 /* Init extern RTC PCF8563 */
  if(IIC_Init() == HAL_ERROR)
  {
    DEBUG("init pcf8563 failed!\r\n");
  }
  else
  {
    /* synchronize internal RTC with pcf8563 */
    sync_time();
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
  osThreadDef(SysTime, SysTime_Thread, systimePRIORITY, 0, systimeSTACK_SIZE);
  SysTimeThreadHandle=osThreadCreate(osThread(SysTime), NULL);
  if(SysTimeThreadHandle == NULL)
  {
    DEBUG("Create System Time Thread failed!\r\n");
    return -1;
  }
  return 0;
}

/**
  * @brief  get System Date and Time. 
  * @retval 0:success;-1:failed
  */
int32_t get_sys_time(RTC_DateTypeDef *sDate,RTC_TimeTypeDef *sTime)
{
  /* Wait until a Mutex becomes available */
  if(osMutexWait(mutex,500)==osOK)
  {
    if(sDate)
    {
      *sDate=sys_date;
    }
    if(sTime)
    {
      *sTime=sys_time;
    }
    
    /* Release mutex */
    osMutexRelease(mutex);
    
    return 0;
  }
  else
  {
    /* Time */
    if(sTime)
    {
      sTime->Seconds=0;
      sTime->Minutes=0;
      sTime->Hours=0;
    }
    /* Date */
    if(sDate)
    {
      sDate->Date=1;
      sDate->WeekDay=RTC_WEEKDAY_SUNDAY;
      sDate->Month=(uint8_t)RTC_Bcd2ToByte(RTC_MONTH_JANUARY);
      sDate->Year=0;
    }
    
    return -1;
  }
}

int32_t get_sys_time_tm(struct tm *DateTime)
{
  /* Wait until a Mutex becomes available */
  if(osMutexWait(mutex,500)==osOK)
  {
    if(DateTime)
    {
      DateTime->tm_year	=	sys_date.Year+2000;
      DateTime->tm_mon	=	sys_date.Month;
      DateTime->tm_mday	=	sys_date.Date;
      DateTime->tm_hour	=	sys_time.Hours;
      DateTime->tm_min	=	sys_time.Minutes;
      DateTime->tm_sec	=	sys_time.Seconds;
    }
    
    /* Release mutex */
    osMutexRelease(mutex);
    
    return 0;
  }
  else
  {
    if(DateTime)
    {
      DateTime->tm_year=2000;
      DateTime->tm_mon=0;
      DateTime->tm_mday=0;
      DateTime->tm_hour=0;
      DateTime->tm_min=0;
      DateTime->tm_sec=0;
    }
    
    return -1;
  }
}

int32_t set_sys_time(RTC_DateTypeDef *sDate,RTC_TimeTypeDef *sTime)
{
  int32_t res=0;
  
  /* Wait until a Mutex becomes available */
  if(osMutexWait(mutex,500)==osOK)
  {
    if(sDate)
    {
      sys_date=*sDate;
    }
    if(sTime)
    {
      sys_time=*sTime;
    }
    
    /* check param */
    if(IS_RTC_YEAR(sys_date.Year) && IS_RTC_MONTH(sys_date.Month) && IS_RTC_DATE(sys_date.Date) &&
       IS_RTC_HOUR24(sys_time.Hours) && IS_RTC_MINUTES(sys_time.Minutes) && IS_RTC_SECONDS(sys_time.Seconds))
    {
    
      if((HAL_RTC_SetDate(&hrtc,&sys_date,FORMAT_BIN)==HAL_OK)&&  /* internal RTC */
         (HAL_RTC_SetTime(&hrtc,&sys_time,FORMAT_BIN)==HAL_OK)&&
         (PCF8563_Set_Time(sys_date.Year,sys_date.Month,sys_date.Date,sys_time.Hours,sys_time.Minutes,sys_time.Seconds)==HAL_OK) )     /* PCF8563 */
      {
        res=0;
      }
      else
      {
        res=-1;
      }
    }
    else
    {
      res=-1;
    }
    
    /* Release mutex */
    osMutexRelease(mutex);
    
    return res;
  }
  else
  {
    return -1;
  }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	 /* Disable Interrupt */
  HAL_NVIC_DisableIRQ((IRQn_Type)(EXTI1_IRQn));
	HAL_NVIC_DisableIRQ((IRQn_Type)(EXTI15_10_IRQn));
	
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6) == 0)
		{
				rain.RAIN_1M++ ;/*分钟雨量*/
				rain.RAIN_1H++;/*小时雨量*/
				rain.RAIN_1D++;/*日雨量*/
				DEBUG("rain numbers++\r\n");
		}
	else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6) == 1)
		{
				DEBUG("no rain\r\n");
		}
}
/**
  * @brief  System sys_time update
  * @param  thread not used
  * @retval None
  */
static void SysTime_Thread(void const *argument)
{
	//int i = 0;
  /* Init IWDG  */
  IWDG_Init();
	
	 while(osSemaphoreWait(semaphore, 1)	==	osOK);					//不能消耗掉时间任务的信号量，因为喂狗是在该任务里面进行的，如果消耗掉该信号量将导致第一次喂狗不能及时，导致CPU复位
  
  while(1)
  {
    /* Try to obtain the semaphore */
    if(osSemaphoreWait(semaphore,osWaitForever)==osOK)
    {
      /* Wait until a Mutex becomes available */
      if(osMutexWait(mutex,500)==osOK)
      {
        HAL_RTC_GetTime(&hrtc,&sys_time,FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc,&sys_date,FORMAT_BIN);
        
        DEBUG("RTC:20%02u-%02u-%02u %02u:%02u:%02u\r\n",
               sys_date.Year,sys_date.Month,sys_date.Date,
               sys_time.Hours,sys_time.Minutes,sys_time.Seconds);
				
        /* synchronize internal RTC with pcf8563 */
        if((sys_time.Minutes==25) && (sys_time.Seconds==15))
        {
          sync_time();
        }
				
//				//测量系统任务运行占的堆大小
//					i = xPortGetFreeHeapSize();
//					DEBUG("i = %d\r\n", i);
//					
//					i = xPortGetMinimumEverFreeHeapSize();
//					DEBUG("i = %d\r\n", i);
				
				/*测量函数运行*/
				if(sys_time.Seconds % 2 == 0)
				{
					start_measure();
				}
				
				/*每分钟*/
				if(sys_time.Seconds == 1)
				{
//					DEBUG("temp.TEMP_1H_MAX: %d	temp.TEMP_1H_MAX_TIME: %d\r\n", temp.TEMP_1H_MAX, temp.TEMP_1H_MAX_TIME);
//					DEBUG("temp.TEMP_1H_MIN: %d	temp.TEMP_1H_MIN_TIME: %d\r\n", temp.TEMP_1H_MIN, temp.TEMP_1H_MIN_TIME);
					
					/*每小时*/
					if(sys_time.Minutes == 0)
					{
						rain.RAIN_1H          = 0;                               	  /*清零上一小时的雨量*/
						temp.TEMP_1H_MAX      = -600;                              	   /*清零小时最大温度*/
						temp.TEMP_1H_MAX_TIME = 0;                              	   /*清零小时最大温度出现的时间*/
						temp.TEMP_1H_MIN      = +800;                                	 /*清零小时最小温度*/
						temp.TEMP_1H_MIN_TIME = 0;																	/*清零小时最小温度出现的时间*/
						DEBUG("clean temp max and min\r\n");
						/*一天*/
						if(sys_time.Hours == 20)
						{
							rain.RAIN_1D =0;                                         /*清零昨天的日雨量*/
						}
					}
				}
				
				
        /* Release mutex */
        osMutexRelease(mutex);
        
      }
      else
      {
        DEBUG("没有等到互斥信号量\r\n");
      }
    }
    
    
    if(hiwdg.Instance)
    {
      HAL_IWDG_Refresh(&hiwdg);  /* refresh the IWDG */
			//DEBUG("喂狗了\r\n");
    }
    
  }
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  
  /* Release the semaphore every 1 second */
   if(semaphore!=NULL)
  {
		//DEBUG("闹钟A中断产生了\r\n");
    osSemaphoreRelease(semaphore);
  }
}


