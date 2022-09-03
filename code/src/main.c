#include <fcntl.h>         
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>       
#include <unistd.h>        

#include "bme280.h"
#include "crc16.h"
#include "pid.h"
#include "temperature_sensor.h"
#include "uart.h"
#include "util.h"

struct bme280_dev bme_connection;
int temp = 0;
int ligado = -1;
int uart0_filestream = -1;



int main(int argc, const char * argv[]) {
    int option;
    int debug = 1;
    unsigned char output[20];
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
        sleep(1);
        readOutput(subcode, uart0_filestream,  output);
        if(subcode == 0xC3){
            int *additional_info;
            int command =getIntOutput(output);
            if(command == 0x01 || command == 0x02){
                additional_info = &ligado;
            } else {
                additional_info = &temp;
            }

            handleUserCommand(command, uart0_filestream, additional_info);
        }
        sleep(1);
    }
    close(uart0_filestream);
   return 0;
}
