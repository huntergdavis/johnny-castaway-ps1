/*
 * PSn00bSDK controller polling example (SPI driver)
 * (C) 2021 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxpad.h>
#include <hwregs_c.h>

#include "spi.h"

#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
#endif

/* Internal structures and globals */

typedef struct _SPI_Context {
	uint8_t			tx_buff[SPI_BUFF_LEN];
	uint8_t			rx_buff[SPI_BUFF_LEN];
	uint32_t		tx_len, rx_len, port;
	SPI_Callback	callback;
} SPI_Context;

static volatile SPI_Context  _context;
static volatile SPI_Request  *_current_req;
static volatile SPI_Callback _default_cb;

#if PS1_VERBOSE_DIAGNOSTICS
/* JCSPI: instrumentation counters. Live in this TU so the IRQ handlers
 * can bump them without crossing a function-call boundary. Read by the
 * main loop (events_ps1.c) via SPI_DbgSnapshot(). All volatile so the
 * compiler doesn't cache reads in the snapshotter. */
volatile uint32_t spi_dbg_poll_count = 0;   /* timer 2 IRQ enters */
volatile uint32_t spi_dbg_ack_count  = 0;   /* SIO0 /ACK IRQ enters */
volatile uint32_t spi_dbg_cb_count   = 0;   /* user callback invoked */
volatile uint32_t spi_dbg_last_rxlen = 0;   /* rx_len at last poll IRQ */

/* JCSPI T18: register snapshot taken at the TOP of the poll IRQ handler
 * (before any of our writes). Tells us what the hardware looks like at
 * the moment a poll cycle begins. */
volatile uint32_t spi_dbg_at_irq_sio_ctrl = 0;
volatile uint32_t spi_dbg_at_irq_sio_stat = 0;
volatile uint32_t spi_dbg_at_irq_irq_stat = 0;

/* JCSPI T21: GP register at IRQ entry. If $gp is wrong, we can't
 * access globals. Captured by the IRQ handler before any global access. */
volatile uint32_t spi_dbg_at_irq_gp = 0;
volatile uint32_t spi_dbg_at_irq_sp = 0;

/* JCSPI: rolling rx_buff history. Each time the callback fires, we
 * snapshot the first 8 bytes of rx_buff into the next slot. Lets us
 * see WHAT the controller is actually replying with even if our
 * pad_buff translation is wrong. */
#define SPI_DBG_RX_HIST_SLOTS 4
volatile uint8_t  spi_dbg_rx_hist[SPI_DBG_RX_HIST_SLOTS][8];
volatile uint32_t spi_dbg_rx_hist_rxlen[SPI_DBG_RX_HIST_SLOTS];
volatile uint32_t spi_dbg_rx_hist_port[SPI_DBG_RX_HIST_SLOTS];
volatile uint32_t spi_dbg_rx_hist_idx = 0;

/* JCSPI: handler addresses we INTENDED to install. If these don't
 * match what the BIOS has in its event table, something stomped them. */
volatile uint32_t spi_dbg_poll_handler_addr = 0;
volatile uint32_t spi_dbg_ack_handler_addr  = 0;
#endif

/* Request queue management */

static void _spi_create_poll_req(void) {
	PadRequest *req = (PadRequest *) _context.tx_buff;

	req->addr     = 0x01;
	req->cmd      = PAD_CMD_READ;
	req->tap_mode = 0x00; // 0x01 to enable extended multitap response
	req->motor_l  = 0x00;
	req->motor_r  = 0x00;

	/* tx_len = 5 sends the full PS1 polling sequence "01 42 00 00 00"
	 * from the TX buffer, instead of 4 + an implicit 0x00 fallback.
	 * Test: see if DuckStation's controller emulation latches button
	 * state differently when the 5th clock byte comes from the actual
	 * TX buffer rather than the driver's fallback. spicyjpeg's example
	 * uses tx_len=4 for compactness; the protocol allows either. */
	_context.tx_len   = 5;
	_context.rx_len   = 0;
	_context.port    ^= 1;
	_context.callback = _default_cb;
}

static void _spi_next_req(void) {
	// Copy the contents of the first request in the queue into the TX buffer.
	memcpy(
		(void *) _context.tx_buff,
		(void *) _current_req->payload.data,
		_current_req->len
	);

	_context.tx_len   = _current_req->len;
	_context.rx_len   = 0;
	_context.port     = _current_req->port;
	_context.callback = _current_req->callback;

	// Pop the first request from the queue by deallocating it and adjusting
	// the pointer to the first queue item.
	SPI_Request *next = _current_req->next;

	free((void *) _current_req);
	_current_req = next;
}

/* Interrupt handlers */

static void _spi_poll_irq_handler(void) {
#if PS1_VERBOSE_DIAGNOSTICS
	/* JCSPI T21: capture $gp/$sp first thing — before touching globals.
	 * If $gp is wrong we won't even reach the spi_dbg_*++ writes (or
	 * they'll write to garbage memory). Capturing into a register first
	 * via `move` lets us at least see the values if globals end up
	 * accessible. */
	uint32_t gp_val, sp_val;
	__asm__ volatile("move %0, $gp" : "=r"(gp_val));
	__asm__ volatile("move %0, $sp" : "=r"(sp_val));
	spi_dbg_at_irq_gp = gp_val;
	spi_dbg_at_irq_sp = sp_val;

	/* JCSPI T18: snapshot SIO/IRQ_STAT BEFORE we touch anything. */
	spi_dbg_at_irq_sio_ctrl = (uint32_t)SIO_CTRL(0);
	spi_dbg_at_irq_sio_stat = (uint32_t)SIO_STAT(0);
	spi_dbg_at_irq_irq_stat = (uint32_t)IRQ_STAT;

	spi_dbg_poll_count++;
	spi_dbg_last_rxlen = _context.rx_len;
#endif

	// Fetch the last response byte, which wasn't followed by a pulse on /ACK,
	// from the RX FIFO.
	if (SIO_STAT(0) & 0x0002)
		_context.rx_buff[_context.rx_len - 1] = (uint8_t) SIO_DATA(0);

#if PS1_VERBOSE_DIAGNOSTICS
	/* JCSPI: snapshot rx history slot for this completed poll cycle. */
	{
		uint32_t slot = spi_dbg_rx_hist_idx & (SPI_DBG_RX_HIST_SLOTS - 1);
		for (int i = 0; i < 8; i++)
			spi_dbg_rx_hist[slot][i] = _context.rx_buff[i];
		spi_dbg_rx_hist_rxlen[slot] = _context.rx_len;
		spi_dbg_rx_hist_port[slot]  = _context.port;
		spi_dbg_rx_hist_idx++;
	}
#endif

	if (_context.callback) {
#if PS1_VERBOSE_DIAGNOSTICS
		spi_dbg_cb_count++;
#endif
		_context.callback(_context.port, _context.rx_buff, _context.rx_len);
	}

	// If the request queue is empty, create a pad polling request.
	if (_current_req)
		_spi_next_req();
	else
		_spi_create_poll_req();

	// Prepare the SPI port by clearing any pending IRQ, pulling /CS high and
	// enabling the /ACK IRQ. In order to communicate with controllers, /CS has
	// to be driven low again for about 20 us before sending the first byte.
	// TODO: these delays can be probably tweaked for better performance
	SIO_CTRL(0) = 0x0010;
	for (uint32_t i = 0; i < 1000; i++)
		__asm__ volatile("");

	SIO_CTRL(0) = 0x1003 | (_context.port << 13);
	for (uint32_t i = 0; i < 2000; i++)
		__asm__ volatile("");

	// Send the first byte indicating which device to address. If the matching
	// device is connected, it will reply by triggering the /ACK IRQ.
	SIO_DATA(0) = _context.tx_buff[0];
}

static void _spi_ack_irq_handler(void) {
#if PS1_VERBOSE_DIAGNOSTICS
	spi_dbg_ack_count++;
#endif

	// Wait until /ACK is pulled up by the controller before sending the next
	// byte. According to nocash docs, this has to be done before resetting the
	// IRQ.
	while (SIO_STAT(0) & 0x0080)
		__asm__ volatile("");

	// Keep /CS pulled low and acknowledge the IRQ (bit 4) to ensure it can be
	// triggered again.
	SIO_CTRL(0) = 0x1013 | (_context.port << 13);

	if (!_context.rx_len) {
		// We just sent the first address byte. Obviously the response we
		// received was read from an open bus, so the SPI port's internal FIFO
		// must be flushed (by performing dummy reads) to ensure we are only
		// going to read valid data from now on.
		SIO_DATA(0);

	} else if (_context.rx_len <= SPI_BUFF_LEN) {
		// If this is not the first byte, put it in the RX buffer.
		_context.rx_buff[_context.rx_len - 1] = (uint8_t) SIO_DATA(0);
	}

	// Send the next byte, or a null byte if there is no more data to send and
	// we're just reading a response.
	_context.rx_len++;
	if (_context.rx_len < _context.tx_len)
		SIO_DATA(0) = (uint32_t) _context.tx_buff[_context.rx_len];
	else
		SIO_DATA(0) = 0x00;
}

#if PS1_VERBOSE_DIAGNOSTICS
/* JCSPI: snapshot helper. Copies all relevant state into the caller's
 * struct. Designed to be called from the main loop, NOT from an IRQ
 * handler — reads volatile registers and the static SPI context. */
void SPI_DbgSnapshot(SPI_DbgState *out) {
	if (!out) return;

	/* Counters (T1, T2, T6, T13). */
	out->poll_count  = spi_dbg_poll_count;
	out->ack_count   = spi_dbg_ack_count;
	out->cb_count    = spi_dbg_cb_count;
	out->last_rxlen  = spi_dbg_last_rxlen;

	/* IRQ state (T1, T6, T8). IRQ_MASK = 0x1F801074, IRQ_STAT = 0x1F801070.
	 * If bits 6 (timer 2) and 7 (controller/memcard) aren't set in
	 * IRQ_MASK, the IRQs never reach the CPU. */
	out->irq_mask    = (uint32_t)IRQ_MASK;
	out->irq_stat    = (uint32_t)IRQ_STAT;

	/* SIO state (T4, T11). After SPI_Init we expect:
	 *   SIO_MODE = 0x000d, SIO_BAUD = 0x0088, SIO_CTRL bit 12 set
	 *   while polling. If MODE/BAUD revert to 0, someone else is
	 *   resetting the port. */
	out->sio_ctrl    = (uint32_t)SIO_CTRL(0);
	out->sio_mode    = (uint32_t)SIO_MODE(0);
	out->sio_baud    = (uint32_t)SIO_BAUD(0);
	out->sio_stat    = (uint32_t)SIO_STAT(0);

	/* Timer 2 state (T5, T9). TIMER_CTRL(2) we set to 0x0258,
	 * TIMER_RELOAD(2) we set to (F_CPU / 8) / 250 = 16934. If reload
	 * read-back differs from expected, the write didn't land. */
	out->timer_ctrl  = (uint32_t)TIMER_CTRL(2);
	out->timer_reload= (uint32_t)TIMER_RELOAD(2);
	out->timer_value = (uint32_t)TIMER_VALUE(2);

	/* Internal context (T7, T10, T12). port should toggle every poll;
	 * tx_buff[0..3] should be 01 42 00 00 for a digital pad request. */
	out->ctx_port    = _context.port;
	out->ctx_tx_len  = _context.tx_len;
	out->ctx_rx_len  = _context.rx_len;
	out->default_cb  = (uint32_t)_default_cb;
	out->tx0         = _context.tx_buff[0];
	out->tx1         = _context.tx_buff[1];
	out->tx2         = _context.tx_buff[2];
	out->tx3         = _context.tx_buff[3];
	out->rx0         = _context.rx_buff[0];
	out->rx1         = _context.rx_buff[1];
	out->rx2         = _context.rx_buff[2];
	out->rx3         = _context.rx_buff[3];
	out->rx4         = _context.rx_buff[4];

	/* T18/T21: state captured at IRQ entry. */
	out->at_irq_sio_ctrl = spi_dbg_at_irq_sio_ctrl;
	out->at_irq_sio_stat = spi_dbg_at_irq_sio_stat;
	out->at_irq_irq_stat = spi_dbg_at_irq_irq_stat;
	out->at_irq_gp       = spi_dbg_at_irq_gp;
	out->at_irq_sp       = spi_dbg_at_irq_sp;

	/* T19: handler addresses we intended to install. */
	out->poll_handler    = spi_dbg_poll_handler_addr;
	out->ack_handler     = spi_dbg_ack_handler_addr;

	/* T16: address of _context (high nibble reveals KSEG0/KSEG1/KUSEG). */
	out->ctx_addr        = (uint32_t)&_context;
	out->tx_buff_addr    = (uint32_t)_context.tx_buff;
	out->rx_buff_addr    = (uint32_t)_context.rx_buff;

	/* Rolling rx history. Copy the most recent 4 slots in
	 * chronological order. */
	out->rx_hist_count = spi_dbg_rx_hist_idx;
	for (int s = 0; s < SPI_DBG_RX_HIST_SLOTS; s++) {
		out->rx_hist_rxlen[s] = spi_dbg_rx_hist_rxlen[s];
		out->rx_hist_port[s]  = spi_dbg_rx_hist_port[s];
		for (int i = 0; i < 8; i++)
			out->rx_hist[s][i] = spi_dbg_rx_hist[s][i];
	}
}
#endif

/* Public API */

SPI_Request *SPI_CreateRequest(void) {
	SPI_Request *req = malloc(sizeof(SPI_Request));

	req->len      = 0;
	req->port     = 0;
	req->callback = 0;
	req->next     = 0;

	// Find the last queued request by traversing the linked list and append a
	// pointer to the new request.
	if (!_current_req) {
		_current_req = req;
	} else {
		volatile SPI_Request *volatile last = _current_req;
		while (last->next)
			last = last->next;

		last->next = req;
	}

	return req;
}

void SPI_SetPollRate(uint32_t value) {
	TIMER_CTRL(2) = 0x0258; // CLK/8 input, IRQ on reload, disable one-shot IRQ

	if (value < 65)
		TIMER_RELOAD(2) = 0xffff;
	else
		TIMER_RELOAD(2) = (F_CPU / 8) / value;
}

void SPI_Init(SPI_Callback callback) {
#if PS1_VERBOSE_DIAGNOSTICS
	/* JCSPI T19: record the addresses we INTENDED to install so the
	 * snapshot can compare against whatever the BIOS event table
	 * actually has. If the BIOS chained someone else's handler ahead
	 * of ours, these addresses won't run. */
	spi_dbg_poll_handler_addr = (uint32_t)&_spi_poll_irq_handler;
	spi_dbg_ack_handler_addr  = (uint32_t)&_spi_ack_irq_handler;
#endif

	// Disable the BIOS timer handler (which for some stupid reason is enabled
	// by default, even though it does nothing) and set up custom interrupt
	// handlers.
	EnterCriticalSection();
	ChangeClearRCnt(2, 0);
	InterruptCallback(6, &_spi_poll_irq_handler);
	InterruptCallback(7, &_spi_ack_irq_handler);
	ExitCriticalSection();

	SIO_CTRL(0) = 0x0040; // Reset all registers
	SIO_MODE(0) = 0x000d; // 1x multiplier, 8 data bits, no parity
	SIO_BAUD(0) = 0x0088; // 250000 bps

	SPI_SetPollRate(250);
	_current_req = 0;
	_default_cb  = callback;
}
