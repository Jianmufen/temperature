/**
  ******************************************************************************
  * @file    stm32l152c_discovery.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    09-October-2015
  * @brief   This file contains definitions for STM32L152C-Discovery Leds, push-buttons
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32L152C_DISCOVERY
  * @{
  */ 
  
/** @addtogroup STM32L152C_Discovery_Common
  * @{
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L152C_DISCOVERY_H
#define __STM32L152C_DISCOVERY_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

  
/** @defgroup STM32L152C_DISCOVERY_Exported_Types Exported Types
  * @{
  */
typedef enum 
{
  LED3 = 0,
  LED4 = 1,
  
  LED_GREEN = LED3,
  LED_BLUE = LED4
    
} Led_TypeDef;

typedef enum 
{
  BUTTON_USER = 0,
} Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;
/**
  * @}
  */ 

/** @defgroup STM32L152C_DISCOVERY_Exported_Constants Exported Constants
  * @{
  */ 

/** 
  * @brief  Define for STM32L152C-Discovery board
  */ 
#if !defined (USE_STM32L152C_DISCO)
 #define USE_STM32L152C_DISCO
#endif
  
/** @addtogroup STM32L152C_DISCOVERY_LED LED Constants
  * @{
  */
#define LEDn                             2

#define LED3_PIN                         GPIO_PIN_7           /* PB.07 */
#define LED3_GPIO_PORT                   GPIOB
#define LED3_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()  
#define LED3_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()  

#define LED4_PIN                         GPIO_PIN_6           /* PB.06 */
#define LED4_GPIO_PORT                   GPIOB
#define LED4_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()  
#define LED4_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()  

#define LEDx_GPIO_CLK_ENABLE(__INDEX__)   do { if ((__INDEX__) == 0) LED3_GPIO_CLK_ENABLE(); else LED4_GPIO_CLK_ENABLE();} while(0)

#define LEDx_GPIO_CLK_DISABLE(__INDEX__)  (((__INDEX__) == 0) ? LED3_GPIO_CLK_DISABLE() : LED4_GPIO_CLK_DISABLE())
/**
  * @}
  */
  
/** @addtogroup STM32L152C_DISCOVERY_BUTTON BUTTON Constants
  * @{
  */  
#define BUTTONn                          1
/**
 * @brief USER push-button
 */
#define USER_BUTTON_PIN                   GPIO_PIN_0           /* PA.00 */
#define USER_BUTTON_GPIO_PORT             GPIOA
#define USER_BUTTON_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define USER_BUTTON_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define USER_BUTTON_EXTI_IRQn             EXTI0_IRQn

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__)    do { USER_BUTTON_GPIO_CLK_ENABLE();} while(0)

#define BUTTONx_GPIO_CLK_DISABLE(__INDEX__)   (USER_BUTTON_GPIO_CLK_DISABLE())

/**
  * @}
  */ 

/**
  * @}
  */


/** @addtogroup STM32L152C_DISCOVERY_Exported_Functions
  * @{
  */ 
uint32_t  BSP_GetVersion(void);

/** @addtogroup STM32152C_DISCOVERY_LED_Functions
  * @{
  */ 
void      BSP_LED_Init(Led_TypeDef Led);
void      BSP_LED_On(Led_TypeDef Led);
void      BSP_LED_Off(Led_TypeDef Led);
void      BSP_LED_Toggle(Led_TypeDef Led);

/**
  * @}
  */

/** @addtogroup STM32152C_DISCOVERY_BUTTON_Functions
  * @{
  */

void      BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef Mode);
uint32_t  BSP_PB_GetState(Button_TypeDef Button);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */ 
  
#ifdef __cplusplus
}
#endif
  
#endif /* __STM32L152C_DISCOVERY_H */

/**
  * @}
  */

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
