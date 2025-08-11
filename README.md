# FreeRTOS based AVR library for I2C bus

## Features

1. Simple read and write data.

## Dependencies

1. [zh_avr_free_rtos](http://git.zh.com.ru/avr_libraries/zh_avr_free_rtos)
2. [zh_avr_common](http://git.zh.com.ru/avr_libraries/zh_avr_common)

## Using

In an existing project, run the following command to install the component:

```text
cd ../your_project/lib
git clone http://git.zh.com.ru/avr_libraries/zh_avr_free_rtos
git clone http://git.zh.com.ru/avr_libraries/zh_avr_i2c
git clone http://git.zh.com.ru/avr_libraries/zh_avr_common
```

In the application, add the component:

```c
#include "zh_avr_i2c.h"
```

## Example

Master read and write data:

```c
#include "avr/io.h"
#include "stdio.h"
#include "zh_avr_i2c.h"

#define BAUD_RATE 9600
#define BAUD_PRESCALE (F_CPU / 16 / BAUD_RATE - 1)

int usart(char byte, FILE *stream)
{
    while ((UCSR0A & (1 << UDRE0)) == 0)
    {
    }
    UDR0 = byte;
    return 0;
}
FILE uart = FDEV_SETUP_STREAM(usart, NULL, _FDEV_SETUP_WRITE);

void i2c_example_task(void *pvParameters)
{
    zh_avr_i2c_master_init(false);
    for (;;)
    {
        avr_err_t err = zh_avr_i2c_master_probe(0x38, 100 / portTICK_PERIOD_MS);
        if (err == AVR_OK)
        {
            uint8_t data_send = 111;
            uint8_t data_read = 0;
            printf("Data Send %d.\n", data_send);
            zh_avr_i2c_master_transmit(0x38, (uint8_t *)&data_send, sizeof(data_send), 100 / portTICK_PERIOD_MS);
            zh_avr_i2c_master_receive(0x38, (uint8_t *)&data_read, sizeof(data_read), 100 / portTICK_PERIOD_MS);
            printf("Data Read %d.\n", data_read);
            data_send = 55;
            printf("Data Send %d.\n", data_send);
            zh_avr_i2c_master_transmit(0x38, (uint8_t *)&data_send, sizeof(data_send), 100 / portTICK_PERIOD_MS);
            zh_avr_i2c_master_receive(0x38, (uint8_t *)&data_read, sizeof(data_read), 100 / portTICK_PERIOD_MS);
            printf("Data Read %d.\n", data_read);
            data_send = 14;
            printf("Data Send %d.\n", data_send);
            zh_avr_i2c_master_transmit(0x38, (uint8_t *)&data_send, sizeof(data_send), 100 / portTICK_PERIOD_MS);
            zh_avr_i2c_master_receive(0x38, (uint8_t *)&data_read, sizeof(data_read), 100 / portTICK_PERIOD_MS);
            printf("Data Read %d.\n", data_read);
        }
        else
        {
            printf("Device Not Found.\n");
        }
        printf("Task Remaining Stack Size %d.\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

int main(void)
{
    UBRR0H = (BAUD_PRESCALE >> 8);
    UBRR0L = BAUD_PRESCALE;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    stdout = &uart;
    xTaskCreate(i2c_example_task, "i2c example task", 110, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    return 0;
}
```
