// 生成按键编码表，用于LASER310模拟器的自动按键功能
// gcc -o gen_vkey gen_vkey.c
// gen_vkey.exe > ../vkey.c
#include <stdio.h>

typedef struct {
	char	ch;
	int		i;
	int		j;
	int		shift;
} keymap;

typedef struct {
	int		i;
	int		j;
	int		shift;
} vkeymap;

keymap km[] = {
	{'R',0,5,0},
	{'Q',0,4,0},
	{'E',0,3,0},
	//{'',0,2,0},
	{'W',0,1,0},
	{'T',0,0,0},

	{'F',1,5,0},
	{'A',1,4,0},
	{'D',1,3,0},
	//{'LCTRL''RCTRL',1,2,0},
	{'S',1,1,0},
	{'G',1,0,0},

	{'V',2,5,0},
	{'Z',2,4,0},
	{'C',2,3,0},
	//{'LSHIFT''RSHIFT',2,2,0},
	{'X',2,1,0},
	{'B',2,0,0},

	{'4',3,5,0},
	{'1',3,4,0},
	{'3',3,3,0},
	//{'',3,2,0},
	{'2',3,1,0},
	{'5',3,0,0},

	{'M',4,5,0},
	{' ',4,4,0},
	{',',4,3,0},
	//{'',4,2,0},
	{'.',4,1,0},
	{'N',4,0,0},

	{'7',5,5,0},
	{'0',5,4,0},
	{'8',5,3,0},
	{'-',5,2,0},
	{'9',5,1,0},
	{'6',5,0,0},

	{'U',6,5,0},
	{'P',6,4,0},
	{'I',6,3,0},
	{'\n',6,2,0},
	{'O',6,1,0},
	{'Y',6,0,0},

	{'J',7,5,0},
	{';',7,4,0},
	{'K',7,3,0},
	{':',7,2,0},
	{'L',7,1,0},
	{'H',7,0,0},

	{'!',3,4,1},	// shift 1
	{'"',3,1,1},	// shift 2
	{'#',3,3,1},	// shift 3
	{'$',3,5,1},	// shift 4
	{'%',3,0,1},	// shift 5
	{'&',5,0,1},	// shift 6
	{'\'',5,5,1},	// shift 7
	{'(',5,3,1},	// shift 8
	{')',5,1,1},	// shift 9
	{'@',5,4,1},	// shift 0
	{'=',5,2,1},	// shift -

	{'[',6,1,1},	// shift O
	{']',6,4,1},	// shift P

	{'/',7,3,1},	// shift K
	{'?',7,1,1},	// shift L
	{'+',7,4,1},	// shift ;
	{'*',7,2,1},	// shift :

	{'^',4,0,1},	// shift N
	{'\\',4,5,1},	// shift M
	{'<',4,3,1},	// shift ,
	{'>',4,1,1},	// shift .

	// 图形符号
	{'q',0,4,1},
	{'w',0,1,1},
	{'e',0,3,1},
	{'r',0,5,1},
	{'t',0,0,1},
	{'y',6,0,1},
	{'u',6,5,1},
	{'i',6,3,1},

	{'a',1,4,1},
	{'s',1,1,1},
	{'d',1,3,1},
	{'f',1,5,1},
	{'g',1,0,1},
	{'h',7,0,1},
	{'j',7,5,1},

	{'z',2,4,1},

	// 扩展按键  scancode[8] 上下左右 Backspace TAB ESC `  scancode[9] = [ ] \ / LALT RALT
	//{'UP',8,0,0},
	//{'DOWN',8,1,0},
	//{'LEFT',8,2,0},
	//{'RIGHT',8,3,0},
	//{'BACKSPACE',8,4,0},
	//{'TAB',8,5,0},
	//{'ESCAPE',8,6,0},
	//{'NONUSHASH',8,7,0},

	//{'EQUALS',9,1,0},
	//{'LEFTBRACKET',9,2,0},
	//{'RIGHTBRACKET',9,3,0},
	//{'BACKSLASH',9,4,0},
	//{'SLASH',9,5,0},
	//{'LALT',	9,6,0},
	//{'RALT',	9,7,0},
};

vkeymap vkey[256];

int main()
{
	int c;
	int ch;
	for(ch=0; ch<256; ch++) {
		vkey[ch].i = -1;
		vkey[ch].j = -1;
		vkey[ch].shift = -1;
	}
	for(c=0; c<sizeof(km)/sizeof(keymap); c++ ) {
		ch = km[c].ch;
		vkey[ch].i = km[c].i;
		vkey[ch].j = km[c].j;
		vkey[ch].shift = km[c].shift;
	}

	printf("\
#include \"vkey.h\"\n\
vkeymap vkey[256] = {\n\
");

	for(ch=0; ch<256-1; ch++) {
		if(ch>' ' && ch<127)
			printf("\t{%d,%d,%d}, // %c\n",vkey[ch].i,vkey[ch].j,vkey[ch].shift, ch);
		else
			printf("\t{%d,%d,%d}, // %d\n",vkey[ch].i,vkey[ch].j,vkey[ch].shift, ch);
	}

	printf("\
	{-1,-1,-1}\n\
};\n");
	return 0;
}
