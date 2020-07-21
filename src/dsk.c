#include <stdint.h>
#include <string.h>

#include "fd.h"

#define CLK_BIT 28
#define DAT_BIT 87

// 98560 --- 99400
// 98560 = 154 * 16  * 40
#define DSK_FMT		98560
#define DSK_FMT_T_LEN 2464

#define DSK_FMT_MAX 99400
#define DSK_FMT_T_LEN_MAX 2485

// 最后一轨不少于 2464 字节


int dsk_put_q(uint8_t *pt, int pos, uint8_t q, int cnt)
{
	while(pos>=FD_TRACK_LEN) pos-=FD_TRACK_LEN;

	while(cnt) {
		pt[pos]=q;
		pos++;
		if(pos>=FD_TRACK_LEN) pos-=FD_TRACK_LEN;

		cnt--;
	}

	return pos;
}

int dsk_put_byte(uint8_t* pt, int pos, uint8_t ch, uint8_t *Q)
{
	uint8_t q = *Q;
	int bit;
	for(bit=0;bit<8;bit++) {
		if((ch<<bit)&0x80) {
			q = q?0:1;
			pos = dsk_put_q(pt,pos,q,CLK_BIT);
			q = q?0:1;
			pos = dsk_put_q(pt,pos,q,DAT_BIT);
		} else {
			q = q?0:1;
			pos = dsk_put_q(pt,pos,q,CLK_BIT+DAT_BIT);
		}
	}
	*Q = q;
	return pos;
}

int dsk_load(uint8_t* buf, uint8_t *dsk_dat, int dsk_len)
{
	uint8_t *p, *pt;
	uint8_t q;
	int pos;
	int szDskTrack, szDskTrack39;
	int szDskT;
	// 98560 = 154 * 16  * 40
	// 2464 --- 2485
	szDskTrack = DSK_FMT_T_LEN;
	while(szDskTrack<=DSK_FMT_T_LEN_MAX) {
		if(szDskTrack*40>=dsk_len) {
			szDskTrack39 = dsk_len-szDskTrack*39;
			break;
		}
		szDskTrack++;
	}

	if(szDskTrack>DSK_FMT_T_LEN_MAX) return 0;

	// 判断格式是否支持
	//if(szDskTrack39<DSK_FMT_T_LEN) return 0;

	// 有不少镜像最后一道的长度错误
	if(szDskTrack39<DSK_FMT_T_LEN-154) return 0;
	//if(szDskTrack39<DSK_FMT_T_LEN-154*2) return 0;


	memset(buf, 0, FD_DISK_LEN);
	for(int t=0;t<40;t++) {
		pos=200;
		q = 0;
		p = dsk_dat + (t*szDskTrack);
		pt = buf+(t*FD_TRACK_LEN);
		szDskT = (t==39)?szDskTrack39:szDskTrack;
		for(int i=0;i<szDskT;i++) {
			pos = dsk_put_byte(pt, pos, p[i], &q);
		}
	}
	return dsk_len;
}

