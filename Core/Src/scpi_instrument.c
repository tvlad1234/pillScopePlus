#include "main.h"
#include "scpi/scpi.h"
#include "scpi_instrument.h"
#include "scope.h"

static scpi_result_t My_CoreTstQ(scpi_t *context)
{
    SCPI_ResultInt32(context, 0);
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SetTdiv(scpi_t *context)
{
    double param1;
    if (!SCPI_ParamDouble(context, &param1, TRUE))
        return SCPI_RES_ERR;
    tdiv = param1;
    sampRate = (PIXDIV * 1000 * 1000) / tdiv;
    sampPer = tdiv / (float)PIXDIV;
    setTimerFreq(sampRate);
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SetVdiv(scpi_t *context)
{
    double param1;
    if (!SCPI_ParamDouble(context, &param1, TRUE))
        return SCPI_RES_ERR;
    vdiv = param1;
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SetAtten(scpi_t *context)
{
    int param1;
    if (!SCPI_ParamInt(context, &param1, TRUE))
        return SCPI_RES_ERR;
    atten = param1;
    return SCPI_RES_OK;
}

extern uint8_t autocalFlag;
scpi_result_t SCPI_Autocal(scpi_t *context)
{
    autocalFlag = 1;
    return SCPI_RES_OK;
}

scpi_result_t SCPI_MeasurePpkQ(scpi_t *context)
{
    float ppk = maxVoltage - minVoltage;
    char s[30];
    printFloat(ppk, 2, s);
    SCPI_ResultMnemonic(context, s);
    return SCPI_RES_OK;
}

scpi_result_t SCPI_MeasureMaxVoltQ(scpi_t *context)
{
    char s[30];
    printFloat(maxVoltage, 2, s);
    SCPI_ResultMnemonic(context, s);
    return SCPI_RES_OK;
}

scpi_result_t SCPI_MeasureMinVoltQ(scpi_t *context)
{
    char s[30];
    printFloat(minVoltage, 2, s);
    SCPI_ResultMnemonic(context, s);
    return SCPI_RES_OK;
}

scpi_result_t SCPI_MeasureFreqQ(scpi_t *context)
{
    char s[30];
    printFloat(measuredFreq, 0, s);
    SCPI_ResultMnemonic(context, s);
    return SCPI_RES_OK;
}

scpi_result_t SCPI_IsTrigdQ(scpi_t *context)
{
    if (trigged)
        SCPI_ResultMnemonic(context, "1");
    else
        SCPI_ResultMnemonic(context, "0");
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SetTrigLevel(scpi_t *context)
{
    double param1;
    if (!SCPI_ParamDouble(context, &param1, TRUE))
        return SCPI_RES_ERR;
    trigVoltage = param1;
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SetTrigSlope(scpi_t *context)
{
    bool param1;
    if (!SCPI_ParamBool(context, &param1, TRUE))
        return SCPI_RES_ERR;
    trig = param1;
    return SCPI_RES_OK;
}

const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    {.pattern = "*CLS", .callback = SCPI_CoreCls},
    {.pattern = "*ESE", .callback = SCPI_CoreEse},
    {.pattern = "*ESE?", .callback = SCPI_CoreEseQ},
    {.pattern = "*ESR?", .callback = SCPI_CoreEsrQ},
    {.pattern = "*IDN?", .callback = SCPI_CoreIdnQ},
    {.pattern = "*OPC", .callback = SCPI_CoreOpc},
    {.pattern = "*OPC?", .callback = SCPI_CoreOpcQ},
    {.pattern = "*RST", .callback = SCPI_CoreRst},
    {.pattern = "*SRE", .callback = SCPI_CoreSre},
    {.pattern = "*SRE?", .callback = SCPI_CoreSreQ},
    {.pattern = "*STB?", .callback = SCPI_CoreStbQ},
    {.pattern = "*TST?", .callback = My_CoreTstQ},
    {.pattern = "*WAI", .callback = SCPI_CoreWai},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ},
    {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ},
    {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ},

    /* Oscilloscope */
    //Trigger
    {.pattern = "TRIGger:STATus?", .callback = SCPI_IsTrigdQ},
    {.pattern = "TRIGger:LEVel", .callback = SCPI_SetTrigLevel},
    {.pattern = "TRIGger:SLOpe", .callback = SCPI_SetTrigSlope},

    // Timebase
    {.pattern = "HORIZontal:DIV", .callback = SCPI_SetTdiv},

    // Vertical
    {.pattern = "VERTical:DIV", .callback = SCPI_SetVdiv},
    {.pattern = "VERTical:ATTEN", .callback = SCPI_SetAtten},
    {.pattern = "VERTical:CALibrate", .callback = SCPI_Autocal},

    // Measurements
    {.pattern = "MEASure:VOLTage:PPK?", .callback = SCPI_MeasurePpkQ},
    {.pattern = "MEASure:VOLTage:MIN?", .callback = SCPI_MeasureMinVoltQ},
    {.pattern = "MEASure:VOLTage:MAX?", .callback = SCPI_MeasureMaxVoltQ},
    {.pattern = "MEASure:FREQuency?", .callback = SCPI_MeasureFreqQ},

    SCPI_CMD_LIST_END};

size_t myWrite(scpi_t *context, const char *data, size_t len)
{
    (void)context;
    sendSerial(data, len);
    return len;
}

scpi_result_t myReset(scpi_t *context)
{
    (void)context;
    HAL_NVIC_SystemReset();
}

scpi_interface_t scpi_interface = {
    .write = myWrite,
    .reset = myReset,
};

char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

scpi_t scpi_context;