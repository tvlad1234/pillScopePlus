#ifndef _SCPI_INSTR_H
#define _SCPI_INSTR_H

#include "scpi/scpi.h"

#define SCPI_INPUT_BUFFER_LENGTH 256
#define SCPI_ERROR_QUEUE_SIZE 17
#define SCPI_IDN1 "Pillscope"
#define SCPI_IDN2 "Plus"
#define SCPI_IDN3 "001"
#define SCPI_IDN4 __TIMESTAMP__

extern const scpi_command_t scpi_commands[];
extern scpi_interface_t scpi_interface;
extern char scpi_input_buffer[];
extern scpi_error_t scpi_error_queue_data[];
extern scpi_t scpi_context;

#endif