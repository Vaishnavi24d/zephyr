/*
 * Copyright (c) 2018 Zilogic Systems.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __SSD0323_REGS_H__
#define __SSD0323_REGS_H__

/* Command values */
#define SSD0323_CMD_SET_COL_ADDR  0x15
#define SSD0323_CMD_SET_ROW_ADDR  0x75
#define SSD0323_CMD_SET_REMAP     0xA0
#define SSD0323_CMD_DISP_ON       0xAE
#define SSD0323_CMD_DISP_OFF      0xAF
#define SSD0323_CMD_SET_CONTRAST  0x81

/* Data inputs for command */
#define SSD0323_CMD_DAT_VERT_INCR 0x04
#define SSD0323_ROW_END DT_SSD0323_PANEL_HEIGHT
#define SSD0323_COLUMN_END (DT_SSD0323_PANEL_WIDTH / 2) /* Convert pixels to bytes(/2) */

#endif /* __SSD0323_REGS_H__ */
