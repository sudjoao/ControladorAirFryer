#include <stdio.h>
#include <unistd.h>        
#include <fcntl.h>         
#include <termios.h>       
#include <string.h>
#include "crc16.h"

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

void debugMenu(){
    printf("Digite a opção desejada:\n");
    printf("1. Solicitar Temperatura Interna\n");
    printf("2. Solicita Temperatura de Referência\n");
    printf("3. Lê comandos do usuário\n");
    printf("4. Envia sinal de controle\n");
    printf("5. Envia sinal de Referência\n");
    printf("6. Envia Estado do Sistema\n");
    printf("7. Envia Estado de Funcionamento\n");
    printf("8. Envia valor do Temporizador\n");
}

void printOutput(int option, unsigned char* buffer, int size){}

void sendMessage(char code, char subcode, int uart0_filestream ){
    unsigned char infos[7] = {0x01, code, subcode, 3, 9, 1, 0};
    unsigned char tx_buffer[20];
    short crc = calcula_CRC(infos, 7);
    memcpy(tx_buffer, &infos, 7);
    memcpy(&tx_buffer[7], &crc, 2);
    printf("Buffers de memória criados!\n");
    
    if (uart0_filestream != -1)
    {
        printf("Escrevendo caracteres na UART ...\n");
        int count = write(uart0_filestream, &tx_buffer[0], 9);
        if (count < 0)
        {
            printf("UART TX error\n");
        }
        else
        {
            printf("escrito.\n");
        }
    }
}

void readOutput(int uart0_filestream){
    if (uart0_filestream != -1)
    {
        printf("tentando ler\n");
        unsigned char rx_buffer[256];
        int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);     
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
            printf("Consegui ler =)\n");
        }
    }else{
        printf("Opaaaaaaa");
    }
}


int main(int argc, const char * argv[]) {
    int option;
    int uart0_filestream = -1;
    do {
        debugMenu();
        scanf("%d", &option);


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

        char code = getCode(option); 
        char subcode = getSubCode(option);

        sendMessage(code, subcode, uart0_filestream);
        sleep(1);
        readOutput(uart0_filestream);
        
    } while(option != 0);
    close(uart0_filestream);
   return 0;
}
