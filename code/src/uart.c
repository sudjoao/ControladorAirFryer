#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "crc16.h"
#include "vars.h"
#include "util.h"

void requestData(char code, char subcode, int uart0_filestream);
void sendInt(char code, char subcode, int num, int uart0_filestream);
void sendByte(char code, char subcode, char byte, int uart0_filestream);
void readOutput(char subcode, int uart0_filestream, unsigned char *output);
void handleUserCommand(int command, int uart0_filestream);
float getFloatOutput(unsigned char *buffer);
int getIntOutput(unsigned char *buffer);

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

void requestData(char code, char subcode, int uart0_filestream)
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
    sleep(1);
}

void sendByte(char code, char subcode, char byte, int uart0_filestream)
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

void sendInt(char code, char subcode, int num, int uart0_filestream)
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

void readOutput(char subcode, int uart0_filestream, unsigned char *output)
{
    if (uart0_filestream != -1)
    {
        int rx_length = read(uart0_filestream, (void *)output, 20);
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

void handleUserCommand(int command, int uart0_filestream)
{
    if (command == 0x01)
    {
        if (on != 1)
        {
            printf("Enviando comando de ligar\n");
            sendByte(0x16, 0xD3, 1, uart0_filestream);
            on = 1;
        }
    }
    else if (command == 0x02)
    {
        if (on != 0)
        {
            on = 0;
            printf("Enviando comando de desligar\n");
            sendByte(0x16, 0xD3, 0, uart0_filestream);
        }
    }
    else if (command == 0x03)
    {
        printf("Enviando comando de começar\n");
        running = 1;
        sendByte(0x16, 0xD5, 1, uart0_filestream);
    }
    else if (command == 0x04)
    {
        running = 0;
        printf("Enviando comando de terminar\n");
        sendByte(0x16, 0xD5, 0, uart0_filestream);
    }
    else if (command == 0x05)
    {
        current_time += 3;
        printf("Enviando comando de temperatura\n");
        sendInt(0x16, 0xD6, current_time, uart0_filestream);
    }
    else if (command == 0x06)
    {
        current_time -= 3;
        printf("Enviando comando de temperatura\n");
        sendInt(0x16, 0xD6, current_time, uart0_filestream);
    }
    else if (command == 0x07)
    {
        sendInt(0x16, 0xD3, 1, uart0_filestream);
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
    int i = 0;
    while (i < 50)
    {
        char code = getCode(option);
        char subcode = getSubCode(option);

        requestData(code, subcode, uart0_filestream);
        sleep(1);
        readOutput(subcode, uart0_filestream, output);
        if (subcode == 0xC3)
        {
            int command = getIntOutput(output);
            handleUserCommand(command, uart0_filestream);
        }
        sleep(1);
        i++;
    }
}