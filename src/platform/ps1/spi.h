/*
 * PSn00bSDK controller polling example (SPI driver)
 * (C) 2021 spicyjpeg - MPL licensed
 */

/**
 * @file spi.h
 * @brief Asynchronous SPI controller driver
 *
 * @details This is a fairly complete timer driven, asynchronous high-speed SPI
 * driver, with support for sending custom commands (including memory card
 * access), in about 200 lines of code. Feel free to copy and adapt it.
 *
 * The way this works is by maintaining a queue of requests to send, each with
 * its own payload and callback. Timer 2 is configured to trigger an IRQ at
 * regular intervals. On each tick, the next request in the queue (or a poll
 * command if no request is pending) is prepared and the first byte is
 * sent; if the controller asks for more data by pulling /ACK low, the next
 * byte is sent and the received byte is placed into a buffer. This goes on
 * until the last byte is exchanged and the controller stops asserting /ACK.
 *
 * On the next tick, the response buffer is passed to the request's callback
 * and reset, and the next request in the queue is sent. This blindly assumes
 * it only takes one tick for a request/response to be sent, which is the case
 * for controllers' very small packets but not for memory cards. It is
 * advisable to call spi_set_poll_rate() to temporarily reduce poll rate while
 * accessing memory cards.
 *
 * Note that this driver completely takes over the SPI bus, so you won't be
 * able to use any BIOS functions that rely on SPI access (i.e. pad and memory
 * card APIs) alongside it.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <psxpad.h>

// Maximum request/response length (34 bytes for pads, 140 for memory cards).
// Must be a multiple of 4 to avoid memory alignment issues.
//#define SPI_BUFF_LEN 36
#define SPI_BUFF_LEN 140

/* Request structures */

typedef void (*SPI_Callback)(uint32_t port, const volatile uint8_t *buff, size_t rx_len);

typedef struct _SPI_Request {
	union TSpiRequestPayload {
		uint8_t			data[SPI_BUFF_LEN];
		PadRequest		pad_req;
		MemCardRequest	mcd_req;
	} payload;
	uint32_t			len, port;
	SPI_Callback		callback;
	struct _SPI_Request	*next;
} SPI_Request;

/* JCSPI: diagnostic state snapshot. Filled by SPI_DbgSnapshot. */
#define SPI_DBG_RX_HIST_SLOTS 4

typedef struct _SPI_DbgState {
	/* Counters / last_rxlen */
	uint32_t	poll_count;
	uint32_t	ack_count;
	uint32_t	cb_count;
	uint32_t	last_rxlen;
	/* Live IRQ control */
	uint32_t	irq_mask;
	uint32_t	irq_stat;
	/* Live SIO0 registers */
	uint32_t	sio_ctrl;
	uint32_t	sio_mode;
	uint32_t	sio_baud;
	uint32_t	sio_stat;
	/* Live Timer-2 */
	uint32_t	timer_ctrl;
	uint32_t	timer_reload;
	uint32_t	timer_value;
	/* Driver context */
	uint32_t	ctx_port;
	uint32_t	ctx_tx_len;
	uint32_t	ctx_rx_len;
	uint32_t	default_cb;
	uint8_t		tx0, tx1, tx2, tx3;
	uint8_t		rx0, rx1, rx2, rx3, rx4;
	/* T18/T21: snapshot at IRQ entry */
	uint32_t	at_irq_sio_ctrl;
	uint32_t	at_irq_sio_stat;
	uint32_t	at_irq_irq_stat;
	uint32_t	at_irq_gp;
	uint32_t	at_irq_sp;
	/* T19: installed handler addresses */
	uint32_t	poll_handler;
	uint32_t	ack_handler;
	/* T16: context address (KSEG0 vs KSEG1 vs KUSEG check) */
	uint32_t	ctx_addr;
	uint32_t	tx_buff_addr;
	uint32_t	rx_buff_addr;
	/* Rolling rx history — last 4 captures in chronological order */
	uint32_t	rx_hist_count;
	uint32_t	rx_hist_rxlen[SPI_DBG_RX_HIST_SLOTS];
	uint32_t	rx_hist_port[SPI_DBG_RX_HIST_SLOTS];
	uint8_t		rx_hist[SPI_DBG_RX_HIST_SLOTS][8];
} SPI_DbgState;

extern volatile uint32_t spi_dbg_poll_count;
extern volatile uint32_t spi_dbg_ack_count;
extern volatile uint32_t spi_dbg_cb_count;
extern volatile uint32_t spi_dbg_last_rxlen;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Snapshot driver counters + IRQ/SIO/Timer registers. Safe from
 * the main loop, NOT from an IRQ handler.
 */
void SPI_DbgSnapshot(SPI_DbgState *out);

/**
 * @brief Allocates a new request object and adds it to the request queue. The
 * object must be populated afterwards by setting the length, callback and
 * filling in the TX data buffer.
 */
SPI_Request *SPI_CreateRequest(void);

/**
 * @brief Changes the controller polling rate. The lowest supported rate is 65
 * Hz (requests sent every 1/65th of a second, each port polled at 32.5 Hz when
 * no request is pending).
 *
 * @param value
 */
void SPI_SetPollRate(uint32_t value);

/**
 * @brief Installs the SPI and timer 2 interrupt handlers and starts the poll
 * timer. By default the polling rate is set to 250 Hz (125 Hz per port),
 * however it can be changed at any time by calling SPI_SetPollRate().
 *
 * The provided callback (if any) is called to report the result of poll
 * requests, which are issued automatically when no other request is in the
 * queue. Passing NULL as callback does not disable auto-polling.
 *
 * @param callback
 */
void SPI_Init(SPI_Callback callback);

#ifdef __cplusplus
}
#endif
