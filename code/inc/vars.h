#ifndef VARS_H_
#define VARS_H_

extern float external_temp, internal_temp, reference_temp;
extern int running, uart0_filestream, i2cFd, current_time, on, should_run, start_time, not_read, key;

void cleanVariables();
#endif /* VARS_H_ */