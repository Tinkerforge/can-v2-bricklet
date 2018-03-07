/* can-v2-bricklet
 * Copyright (C) 2018 Matthias Bolte <matthias@tinkerforge.com>
 *
 * tfcan_mo.c: XMC CAN Message Object helpers
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

#include "tfcan_mo.h"

#include "configs/config.h"
#include "configs/config_tfcan.h"
#include "bricklib2/logging/logging.h"

#include "xmc_can.h"
#include "xmc_can_map.h"

void tfcan_mo_init_tx(CAN_MO_TypeDef *const mo) {
	// reset all status bits
	tfcan_mo_change_status(mo, TFCAN_MO_RESET_STATUS_RX_PENDING |
	                           TFCAN_MO_RESET_STATUS_TX_PENDING |
	                           TFCAN_MO_RESET_STATUS_RX_UPDATING |
	                           TFCAN_MO_RESET_STATUS_NEW_DATA |
	                           TFCAN_MO_RESET_STATUS_MESSAGE_LOST |
	                           TFCAN_MO_RESET_STATUS_MESSAGE_VALID |
	                           TFCAN_MO_RESET_STATUS_RX_TX_SELECTED |
	                           TFCAN_MO_RESET_STATUS_RX_ENABLE |
	                           TFCAN_MO_RESET_STATUS_TX_REQUEST |
	                           TFCAN_MO_RESET_STATUS_TX_ENABLE0 |
	                           TFCAN_MO_RESET_STATUS_TX_ENABLE1 |
	                           TFCAN_MO_RESET_STATUS_DIRECTION);

	// configure MPN and clear all other MOIPR parts
	mo->MOIPR = (((uint32_t)mo - CAN_BASE - 0x1000U) / 0x0020U) << CAN_MO_MOIPR_MPN_Pos;

	// reset all MOFCR parts
	mo->MOFCR = 0;

	// reset all MOFGPR parts
	mo->MOFGPR = 0;

	// reset all MOAMR parts
	mo->MOAMR = 0x3FFFFFFF;

	// clear idenfifier and set priority to 1
	mo->MOAR = 1U << CAN_MO_MOAR_PRI_Pos;

	// clear data
	mo->MODATAL = 0;
	mo->MODATAH = 0;

	// set direction to TX and mark as valid
	tfcan_mo_change_status(mo, TFCAN_MO_SET_STATUS_DIRECTION |
	                           TFCAN_MO_SET_STATUS_MESSAGE_VALID);
}

void tfcan_mo_init_tx_fifo_base(CAN_MO_TypeDef *const mo, const uint8_t base,
                                const uint8_t top, const uint8_t bottom) {
	mo->MOFCR = (mo->MOFCR & ~(uint32_t)CAN_MO_MOFCR_MMC_Msk) |
	            (((uint32_t)0x2U << CAN_MO_MOFCR_MMC_Pos) & (uint32_t)CAN_MO_MOFCR_MMC_Msk);

	mo->MOFGPR = (mo->MOFGPR & ~(uint32_t)(CAN_MO_MOFGPR_BOT_Msk |
	                                       CAN_MO_MOFGPR_TOP_Msk |
	                                       CAN_MO_MOFGPR_CUR_Msk)) |
	             (((uint32_t)base << CAN_MO_MOFGPR_CUR_Pos) & (uint32_t)CAN_MO_MOFGPR_CUR_Msk) |
	             (((uint32_t)top << CAN_MO_MOFGPR_TOP_Pos) & (uint32_t)CAN_MO_MOFGPR_TOP_Msk) |
	             (((uint32_t)bottom << CAN_MO_MOFGPR_BOT_Pos) & (uint32_t)CAN_MO_MOFGPR_BOT_Msk);

	tfcan_mo_change_status(mo, TFCAN_MO_SET_STATUS_TX_ENABLE0 |
	                           TFCAN_MO_SET_STATUS_TX_ENABLE1);
}

void tfcan_mo_init_tx_fifo_slave(CAN_MO_TypeDef *const mo, const uint8_t base) {
	mo->MOFCR = (mo->MOFCR & ~(uint32_t)CAN_MO_MOFCR_MMC_Msk) |
	            (((uint32_t)0x3U << CAN_MO_MOFCR_MMC_Pos) & (uint32_t)CAN_MO_MOFCR_MMC_Msk);

	mo->MOFGPR = (mo->MOFGPR & ~(uint32_t)(CAN_MO_MOFGPR_BOT_Msk |
	                                       CAN_MO_MOFGPR_TOP_Msk |
	                                       CAN_MO_MOFGPR_CUR_Msk)) |
	             (((uint32_t)base << CAN_MO_MOFGPR_CUR_Pos) & (uint32_t)CAN_MO_MOFGPR_CUR_Msk);

	tfcan_mo_change_status(mo, TFCAN_MO_SET_STATUS_TX_ENABLE0 |
	                           TFCAN_MO_RESET_STATUS_TX_ENABLE1);
}

uint32_t tfcan_mo_get_status(const CAN_MO_TypeDef *const mo) {
	return mo->MOCTR;
}

void tfcan_mo_change_status(CAN_MO_TypeDef *const mo, const uint32_t mask) {
	mo->MOCTR = mask;
}

void tfcan_mo_set_irq_pointer(CAN_MO_TypeDef *const mo,
                              const TFCAN_MOIRQPointer irq_pointer,
                              const uint32_t srq_index) {
	mo->MOIPR = (mo->MOIPR & ~(uint32_t)((uint32_t)CAN_MO_MOIPR_Msk << (uint32_t)irq_pointer)) |
	            (srq_index << (uint32_t)irq_pointer);
}

void tfcan_mo_enable_event(CAN_MO_TypeDef *const mo, const uint32_t event) {
	mo->MOFCR |= event;
}

void tfcan_mo_set_identifier(CAN_MO_TypeDef *const mo, const bool extended,
                             const uint32_t identifier) {
	if (extended) {
		mo->MOAR |= (uint32_t)CAN_MO_MOAR_IDE_Msk;
		mo->MOAR = (mo->MOAR & ~(uint32_t)CAN_MO_MOAR_ID_Msk) |
		           (identifier & (uint32_t)CAN_MO_MOAR_ID_Msk);
	} else {
		mo->MOAR &= ~(uint32_t)CAN_MO_MOAR_IDE_Msk;
		mo->MOAR = (mo->MOAR & ~(uint32_t)CAN_MO_MOAR_ID_Msk) |
		           ((identifier << 18) & (uint32_t)CAN_MO_MOAR_ID_Msk);
	}
}

void tfcan_mo_set_data(CAN_MO_TypeDef *const mo, const uint8_t data[8],
                       const uint8_t length) {
	tfcan_mo_change_status(mo, TFCAN_MO_RESET_STATUS_MESSAGE_VALID);

	mo->MOFCR = (mo->MOFCR & ~(uint32_t)CAN_MO_MOFCR_DLC_Msk) |
	            (((uint32_t)length << CAN_MO_MOFCR_DLC_Pos) & (uint32_t)CAN_MO_MOFCR_DLC_Msk);

	mo->MODATAL = (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
	mo->MODATAH = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) | ((uint32_t)data[7] << 24);

	tfcan_mo_change_status(mo, TFCAN_MO_RESET_STATUS_RX_TX_SELECTED |
	                           TFCAN_MO_SET_STATUS_MESSAGE_VALID |
	                           TFCAN_MO_SET_STATUS_NEW_DATA);
}
