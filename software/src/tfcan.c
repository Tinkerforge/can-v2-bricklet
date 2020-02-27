/* can-v2-bricklet
 * Copyright (C) 2018-2019 Matthias Bolte <matthias@tinkerforge.com>
 *
 * tfcan.c: Transfer data over CAN bus
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "tfcan.h"

#include "configs/config.h"
#include "configs/config_tfcan.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"

#include "xmc_gpio.h"
#include "xmc_can.h"
#include "xmc_can_map.h"

TFCAN tfcan;

void IRQ_Hdlr_0(void) {
	for (uint8_t i = 0; i < TFCAN_NODE_SIZE; ++i) {
		const uint8_t lec = (tfcan.node[i]->NSR & (uint32_t)CAN_NODE_NSR_LEC_Msk) >> CAN_NODE_NSR_LEC_Pos;

		switch (lec) {
			case TFCAN_NODE_LEC_STUFFING_ERROR: tfcan.error_occured = true; ++tfcan.transceiver_stuffing_error_count; break;
			case TFCAN_NODE_LEC_FORMAT_ERROR:   tfcan.error_occured = true; ++tfcan.transceiver_format_error_count;   break;
			case TFCAN_NODE_LEC_ACK_ERROR:      tfcan.error_occured = true; ++tfcan.transceiver_ack_error_count;      break;
			case TFCAN_NODE_LEC_BIT1_ERROR:     tfcan.error_occured = true; ++tfcan.transceiver_bit1_error_count;     break;
			case TFCAN_NODE_LEC_BIT0_ERROR:     tfcan.error_occured = true; ++tfcan.transceiver_bit0_error_count;     break;
			case TFCAN_NODE_LEC_CRC_ERROR:      tfcan.error_occured = true; ++tfcan.transceiver_crc_error_count;      break;
			default:                                                                                                  break;
		}
	}
}

void tfcan_init(void) {
	logd("sizeof(TFCAN) %u\n\r", sizeof(TFCAN));

	memset(&tfcan, 0, sizeof(TFCAN));

	tfcan.reconfigure_transceiver = true;
	tfcan.reconfigure_queues = true;
	tfcan.reconfigure_rx_filters = 0;

	tfcan.baud_rate = 125000; // config default
	tfcan.sample_point = 625; // config default
	tfcan.sync_jump_width = 1;
	tfcan.transceiver_mode = TFCAN_TRANSCEIVER_MODE_NORMAL; // config default
	tfcan.timestamped_frame_enabled = false;

	tfcan.node[0] = CAN_NODE0;
	tfcan.node[1] = CAN_NODE1;
	tfcan.tx_node = tfcan.node[0];
	tfcan.rx_node = tfcan.node[0];

	tfcan.tx_buffer_size = TFCAN_BUFFER_SIZE / 4; // config default
	tfcan.tx_buffer_timeout = 0; // config default

	tfcan.rx_buffer_size[0] = TFCAN_BUFFER_SIZE / 2; // config default
	tfcan.rx_buffer_type[0] = TFCAN_BUFFER_TYPE_DATA; // config default

	tfcan.rx_buffer_size[1] = TFCAN_BUFFER_SIZE / 4; // config default
	tfcan.rx_buffer_type[1] = TFCAN_BUFFER_TYPE_REMOTE; // config default

	for (uint8_t i = 0; i < TFCAN_BUFFER_SIZE; ++i) {
		tfcan.rx_filter_mode[i] = TFCAN_FILTER_MODE_ACCEPT_ALL; // config default
		tfcan.rx_filter_mask[i] = 0; // config default
		tfcan.rx_filter_identifier[i] = 0; // config default
	}

	tfcan.tx_backlog_size = TFCAN_BACKLOG_SIZE / 2; // config default
	tfcan.tx_timestamped_backlog_size = TFCAN_TIMESTAMPED_BACKLOG_SIZE / 2; // config default

	tfcan.rx_backlog_size = TFCAN_BACKLOG_SIZE / 2; // config default
	tfcan.rx_timestamped_backlog_size = TFCAN_TIMESTAMPED_BACKLOG_SIZE / 2; // config default

#ifdef TFCAN_BUFFER_DEBUG
	tfcan.last_buffer_debug = system_timer_get_ms();
#endif

	// configure clock
	XMC_CAN_Init(CAN, TFCAN_CLOCK_SOURCE, TFCAN_FREQUENCY);

	// enter config mode
	tfcan_set_config_mode(true);

	// configure RX pin
	const XMC_GPIO_CONFIG_t rx_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD,
	};

	XMC_GPIO_Init(TFCAN_RX_PIN, &rx_pin_config);
	XMC_CAN_NODE_SetReceiveInput(tfcan.node[0], TFCAN_RECEIVE_INPUT);

	// configure TX pin
	const XMC_GPIO_CONFIG_t tx_pin_config = {
		.mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT9,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	XMC_GPIO_Init(TFCAN_TX_PIN, &tx_pin_config);

	// leave config mode
	tfcan_set_config_mode(false);

	// configure transceiver and queues
	tfcan_reconfigure_transceiver();
	tfcan_reconfigure_queues();

	tfcan.transceiver_state = TFCAN_TRANSCEIVER_STATE_ACTIVE;
	tfcan.transceiver_tx_error_level = 0;
	tfcan.transceiver_rx_error_level = 0;
	tfcan.transceiver_stuffing_error_count = 0;
	tfcan.transceiver_format_error_count = 0;
	tfcan.transceiver_ack_error_count = 0;
	tfcan.transceiver_bit1_error_count = 0;
	tfcan.transceiver_bit0_error_count = 0;
	tfcan.transceiver_crc_error_count = 0;
	tfcan.tx_buffer_timeout_error_count = 0;
	tfcan.rx_buffer_overflow_error_count = 0;
	tfcan.rx_buffer_overflow_error_occurred = 0;
	tfcan.rx_backlog_overflow_error_count = 0;

	// configure LEC interrupt
	NVIC_EnableIRQ(TFCAN_NODE_LEC_IRQ_INDEX);
	NVIC_SetPriority(TFCAN_NODE_LEC_IRQ_INDEX, 0);
	XMC_SCU_SetInterruptControl(TFCAN_NODE_LEC_IRQ_INDEX, XMC_SCU_IRQCTRL_CAN0_SR0_IRQ0);

	// configure communication LED
	XMC_GPIO_CONFIG_t com_led_pin_config = {
		.mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_LOW
	};

	XMC_GPIO_Init(TFCAN_COM_LED_PIN, &com_led_pin_config);

	tfcan.com_led_state.config = LED_FLICKER_CONFIG_STATUS;
	tfcan.com_led_state.counter = 0;
	tfcan.com_led_state.start = 0;

	// configure error LED
	XMC_GPIO_CONFIG_t error_led_pin_config = {
		.mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	XMC_GPIO_Init(TFCAN_ERROR_LED_PIN, &error_led_pin_config);

	tfcan.error_led_state.config = LED_FLICKER_CONFIG_EXTERNAL;
	tfcan.error_led_state.counter = 0;
	tfcan.error_led_state.start = 0;

	tfcan.error_led_config = TFCAN_ERROR_LED_CONFIG_SHOW_TRANSCEIVER_STATE;
	tfcan.error_occured = false;
	tfcan.error_state = TFCAN_ERROR_STATE_IDLE;
}

void tfcan_tick(void) {
#ifdef TFCAN_BUFFER_DEBUG
	if (system_timer_is_time_elapsed_ms(tfcan.last_buffer_debug, 1000)) {
		uartbb_printf(">>>>>>\n\r");

		for (uint8_t i = 0; i < tfcan.tx_buffer_size; ++i) {
			logd("st-tx %02u %032_8b\n\r", i, tfcan_mo_get_status(tfcan.tx_buffer_mo[i]));
		}

		if (tfcan.tx_buffer_size > 0) {
			logd("fgpr-tx %032_8b\n\r", tfcan.tx_buffer_mo[0]->MOFGPR);
		}

		logd("nx-tx %u\n\r", tfcan.tx_buffer_mo_next_index);

		// FIXME: adapt for timestamped frame
		if (tfcan.tx_backlog_size > 0) {
			logd("bl-tx %u[%u] -> %u[%u]\n\r", tfcan.tx_backlog_start, tfcan.tx_backlog[tfcan.tx_backlog_start].mo_type,
			                                   tfcan.tx_backlog_end, tfcan.tx_backlog[tfcan.tx_backlog_end].mo_type);
		}

		uartbb_printf("------\n\r");

		for (uint8_t k = 0; k < TFCAN_BUFFER_SIZE; ++k) {
			for (uint8_t i = 0; i < tfcan.rx_buffer_size[k]; ++i) {
				logd("st-rx:%02u %02u %032_8b\n\r", k, i, tfcan_mo_get_status(tfcan.rx_buffer_mo[k][i]));
			}

			if (tfcan.rx_buffer_size[k] > 0) {
				logd("fgpr-rx:%02u %032_8b\n\r", k, tfcan.rx_buffer_mo[k][0]->MOFGPR);
				logd("nx-rx:%02u %d\n\r", k, tfcan.rx_buffer_mo_next_index[k]);
			}
		}

		// FIXME: adapt for timestamped frame
		if (tfcan.rx_backlog_size > 0) {
			logd("bl-rx %u[%u] -> %u[%u]\n\r", tfcan.rx_backlog_start, tfcan.rx_backlog[tfcan.rx_backlog_start].mo_type,
			                                   tfcan.rx_backlog_end, tfcan.rx_backlog[tfcan.rx_backlog_end].mo_type);
		}

		uartbb_printf("<<<<<<\n\r");

		tfcan.last_buffer_debug = system_timer_get_ms();
	}
#endif

	tfcan_reconfigure_transceiver();
	tfcan_reconfigure_queues();
	tfcan_reconfigure_rx_filters();
	tfcan_check_tx_buffer_timeout();

	// write at most TX FIFO size frames
	for (uint8_t i = 0; i < tfcan.tx_buffer_size; ++i) {
		// check TX backlog
		if (!tfcan.timestamped_frame_enabled) {
			if (tfcan.tx_backlog_size == 0 ||
			    tfcan.tx_backlog[tfcan.tx_backlog_start].mo_type == TFCAN_MO_TYPE_INVALID) {
				break; // TX backlog empty
			}
		} else {
			if (tfcan.tx_timestamped_backlog_size == 0 ||
			    tfcan.tx_timestamped_backlog[tfcan.tx_backlog_start].frame.mo_type == TFCAN_MO_TYPE_INVALID ||
			    tfcan.tx_timestamped_backlog[tfcan.tx_backlog_start].timestamp > system_timer_get_us()) {
				break; // TX backlog empty
			}
		}

		// check TX FIFO
		CAN_MO_TypeDef *tx_buffer_mo_next = tfcan.tx_buffer_mo[tfcan.tx_buffer_mo_next_index];
		const uint32_t status = tfcan_mo_get_status(tx_buffer_mo_next);

		if ((status & TFCAN_MO_STATUS_TX_REQUEST) != 0) {
			break; // TX FIFO full
		}

		logd("tx %u -> %u\n\r", tfcan.tx_backlog_start, tfcan.tx_buffer_mo_next_index);

		// copy frame into MO
		TFCAN_Frame *frame;

		if (!tfcan.timestamped_frame_enabled) {
			frame = &tfcan.tx_backlog[tfcan.tx_backlog_start];
		} else {
			frame = &tfcan.tx_timestamped_backlog[tfcan.tx_backlog_start].frame;
		}

		tfcan_mo_set_identifier(tx_buffer_mo_next, frame->mo_type, frame->identifier);
		tfcan_mo_set_data(tx_buffer_mo_next, frame->data, frame->length);

		if (!tfcan.timestamped_frame_enabled) {
			tfcan.tx_backlog[tfcan.tx_backlog_start].mo_type = TFCAN_MO_TYPE_INVALID;
			tfcan.tx_backlog_start = (tfcan.tx_backlog_start + 1) % tfcan.tx_backlog_size;
		} else {
			tfcan.tx_timestamped_backlog[tfcan.tx_backlog_start].frame.mo_type = TFCAN_MO_TYPE_INVALID;
			tfcan.tx_backlog_start = (tfcan.tx_backlog_start + 1) % tfcan.tx_timestamped_backlog_size;
		}

		// schedule MO for transmission
		if (tfcan.tx_buffer_size == 1) {
			// FIXME: in a TX FIFO with a single MO the TXEN1 bit gets lost on transmission,
			//        stopping the TX FIFO logic. force the TXEN1 bit in this case
			tfcan_mo_change_status(tfcan.tx_buffer_mo[0], TFCAN_MO_SET_STATUS_TX_ENABLE1);
		}

		if (tfcan.tx_buffer_timeout < 0) {
			tx_buffer_mo_next->MOFCR |= (uint32_t)CAN_MO_MOFCR_STT_Msk;
		} else {
			tx_buffer_mo_next->MOFCR &= ~(uint32_t)CAN_MO_MOFCR_STT_Msk;
		}

		tfcan_mo_change_status(tx_buffer_mo_next, TFCAN_MO_SET_STATUS_TX_REQUEST);

		tfcan.tx_buffer_mo_timestamp[tfcan.tx_buffer_mo_next_index] = system_timer_get_ms();
		tfcan.tx_buffer_mo_next_index = (tfcan.tx_buffer_mo_next_index + 1) % tfcan.tx_buffer_size;

		led_flicker_increase_counter(&tfcan.com_led_state);
	}

	// collect RX MO frame counter values
	for (uint8_t k = 0; k < TFCAN_BUFFER_SIZE; ++k) {
		tfcan.mo_frame_counter[k] = -1;
	}

	bool rx_buffer_overflow = false;

	for (uint8_t k = 0; k < TFCAN_BUFFER_SIZE; ++k) {
		const uint8_t rx_buffer_size = tfcan.rx_buffer_size[k];

		if (rx_buffer_size == 0) {
			break; // remaining RX FIFOs not configured
		}

		CAN_MO_TypeDef **rx_buffer_mo = tfcan.rx_buffer_mo[k];
		int32_t *rx_buffer_mo_frame_counter = tfcan.rx_buffer_mo_frame_counter[k];

		for (uint8_t i = 0; i < rx_buffer_size; ++i) {
			const uint32_t status = tfcan_mo_get_status(rx_buffer_mo[i]);

			if ((status & TFCAN_MO_STATUS_MESSAGE_LOST) != 0) {
				tfcan_mo_change_status(rx_buffer_mo[i], TFCAN_MO_RESET_STATUS_MESSAGE_LOST);

				++tfcan.rx_buffer_overflow_error_count;
				tfcan.rx_buffer_overflow_error_occurred |= 1u << k;
				rx_buffer_overflow = true;
			}

			if ((status & TFCAN_MO_STATUS_RX_PENDING) != 0) {
				rx_buffer_mo_frame_counter[i] = tfcan_mo_get_frame_counter(rx_buffer_mo[i]);
			}
		}
	}

	if (rx_buffer_overflow) {
		tfcan.error_occured = true;
	}

	// calculate RX MO age values
	const uint16_t rx_frame_counter = (tfcan.rx_node->NFCR & (uint32_t)CAN_NODE_NFCR_CFC_Msk) >> CAN_NODE_NFCR_CFC_Pos;

	for (uint8_t k = 0; k < TFCAN_BUFFER_SIZE; ++k) {
		if (tfcan.mo_frame_counter[k] >= 0) {
			tfcan.mo_age[k] = rx_frame_counter - (uint16_t)tfcan.mo_frame_counter[k];
		}
	}

	// read at most RX FIFO size frames, sorted by RX MO age, oldest first
	bool rx_backlog_overflow = false;

	for (uint8_t z = 0; z < TFCAN_BUFFER_SIZE; ++z) {
		int8_t k = -1;
		uint16_t rx_buffer_mo_next_age_max = 0;

		// find oldest RX MO
		for (int8_t n = 0; n < TFCAN_BUFFER_SIZE; ++n) {
			const uint8_t rx_buffer_size = tfcan.rx_buffer_size[n];

			if (rx_buffer_size == 0) {
				break; // remaining RX FIFOs not configured
			}

			const uint8_t rx_buffer_mo_next_index = tfcan.rx_buffer_mo_next_index[n];

			if (tfcan.rx_buffer_mo_frame_counter[n][rx_buffer_mo_next_index] < 0) {
				continue; // RX FIFO empty
			}

			const uint16_t rx_buffer_mo_next_age = tfcan.rx_buffer_mo_age[n][rx_buffer_mo_next_index];

			if (rx_buffer_mo_next_age > rx_buffer_mo_next_age_max) {
				k = n;
				rx_buffer_mo_next_age_max = rx_buffer_mo_next_age;
			}
		}

		if (k < 0) {
			break; // all RX FIFOs empty
		}

		// read MO data
		uint8_t rx_buffer_mo_next_index = tfcan.rx_buffer_mo_next_index[k];
		CAN_MO_TypeDef *rx_buffer_mo_next = tfcan.rx_buffer_mo[k][rx_buffer_mo_next_index];
		const int32_t rx_buffer_mo_age = tfcan.rx_buffer_mo_age[k][rx_buffer_mo_next_index];

		// check RX backlog
		bool rx_backlog_available;

		if (!tfcan.timestamped_frame_enabled) {
			rx_backlog_available = tfcan.rx_backlog_size > 0 &&
			                       tfcan.rx_backlog[tfcan.rx_backlog_end].mo_type == TFCAN_MO_TYPE_INVALID;
		} else {
			rx_backlog_available = tfcan.rx_timestamped_backlog_size > 0 &&
			                       tfcan.rx_timestamped_backlog[tfcan.rx_backlog_end].frame.mo_type == TFCAN_MO_TYPE_INVALID;
		}

		if (rx_backlog_available) {
			logd("rx %u:%u (%d) -> %u\n\r", k, rx_buffer_mo_next_index, rx_buffer_mo_age, tfcan.rx_backlog_end);

			TFCAN_Frame *frame;
			uint8_t status;

			if (!tfcan.timestamped_frame_enabled) {
				frame = &tfcan.rx_backlog[tfcan.rx_backlog_end];
			} else {
				frame = &tfcan.rx_timestamped_backlog[tfcan.rx_backlog_end].frame;

				tfcan.rx_timestamped_backlog[tfcan.rx_backlog_end].timestamp = system_timer_get_us();
			}

			// FIXME: explain this loop
			do {
				tfcan_mo_change_status(rx_buffer_mo_next, TFCAN_MO_RESET_STATUS_NEW_DATA |
				                                          TFCAN_MO_RESET_STATUS_RX_PENDING);

				// copy MO to frame
				TFCAN_MOType mo_type;
				uint32_t identifier;

				tfcan_mo_get_identifier(rx_buffer_mo_next, &mo_type, &identifier);

				frame->mo_type = mo_type;
				frame->identifier = identifier;

				tfcan_mo_get_data(rx_buffer_mo_next, frame->data, &frame->length);

				logd("rx %u:%u (%d) -> %u try\n\r", k, rx_buffer_mo_next_index, rx_buffer_mo_age, tfcan.rx_backlog_end);

				status = tfcan_mo_get_status(rx_buffer_mo_next);
			} while ((status & TFCAN_MO_STATUS_NEW_DATA) != 0 ||
			         (status & TFCAN_MO_STATUS_RX_UPDATING) != 0); // FIXME: add timeout

			if (!tfcan.timestamped_frame_enabled) {
				tfcan.rx_backlog_end = (tfcan.rx_backlog_end + 1) % tfcan.rx_backlog_size;
			} else {
				tfcan.rx_backlog_end = (tfcan.rx_backlog_end + 1) % tfcan.rx_timestamped_backlog_size;
			}
		} else {
			logd("rx %u:%u (%d) drop\n\r", k, rx_buffer_mo_next_index, rx_buffer_mo_age);

			++tfcan.rx_backlog_overflow_error_count;
			rx_backlog_overflow = true;

			tfcan_mo_change_status(rx_buffer_mo_next, TFCAN_MO_RESET_STATUS_NEW_DATA |
			                                          TFCAN_MO_RESET_STATUS_RX_PENDING);
		}

		tfcan.rx_buffer_mo_frame_counter[k][rx_buffer_mo_next_index] = -1;
		tfcan.rx_buffer_mo_next_index[k] = (rx_buffer_mo_next_index + 1) % tfcan.rx_buffer_size[k];

		led_flicker_increase_counter(&tfcan.com_led_state);
	}

	if (rx_backlog_overflow) {
		tfcan.error_occured = true;
	}

	tfcan.transceiver_tx_error_level = (tfcan.node[0]->NECNT & (uint32_t)CAN_NODE_NECNT_TEC_Msk) >> CAN_NODE_NECNT_TEC_Pos;
	tfcan.transceiver_rx_error_level = (tfcan.node[0]->NECNT & (uint32_t)CAN_NODE_NECNT_REC_Msk) >> CAN_NODE_NECNT_REC_Pos;

	if ((tfcan.node[0]->NSR & (uint32_t)CAN_NODE_NSR_BOFF_Msk) != 0) {
		tfcan.transceiver_state = TFCAN_TRANSCEIVER_STATE_DISABLED;
	} else {
		// the INIT bit is automatically set when the transceiver enters
		// bus-off state, but the INIT bit is not automatically cleared again
		tfcan.node[0]->NCR &= ~(uint32_t)CAN_NODE_NCR_INIT_Msk;

		if (tfcan.transceiver_tx_error_level >= 128 || tfcan.transceiver_rx_error_level >= 128) {
			tfcan.transceiver_state = TFCAN_TRANSCEIVER_STATE_PASSIVE;
		} else {
			tfcan.transceiver_state = TFCAN_TRANSCEIVER_STATE_ACTIVE;
		}
	}

	if (tfcan.error_led_config == TFCAN_ERROR_LED_CONFIG_SHOW_TRANSCEIVER_STATE) {
		if (tfcan.transceiver_state == TFCAN_TRANSCEIVER_STATE_ACTIVE) {
			XMC_GPIO_SetOutputHigh(TFCAN_ERROR_LED_PIN);
		} else {
			XMC_GPIO_SetOutputLow(TFCAN_ERROR_LED_PIN);
		}
	}

	if (tfcan.error_occured) {
		tfcan.error_occured = false;
		if (tfcan.error_led_config == TFCAN_ERROR_LED_CONFIG_SHOW_ERROR) {
			XMC_GPIO_SetOutputLow(TFCAN_ERROR_LED_PIN);
		}

		if (tfcan.error_state == TFCAN_ERROR_STATE_IDLE) {
			tfcan.error_state = TFCAN_ERROR_STATE_ERROR_OCCURED;
		}
	}

	led_flicker_tick(&tfcan.com_led_state, system_timer_get_ms(), TFCAN_COM_LED_PIN);
	led_flicker_tick(&tfcan.error_led_state, system_timer_get_ms(), TFCAN_ERROR_LED_PIN);
}

void tfcan_set_config_mode(const bool enable) {
	if (enable) {
		for (uint8_t i = 0; i < TFCAN_NODE_SIZE; ++i) {
			tfcan.node[i]->NCR |= (uint32_t)CAN_NODE_NCR_INIT_Msk | (uint32_t)CAN_NODE_NCR_CCE_Msk;
		}
	} else {
		for (uint8_t i = 0; i < TFCAN_NODE_SIZE; ++i) {
			tfcan.node[i]->NCR &= ~((uint32_t)CAN_NODE_NCR_INIT_Msk | (uint32_t)CAN_NODE_NCR_CCE_Msk);
		}
	}
}

void tfcan_reconfigure_transceiver(void) {
	if (!tfcan.reconfigure_transceiver) {
		return;
	}

	tfcan.reconfigure_transceiver = false;

	logd("reconfigure transceiver\n\r");

	tfcan_set_config_mode(true);

	uint32_t best_div = 1;
	uint32_t best_brp = 1;
	uint32_t best_time_quanta = 8;
	uint32_t best_baud_rate_error = tfcan.baud_rate;

	// find best DIV8/BRP combination based on CAN frequency and baud rate
	for (int32_t div = 8; div > 0; div -= 7) {
		for (uint32_t brp = 1; brp <= 64; ++brp) {
			const uint32_t frequency = (TFCAN_FREQUENCY * 10) / (brp * div); // in 0.1 Hz
			const uint32_t time_quanta = ((frequency / tfcan.baud_rate) + 5) / 10;

			if (time_quanta < 8 || time_quanta > 20) {
				continue;
			}

			const uint32_t baud_rate = frequency / (time_quanta * 10);
			uint32_t baud_rate_error;

			if (baud_rate >= tfcan.baud_rate) {
				baud_rate_error = baud_rate - tfcan.baud_rate;
			} else {
				baud_rate_error = tfcan.baud_rate - baud_rate;
			}

			if (baud_rate_error < best_baud_rate_error) {
				best_div = div;
				best_brp = brp;
				best_time_quanta = time_quanta;
				best_baud_rate_error = baud_rate_error;
			}
		}
	}

	// find best TSEG1 based on sample-point configuration
	uint32_t best_tseg1 = 0;
	uint32_t best_sample_point_error = 1000; // in 0.1 %

	for (uint32_t tseg1 = 16; tseg1 >= 3; --tseg1) {
		const uint32_t sample_point = ((1 + tseg1) * 1000) / best_time_quanta;
		uint32_t sample_point_error;

		if (sample_point >= tfcan.sample_point) {
			sample_point_error = sample_point - tfcan.sample_point;
		} else {
			sample_point_error = tfcan.sample_point - sample_point;
		}

		if (sample_point_error < best_sample_point_error) {
			best_tseg1 = tseg1;
			best_sample_point_error = sample_point_error;
		}

		if (sample_point < tfcan.sample_point) {
			break;
		}
	}

	const uint32_t best_tseg2 = best_time_quanta - best_tseg1 - 1;

	logd("baud rate %u, div %u, brp %u, tseg1 %u, tseg2 %u\n\r",
	     tfcan.baud_rate, best_div, best_brp, best_tseg1, best_tseg2);

	const uint32_t nbtr = (((uint32_t)(best_brp - 1)              << CAN_NODE_NBTR_BRP_Pos)   & (uint32_t)CAN_NODE_NBTR_BRP_Msk) |
	                      (((uint32_t)(tfcan.sync_jump_width - 1) << CAN_NODE_NBTR_SJW_Pos)   & (uint32_t)CAN_NODE_NBTR_SJW_Msk) |
	                      (((uint32_t)(best_tseg1 - 1)            << CAN_NODE_NBTR_TSEG1_Pos) & (uint32_t)CAN_NODE_NBTR_TSEG1_Msk) |
	                      (((uint32_t)(best_tseg2 - 1)            << CAN_NODE_NBTR_TSEG2_Pos) & (uint32_t)CAN_NODE_NBTR_TSEG2_Msk) |
	                      (((uint32_t)(best_div == 8 ? 1 : 0)     << CAN_NODE_NBTR_DIV8_Pos)  & (uint32_t)CAN_NODE_NBTR_DIV8_Msk);

	for (uint8_t i = 0; i < TFCAN_NODE_SIZE; ++i) {
		// bit-timing
		tfcan.node[i]->NBTR = nbtr;

		// frame counter
		tfcan.node[i]->NFCR = (tfcan.node[i]->NFCR & ~(uint32_t)(CAN_NODE_NFCR_CFMOD_Msk |
		                                                         CAN_NODE_NFCR_CFSEL_Msk)) |
		                      (((uint32_t)0b00 << CAN_NODE_NFCR_CFMOD_Pos) & (uint32_t)CAN_NODE_NFCR_CFMOD_Msk) |
		                      (((uint32_t)0b010 << CAN_NODE_NFCR_CFSEL_Pos) & (uint32_t)CAN_NODE_NFCR_CFSEL_Msk);

		// loopback mode
		if (tfcan.transceiver_mode == TFCAN_TRANSCEIVER_MODE_LOOPBACK) {
			if ((tfcan.node[i]->NPCR & (uint32_t)CAN_NODE_NPCR_LBM_Msk) == 0) {
				tfcan.reconfigure_queues = true;
			}

			tfcan.node[i]->NPCR |= (uint32_t)CAN_NODE_NPCR_LBM_Msk;
		} else {
			if ((tfcan.node[i]->NPCR & (uint32_t)CAN_NODE_NPCR_LBM_Msk) != 0) {
				tfcan.reconfigure_queues = true;
			}

			tfcan.node[i]->NPCR &= ~(uint32_t)CAN_NODE_NPCR_LBM_Msk;
		}

		// read-only mode
		if (tfcan.transceiver_mode == TFCAN_TRANSCEIVER_MODE_READ_ONLY) {
			tfcan.node[i]->NCR |= (uint32_t)CAN_NODE_NCR_CALM_Msk;
		} else {
			tfcan.node[i]->NCR &= ~(uint32_t)CAN_NODE_NCR_CALM_Msk;
		}

		// enable LEC event
		tfcan.node[i]->NCR |= (uint32_t)CAN_NODE_NCR_LECIE_Msk;
		tfcan.node[i]->NIPR = (tfcan.node[i]->NIPR & ~(uint32_t)CAN_NODE_NIPR_LECINP_Msk) |
		                      ((uint32_t)TFCAN_NODE_LEC_SRQ_INDEX << CAN_NODE_NIPR_LECINP_Pos);
	}

	tfcan.tx_node = tfcan.node[0];

	if (tfcan.transceiver_mode == TFCAN_TRANSCEIVER_MODE_LOOPBACK) {
		tfcan.rx_node = tfcan.node[1];
	} else {
		tfcan.rx_node = tfcan.node[0];
	}

	tfcan_set_config_mode(false);
}

void tfcan_reconfigure_queues(void) {
	if (!tfcan.reconfigure_queues) {
		return;
	}

	tfcan.reconfigure_queues = false;

	logd("reconfigure queues\n\r");

	// reset queues
	tfcan.tx_buffer_mo_next_index = 0;
	tfcan.tx_buffer_timeout_pending = false;

	for (uint8_t i = 0; i < TFCAN_BUFFER_SIZE; ++i) {
		tfcan.rx_buffer_mo_next_index[i] = 0;
	}

	if (!tfcan.timestamped_frame_enabled) {
		tfcan.tx_backlog = tfcan.backlog;
		tfcan.tx_timestamped_backlog = NULL;
	} else {
		tfcan.tx_backlog = NULL;
		tfcan.tx_timestamped_backlog = tfcan.timestamped_backlog;
	}

	tfcan.tx_backlog_start = 0;
	tfcan.tx_backlog_end = 0;

	if (!tfcan.timestamped_frame_enabled) {
		for (uint16_t i = 0; i < tfcan.tx_backlog_size; ++i) {
			tfcan.tx_backlog[i].mo_type = TFCAN_MO_TYPE_INVALID;
		}
	} else {
		for (uint16_t i = 0; i < tfcan.tx_timestamped_backlog_size; ++i) {
			tfcan.tx_timestamped_backlog[i].frame.mo_type = TFCAN_MO_TYPE_INVALID;
		}
	}

	if (!tfcan.timestamped_frame_enabled) {
		tfcan.rx_backlog = &tfcan.backlog[tfcan.tx_backlog_size];
		tfcan.rx_timestamped_backlog = NULL;
	} else {
		tfcan.rx_backlog = NULL;
		tfcan.rx_timestamped_backlog = &tfcan.timestamped_backlog[tfcan.tx_timestamped_backlog_size];
	}

	tfcan.rx_backlog_start = 0;
	tfcan.rx_backlog_end = 0;

	if (!tfcan.timestamped_frame_enabled) {
		for (uint16_t i = 0; i < tfcan.rx_backlog_size; ++i) {
			tfcan.rx_backlog[i].mo_type = TFCAN_MO_TYPE_INVALID;
		}
	} else {
		for (uint16_t i = 0; i < tfcan.rx_timestamped_backlog_size; ++i) {
			tfcan.rx_timestamped_backlog[i].frame.mo_type = TFCAN_MO_TYPE_INVALID;
		}
	}

	const uint8_t tx_node_index = 0;
	uint8_t rx_node_index = 0;

	if (tfcan.transceiver_mode == TFCAN_TRANSCEIVER_MODE_LOOPBACK) {
		rx_node_index = 1;
	}

	// reset MOs
	for (uint8_t i = 0; i < TFCAN_NODE_SIZE; ++i) {
		XMC_CAN_NODE_EnableConfigurationChange(tfcan.node[i]);
		XMC_CAN_NODE_SetInitBit(tfcan.node[i]);
	}

	while (!XMC_CAN_IsPanelControlReady(CAN)) {}
	XMC_CAN_PanelControl(CAN, XMC_CAN_PANCMD_INIT_LIST, 0, 0);
	while (!XMC_CAN_IsPanelControlReady(CAN)) {}

	for (uint8_t i = 0; i < TFCAN_NODE_SIZE; ++i) {
		XMC_CAN_NODE_DisableConfigurationChange(tfcan.node[i]);
		XMC_CAN_NODE_ResetInitBit(tfcan.node[i]);
	}

	// configure TX MOs
	uint8_t mo_offset = 0;

	tfcan.tx_buffer_mo = &tfcan.mo[mo_offset];

	for (uint8_t i = 0; i < tfcan.tx_buffer_size; ++i) {
		tfcan.tx_buffer_mo[i] = (CAN_MO_TypeDef *)&CAN_MO->MO[mo_offset + i];

		tfcan_mo_init_tx(tfcan.tx_buffer_mo[i]);

		XMC_CAN_AllocateMOtoNodeList(CAN, tx_node_index, mo_offset + i);
		while (!XMC_CAN_IsPanelControlReady(CAN)) {}
	}

	// configure TX FIFO
	if (tfcan.tx_buffer_size > 0) {
		for (uint8_t i = 1; i < tfcan.tx_buffer_size; ++i) {
			tfcan_mo_init_tx_fifo_slave(tfcan.tx_buffer_mo[i], mo_offset);
		}

		tfcan_mo_init_tx_fifo_base(tfcan.tx_buffer_mo[0], mo_offset, mo_offset + tfcan.tx_buffer_size - 1);
	}

	// configure RX MOs and FIFOs
	mo_offset += tfcan.tx_buffer_size;

	for (uint8_t k = 0; k < TFCAN_BUFFER_SIZE; ++k) {
		tfcan.rx_buffer_mo[k] = &tfcan.mo[mo_offset];
		tfcan.rx_buffer_mo_frame_counter[k] = &tfcan.mo_frame_counter[mo_offset];
		tfcan.rx_buffer_mo_age[k] = &tfcan.mo_age[mo_offset];

		CAN_MO_TypeDef **rx_buffer_mo = tfcan.rx_buffer_mo[k];
		const uint8_t rx_buffer_size = tfcan.rx_buffer_size[k];
		const TFCAN_BufferType rx_buffer_type = tfcan.rx_buffer_type[k];

		// configure RX MOs
		for (uint8_t i = 0; i < rx_buffer_size; ++i) {
			rx_buffer_mo[i] = (CAN_MO_TypeDef *)&CAN_MO->MO[mo_offset + i];

			tfcan_mo_init_rx(rx_buffer_mo[i], rx_buffer_type);

			XMC_CAN_AllocateMOtoNodeList(CAN, rx_node_index, mo_offset + i);
			while (!XMC_CAN_IsPanelControlReady(CAN)) {}
		}

		// configure RX FIFO
		if (rx_buffer_size > 0) {
			for (uint8_t i = 1; i < rx_buffer_size; ++i) {
				tfcan_mo_init_rx_fifo_slave(rx_buffer_mo[i], mo_offset);
			}

			tfcan_mo_set_rx_filter(rx_buffer_mo[0], tfcan.rx_filter_mode[k],
			                       tfcan.rx_filter_mask[k], tfcan.rx_filter_identifier[k]);
			tfcan_mo_init_rx_fifo_base(rx_buffer_mo[0], mo_offset, mo_offset + rx_buffer_size - 1);
			tfcan_mo_change_status(rx_buffer_mo[0], TFCAN_MO_SET_STATUS_RX_ENABLE);
		}

		mo_offset += rx_buffer_size;
	}
}

void tfcan_reconfigure_rx_filters(void) {
	if (tfcan.reconfigure_rx_filters == 0) {
		return;
	}

	logd("reconfigure rx filters: %032_8b\n\r", tfcan.reconfigure_rx_filters);

	for (uint8_t k = 0; k < TFCAN_BUFFER_SIZE; ++k) {
		CAN_MO_TypeDef **rx_buffer_mo = tfcan.rx_buffer_mo[k];
		const uint8_t rx_buffer_size = tfcan.rx_buffer_size[k];

		if (rx_buffer_size == 0) {
			break; // remaining RX FIFOs not configured
		}

		if ((tfcan.reconfigure_rx_filters & (1u << k)) == 0) {
			continue;
		}

		tfcan_mo_change_status(rx_buffer_mo[0], TFCAN_MO_RESET_STATUS_RX_ENABLE);
		tfcan_mo_set_rx_filter(rx_buffer_mo[0], tfcan.rx_filter_mode[k],
		                       tfcan.rx_filter_mask[k], tfcan.rx_filter_identifier[k]);
		tfcan_mo_change_status(rx_buffer_mo[0], TFCAN_MO_SET_STATUS_RX_ENABLE);
	}

	tfcan.reconfigure_rx_filters = 0;
}

void tfcan_check_tx_buffer_timeout(void) {
	// FIXME: how to detect transmission failure for a single-shot frame?
	if (tfcan.tx_buffer_timeout <= 0 || tfcan.tx_buffer_size == 0) {
		return;
	}

	if (!tfcan.tx_buffer_timeout_pending) {
		// reading MOFGPR.CUR has a race-condition, because it can be changed by
		// hardware at any time, but the actual value will always be between
		// the last read value and the tx_buffer_mo_next_index value, because
		// MOFGPR.CUR can only increase, but not past tx_buffer_mo_next_index
		tfcan.tx_buffer_timeout_mo_index = tfcan_mo_get_tx_fifo_current(tfcan.tx_buffer_mo[0]);

		CAN_MO_TypeDef *mo = tfcan.tx_buffer_mo[tfcan.tx_buffer_timeout_mo_index];
		uint32_t status = tfcan_mo_get_status(mo);

		if ((status & TFCAN_MO_STATUS_TX_REQUEST) == 0) {
			// either TX FIFO is empty or MOFGPR.CUR changed after reading
			// it. in both cases no timeout can occur, because either no
			// frame is to be transmitted or the current frame has just been
			// transmitted
			const uint8_t index = tfcan_mo_get_tx_fifo_current(tfcan.tx_buffer_mo[0]);

			if (tfcan.tx_buffer_timeout_mo_index != index) {
				logd("tx %d timeout? current changed to %u\n\r", tfcan.tx_buffer_timeout_mo_index, index);
			}

			return;
		}

		if (!system_timer_is_time_elapsed_ms(tfcan.tx_buffer_mo_timestamp[tfcan.tx_buffer_timeout_mo_index],
		                                     tfcan.tx_buffer_timeout)) {
			// current MO is not timed-out yet, therefore MOs further back
			// in the FIFO cannot be timed-out yet either
			return;
		}

		tfcan.tx_buffer_timeout_pending = true;
		tfcan.tx_buffer_timeout_settle_timestamp = system_timer_get_ms();

		logd("tx %d timeout!\n\r", tfcan.tx_buffer_timeout_mo_index);

		++tfcan.tx_buffer_timeout_error_count;

		tfcan.error_occured = true;

		// disable transmission to ensure consitent register content while
		// modifying MOFGPR.CUR and MOSTAT.TXEN1
		tfcan.tx_node->NCR |= (uint32_t)CAN_NODE_NCR_TXDIS_Msk;

		// clear MOSTAT.TXRQ and MOSTAT.RTSEL for timed-out MO to stop
		// TX FIFO processing. see XMC 1400 datasheet figure 18-19 about the
		// MO transmission:
		//
		// - if the MO hasn't won acceptance filtering yet, then it cannot
		//   win it now, because TXRQ is not set
		// - if the MO is being copied to internal transmit buffer then this
		//   will be aborted because TXRQ wasn't always set during the copy
		//   process
		// - if the MO got copied to the internal transmit buffer then the
		//   first RTSEL check will fail now
		// - if the internal buffer was successfully transmitted then the
		//   second RTSEL check will fail now
		// - if the second RTSEL check passed, then the TX FIFO cannot be
		//   stopped anymore from modifying MOFGPR.CUR and MOSTAT.TXEN1. handle
		//   this by waiting 2ms to let this settle
		tfcan_mo_change_status(mo, TFCAN_MO_RESET_STATUS_TX_REQUEST |
		                           TFCAN_MO_RESET_STATUS_RX_TX_SELECTED);
	} else if (system_timer_is_time_elapsed_ms(tfcan.tx_buffer_timeout_settle_timestamp,
	                                           TFCAN_TX_BUFFER_TIMEOUT_SETTLE_DURATION)) {
		const uint8_t index = tfcan_mo_get_tx_fifo_current(tfcan.tx_buffer_mo[0]);

		// check if MOFGPR.CUR changed during the settle period, if not advance
		// TX FIFO read pointer, if it did then no actual timout occured
		if (tfcan.tx_buffer_timeout_mo_index == index) {
			CAN_MO_TypeDef *mo = tfcan.tx_buffer_mo[tfcan.tx_buffer_timeout_mo_index];
			const uint8_t index_next = (tfcan.tx_buffer_timeout_mo_index + 1) % tfcan.tx_buffer_size;
			CAN_MO_TypeDef *mo_next = tfcan.tx_buffer_mo[index_next];

			tfcan_mo_change_status(mo, TFCAN_MO_RESET_STATUS_TX_ENABLE1);
			tfcan_mo_set_tx_fifo_current(tfcan.tx_buffer_mo[0], index_next);
			tfcan_mo_change_status(mo_next, TFCAN_MO_SET_STATUS_TX_ENABLE1);
		} else if (tfcan.tx_buffer_timeout_mo_index != index) {
			logd("tx %d timeout! current changed to %u\n\r", tfcan.tx_buffer_timeout_mo_index, index);
		}

		// re-enable transmission
		tfcan.tx_buffer_timeout_pending = false;
		tfcan.tx_node->NCR &= ~(uint32_t)CAN_NODE_NCR_TXDIS_Msk;
	}
}

// add frame to TX backlog
bool tfcan_enqueue_frame(TFCAN_Frame *frame) {
	if (tfcan.reconfigure_queues ||
	    tfcan.transceiver_mode == TFCAN_TRANSCEIVER_MODE_READ_ONLY ||
	    tfcan.timestamped_frame_enabled ||
	    tfcan.tx_buffer_size == 0 ||
	    tfcan.tx_backlog_size == 0) {
		return false;
	}

	if (tfcan.tx_backlog[tfcan.tx_backlog_end].mo_type != TFCAN_MO_TYPE_INVALID) {
		return false; // TX backlog full
	}

	memcpy(&tfcan.tx_backlog[tfcan.tx_backlog_end], frame, sizeof(TFCAN_Frame));
	tfcan.tx_backlog_end = (tfcan.tx_backlog_end + 1) % tfcan.tx_backlog_size;

	return true;
}

// add timestamped frame to TX backlog
bool tfcan_enqueue_timestamped_frame(TFCAN_TimestampedFrame *timestamped_frame) {
	if (tfcan.reconfigure_queues ||
	    tfcan.transceiver_mode == TFCAN_TRANSCEIVER_MODE_READ_ONLY ||
	    !tfcan.timestamped_frame_enabled ||
	    tfcan.tx_buffer_size == 0 ||
	    tfcan.tx_timestamped_backlog_size == 0) {
		return false;
	}

	if (tfcan.tx_timestamped_backlog[tfcan.tx_backlog_end].frame.mo_type != TFCAN_MO_TYPE_INVALID) {
		return false; // TX backlog full
	}

	memcpy(&tfcan.tx_timestamped_backlog[tfcan.tx_backlog_end], timestamped_frame, sizeof(TFCAN_TimestampedFrame));
	tfcan.tx_backlog_end = (tfcan.tx_backlog_end + 1) % tfcan.tx_timestamped_backlog_size;

	return true;
}

bool tfcan_frame_readable() {
	if (tfcan.reconfigure_queues ||
	    tfcan.timestamped_frame_enabled ||
	    tfcan.rx_backlog_size == 0) {
		return false;
	}

	if (tfcan.rx_backlog[tfcan.rx_backlog_start].mo_type == TFCAN_MO_TYPE_INVALID) {
		return false; // RX backlog empty
	}

	return true;
}

// remove frame from RX backlog
bool tfcan_dequeue_frame(TFCAN_Frame *frame) {
	if(!tfcan_frame_readable()) {
		return false;
	}

	memcpy(frame, &tfcan.rx_backlog[tfcan.rx_backlog_start], sizeof(TFCAN_Frame));

	tfcan.rx_backlog[tfcan.rx_backlog_start].mo_type = TFCAN_MO_TYPE_INVALID;
	tfcan.rx_backlog_start = (tfcan.rx_backlog_start + 1) % tfcan.rx_backlog_size;

	return true;
}

// remove timestamped frame from RX backlog
bool tfcan_dequeue_timestamped_frame(TFCAN_TimestampedFrame *timestamped_frame) {
	if (tfcan.reconfigure_queues ||
	    !tfcan.timestamped_frame_enabled ||
	    tfcan.rx_timestamped_backlog_size == 0) {
		return false;
	}

	if (tfcan.rx_timestamped_backlog[tfcan.rx_backlog_start].frame.mo_type == TFCAN_MO_TYPE_INVALID) {
		return false; // RX backlog empty
	}

	memcpy(timestamped_frame, &tfcan.rx_timestamped_backlog[tfcan.rx_backlog_start], sizeof(TFCAN_TimestampedFrame));

	tfcan.rx_timestamped_backlog[tfcan.rx_backlog_start].frame.mo_type = TFCAN_MO_TYPE_INVALID;
	tfcan.rx_backlog_start = (tfcan.rx_backlog_start + 1) % tfcan.rx_timestamped_backlog_size;

	return true;
}
