#include "vars.h"

float external_temp = 0.0, internal_temp = 0.0, reference_temp = 0.0;

int running = -1, uart0_filestream = -1, i2cFd = -1, current_time = 1, on = 0, should_run = 1, start_time = 0, not_read = 0;


void cleanVariables(){
    running = -1;
    current_time = 0;
    on = 0;
    start_time = 0;
    external_temp = 0.0;
    internal_temp = 0.0;
    reference_temp = 0.0;
}