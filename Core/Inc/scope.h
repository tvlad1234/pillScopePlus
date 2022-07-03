#ifndef __SCOPE_H
#define __SCOPE_H

#define PIXDIV 16
#define XDIV 8
#define YDIV 6

#define CLOCKTIM_PRESC 0
#define SYSCLK_FREQ 96000000
#define BUFFER_LEN (2 * PIXDIV * XDIV)

#define UPPER_VOLTAGE (atten * 3.3)
#define LOWER_VOLTAGE (atten * -3.3)

#define RISING 1
#define FALLING 0

void scopeInit();
void scopeLoop();
void sample();

#endif