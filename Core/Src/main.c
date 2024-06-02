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
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "FreeRTOS.h"
#include "circular_queue.h"
#include "lcd.h"
#include "lcd_init.h"
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
u8 is_stop = 0;

struct OLED {
    u64 step;
    u16 delay;
    u8 circular;
};

TaskHandle_t handle_lcd;
TaskHandle_t handleOled;
QueueHandle_t queueSwitch;
QueueHandle_t queueOled;

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
u32 delay_fn(u16 x) {
    return (3000 * x) / (x + 30) + 10;
}

u64 compute_hash(bool array[H_SIZE][W_SIZE]) {
    u64 hash = 0x3a7eb429;
    for (int i = 0; i < H_SIZE; i++) {
        for (int j = 0; j < W_SIZE; j++) {
            hash = (hash >> 1) | (hash << (sizeof(u64) * 8 - 1));
            hash ^= array[i][j] * 0xee6b2807;
        }
    }

    hash *= 0xee6b2807;
    hash ^= hash >> 32;
    return hash;
}

u8 calc_around(bool world[H_SIZE][W_SIZE], u16 x, u16 y) {
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

void init_data(bool world[H_SIZE][W_SIZE]) {
    for (u16 y = 0; y < H_SIZE; ++y) {
        for (u16 x = 0; x < W_SIZE; ++x) {
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
    bool world[H_SIZE][W_SIZE];
    init_data(world);
    struct circular_queue history = circular_queue_new(5);
    u64 step = 0;
    u8 mode = 255;
    u16 delay = 6;
    struct OLED oled;
    oled.circular = 0;
    while (1) {
        xQueueReceive(queueSwitch, &mode, 0);
        switch (mode) {
            case 0:
                step = 0;
                mode = 255;
                oled.circular = 0;
                init_data(world);
                break;
            case 2:
                delay = delay == 3000 ? 3000 : delay + 1;
                mode = 255;
                break;
            case 3:
                delay = delay == 0 ? 0 : delay - 1;
                mode = 255;
                break;
        }

        if (is_stop) goto end;
        u8g2_ClearBuffer(&u8g2);
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

                if (!world[y][x]) continue;

                u8g2_DrawBox(&u8g2, x * SCALE, y * SCALE, SCALE, SCALE);
            }
        }

        ++step;
        u8g2_SendBuffer(&u8g2);
        circular_queue_push_back(&history, compute_hash(world));
        if (history.size >= 2) {
            u64 sample = circular_queue_get_back(&history, 0);
            for (u64 i = 1; i < history.size; ++i) {
                if (sample == circular_queue_get_back(&history, i)) {
                    oled.circular = i;
                    break;
                }
            }
        }
    end:
        oled.delay = delay_fn(delay);
        oled.step = step;
        xQueueSendFromISR(queueOled, &oled, NULL);
        vTaskDelay(oled.delay);
    }
}

void lcd_handle(void *pvParm) {
    LCD_Fill(0, 0, 240, 240, BLACK);
    struct OLED oled;
    u8 font_szie = 32;
    u8 step[20];
    u8 delay[20];
    u8 stop[20];
    u8 circular[20];
    while (1) {
        xQueueReceive(queueOled, &oled, 0);

        char temp[20];
        snprintf((char *)step, sizeof(step), "Step: %-10s", utoa(oled.step, temp, 10));
        snprintf((char *)delay, sizeof(delay), "Delay: %sms%-5s", utoa(oled.delay, temp, 10), "");
        snprintf((char *)stop, sizeof(stop), "Stop: %-10s", is_stop == 1 ? "True" : "False");
        snprintf((char *)circular, sizeof(circular), "Circular: %-6s", utoa(oled.circular, temp, 10));
        LCD_ShowString(0, 104 - font_szie, step, WHITE, BLACK, font_szie, 0);
        LCD_ShowString(0, 104, delay, WHITE, BLACK, font_szie, 0);
        LCD_ShowString(0, 104 + font_szie, stop, WHITE, BLACK, font_szie, 0);
        LCD_ShowString(0, 104 + font_szie * 2, circular, WHITE, BLACK, font_szie, 0);
        vTaskDelay(10);
    }
}

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
                is_stop = (is_stop == 0 ? 1 : 0);
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
    LCD_Address_Set(0, 0, 240, 240);

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
    LCD_Init();
    LCD_CS_Clr();

    queueSwitch = xQueueCreate(1, sizeof(u8));
    queueOled = xQueueCreate(1, sizeof(struct OLED));
    xTaskCreate(taskOLED, "OLED", 512, NULL, 1, &handleOled);
    xTaskCreate(lcd_handle, "LCD Display", 512, NULL, 1, &handle_lcd);

    vTaskStartScheduler();
    /* USER CODE END 2 */

    /* Init scheduler */
    osKernelInitialize();

    /* Call init function for freertos objects (in cmsis_os2.c) */
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
