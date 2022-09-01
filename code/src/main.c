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

void sendMessage(char code, char subcode, int uart0_filestream ){
    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;
    int i =0;
    printf("hmm 2\n");
    p_tx_buffer = &tx_buffer[0];
    tx_buffer[i++] = 0x01;
    tx_buffer[i++] = code;
    tx_buffer[i++] = subcode;
    tx_buffer[i++] = 3;
    tx_buffer[i++] = 9;
    tx_buffer[i++] = 1;
    tx_buffer[i++] = 0;
    unsigned short crc = calcula_CRC(tx_buffer, sizeof(tx_buffer));
    tx_buffer[i++] = crc;
    printf("Buffers de memória criados!\n");
    
    if (uart0_filestream != -1)
    {
        printf("Escrevendo caracteres na UART ...");
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));
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

void printOutput(int option, unsigned char* buffer, int size){}

int main(int argc, const char * argv[]) {

    int uart0_filestream = -1;
    int option;
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
    printf("hmm\n");
    sendMessage(code, subcode, uart0_filestream);

    sleep(1);

    // if (uart0_filestream != -1)
    // {
    
    //     unsigned char rx_buffer[256];
    //     int data;
    //     int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);     
    //     if (rx_length < 0)
    //     {
    //         printf("Erro na leitura.\n");
    //     }
    //     else if (rx_length == 0)
    //     {
    //         printf("Nenhum dado disponível.\n");
    //     }
    //     else
    //     {
    //         printOutput(option, rx_buffer, rx_length);
    //     }
    // }

    close(uart0_filestream);
   return 0;
}
