################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../MDK-ARM/lab2_oled_example/cmsis_os2.o \
../MDK-ARM/lab2_oled_example/croutine.o \
../MDK-ARM/lab2_oled_example/event_groups.o \
../MDK-ARM/lab2_oled_example/freertos.o \
../MDK-ARM/lab2_oled_example/gpio.o \
../MDK-ARM/lab2_oled_example/heap_4.o \
../MDK-ARM/lab2_oled_example/i2c.o \
../MDK-ARM/lab2_oled_example/list.o \
../MDK-ARM/lab2_oled_example/main.o \
../MDK-ARM/lab2_oled_example/oled.o \
../MDK-ARM/lab2_oled_example/port.o \
../MDK-ARM/lab2_oled_example/queue.o \
../MDK-ARM/lab2_oled_example/startup_stm32f429xx.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_cortex.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_dma.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_dma_ex.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_exti.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_flash.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_flash_ex.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_flash_ramfunc.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_gpio.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_i2c.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_i2c_ex.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_msp.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_pwr.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_pwr_ex.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_rcc.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_rcc_ex.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_tim.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_tim_ex.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_timebase_tim.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_hal_uart.o \
../MDK-ARM/lab2_oled_example/stm32f4xx_it.o \
../MDK-ARM/lab2_oled_example/stream_buffer.o \
../MDK-ARM/lab2_oled_example/system_stm32f4xx.o \
../MDK-ARM/lab2_oled_example/tasks.o \
../MDK-ARM/lab2_oled_example/timers.o \
../MDK-ARM/lab2_oled_example/u8g2_arc.o \
../MDK-ARM/lab2_oled_example/u8g2_bitmap.o \
../MDK-ARM/lab2_oled_example/u8g2_box.o \
../MDK-ARM/lab2_oled_example/u8g2_buffer.o \
../MDK-ARM/lab2_oled_example/u8g2_button.o \
../MDK-ARM/lab2_oled_example/u8g2_circle.o \
../MDK-ARM/lab2_oled_example/u8g2_cleardisplay.o \
../MDK-ARM/lab2_oled_example/u8g2_d_memory.o \
../MDK-ARM/lab2_oled_example/u8g2_d_setup.o \
../MDK-ARM/lab2_oled_example/u8g2_font.o \
../MDK-ARM/lab2_oled_example/u8g2_fonts.o \
../MDK-ARM/lab2_oled_example/u8g2_hvline.o \
../MDK-ARM/lab2_oled_example/u8g2_input_value.o \
../MDK-ARM/lab2_oled_example/u8g2_intersection.o \
../MDK-ARM/lab2_oled_example/u8g2_kerning.o \
../MDK-ARM/lab2_oled_example/u8g2_line.o \
../MDK-ARM/lab2_oled_example/u8g2_ll_hvline.o \
../MDK-ARM/lab2_oled_example/u8g2_message.o \
../MDK-ARM/lab2_oled_example/u8g2_polygon.o \
../MDK-ARM/lab2_oled_example/u8g2_selection_list.o \
../MDK-ARM/lab2_oled_example/u8g2_setup.o \
../MDK-ARM/lab2_oled_example/u8log.o \
../MDK-ARM/lab2_oled_example/u8log_u8g2.o \
../MDK-ARM/lab2_oled_example/u8log_u8x8.o \
../MDK-ARM/lab2_oled_example/u8x8_8x8.o \
../MDK-ARM/lab2_oled_example/u8x8_byte.o \
../MDK-ARM/lab2_oled_example/u8x8_cad.o \
../MDK-ARM/lab2_oled_example/u8x8_capture.o \
../MDK-ARM/lab2_oled_example/u8x8_d_ssd1306_128x64_noname.o \
../MDK-ARM/lab2_oled_example/u8x8_debounce.o \
../MDK-ARM/lab2_oled_example/u8x8_display.o \
../MDK-ARM/lab2_oled_example/u8x8_fonts.o \
../MDK-ARM/lab2_oled_example/u8x8_gpio.o \
../MDK-ARM/lab2_oled_example/u8x8_input_value.o \
../MDK-ARM/lab2_oled_example/u8x8_message.o \
../MDK-ARM/lab2_oled_example/u8x8_selection_list.o \
../MDK-ARM/lab2_oled_example/u8x8_setup.o \
../MDK-ARM/lab2_oled_example/u8x8_string.o \
../MDK-ARM/lab2_oled_example/u8x8_u16toa.o \
../MDK-ARM/lab2_oled_example/u8x8_u8toa.o \
../MDK-ARM/lab2_oled_example/usart.o 


# Each subdirectory must supply rules for building sources it contributes

