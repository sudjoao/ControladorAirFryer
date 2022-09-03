#include <fcntl.h>         
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>       
#include <unistd.h>     
  
#include "crc16.h"

void requestData(char code, char subcode , int uart0_filestream);
void sendInt(char code, char subcode, int num, int uart0_filestream);
void sendByte(char code, char subcode, char byte, int uart0_filestream);
void readOutput(char subcode, int uart0_filestream, unsigned char *output);
void handleUserCommand(int command, int uart0_filestream, int *additional_info);
float getFloatOutput(unsigned char *buffer);
int getIntOutput(unsigned char *buffer);

float getFloatOutput(unsigned char *buffer){
    float output;
    memcpy(&output, &buffer[3], sizeof(float));
    return output;
}

int getIntOutput(unsigned char *buffer){
    int output;
    memcpy(&output, &buffer[3], sizeof(int));
    return output;
}

void requestData(char code, char subcode, int uart0_filestream){
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

void sendByte(char code, char subcode, char byte, int uart0_filestream){
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

void sendInt(char code, char subcode, int num, int uart0_filestream){
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

void readOutput(char subcode, int uart0_filestream, unsigned char *output){
    if (uart0_filestream != -1)
    {
        int rx_length = read(uart0_filestream, (void*)output, 20);     
        if (rx_length < 0)
        {
            printf("Erro na leitura.\n");
        }
        else if (rx_length == 0)
        {
            printf("Nenhum dado disponível.\n");
        }
        else
        {
            output[rx_length] = '\0';
        }
    }
}

void handleUserCommand(int command, int uart0_filestream, int *additional_info){
    if(command == 0x01){
        if(*additional_info != 1){
            printf("Enviando comando de ligar");
            sendByte(0x16, 0xD3, 1, uart0_filestream);
            *additional_info = 1;
        }
    } else if(command == 0x02){
        if(*additional_info != 0){
            *additional_info = 0;
            printf("Enviando comando de desligar");
            sendByte(0x16, 0xD3, 0, uart0_filestream);
        }
    } else if(command == 0x03){
        printf("Enviando comando de começar");
        sendByte(0x16, 0xD5, 1, uart0_filestream);
    } else if(command == 0x04){
        printf("Enviando comando de terminar");
        sendByte(0x16, 0xD5, 0, uart0_filestream);
    } else if(command == 0x05){
        *additional_info+=5;
        printf("Enviando comando de temperatura");
        sendInt(0x16, 0xD6, *additional_info, uart0_filestream);
    } else if(command == 0x06){
        *additional_info-=5;
        printf("Enviando comando de temperatura");
        sendInt(0x16, 0xD6, *additional_info, uart0_filestream);
    } else if(command == 0x07){
        sendInt(0x16, 0xD3, 1, uart0_filestream);
    }
}
