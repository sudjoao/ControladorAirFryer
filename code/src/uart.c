#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "crc16.h"
#include "vars.h"
#include "util.h"
#include "gpio.h"

void requestData(char code, char subcode);
void sendInt(char code, char subcode, int num);
void sendByte(char code, char subcode, char byte);
void readOutput(char subcode, unsigned char *output);
void handleUserCommand(int command);
float getFloatOutput(unsigned char *buffer);
int getIntOutput(unsigned char *buffer);
void requestAndSaveOutput(char code, char subcode, unsigned char *output, int should_retry);

float getFloatOutput(unsigned char *buffer)
{
    float output;
    memcpy(&output, &buffer[3], sizeof(float));
    return output;
}

int getIntOutput(unsigned char *buffer)
{
    int output;
    memcpy(&output, &buffer[3], sizeof(int));
    return output;
}

void requestData(char code, char subcode)
{
    unsigned char infos[7] = {0x01, code, subcode, 3, 9, 1, 0};
    unsigned char tx_buffer[20];
    short crc = calcula_CRC(infos, 7);
    memcpy(tx_buffer, &infos, 7);
    memcpy(&tx_buffer[7], &crc, 2);

    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, &tx_buffer[0], 9);
        if (count < 0)
        {
            printf("UART TX error\n");
        }
    }
}

void sendByte(char code, char subcode, char byte)
{
    unsigned char infos[7] = {0x01, code, subcode, 3, 9, 1, 0};
    unsigned char tx_buffer[20];
    memcpy(tx_buffer, &infos, 7);
    memcpy(&tx_buffer[7], &byte, 1);
    short crc = calcula_CRC(tx_buffer, 8);
    memcpy(&tx_buffer[8], &crc, 2);

    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, &tx_buffer[0], 10);
        if (count < 0)
        {
            printf("UART TX error\n");
        }
    }
}

void sendInt(char code, char subcode, int num)
{
    unsigned char infos[7] = {0x01, code, subcode, 3, 9, 1, 0};
    unsigned char tx_buffer[20];
    memcpy(tx_buffer, &infos, 7);
    memcpy(&tx_buffer[7], &num, 4);
    short crc = calcula_CRC(tx_buffer, 11);
    memcpy(&tx_buffer[11], &crc, 2);

    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, &tx_buffer[0], 13);
        if (count < 0)
        {
            printf("UART TX error\n");
        }
    }
}

void readOutput(char subcode, unsigned char *output)
{
    if (uart0_filestream != -1)
    {
        rx_length = read(uart0_filestream, (void *)output, 20);
        if (rx_length < 0)
        {
            not_read = 1;
            printf("Erro na leitura.\n");
        }
        else if (rx_length == 0)
        {
            not_read = 1;
            printf("Nenhum dado disponível.\n");
        }
        else
        {
            not_read = 0;
            output[rx_length] = '\0';
        }
    }
}

void handleUserCommand(int command)
{
    if (command == 0x01)
    {
        if (on != 1)
        {
            sendByte(0x16, 0xD3, 1);
            on = 1;
        }
    }
    else if (command == 0x02)
    {
        if (on != 0)
        {
            should_run = 0;
            on = 0;
            sendByte(0x16, 0xD3, 0);
            setFan(100);
            setResistance(0);
        }
    }
    else if (command == 0x03)
    {
        running = 1;
        sendByte(0x16, 0xD5, 1);
        sendInt(0x16, 0xD1, -100);
    }
    else if (command == 0x04)
    {
        running = 0;
        sendByte(0x16, 0xD5, 0);
        setFan(100);
        setResistance(0);
        sendInt(0x16, 0xD1, -100);
    }
    else if (command == 0x05)
    {
        current_time += 1;
        sendInt(0x16, 0xD6, current_time);
    }
    else if (command == 0x06)
    {
        current_time -= 1;
        sendInt(0x16, 0xD6, current_time);
    }
    else if (command == 0x07)
    {
        sendInt(0x16, 0xD3, 1);
    }
}

void configUart()
{
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
    }
    else
    {
        printf("UART inicializada!\n");
    }
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);
}

void observerUserCommands()
{
    int option;
    unsigned char output[20];
    option = 3;
    while (should_run)
    {
        while (key)
        {
        }
        key = 1;
        char code = getCode(option);
        char subcode = getSubCode(option);

        requestData(code, subcode);
        delay(500);
        readOutput(subcode, output);
        if (subcode == 0xC3)
        {
            int command = getIntOutput(output);
            handleUserCommand(command);
        }
        key = 0;
        delay(500);
    }
}


void requestAndSaveOutput(char code, char subcode, unsigned char *output, int should_retry){
    int retries_count = 0;
    do {
        requestData(code, subcode);
        delay(500);
        readOutput(subcode, output);
    }while(should_retry && retries_count++ < 10 && rx_length < 1);
}