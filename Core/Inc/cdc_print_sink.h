#ifndef __CDC_PRINT_SINK_H__
#define __CDC_PRINT_SINK_H__

#include <stdint.h>
void cdc_print_sink_tick(void);
void cdc_print_sink_init(void);
void cdc_print_sink_deinit(void);
void cdc_print_sink_transmit_complete(void);
void cdc_print_sink_receive(uint8_t *buffer, uint32_t length);

#endif
