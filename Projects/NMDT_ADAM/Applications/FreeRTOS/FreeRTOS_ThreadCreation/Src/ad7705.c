#include "ad7705.h"	
#include "usart_module.h"

/**
  * @brief LINK AD7705
  */
#define AD7705_DUMMY_BYTE            0xFF    
#define AD7705_NO_RESPONSE_EXPECTED  0x80
   
/**
 * @brief BUS variables
 */

#ifdef HAL_SPI_MODULE_ENABLED
static uint32_t SpixTimeout = AD7705_SPI1_TIMEOUT_MAX;        /*<! Value of Timeout when SPI communication fails */
static SPI_HandleTypeDef had7705_Spi;
#endif /* HAL_SPI_MODULE_ENABLED */



/** @defgroup STM32L1XX_AD7705_Private_Functions Private Functions
  * @{
  */ 
#ifdef HAL_SPI_MODULE_ENABLED
static void               SPI1_Init(void);
static void               SPI1_Write(uint8_t Value);
static uint32_t           SPI1_Read(void);
static void               SPI1_Error (void);
static void               SPI1_MspInit(void);
#endif /* HAL_SPI_MODULE_ENABLED */


#ifdef HAL_SPI_MODULE_ENABLED
/* AD7705 IO functions */


#endif /* HAL_SPI_MODULE_ENABLED */
/**
  * @}
  */ 



#ifdef HAL_SPI_MODULE_ENABLED
/******************************************************************************
                            BUS OPERATIONS
*******************************************************************************/
/**
  * @brief  Initializes SPI MSP.
  * @retval None
  */
static void SPI1_MspInit(void)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};
  
  /*** Configure the GPIOs ***/  
  /* Enable GPIO clock */
  AD7705_SPI1_SCK_GPIO_CLK_ENABLE();
  AD7705_SPI1_MISO_MOSI_GPIO_CLK_ENABLE();

  /* Configure SPI SCK */
  gpioinitstruct.Pin        = AD7705_SPI1_SCK_PIN;
  gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
  gpioinitstruct.Pull       = GPIO_PULLUP;
  gpioinitstruct.Speed      = GPIO_SPEED_HIGH;
  gpioinitstruct.Alternate  = AD7705_SPI1_SCK_AF;
  HAL_GPIO_Init(AD7705_SPI1_SCK_GPIO_PORT, &gpioinitstruct);

  /* Configure SPI MISO and MOSI */ 
  gpioinitstruct.Pin        = AD7705_SPI1_MOSI_PIN;
  gpioinitstruct.Alternate  = AD7705_SPI1_MISO_MOSI_AF;
  gpioinitstruct.Pull       = GPIO_PULLDOWN;
  HAL_GPIO_Init(AD7705_SPI1_MISO_MOSI_GPIO_PORT, &gpioinitstruct);
  
  gpioinitstruct.Pin        = AD7705_SPI1_MISO_PIN;
  HAL_GPIO_Init(AD7705_SPI1_MISO_MOSI_GPIO_PORT, &gpioinitstruct);

  
  /*** Configure the SPI peripheral ***/ 
  /* Enable SPI clock */
  AD7705_SPI1_CLK_ENABLE();
}

/**
  * @brief  Initializes SPI HAL.
  * @retval None
  */
static void SPI1_Init(void)
{
  if(HAL_SPI_GetState(&had7705_Spi) == HAL_SPI_STATE_RESET)
  {
    /* SPI Config */
    had7705_Spi.Instance = AD7705_SPI1;
      /* SPI baudrate is set to 8 MHz maximum (PCLK2/SPI_BaudRatePrescaler = 32/4 = 8 MHz) 
       to verify these constraints:
          - PCLK2 max frequency is 32 MHz 
       */
    //had7705_Spi.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_4;
    had7705_Spi.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_8;
    had7705_Spi.Init.Direction          = SPI_DIRECTION_2LINES;
    had7705_Spi.Init.CLKPhase           = SPI_PHASE_2EDGE;
    had7705_Spi.Init.CLKPolarity        = SPI_POLARITY_HIGH;					//ʱ����λ  ��λ
    had7705_Spi.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;	//ʧ��CRCУ��
    had7705_Spi.Init.CRCPolynomial      = 7;													//7λCRCУ���
    had7705_Spi.Init.DataSize           = SPI_DATASIZE_8BIT;					//����8λ����
    had7705_Spi.Init.FirstBit           = SPI_FIRSTBIT_MSB;						//���ݴ����ȴ���λ
    had7705_Spi.Init.NSS                = SPI_NSS_SOFT;
    had7705_Spi.Init.TIMode             = SPI_TIMODE_DISABLE;					//ʧ��TIMode
    had7705_Spi.Init.Mode               = SPI_MODE_MASTER;						//��������ģʽ
    
    SPI1_MspInit();
    HAL_SPI_Init(&had7705_Spi);
		DEBUG("SPI1 Init OK\r\n");
  }
}

/**
  * @brief  SPI Read 4 bytes from device
  * @retval Read data
*/
static uint32_t SPI1_Read(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t readvalue = 0;
  uint32_t writevalue = 0xFFFFFFFF;
  
  status = HAL_SPI_TransmitReceive(&had7705_Spi, (uint8_t*) &writevalue, (uint8_t*) &readvalue, 1, SpixTimeout);
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    SPI1_Error();
  }

  return readvalue;
}

/**
  * @brief  SPI Write a byte to device
  * @param  Value: value to be written
  * @retval None
  */
static void SPI1_Write(uint8_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit(&had7705_Spi, (uint8_t*) &Value, 1, SpixTimeout);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Execute user timeout callback */
    SPI1_Error();
  }
}

/**
  * @brief  SPI error treatment function
  * @retval None
  */
static void SPI1_Error (void)
{
  /* De-initialize the SPI communication BUS */
  HAL_SPI_DeInit(&had7705_Spi);

	DEBUG("SPI1_Error\r\n");
  /* Re-Initiaize the SPI communication BUS */
  SPI1_Init();
}
#endif


/******************************************************************************
                            LINK OPERATIONS
*******************************************************************************/

/********************************* LINK AD7705 ************************************/
/**
  * @brief  Initializes the AD7705 SPI . 
  *        
  * @retval None
  */
void AD7705_SPI_Init(void)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};
  
  /*------------ AD7705 SPI --------------*/
  /* AD7705 SPI Config */
  SPI1_Init();

  /* AD7705_CS_GPIO Periph clock enable */
  AD7705_GPIO_CLK_ENABLE();

 /*����AD7705��������RESET*/
	gpioinitstruct.Pin        = AD7705_RST;
  gpioinitstruct.Mode       = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull       = GPIO_NOPULL;
  gpioinitstruct.Speed      = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(AD7705_GPIO_PORT, &gpioinitstruct);
	
//		/*����AD7705��������DRDY*/
//	gpioinitstruct.Pin        = AD770_RDY;
//  gpioinitstruct.Mode       = GPIO_MODE_INPUT;
//  gpioinitstruct.Pull       = GPIO_NOPULL;
//  gpioinitstruct.Speed      = GPIO_SPEED_HIGH;
//  HAL_GPIO_Init(AD7705_GPIO_PORT, &gpioinitstruct);
	
	/*��AD7705��λ����Ϊ�ߵ�ƽ  ����λAD7705���ϵ�״̬*/
	HAL_GPIO_WritePin(AD7705_GPIO_PORT, AD7705_RST, GPIO_PIN_SET);
}



/***************************************************************************//**
 * @brief Writes data to SPI.
 *
 * @param data - Write data buffer:
 *               - first byte is the chip select number;
 *               - from the second byte onwards are located data bytes to write.
 * @param bytesNumber - Number of bytes to write.
 *
 * @return Number of written bytes.
*******************************************************************************/
unsigned char AD7705_SPI_Write(unsigned char* data,
                        unsigned char bytesNumber)
{
//    unsigned char chipSelect    = data[0];
    unsigned char writeData[4]  = {0, 0, 0, 0};
    unsigned char byte          = 0;
    
    for(byte = 0;byte < bytesNumber;byte ++)
    {
        writeData[byte] = data[byte + 1];
    }
    for(byte = 0;byte < bytesNumber;byte ++)
    {
       SPI1_Write(writeData[byte]);
    }

    return(bytesNumber);
}

/***************************************************************************//**
 * @brief Reads data from SPI.
 *
 * @param data - As an input parameter, data represents the write buffer:
 *               - first byte is the chip select number;
 *               - from the second byte onwards are located data bytes to write.
 *               As an output parameter, data represents the read buffer:
 *               - from the first byte onwards are located the read data bytes. 
 * @param bytesNumber - Number of bytes to write.
 *
 * @return Number of written bytes.
*******************************************************************************/
unsigned char AD7705_SPI_Read(unsigned char* data,
                       unsigned char bytesNumber)
{
//    unsigned char chipSelect    = data[0];
    unsigned char readData[4]	= {0, 0, 0, 0};
    unsigned char byte          = 0;
    
    
    for(byte = 0;byte < bytesNumber;byte ++)
    {
        readData[byte]=SPI1_Read();
    }
    for(byte = 0;byte < bytesNumber;byte ++)
    {
        data[byte] = readData[byte];
    }
    
	return(bytesNumber);
}
/***************************************************************************//**
 * @brief Initializes the AD7705 and checks if the device is present.
 *
 * @param None.
 *
 * @return status - Result of the initialization procedure.
 *                  Example: 1 - if initialization was successful (ID is 0x0B).
 *                           0 - if initialization was unsuccessful.
*******************************************************************************/
unsigned char AD7705_Init(void)
{ 
	unsigned char status = 0x1;
        
        AD7705_SPI_Init();  /* Init SPI */
        
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);			//PA3=0����ʹAD7705��λ���ϵ�״̬
        
        status=1;
	
	return(status);
}

/***************************************************************************//**
 * @brief Sends 32 consecutive 1's on SPI in order to reset the part.
 *
 * @param None. �ڽӿ����ж�ʧ������£������DIN�ߵ�ƽ��д�����������㹻����ʱ��
 *������32������ʱ�����ڣ���AD7705����ص�Ĭ��״̬
 *
 * @return  None.    
*******************************************************************************/
void AD7705_Reset(void)
{
	unsigned char dataToSend[5] = {0x03, 0xff, 0xff, 0xff, 0xff};   
	AD7705_SPI_Write(dataToSend,4);
}
/***************************************************************************//**
 * @brief Reads the value of the selected register
 *
 * @param regAddress - The address of the register to read.
 * @param size - The size of the register to read.
 *
 * @return data - The value of the selected register register.
*******************************************************************************/
unsigned long AD7705_GetRegisterValue(unsigned char regAddress, unsigned char size,unsigned char channel)
{
	unsigned char data[5] = {0x03, 0x00, 0x00, 0x00, 0x00};
	unsigned long receivedData = 0x00;	
	data[1] = AD7705_COMM_READ |  AD7705_COMM_ADDR(regAddress) | AD7705_COMM_CHAN(channel);

	AD7705_SPI_Write(data,1);			//дdata[0] = 0x03����ͨ�żĴ���д��0x03����һ���Ƕ�ͨ�żĴ�����д������ch0 =1��ch1=1��������дdata[1]
																//����ͨ�żĴ���д����һ�����������ݣ����ĸ��Ĵ������ж����ݡ�ͨ���Ƕ���
	
	AD7705_SPI_Read(data,size);		//��Ҫ���ļĴ����������������Ƕ���

	if(size == 1)
	{
		receivedData += (data[0] << 0);
	}
	if(size == 2)
	{
		receivedData += (data[0] << 8);
		receivedData += (data[1] << 0);
	}
	if(size == 3)
	{
		receivedData += (data[0] << 16);
		receivedData += (data[1] << 8);
		receivedData += (data[2] << 0);
	}
    return receivedData;
}
/***************************************************************************//**
 * @brief Writes the value to the register
 *
 * @param -  regAddress - The address of the register to write to.
 * @param -  regValue - The value to write to the register.
 * @param -  size - The size of the register to write.
 *
 * @return  None.    
*******************************************************************************/
void AD7705_SetRegisterValue(unsigned char regAddress,
                             unsigned long regValue, 
                             unsigned char size,
                             unsigned char channel)
{
	unsigned char data[5] = {0x03, 0x00, 0x00, 0x00, 0x00};	
	data[1] = AD7705_COMM_WRITE |  AD7705_COMM_ADDR(regAddress) | AD7705_COMM_CHAN(channel);//дĳ���Ĵ����Ĳ���
    if(size == 1)
    {
        data[2] = (unsigned char)regValue;
    }
    if(size == 2)
    {
				data[3] = (unsigned char)((regValue & 0x0000FF) >> 0);
        data[2] = (unsigned char)((regValue & 0x00FF00) >> 8);
    }
    if(size == 3)
    {
				data[4] = (unsigned char)((regValue & 0x0000FF) >> 0);
				data[3] = (unsigned char)((regValue & 0x00FF00) >> 8);
        data[2] = (unsigned char)((regValue & 0xFF0000) >> 16);
    }
//	AD7705_CS_LOW;	    
	AD7705_SPI_Write(data,(1 + size));
//	AD7705_CS_HIGH;

}
/***************************************************************************//**
 * @brief Reads /RDY bit of status reg.
 *
 * @param None.
 *
 * @return rdy	- 0 ���ܶ�ȡ���ݼĴ���������
 *              - 1 ���Զ�ȡ���ݼĴ���������
*******************************************************************************/
unsigned char AD7705_Ready(unsigned char channel)
{
    unsigned char rdy = 0;
    rdy = (AD7705_GetRegisterValue( AD7705_REG_COMM,1,channel) & 0x80);   //��ͨ�żĴ��������ݣ���Ҫ�Ƕ�ȡDRDY���ŵĵ�ƽ״̬ PA2=0���Զ����ݼĴ�����PA2=1���ܶ�ȡ���ݼĴ���
	
	return(!rdy);
}

/***************************************************************************//**
 * @brief  Waits for RDY .
 *
 * @return None.
*******************************************************************************/
__weak void AD7705_WaitRdy(unsigned char channel)  /* add __weak */
{
    volatile uint16_t timeout=50;
    
    while(timeout--)
    {
      if(AD7705_Ready(channel))
      {
        break;
      }
      else
      {
        AD7705_WAIT_MS(5);
      }
    }
}

/* AD7705 Measurement */
/***************************************************************************//**
 * @brief Returns the result of a single conversion.
 *
 * @param AD7705Channel - AD7705 Channel Selection.
 * @param gain - Gain.
 * @param buffer - Buffered Mode.
 *
 * @return regData - Result of a single analog-to-digital conversion.
*******************************************************************************/
unsigned long AD7705_SingleMeasurement(unsigned long AD7705Channel,unsigned long gain,unsigned long buffer)
{
  unsigned long value=0;
  unsigned long command=0;
  
	//ʹ����������͵�ƽ��AD7705���븴λ״̬
  /* Reset AD7705 to bring the SPI interface in a known state ʹAD7705�ظ��ϵ�״̬*/
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);			//PA3=0����ʹAD7705��λ���ϵ�״̬
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);				//AD7705��λ���ϵ�״̬��������PA3���ţ�������AD7705һֱ���ڸ�λ״̬
	
//	AD7705_Reset();
  
  /* AD7705 setup Ӳ���ϸ�AD7705�ṩ�ľ���Ϊ4.9152MHz����AD7705��ʱ�ӼĴ�����CLKDIVλ����Ϊ1*/
  /* Clock Rate ����ʱ�ӼĴ���*/
  command = AD7705_CLK_RATE(AD7705_RATE_50HZ) | AD7705_CLK_CLKDIV ;  /* use 50HZ ��ֹ��ʱ���� MCLKOUT ���������*/
	command &= 0x1f;			//ʱ�ӼĴ�����ZEROλ����Ϊ0
  AD7705_SetRegisterValue(
            AD7705_REG_CLOCK,
            command,
            1,
						AD7705Channel
    );
  
  
  /* Start Measurement ��ʼ����*/
  command  = AD7705_CONF_MODE(AD7705_MODE_CAL_SELF) | AD7705_CONF_UNIPOLAR;  //unipolar,Self-Calibration Mode�����Թ�������У׼
  command |= AD7705_CONF_GAIN(gain) | (AD7705_CONF_BUF(buffer));  //����gain��,buffer=0������ѹ��buffer=1��������
  AD7705_SetRegisterValue(
            AD7705_REG_CONF,
            command,
            1,
						AD7705Channel
    );
  
  AD7705_WaitRdy(AD7705Channel);
	DEBUG("AD7705 is ready\r\n");
  value = AD7705_GetRegisterValue(AD7705_REG_DATA, 2,AD7705Channel);
	DEBUG("value=%lu\r\n", value);
  
  return value;
}

