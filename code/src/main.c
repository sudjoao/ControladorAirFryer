#include <fcntl.h>
#include <signal.h>
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


pthread_t temperatureId;
pthread_t uartID;
pthread_t pidID;
pthread_t lcdID;
pthread_t timerID;

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
        run_pid();
        sleep(1);
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
    while (should_run && current_time)
    {
        if (start_time)
        {
            sleep(60);
            current_time -= 1;
        }
    }
    while(key){}
    key=1;
    sendInt(0x16, 0xD6, current_time);
    running = 0;
    key=0;
}

void stop_threads(int signal_number){
    printf("Desligando sistema...\n");
    pthread_cancel(temperatureId);
    pthread_cancel(uartID);
    pthread_cancel(pidID);
    pthread_cancel(lcdID);
    pthread_cancel(timerID);
    should_run = 0;
}

int main()
{
    signal(SIGINT, stop_threads);
    configUart();
    init_config();
    pthread_create(&temperatureId, NULL, temperature_loop, NULL);
    pthread_create(&uartID, NULL, uart_loop, NULL);
    pthread_create(&pidID, NULL, pid_loop, NULL);
    pthread_create(&lcdID, NULL, lcd_loop, NULL);
    pthread_create(&timerID, NULL, timer_loop, NULL);
    while(should_run){}
    should_run = 0;
    close(uart0_filestream);
    close(i2cFd);
    return 0;
}
