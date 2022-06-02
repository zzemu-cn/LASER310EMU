#ifndef FD_H_
#define FD_H_

#define ADDRESS_IO_FDC_CT		0x10
#define ADDRESS_IO_FDC_DATA		0x11
#define ADDRESS_IO_FDC_POLL		0x12
#define ADDRESS_IO_FDC_WP		0x13

// 85
#define FD_TRACK_LEN			2500000
#define FD_DISK_LEN				(FD_TRACK_LEN*40)

// 89
//#define FD_TRACK_LEN			2380000
//#define FD_DISK_LEN				(2380000*40)

// 实际读取操作波形 2.5 + 19.5 + 2.5 + ?
//#define FD_POLL_LEN				22
//#define FD_POLL_LEN				44
//#define FD_POLL_LEN				30

//取POLLING成功（值为1），到读取DATA中间间隔了 0x43 (67) 个时钟周期。
//时钟位长 28 个
//在 5675 5685 两个 POLLING 间隔 33
// 33 + 67 = 100
//#define FD_POLL_LEN				33 --- 48

//#define FD_POLL_LEN				30
//#define FD_POLL_LEN					33
//#define FD_POLL_LEN				40
//#define FD_POLL_LEN				44

// 报错
//#define FD_POLL_LEN				45

//
#define FD_POLL_LEN				28


//两次读取 POLLING 的时间相差 0x16个时钟周期，所以 POLLING 的长度不小于 0x16，这样不会跳过POLLING信号。
//这个值大于 0x21。过大，有可能影响到写入时的定位。
//选定长度定为 0x2C。

#include <stdint.h>

extern uint8_t fd_buf_d1[];
extern uint8_t fd_buf_d2[];

extern uint32_t fd_pos;
// 两个软驱公用一个位置
//extern uint32_t fd_pos_d1;
//extern uint32_t fd_pos_d2;

extern uint8_t fd_track_d1;
extern uint8_t fd_track_d2;

extern uint8_t fdc_poll_q;
extern uint8_t fdc_poll_dat;
extern uint32_t fd_poll_pos;

extern uint8_t fd_ct_latch;

extern uint8_t fdc_dat_latch;

uint8_t fdc_io_ct(uint8_t ct);
uint8_t fdc_io_poll();
uint8_t fdc_io_data();

uint32_t fdc_io_cycles();
uint8_t* fd_date_d1();

#endif	// FD_H_
