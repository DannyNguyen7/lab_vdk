/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "led7seg.h"
#include "timer.h"  // Include your custom timer header

/* Private define ------------------------------------------------------------*/
#define LED_BLINK_INTERVAL 1000    // 1 second (1000ms)
#define SEGMENT_SWITCH_INTERVAL 500 // 500ms for switching between 7-segment displays

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
int current_display = 0; // Tracks the current 7-segment display (0-3)

/* USER CODE BEGIN PV */
int led_toggle = 0; // Toggle flag for LED
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN 0 */

// Timer interrupt callback function
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timer_run();  // Custom timer logic from timer_run()
        
        // Check if it's time to toggle the LEDs (1-second interval)
        if (timerFLag[0] == 1)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4); // Toggle LEDs connected to PA4
            set_timer(0, LED_BLINK_INTERVAL);      // Reset timer for the next blink
            timerFLag[0] = 0;                      // Reset the flag
        }

        // Check if it's time to switch between the 7-segment displays (500ms interval)
        if (timerFLag[1] == 1)
        {
            
            HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_RESET);

            // Switch between displays
            switch (current_display)
            {
                case 0:
                    HAL_GPIO_WritePin(GPIOA, EN0_Pin, GPIO_PIN_SET); // Enable EN0 (first display)
                    display7SEG(1);  // Show number 1 on the first display
                    current_display = 1;
                    break;
                case 1:
                    HAL_GPIO_WritePin(GPIOA, EN1_Pin, GPIO_PIN_SET); // Enable EN1 (second display)
                    display7SEG(2);  // Show number 2 on the second display
                    current_display = 2;
                    break;
                case 2:
                    HAL_GPIO_WritePin(GPIOA, EN2_Pin, GPIO_PIN_SET); // Enable EN2 (third display)
                    display7SEG(3);  // Show number 3 on the third display
                    current_display = 3;
                    break;
                case 3:
                    HAL_GPIO_WritePin(GPIOA, En3_Pin, GPIO_PIN_SET); // Enable En3 (fourth display)
                    display7SEG(0);  // Show number 0 on the fourth display
                    current_display = 0;
                    break;
            }

            set_timer(1, SEGMENT_SWITCH_INTERVAL); // Reset timer for the next segment switch
            timerFLag[1] = 0;                      // Reset the flag
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    /* Start Timer Interrupt */
    HAL_TIM_Base_Start_IT(&htim2);

    /* Initialize the timers */
    set_timer(0, LED_BLINK_INTERVAL);    // Timer for LED blinking (1 second)
    set_timer(1, SEGMENT_SWITCH_INTERVAL); // Timer for 7-segment display switching (500ms)

    /* Infinite loop */
    while (1)
    {
        // The logic is entirely handled in the timer interrupt
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7999;  // Adjust based on your system clock
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 9;  // Timer interrupt fires every 1 ms (tick every 1ms)
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure GPIOA: DOT_Pin, EN0_Pin, EN1_Pin, EN2_Pin, En3_Pin
    GPIO_InitStruct.Pin = DOT_Pin | GPIO_PIN_4 | EN0_Pin | EN1_Pin | EN2_Pin | En3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure GPIOB: seg0_Pin to seg6_Pin
    GPIO_InitStruct.Pin = seg0_Pin | seg1_Pin | seg2_Pin | seg3_Pin | seg4_Pin | seg5_Pin | seg6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
