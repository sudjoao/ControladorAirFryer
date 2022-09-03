#include <stdio.h>
#include <unistd.h>        
#include <fcntl.h>         
#include <termios.h>       
#include <string.h>
#include<stdlib.h>

#include "crc16.h"
#include "bme280.h"
#include "temperature_sensor.h"

char getCode(int option);
char getSubCode(int option);
char* getCodeMeaning(char subcode);
int contains(char item, char *list, int size);
void printOutput(char subcode, unsigned char* buffer, int size, int uart0_filestream);
void requestData(char code, char subcode, int uart0_filestream );
void sendByte(char code, char subcode, int uart0_filestream, char byte);
void readOutput(char subcode, int uart0_filestream);
void handleUserCommand(int command, int uart0_filestream);
struct bme280_dev bme_connection;
int temp = 0;
int ligado = -1;

char getCode(int option){
    return option < 4 ? 0x23 : 0x16;
}

char getSubCode(int option){
    if(option == 1){
        return 0xC1;
    }else if(option == 2){
        return 0xC2;
    }else if(option == 3){
        return 0xC3;
    }else if(option == 4){
        return 0xD1;
    }else if(option == 5){
        return 0xD2;
    }else if (option == 6){
        return 0xD3;
    }
    else if (option == 7){
        return 0xD5;
    }
    else{
        return 0xD6;
    }
}

char* getCodeMeaning(char subcode){
    if(subcode == 0xC1){
        return "Temperatura Interna";
    } else if(subcode == 0xC2){
        return "Temperatura de Referência";
    } else if(subcode == 0xC3){
        return "Comando de usuário recebido";
    } else return "Valor";
}

int contains(char item, char *list, int size){
    for(int i=0; i<size; i++){
        if(item == list[i])
            return 1;
    }
    return 0;
}

void printOutput(char subcode, unsigned char* buffer, int size, int uart0_filestream){
    char float_subcodes[2] = {0xC1, 0xC2};
    if(contains(subcode, float_subcodes, 2)){
        float output;
        memcpy(&output, &buffer[3], sizeof(float));
        if(subcode != 0xC3)
            printf("%s: %f\n",getCodeMeaning(subcode), output);
    }else {
        int output;
        memcpy(&output, &buffer[3], sizeof(int));
        if(subcode != 0xC3)
            printf("%s: %d\n",getCodeMeaning(subcode), output);
        if(subcode == 0xC3){
            handleUserCommand(output, uart0_filestream);
        }
    }

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


void sendByte(char code, char subcode, int uart0_filestream, char byte){
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

void sendInt(char code, char subcode, int uart0_filestream, int num){
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

void readOutput(char subcode, int uart0_filestream){
    if (uart0_filestream != -1)
    {
        unsigned char rx_buffer[20];
        int rx_length = read(uart0_filestream, (void*)rx_buffer, 20);     
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
            rx_buffer[rx_length] = '\0';
            printOutput(subcode, rx_buffer, rx_length, uart0_filestream);
        }
    }
}

void handleUserCommand(int command, int uart0_filestream){
    if(command == 0x01){
        if(ligado != 1){
            printf("Enviando comando de ligar");
            sendByte(0x16, 0xD3, uart0_filestream, 1);
            ligado = 1;
        }
    } else if(command == 0x02){
        if(ligado != 0){
            ligado = 0;
            printf("Enviando comando de desligar");
            sendByte(0x16, 0xD3, uart0_filestream, 0);
        }
    } else if(command == 0x03){
        printf("Enviando comando de começar");
        sendByte(0x16, 0xD5, uart0_filestream, 1);
    } else if(command == 0x04){
        printf("Enviando comando de terminar");
        sendByte(0x16, 0xD5, uart0_filestream, 0);
    } else if(command == 0x05){
        temp+=5;
        printf("Enviando comando de temperatura");
        sendInt(0x16, 0xD6, uart0_filestream, temp);
    } else if(command == 0x06){
        temp-=5;
        printf("Enviando comando de temperatura");
        sendInt(0x16, 0xD6, uart0_filestream, temp);
    } else if(command == 0x07){
        sendInt(0x16, 0xD3, uart0_filestream, 1);
    }
}

int main(int argc, const char * argv[]) {
    int option;
    int debug = 1;
    int uart0_filestream = -1;
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    option = 3;
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
    // initialize_temperature(&bme_connection);
    while(debug>0){
        // stream_sensor_data_forced_mode(&bme_connection);
        char code = getCode(option); 
        char subcode = getSubCode(option);

        requestData(code, subcode, uart0_filestream);
        delay(500);
        readOutput(subcode, uart0_filestream);
        delay(500);
    }
    close(uart0_filestream);
   return 0;
}
