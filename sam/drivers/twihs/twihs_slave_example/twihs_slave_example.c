/**
 * \file
 *
 * \brief TWIHS Slave Example for SAM.
 *
 * Copyright (c) 2013-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage TWIHS Slave Example
 *
 * \section intro Introduction
 *
 * The application demonstrates how to use use the TWIHS in high speed slave mode.
 *
 * \section Requirements
 *
 * This package can be used with the following setup:
 *  - SAMG53 Xplained Pro kit
 *
 * In addition, another device will be needed to act as the TWI master. The
 * twi_eeprom_example can be used for that, in which case a second kit
 * supported by that project is needed.
 * -# Connect TWD (SDA) for the 2 boards.
 * -# Connect TWCK (SCL) for the 2 boards.
 * -# Connect GND for the 2 boards.
 * -# Make sure there is a pull up resistor on TWD and TWCK.
 *
 * \section files Main files:
 *  - twihs.c SAM Two-Wire Interface High Speed driver implementation.
 *  - twihs.h SAM Two-Wire Interface High Speed driver definitions.
 *  - twihs_slave_example.c Example application.
 *
 * \section exampledescription Description of the Example
 * After launching the program, the device will act as a simple TWI-enabled
 * serial memory containing 512 bytes. This enables this project to be used
 * with the twi_eeprom_example project as the master.
 *
 * To write in the memory, the TWIHS master must address the device first, then
 * send two bytes containing the memory address to access. Additional bytes are
 * treated as the data to write.
 *
 * Reading is done in the same fashion, except that after receiving the memory
 * address, the device will start outputting data until a STOP condition is
 * sent by the master.
 * The default address for the TWIHS slave is fixed to 0x40. If the board has a TWIHS
 * component with this address, you can change the define AT24C_ADDRESS in
 * twi_eeprom_example.c of twi_eeprom_example project, and the define
 * SLAVE_ADDRESS in twihs_slave_example.c of twihs_slave_exmaple project.
 *
 * \section compinfo Compilation Info
 * This software was written for the GNU GCC and IAR EWARM.
 * Other compilers may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com/">Atmel</A>.\n
 * Support and FAQ: http://support.atmel.com/
 *
 */

#include "asf.h"
#include "conf_twihs_slave_example.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

/** Device address of slave */
#define SLAVE_ADDRESS       0x40
/** Memory size in bytes */
#define MEMORY_SIZE         512

#define STRING_EOL    "\r"
#define STRING_HEADER "--TWIHS SLAVE Example --\r\n" \
		"-- "BOARD_NAME" --\r\n" \
		"-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL

/** The slave device instance*/
typedef struct _slave_device_t {
	/** PageAddress of the slave device*/
	uint16_t us_page_address;
	/** Offset of the memory access*/
	uint16_t us_offset_memory;
	/** Read address of the request*/
	uint8_t uc_acquire_address;
	/** Memory buffer*/
	uint8_t uc_memory[MEMORY_SIZE];
} slave_device_t;

slave_device_t emulate_driver;

void BOARD_TWIHS_Handler(void)
{
	uint32_t status;

	status = twihs_get_interrupt_status(BOARD_BASE_TWIHS_SLAVE);

	if (((status & TWIHS_SR_SVACC) == TWIHS_SR_SVACC)
			&& (emulate_driver.uc_acquire_address == 0)) {
		twihs_disable_interrupt(BOARD_BASE_TWIHS_SLAVE, TWIHS_IDR_SVACC);
		twihs_enable_interrupt(BOARD_BASE_TWIHS_SLAVE,
				TWIHS_IER_RXRDY | TWIHS_IER_GACC |
				TWIHS_IER_NACK | TWIHS_IER_EOSACC | TWIHS_IER_SCL_WS);
		emulate_driver.uc_acquire_address++;
		emulate_driver.us_page_address = 0;
		emulate_driver.us_offset_memory = 0;
	}

	if ((status & TWIHS_SR_GACC) == TWIHS_SR_GACC) {
		puts("General Call Treatment\n\r");
		puts("not treated");
	}

	if (((status & TWIHS_SR_SVACC) == TWIHS_SR_SVACC) &&
			((status & TWIHS_SR_GACC) == 0) &&
			((status & TWIHS_SR_RXRDY) == TWIHS_SR_RXRDY)) {

		if (emulate_driver.uc_acquire_address == 1) {
			/* Acquire MSB address */
			emulate_driver.us_page_address =
					(twihs_read_byte(BOARD_BASE_TWIHS_SLAVE) & 0xFF) << 8;
			emulate_driver.uc_acquire_address++;
		} else {
			if (emulate_driver.uc_acquire_address == 2) {
				/* Acquire LSB address */
				emulate_driver.us_page_address |=
						(twihs_read_byte(BOARD_BASE_TWIHS_SLAVE) & 0xFF);
				emulate_driver.uc_acquire_address++;
			} else {
				/* Read one byte of data from master to slave device */
				emulate_driver.uc_memory[emulate_driver.us_page_address +
						emulate_driver.us_offset_memory] =
						(twihs_read_byte(BOARD_BASE_TWIHS_SLAVE) & 0xFF);
				emulate_driver.us_offset_memory++;
			}
		}
	} else {
		if (((status & TWIHS_SR_TXRDY) == TWIHS_SR_TXRDY)
				&& ((status & TWIHS_SR_TXCOMP) == TWIHS_SR_TXCOMP)
				&& ((status & TWIHS_SR_EOSACC) == TWIHS_SR_EOSACC)) {
			/* End of transfer, end of slave access */
			emulate_driver.us_offset_memory = 0;
			emulate_driver.uc_acquire_address = 0;
			emulate_driver.us_page_address = 0;
			twihs_enable_interrupt(BOARD_BASE_TWIHS_SLAVE, TWIHS_SR_SVACC);
			twihs_disable_interrupt(BOARD_BASE_TWIHS_SLAVE,
					TWIHS_IDR_RXRDY | TWIHS_IDR_GACC |
					TWIHS_IDR_NACK | TWIHS_IDR_EOSACC | TWIHS_IDR_SCL_WS);
		} else {
			if (((status & TWIHS_SR_SVACC) == TWIHS_SR_SVACC)
					&& ((status & TWIHS_SR_GACC) == 0)
					&& (emulate_driver.uc_acquire_address == 3)
					&& ((status & TWIHS_SR_SVREAD) == TWIHS_SR_SVREAD)
					&& ((status & TWIHS_SR_NACK) == 0)) {
				/* Write one byte of data from slave to master device */
				twihs_write_byte(BOARD_BASE_TWIHS_SLAVE,
						emulate_driver.uc_memory[emulate_driver.us_page_address
						+ emulate_driver.us_offset_memory]);
				emulate_driver.us_offset_memory++;
			}
		}
	}
}

/**
 *  \brief Configure the Console UART.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

/**
 * \brief Application entry point for TWIHS Slave example.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	uint32_t i;

	/* Initialize the SAM system */
	sysclk_init();

	/* Initialize the board */
	board_init();

	/* Initialize the console UART */
	configure_console();

	/* Output example information */
	puts(STRING_HEADER);

	/* Enable the peripheral clock for TWIHS */
	pmc_enable_periph_clk(BOARD_ID_TWIHS_SLAVE);

	for (i = 0; i < MEMORY_SIZE; i++) {
		emulate_driver.uc_memory[i] = 0;
	}
	emulate_driver.us_offset_memory = 0;
	emulate_driver.uc_acquire_address = 0;
	emulate_driver.us_page_address = 0;

	/* Configure TWIHS as slave */
	puts("-I- Configuring the TWIHS in slave mode\n\r");
	twihs_slave_init(BOARD_BASE_TWIHS_SLAVE, SLAVE_ADDRESS);

	/* Clear receipt buffer */
	twihs_read_byte(BOARD_BASE_TWIHS_SLAVE);

	/* Configure TWIHS interrupts */
	NVIC_DisableIRQ(BOARD_TWIHS_IRQn);
	NVIC_ClearPendingIRQ(BOARD_TWIHS_IRQn);
	NVIC_SetPriority(BOARD_TWIHS_IRQn, 0);
	NVIC_EnableIRQ(BOARD_TWIHS_IRQn);
	twihs_enable_interrupt(BOARD_BASE_TWIHS_SLAVE, TWIHS_SR_SVACC);

	while (1) {
	}
}

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
