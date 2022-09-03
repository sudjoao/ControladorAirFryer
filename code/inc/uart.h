#ifndef UART_H_
#define UART_H_

void requestData(char code, char subcode , int uart0_filestream);
void sendInt(char code, char subcode, int num, int uart0_filestream);
void sendByte(char code, char subcode, char byte, int uart0_filestream);
void readOutput(char subcode, int uart0_filestream, unsigned char *output);
void handleUserCommand(int command, int uart0_filestream, int *additional_info);
float getFloatOutput(unsigned char *buffer);
int getIntOutput(unsigned char *buffer);


#endif /* UART_H_ */