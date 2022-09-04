#include <gpio.h>
#include <softPwm.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>

int init_config(){
    int setup = wiringPiSetup();
    if(setup == -1)
        return -1;

    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    softPwmCreate(4, 0, 100);
    softPwmCreate(5, 0, 100);
    return 0;
}

void setResistance(int potency) {
  softPwmWrite(4, potency);
}

void setFan(int potency) {
  softPwmWrite(5, potency);
}
