/*
 * This provides a USB CDC (serial over USB) interface for printf.
 */
#include "cdc_print_sink.h"
#include "usbd_cdc_if.h"

#include <stdint.h>

static volatile uint8_t  rx_buffer[128];
static volatile uint32_t rx_head;
static volatile uint32_t rx_tail;
#define rx_buffer_mask (sizeof(tx_buffer) - 1)


static volatile uint8_t  tx_buffer[256];
static volatile uint32_t tx_head;
static volatile uint32_t tx_tail;

#define tx_buffer_mask (sizeof(tx_buffer) - 1)


static inline uint32_t rx_n_holding(void)
{
	return (rx_head - rx_tail) & rx_buffer_mask;
}

static inline uint32_t rx_space(void)
{
	return (rx_head + 1 - rx_tail) & rx_buffer_mask;
}


static void rx_buffer_copy(uint8_t *buffer, uint32_t buffer_size)
{
	uint32_t space = rx_space();

	if(buffer_size > space)
			buffer_size = space;

	while (buffer_size & 3) {
		rx_buffer[rx_head & rx_buffer_mask] = *buffer;
		buffer++;
		rx_head++;
		buffer_size--;
	}

	while (buffer_size) {
		rx_buffer[rx_head & rx_buffer_mask] = *buffer;
		buffer++;
		rx_head++;
		rx_buffer[rx_head & rx_buffer_mask] = *buffer;
		buffer++;
		rx_head++;
		rx_buffer[rx_head & rx_buffer_mask] = *buffer;
		buffer++;
		rx_head++;
		rx_buffer[rx_head & rx_buffer_mask] = *buffer;
		buffer++;
		rx_head++;
		buffer_size-=4;
	}
}


static inline uint32_t tx_n_holding(void)
{
	return (tx_head - tx_tail) & tx_buffer_mask;
}

static inline uint32_t tx_has_space(void)
{
	return (tx_head + 1 - tx_tail) & tx_buffer_mask;
}


static void tx_buffer_copy(uint8_t *buffer, uint32_t buffer_size)
{
	while (buffer_size & 3) {
		*buffer = tx_buffer[tx_tail & tx_buffer_mask];
		buffer++;
		tx_tail++;
		buffer_size--;
	}

	while (buffer_size) {
		*buffer = tx_buffer[tx_tail & tx_buffer_mask];
		buffer++;
		tx_tail++;
		*buffer = tx_buffer[tx_tail & tx_buffer_mask];
		buffer++;
		tx_tail++;
		*buffer = tx_buffer[tx_tail & tx_buffer_mask];
		buffer++;
		tx_tail++;
		*buffer = tx_buffer[tx_tail & tx_buffer_mask];
		buffer++;
		tx_tail++;
		buffer_size-=4;
	}
}

static inline uint32_t tx_fetch(uint8_t *buffer, uint32_t buffer_size)
{
	uint32_t holding = tx_n_holding();

	if (holding < buffer_size)
		buffer_size = holding;

	tx_buffer_copy(buffer, buffer_size);

	return buffer_size;
}

void cdc_print_sink_receive(uint8_t *buffer, uint32_t length)
{
	rx_buffer_copy(buffer, length);
}

static uint32_t cdc_print_sink_initialised;
static uint32_t cdc_sending;
static uint8_t out_buffer[64];
static uint32_t out_buffer_holding;


static void tx_flush(void)
{
	int ret;

	if (!cdc_print_sink_initialised)
		return;

	if (cdc_sending)
		return;

	cdc_sending = 1;

	if (!out_buffer_holding)
		out_buffer_holding = tx_fetch(out_buffer, sizeof(out_buffer));

	if (!out_buffer_holding) {
		cdc_sending = 0;
		return;
	}

	ret = CDC_Transmit_FS(out_buffer, out_buffer_holding);

	if (ret == 0) {
			out_buffer_holding = 0;
	}
}

static inline void tx_add(uint8_t ch)
{
	while (!tx_has_space()) {
		/* Spin. */
	}
	tx_buffer[tx_head &tx_buffer_mask] = ch;
	tx_head++;

	if (tx_n_holding() >= sizeof(out_buffer)) {
		tx_flush();
	}
}


int __io_getchar(void)
{
	int ret;

	while(!rx_n_holding()) {
		/* Spin. */
	}

	ret = rx_buffer[rx_tail & rx_buffer_mask];
	rx_tail++;
	return ret;
}

int _read(int file, char *ptr, int len)
{
	*ptr = __io_getchar();
	return 1;
}

int __io_putchar(int ch)
{

	uint8_t x = (uint8_t) ch;
#if 0
	uint32_t cflags;

	cflags = critical_lock();
	if (out_buffer_holding < sizeof(out_buffer)) {
		out_buffer[out_buffer_holding] = x;
		out_buffer_holding++;
	}
	critical_unlock(cflags);
#else
	tx_add(x);
	if(x == '\n')
			tx_add('\r');

#endif
	return ch;
}

void cdc_print_sink_tick(void)
{
	tx_flush();
}

void cdc_print_sink_init(void)
{
	cdc_print_sink_initialised = 1;
}

void cdc_print_sink_deinit(void)
{
	cdc_print_sink_initialised = 0;
}

void cdc_print_sink_transmit_complete(void)
{
	cdc_sending = 0;
	tx_flush();
}

