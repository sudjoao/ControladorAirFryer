#ifndef UART_H_
#define UART_H_

void requestData(char code, char subcode);
void sendInt(char code, char subcode, int num);
void sendByte(char code, char subcode, char byte);
void readOutput(char subcode, unsigned char *output);
void handleUserCommand(int command);
float getFloatOutput(unsigned char *buffer);
int getIntOutput(unsigned char *buffer);
void requestAndSaveOutput(char code, char subcode, unsigned char *output, int should_retry);
void configUart();
void observerUserCommands();

#endif /* UART_H_ */