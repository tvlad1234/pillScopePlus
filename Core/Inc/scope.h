#ifndef __SCOPE_H
#define __SCOPE_H

#define CLOCKTIM_PRESC 0
#define SYSCLK_FREQ 96000000
#define BUFFER_LEN 200

#define UPPER_VOLTAGE (atten * 3.3)
#define LOWER_VOLTAGE (atten * -3.3)

#define RISING 1
#define FALLING 0

void scopeInit();
void scopeLoop();

#endif