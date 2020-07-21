#include "prgdef.h"
#include "bithacks.h"
#include "fd.h"

/*
PHASE0			FDC_CT[0]
PHASE1			FDC_CT[1]
PHASE2			FDC_CT[2]
PHASE3			FDC_CT[3]
DRIVE1			FDC_CT[4]
DRIVE2			FDC_CT[7]
MOTOR			(FDC_CT[4])|(FDC_CT[7])
WRITE_REQUEST_N		FDC_CT[6]
WRITE_DATA_BIT		FDC_CT[5]
*/

/*
	旋转一圈需要的时钟数 60/85*1000000*3.54 = 2498824
	按每轨道2500000个数据模拟

	DSK 文件
	98560 = 154 * 16  * 40

	POLLING 参数
	0.7083us 1 时钟位 2.5个时钟周期
	5.5us    0        19.47个时钟周期
	0.7083us 1 数据位 2.5个时钟周期
	23.29us  0        82.446个时钟周期
	总长30.2066us 107个时钟周期
	
	RD DATA 参数
	1.125us  1 位于两个 polling 之间
	31.08us  0

	记录数据0，需要写入的信号翻转1次，记录1个1us方波信号。
	记录数据1，需要写入的信号翻转2次，记录2个1us方波信号。

	两次读取 POLLING 的时间相差 0x16个时钟周期

来自bill的 FM Decode Y1 RDDATA A D0 D12_CLR.logicdata

	物理软驱抓到的数据，过0有1.125us的上升沿：
	数据 0

	整个数据
	32.25us 其中上沿1.125us
	31.88us 其中上沿1.125us
	31.63us 其中上沿1.125us
	30.25us 其中上沿1.125us
	30us 其中上沿1.125us

	数据 1

	时钟位
	12.25us 其中上沿1.125us
	数据位
	19.75us 其中上沿1.125us
*/

uint8_t fd_buf_d1[FD_DISK_LEN];
uint8_t fd_buf_d2[FD_DISK_LEN];

uint32_t fd_pos;
// fd_pos+elapsed_cycles 是软驱位置
// 两个软驱公用一个位置
//uint32_t fd_pos_drive1;
//uint32_t fd_pos_drive2;

uint8_t fd_track_d1;
uint8_t fd_track_d2;

uint8_t fdc_poll_q;
uint8_t fdc_poll_dat;
uint32_t fd_poll_pos;


uint8_t fdc_dat_latch;

uint8_t fd_ct_latch;
uint32_t fd_last_pos;


#define DRIVE1(CT)	(CT&0x10)
#define DRIVE2(CT)	(CT&0x80)

#define	WRITE_DATA_BIT(CT)	(CT&0x20)?1:0
#define WRITE_REQUEST_N(CT)	(CT&0x40)


// 未实现，可以改进的地方：支持 1/4 track

// ph0 ph2 或 ph1 ph3 可以抵消
uint8_t seek_phase_mask(uint8_t ct)
{
	if((ct&0x05)==0x05) ct &= 0xFA;	// ph0 ph2
	if((ct&0x0A)==0x0A) ct &= 0xF5;	// ph1 ph3
	return ct;
}

void seek_phase(uint8_t ct)
{
	uint8_t last_ct;

	ct = seek_phase_mask(ct);
	last_ct = seek_phase_mask(fd_ct_latch);

	if(
			((ct&0x0f)==0x01) && (last_ct&0x02) // PH0 << PH1
		||	((ct&0x0f)==0x02) && (last_ct&0x04) // PH1 << PH2
		||	((ct&0x0f)==0x04) && (last_ct&0x08) // PH2 << PH3
		||	((ct&0x0f)==0x08) && (last_ct&0x01) // PH3 << PH0
	) {
		if(DRIVE1(ct)&&(fd_track_d1>0)) fd_track_d1--;
		if(DRIVE2(ct)&&(fd_track_d2>0)) fd_track_d2--;
	}

	if(
			((ct&0x0f)==0x01) && (last_ct&0x08) // PH0 << PH3
		||	((ct&0x0f)==0x02) && (last_ct&0x01) // PH1 << PH0
		||	((ct&0x0f)==0x04) && (last_ct&0x02) // PH2 << PH1
		||	((ct&0x0f)==0x08) && (last_ct&0x04) // PH3 << PH2
	) {
		if(DRIVE1(ct)&&(fd_track_d1<79)) fd_track_d1++;
		if(DRIVE2(ct)&&(fd_track_d2<79)) fd_track_d2++;
	}
}

uint8_t fdc_io_ct(uint8_t ct)
{
	//int d1, d2;
	//static uint8_t last_phase1, last_phase2;
	uint32_t pos;
	uint8_t dat;
	int c1, c2, i;


	// 寻道
	seek_phase(ct);

	// 计算当前位置
	pos = fd_pos;
	if(pos>=FD_TRACK_LEN) pos-=FD_TRACK_LEN;

	// 写入磁道
	// 先补上次写入数据之后的写操作
	// 通常写入和读取不会同时进行，延迟处理写入不会带来问题。
	if(!WRITE_REQUEST_N(fd_ct_latch)) {
		dat = WRITE_DATA_BIT(fd_ct_latch);
		
		if(pos<fd_last_pos)
			c1 = pos+FD_TRACK_LEN-fd_last_pos;
		else
			c1 = pos-fd_last_pos;
		
		c2 = c1;

		if(DRIVE1(fd_ct_latch)) {
			i=fd_last_pos;
			while(c1) {
				fd_buf_d1[(fd_track_d1/2)*FD_TRACK_LEN+i] = dat;
				i++;if(i>=FD_TRACK_LEN) i-=FD_TRACK_LEN;
				c1--;
			}
		}

		if(DRIVE2(fd_ct_latch)) {
			i=fd_last_pos;
			while(c2) {
				fd_buf_d2[(fd_track_d2/2)*FD_TRACK_LEN+i] = dat;
				i++;if(i>=FD_TRACK_LEN) i-=FD_TRACK_LEN;
				c2--;
			}
		}
	}

/*
	if(!WRITE_REQUEST_N(ct)) {
		if(DRIVE1(ct)) {
			// 当前位置
			fd_buf_d1[(fd_track_d1/2)*FD_TRACK_LEN+pos] = WRITE_DATA_BIT(ct);
		}

		if(DRIVE2(ct));
	}
*/

	fd_ct_latch = ct;
	fd_last_pos = pos;

	return ct;
}

uint8_t fd_polling();

uint8_t fdc_io_poll()
{
	uint8_t x;

	fd_polling();

	x=fdc_poll_q?0x80:0;

	return x;
}

uint8_t fdc_io_data()
{
	fd_polling();

	// 先移位后清除，有先后顺序

	// 数据的移位操作
	fdc_dat_latch <<= 1;
	if(!fdc_poll_q)
		fdc_dat_latch|=1;

	// 清POLLING
	fdc_poll_q = 0;

	return fdc_dat_latch;
}

uint8_t fd_polling()
{
	int c, i, pos;
	uint8_t *fd_buf;
	uint8_t *p;
	uint8_t dat = 0xFF;
	int turn;

	if(DRIVE1(fd_ct_latch)) {
		fd_buf = fd_buf_d1;
	} else if(DRIVE1(fd_ct_latch)) {
		fd_buf = fd_buf_d2;
	} else {
		fd_buf = NULL;
	}

	pos = fd_pos;

	if(pos>=fd_poll_pos)
		c = pos - fd_poll_pos;
	else
		c = pos+FD_TRACK_LEN - fd_poll_pos;

	if(fd_buf)
		p = fd_buf_d1+(fd_track_d1/2)*FD_TRACK_LEN;
	else
		p = NULL;

	i = fd_poll_pos;

	dat = 0;
	while(c>0) {
		if(p) dat = p[i];

		turn=0;
		if(fdc_poll_dat!=dat) {	// 翻转
			fdc_poll_dat = dat;
			turn=1;
		}

		if(turn)
			fdc_poll_q = fdc_poll_q?0:1;

		i++;
		c--;
	}

	if(pos>=FD_TRACK_LEN) pos -= FD_TRACK_LEN;
	fd_poll_pos = pos;

	return fdc_poll_q;
}


// 辅助调试用，两次 IO 间的计数

uint32_t fdc_io_last_pos=0;

uint32_t fdc_io_cycles()
{
	uint32_t	pos, c;

	pos = fd_pos;
	if(pos>=FD_TRACK_LEN) pos-=FD_TRACK_LEN;

	if(fdc_io_last_pos>pos)
		c = pos+FD_TRACK_LEN - fdc_io_last_pos;
	else
		c = pos - fdc_io_last_pos;

	fdc_io_last_pos = pos;

	return c;
}


uint8_t* fd_date_d1()
{
	uint8_t *p;
	int pos;
	int i;
	p = fd_buf_d1+(fd_track_d1/2)*FD_TRACK_LEN;
	pos = fd_pos-110;
	if(pos<0) pos+=FD_TRACK_LEN;

	for(i=0;i<141;i++) {
		if(pos>=FD_TRACK_LEN) pos-=FD_TRACK_LEN;
		if(i!=110)
			tmp_buf[i] = p[pos]?'1':'0';
		else
			tmp_buf[i] = p[pos]?'*':'^';
		pos++;
	}
	tmp_buf[i]=0;

	return tmp_buf;
}
