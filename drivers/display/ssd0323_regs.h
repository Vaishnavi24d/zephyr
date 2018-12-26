#ifndef __SSD0323_REGS_H__
#define __SSD0323_REGS_H__

/*Command values*/
#define SSD0323_CMD_SET_COL_ADDR  0x15
#define SSD0323_CMD_SET_ROW_ADDR  0x75
#define SSD0323_CMD_SET_REMAP     0xA0
#define SSD0323_CMD_DISP_ON       0xAE
#define SSD0323_CMD_DISP_OFF      0xAF
#define SSD0323_CMD_SET_CONTRAST  0x81

/*Data inputs for command*/
#define SSD0323_CMD_DAT_VERT_INCR 0x04
#define SSD0323_ROW_END 0x4F
#define SSD0323_COLUMN_END 0x3F

#endif /* __SSD0323_REGS_H__ */

