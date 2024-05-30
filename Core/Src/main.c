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

#include "cmsis_os.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <time.h>

#include "FreeRTOS.h"
#include "oled.h"
#include "queue.h"
#include "rust.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "task.h"
#include "u8g2.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WIDTH 128
#define HEIGHT 64
#define SCALE 4
#define W_SIZE WIDTH / SCALE
#define H_SIZE HEIGHT / SCALE
TaskHandle_t handleOled;
QueueHandle_t queueSwitch;

u8g2_t u8g2;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
u8 calc_around(u8 world[H_SIZE][W_SIZE], u16 x, u16 y) {
    u8 count = 0;
    for (i8 dy = -1; dy <= 1; ++dy) {
        for (i8 dx = -1; dx <= 1; ++dx) {
            if ((x + dx < 0 || x + dx >= W_SIZE || y + dy < 0 || y + dy >= H_SIZE) || (dx == 0 && dy == 0)) {
                continue;
            }

            count += (world[y + dy][x + dx] ? 1 : 0);

            if (count > 3) {
                return count;
            }
        }
    }

    return count;
}

void init_data(u8 world[H_SIZE][W_SIZE], u8 near[H_SIZE][W_SIZE]) {
    for (u16 y = 0; y < H_SIZE; ++y) {
        for (u16 x = 0; x < W_SIZE; ++x) {
            near[y][x] = 0;
            world[y][x] = (rand() % 5 < 1 ? 1 : 0);
        }
    }
}
/**********************
receive data from queue
success -> pdPASS(1) -> update OLED
fail -> pdFAIL(0) -> taskYIELD();
**********************/
void taskOLED(void *pvParm) {
    srand(time(NULL));
    u8g2Init(&u8g2);
    u8g2_ClearBuffer(&u8g2);
    u8g2_ClearDisplay(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_samim_16_t_all);
    u8 near[H_SIZE][W_SIZE];
    u8 world[H_SIZE][W_SIZE];
    init_data(world, near);
    u64 step = 0;
    u8 mode = 255;
    u32 xTicksToDelay = 500;
    while (1) {
        xQueueReceive(queueSwitch, &mode, 0);
        switch (mode) {
            case 0:
                step = 0;
                mode = 255;
                init_data(world, near);
                break;
            case 1:
                vTaskDelay(500);
                continue;
            case 2:
                xTicksToDelay += 50;
                mode = 255;
                break;
            case 3:
                if (xTicksToDelay <= 60)
                    xTicksToDelay = 10;
                else
                    xTicksToDelay -= 50;
                mode = 255;
                break;
        }
        u8g2_ClearBuffer(&u8g2);
        u32 survive_amount = 0;
        if (step != 0) {
            for (u16 y = 0; y < H_SIZE; ++y) {
                for (u16 x = 0; x < W_SIZE; ++x) {
                    near[y][x] = calc_around(world, x, y);
                }
            }
        }

        for (u16 y = 0; y < H_SIZE; ++y) {
            for (u16 x = 0; x < W_SIZE; ++x) {
                if (step != 0) {
                    u8 around_life = near[y][x];
                    if (world[y][x] && (around_life < 2 || around_life > 3)) {
                        world[y][x] = 0;
                    } else if (world[y][x] == 0 && around_life == 3) {
                        world[y][x] = 1;
                    }
                }

                if (world[y][x]) {
                    u8g2_DrawBox(&u8g2, x * SCALE, y * SCALE, SCALE, SCALE);
                }

                survive_amount += world[y][x];
            }
        }

        // No survive life
        if (survive_amount == 0) {
            step = 0;
            init_data(world, near);
        }

        ++step;
        u8g2_SendBuffer(&u8g2);
        vTaskDelay(xTicksToDelay);
    }
}

u8 is_stop = 0;
void HAL_GPIO_EXTI_Callback(u16 GPIO_Pin) {
    u8 mode = 255;

    switch (GPIO_Pin) {
        // sw1 PE3
        case GPIO_PIN_3: {
            if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == 1) {
                mode = 0;
                xQueueSendFromISR(queueSwitch, &mode, NULL);
            }
            break;
        }

        // sw2 PE4
        case GPIO_PIN_4: {
            if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == 1) {
                if (is_stop == 0) {
                    is_stop = 1;
                    mode = 1;
                } else {
                    is_stop = 0;
                    mode = 255;
                }
                xQueueSendFromISR(queueSwitch, &mode, NULL);
            }
            break;
        }

        // sw3 PE5
        case GPIO_PIN_5: {
            if (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == 1) {
                mode = 2;
                xQueueSendFromISR(queueSwitch, &mode, NULL);
            }
            break;
        }

        // sw4 PE6
        case GPIO_PIN_6: {
            if (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == 1) {
                mode = 3;
                xQueueSendFromISR(queueSwitch, &mode, NULL);
            }
            break;
        }

        default:
            mode = 255;
            xQueueSendFromISR(queueSwitch, &mode, NULL);
            break;
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

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
    MX_I2C1_Init();
    MX_USART3_UART_Init();
    /* USER CODE BEGIN 2 */
    queueSwitch = xQueueCreate(1, sizeof(u8));
    xTaskCreate(taskOLED, "OLED", 512, NULL, 1, &handleOled);

    vTaskStartScheduler();
    /* USER CODE END 2 */

    /* Init scheduler */
    osKernelInitialize();

    /* Call init function for freertos objects (in freertos.c) */
    MX_FREERTOS_Init();

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
