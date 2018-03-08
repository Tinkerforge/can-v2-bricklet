/* can-v2-bricklet
 * Copyright (C) 2018 Matthias Bolte <matthias@tinkerforge.com>
 *
 * tfcan_mo.h: XMC CAN Message Object helpers
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

#ifndef TFCAN_MO_H
#define TFCAN_MO_H

#include <stdint.h>
#include <stdbool.h>

#include "xmc_can.h"

// must match Bricklet API FrameType enum
typedef enum {
	TFCAN_MO_TYPE_STANDARD_DATA = 0,
	TFCAN_MO_TYPE_STANDARD_REMOTE,
	TFCAN_MO_TYPE_EXTENDED_DATA,
	TFCAN_MO_TYPE_EXTENDED_REMOTE
} TFCAN_MOType;

// must match Bricklet API BufferType enum
typedef enum {
	TFCAN_BUFFER_TYPE_DATA = 0,
	TFCAN_BUFFER_TYPE_REMOTE
} TFCAN_BufferType;

typedef enum {
	TFCAN_MO_STATUS_RX_PENDING             = CAN_MO_MOSTAT_RXPND_Msk,
	TFCAN_MO_STATUS_TX_PENDING             = CAN_MO_MOSTAT_TXPND_Msk,
	TFCAN_MO_STATUS_RX_UPDATING            = CAN_MO_MOSTAT_RXUPD_Msk,
	TFCAN_MO_STATUS_NEW_DATA               = CAN_MO_MOSTAT_NEWDAT_Msk,
	TFCAN_MO_STATUS_MESSAGE_LOST           = CAN_MO_MOSTAT_MSGLST_Msk,
	TFCAN_MO_STATUS_MESSAGE_VALID          = CAN_MO_MOSTAT_MSGVAL_Msk,
	TFCAN_MO_STATUS_RX_TX_SELECTED         = CAN_MO_MOSTAT_RTSEL_Msk,
	TFCAN_MO_STATUS_RX_ENABLE              = CAN_MO_MOSTAT_RXEN_Msk,
	TFCAN_MO_STATUS_TX_REQUEST             = CAN_MO_MOSTAT_TXRQ_Msk,
	TFCAN_MO_STATUS_TX_ENABLE0             = CAN_MO_MOSTAT_TXEN0_Msk,
	TFCAN_MO_STATUS_TX_ENABLE1             = CAN_MO_MOSTAT_TXEN1_Msk,
	TFCAN_MO_STATUS_DIRECTION              = CAN_MO_MOSTAT_DIR_Msk,
	TFCAN_MO_STATUS_LIST                   = CAN_MO_MOSTAT_LIST_Msk,
	TFCAN_MO_STATUS_POINTER_TO_PREVIOUS_MO = CAN_MO_MOSTAT_PPREV_Msk,
	TFCAN_MO_STATUS_POINTER_TO_NEXT_MO     = (int32_t)CAN_MO_MOSTAT_PNEXT_Msk
} TFCAN_MOStatus;

typedef enum {
	TFCAN_MO_SET_STATUS_RX_PENDING       = CAN_MO_MOCTR_SETRXPND_Msk,
	TFCAN_MO_RESET_STATUS_RX_PENDING     = CAN_MO_MOCTR_RESRXPND_Msk,
	TFCAN_MO_SET_STATUS_TX_PENDING       = CAN_MO_MOCTR_SETTXPND_Msk,
	TFCAN_MO_RESET_STATUS_TX_PENDING     = CAN_MO_MOCTR_RESTXPND_Msk,
	TFCAN_MO_SET_STATUS_RX_UPDATING      = CAN_MO_MOCTR_SETRXUPD_Msk,
	TFCAN_MO_RESET_STATUS_RX_UPDATING    = CAN_MO_MOCTR_RESRXUPD_Msk,
	TFCAN_MO_SET_STATUS_NEW_DATA         = CAN_MO_MOCTR_SETNEWDAT_Msk,
	TFCAN_MO_RESET_STATUS_NEW_DATA       = CAN_MO_MOCTR_RESNEWDAT_Msk,
	TFCAN_MO_SET_STATUS_MESSAGE_LOST     = CAN_MO_MOCTR_SETMSGLST_Msk,
	TFCAN_MO_RESET_STATUS_MESSAGE_LOST   = CAN_MO_MOCTR_RESMSGLST_Msk,
	TFCAN_MO_SET_STATUS_MESSAGE_VALID    = CAN_MO_MOCTR_SETMSGVAL_Msk,
	TFCAN_MO_RESET_STATUS_MESSAGE_VALID  = CAN_MO_MOCTR_RESMSGVAL_Msk,
	TFCAN_MO_SET_STATUS_RX_TX_SELECTED   = CAN_MO_MOCTR_SETRTSEL_Msk,
	TFCAN_MO_RESET_STATUS_RX_TX_SELECTED = CAN_MO_MOCTR_RESRTSEL_Msk,
	TFCAN_MO_SET_STATUS_RX_ENABLE        = CAN_MO_MOCTR_SETRXEN_Msk,
	TFCAN_MO_RESET_STATUS_RX_ENABLE      = CAN_MO_MOCTR_RESRXEN_Msk,
	TFCAN_MO_SET_STATUS_TX_REQUEST       = CAN_MO_MOCTR_SETTXRQ_Msk,
	TFCAN_MO_RESET_STATUS_TX_REQUEST     = CAN_MO_MOCTR_RESTXRQ_Msk,
	TFCAN_MO_SET_STATUS_TX_ENABLE0       = CAN_MO_MOCTR_SETTXEN0_Msk,
	TFCAN_MO_RESET_STATUS_TX_ENABLE0     = CAN_MO_MOCTR_RESTXEN0_Msk,
	TFCAN_MO_SET_STATUS_TX_ENABLE1       = CAN_MO_MOCTR_SETTXEN1_Msk,
	TFCAN_MO_RESET_STATUS_TX_ENABLE1     = CAN_MO_MOCTR_RESTXEN1_Msk,
	TFCAN_MO_SET_STATUS_DIRECTION        = CAN_MO_MOCTR_SETDIR_Msk,
	TFCAN_MO_RESET_STATUS_DIRECTION      = CAN_MO_MOCTR_RESDIR_Msk
} TFCAN_MOChangeStatus;

typedef enum {
	TFCAN_MO_IRQ_POINTER_TRANSMIT = CAN_MO_MOIPR_TXINP_Pos,
	TFCAN_MO_IRQ_POINTER_RECEIVE  = CAN_MO_MOIPR_RXINP_Pos
} TFCAN_MOIRQPointer;

typedef enum {
	TFCAN_MO_EVENT_TRANSMIT = CAN_MO_MOFCR_TXIE_Msk,
	TFCAN_MO_EVENT_RECEIVE  = CAN_MO_MOFCR_RXIE_Msk,
	TFCAN_MO_EVENT_OVERFLOW = CAN_MO_MOFCR_OVIE_Msk
} TFCAN_MOEvent;

void tfcan_mo_init_tx(CAN_MO_TypeDef *const mo);

void tfcan_mo_init_tx_fifo_base(CAN_MO_TypeDef *const mo, const uint8_t base_and_bottom,
                                const uint8_t top);

void tfcan_mo_init_tx_fifo_slave(CAN_MO_TypeDef *const mo, const uint8_t base);

void tfcan_mo_init_rx(CAN_MO_TypeDef *const mo, const TFCAN_BufferType type);

void tfcan_mo_init_rx_fifo_base(CAN_MO_TypeDef *const mo, const uint8_t base_and_bottom,
                                const uint8_t top);

void tfcan_mo_init_rx_fifo_slave(CAN_MO_TypeDef *const mo, const uint8_t base);

uint32_t tfcan_mo_get_status(const CAN_MO_TypeDef *const mo);

void tfcan_mo_change_status(CAN_MO_TypeDef *const mo, const uint32_t mask);

void tfcan_mo_set_irq_pointer(CAN_MO_TypeDef *const mo,
                              const TFCAN_MOIRQPointer irq_pointer,
                              const uint32_t srq_index);

void tfcan_mo_enable_event(CAN_MO_TypeDef *const mo, const uint32_t event);

void tfcan_mo_set_identifier(CAN_MO_TypeDef *const mo, const TFCAN_MOType type,
                             const uint32_t identifier);

void tfcan_mo_set_data(CAN_MO_TypeDef *const mo, const uint8_t *const data,
                       const uint8_t length);

void tfcan_mo_get_identifier(CAN_MO_TypeDef *const mo, TFCAN_MOType * consttype,
                             uint32_t *const identifier);

void tfcan_mo_get_data(CAN_MO_TypeDef *const mo, uint8_t *const data,
                       uint8_t *const length);

#endif
