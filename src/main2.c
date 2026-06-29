#include "stm32l4xx_hal.h"
#include <string.h>

// LED1: PA5  (Nucleo green LD2)
// LED2: PA6  (Arduino D12)
// UART: USART2 — PA2 TX / PA3 RX (ST-LINK virtual COM, 115200 8N1)

static UART_HandleTypeDef huart2;

void SysTick_Handler(void) {
    HAL_IncTick();
}

static void gpio_init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &gpio);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_RESET);
}

static void uart_init(void) {
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

static void process_command(const char *cmd) {
    if      (strcmp(cmd, "led on 1")  == 0) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,  GPIO_PIN_SET);
    else if (strcmp(cmd, "led on 2")  == 0) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    else if (strcmp(cmd, "led off 1") == 0) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    else if (strcmp(cmd, "led off 2") == 0) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
}

int main(void) {
    HAL_Init();
    gpio_init();
    uart_init();

    char    buf[32];
    uint8_t idx = 0;
    uint8_t ch;

    while (1) {
        if (HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY) != HAL_OK)
            continue;

        if (ch == '\r' || ch == '\n') {
            if (idx > 0) {
                buf[idx] = '\0';
                process_command(buf);
                idx = 0;
            }
        } else if (idx < sizeof(buf) - 1) {
            buf[idx++] = ch;
        }
    }
}
