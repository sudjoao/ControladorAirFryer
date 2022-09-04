#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

#include "bme280.h"
#include "crc16.h"
#include "pid.h"
#include "temperature_sensor.h"
#include "uart.h"
#include "util.h"
#include "lcd.h"
#include "vars.h"
#include "gpio.h"

void *uart_loop(void *args)
{
    observerUserCommands();
    return NULL;
}

void *temperature_loop(void *args)
{
    initializeTemperatureSensor();
    return NULL;
}

void *pid_loop(void *args)
{
    int i = 0;
    while (i < 100 && should_run)
    {
        printf("Pid loop: %d\n", i);
        run_pid();
        sleep(2);
        i++;
    }
    return NULL;
}

void *lcd_loop(void *args)
{
    lcd_config();
    return NULL;
}

void *timer_loop(void *args)
{
    printf("Start: %d\n", start_time);
    while (1 && should_run && current_time)
    {
        if (start_time)
        {
            printf("Time: %d\n", current_time);
            sleep(60);
            current_time -= 1;
        }
    }
}

int main()
{
    // lcd_config();
    configUart();
    init_config();
    pthread_t temperatureId;
    pthread_create(&temperatureId, NULL, temperature_loop, NULL);
    pthread_t uartID;
    pthread_create(&uartID, NULL, uart_loop, NULL);
    pthread_t pidID;
    pthread_create(&pidID, NULL, pid_loop, NULL);
    pthread_t lcdID;
    pthread_create(&lcdID, NULL, lcd_loop, NULL);
    pthread_t timerID;
    pthread_create(&timerID, NULL, timer_loop, NULL);
    sleep(60);
    should_run = 0;
    close(uart0_filestream);
    close(i2cFd);
    return 0;
}
