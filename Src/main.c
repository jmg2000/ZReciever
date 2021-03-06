/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "crc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Exported macro ------------------------------------------------------------*/
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
/* Private defines -----------------------------------------------------------*/
#define PREAMBLE_MARK_TIME 4500
#define PREAMBLE_SPACE_TIME 4500
#define PREAMBLE_TIME (PREAMBLE_MARK_TIME + PREAMBLE_SPACE_TIME)
#define LOGIC_ONE_TIME 2150
#define LOGIC_ZERO_TIME 1300
#define MARK_TIME 650
#define SPACE_TIME_1 1500
#define SPACE_TIME_0 650

#define INTERVAL 250
#define INTERVAL_PREAMBLE 1500

#define MSG_LEN 5
#define CRC_IDX 4

/* Private variables ---------------------------------------------------------*/
volatile uint8_t    irState = 0;
volatile uint8_t    irAddr = 0;
volatile uint8_t    irLen = 0;
volatile uint8_t    irReceive[8];
volatile uint8_t    irMsg[MSG_LEN];
volatile uint8_t    irMsgIdx = 0;
volatile uint8_t    flgMsgReceive = 0;
volatile uint8_t    flgBadMsg = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void RF_SetReady(void);
uint8_t Crc8(uint8_t *pcBlock, uint16_t len);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	uint8_t CRCValue = 0;
	uint16_t ADC_Value = 0;
	float u = 0;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_CRC_Init();

  /* USER CODE BEGIN 2 */
	printf("Reciever started..\n\r");
	RF_SetReady();
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if (flgBadMsg) {
			RF_SetReady();
    }
    if (flgMsgReceive) {
			CRCValue = Crc8((uint8_t*)irMsg, MSG_LEN - 1);
			if (irMsg[CRC_IDX] == CRCValue) {
				ADC_Value = (uint16_t)(irMsg[1]<<8) + irMsg[2];
				u = ((float)ADC_Value)*3/4096;
				switch(irMsg[3]) {
					case 0:
						printf("device disconnect, ADC = %04d V=%.2fv - \n\r",  ADC_Value, u);
						break;
					case 1:
						printf("device connected, ADC = %04d V=%.2fv - \n\r",  ADC_Value, u);
						break;
					case 2:
						printf("device started..\n\r");
						break;
				}
				RF_SetReady();
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
      }
    }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
void RF_SetReady(void)
{
    flgBadMsg = 0;
    irLen = 0;
    irState = 0;
    flgMsgReceive = 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint16_t duration;
	
	if(GPIO_Pin == GPIO_PIN_2)
	{
		switch (irState) {
    case 0:
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        __HAL_TIM_ENABLE(&htim4);
        irState = 1;
        break;
    case 1:
        __HAL_TIM_DISABLE(&htim4);
        duration = __HAL_TIM_GET_COUNTER(&htim4);
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        __HAL_TIM_ENABLE(&htim4);
        if ((duration > PREAMBLE_TIME - INTERVAL_PREAMBLE)&&(duration < PREAMBLE_TIME + INTERVAL_PREAMBLE)) {
            irState = 2;
            irLen = 0;
        }
        else {
            flgBadMsg = 1;
            irState = 4;
        }
        break;
    case 2:
        __HAL_TIM_DISABLE(&htim4);
        duration = __HAL_TIM_GET_COUNTER(&htim4);
        if (duration < INTERVAL) {
            __HAL_TIM_ENABLE(&htim4);
            break;
        }
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        __HAL_TIM_ENABLE(&htim4);
        if ((duration > LOGIC_ONE_TIME - INTERVAL)&&(duration < LOGIC_ONE_TIME + INTERVAL)) {
            irReceive[irLen] = 1;
            irLen++;
        }
        else if ((duration > LOGIC_ZERO_TIME - INTERVAL)&&(duration < LOGIC_ZERO_TIME + INTERVAL)) {
            irReceive[irLen] = 0;
            irLen++;
        }
        else {
            flgBadMsg = 1;
            irState = 4;
            irLen = 0;
        }
            
        if (irLen == 8) {
                
            if (irReceive[0] == 6) irState = 0;
            if (irReceive[1] == 6) irState = 0;
            if (irReceive[2] == 6) irState = 0;
            if (irReceive[3] == 6) irState = 0;
            if (irReceive[4] == 6) irState = 0;
            if (irReceive[5] == 6) irState = 0;
            if (irReceive[6] == 6) irState = 0;
            if (irReceive[7] == 6) irState = 0;
                
            irMsg[irMsgIdx++] = (irReceive[0] << 7) + (irReceive[1] << 6) + (irReceive[2] << 5) + (irReceive[3] << 4) + (irReceive[4] << 3) + (irReceive[5] << 2) + 
																(irReceive[6] << 1) + irReceive[7];
                
            if (irMsgIdx == MSG_LEN) {
                __HAL_TIM_DISABLE(&htim4);
                __HAL_TIM_SET_COUNTER(&htim4, 0);
                irMsgIdx = 0;
                flgMsgReceive = 1;
                irState = 4;
            }
            else {
                irLen = 0;
            }
        }
        break;
    case 4:
        break;
    }
	}
}

uint8_t Crc8(uint8_t *pcBlock, uint16_t len)
{
    uint8_t crc = 0xFF;
    uint16_t i;

    while (len--)
    {
        crc ^= *pcBlock++;

        for (i = 0; i < 8; i++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }

    return crc;
}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF); 

  return ch;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
