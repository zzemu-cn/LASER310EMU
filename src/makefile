TARGET = ../LASER310EMU

# ANSI
UTF=ANSI

# UNICODE
#UTF=UNICODE

# 配置工具链
AS=as	# 汇编器
CC=gcc	# C编译器
CXX=g++	# C++编译器
RC=windres # 资源编译器
LD=gcc	# 链接器
#LD=g++	# 链接器
OBJDUMP=objdump

# 配置工具链的调用参数
ASFLAGS=-g

CCFLAGS =-g -std=c11   -I. -I./z80 -I./utils -I./win -I./plat/win -IC:/MinGW/include -D$(UTF) -D_FILE_OFFSET_BITS=64
CCXXFLAGS=-g -std=c++17 -I. -I./z80 -I./utils -I./win -I./plat/win -IC:/MinGW/include -D$(UTF) -D_FILE_OFFSET_BITS=64

RCFLAGS=
LDFLAGS=-mwindows -Wl,--stack=0x1000000,-lshlwapi
OBJDUMPFLAGS=-a -f

LDFLAGS_SDL2 = -lsdl2 -lwinmm -limm32 -lole32 -loleaut32 -lversion -lopengl32 -lgdi32 -lgdiplus -lsetupapi

# 我们目标，可用'\'换行

#SRCS=$(wildcard *.c)
#OBJS=$(SRCS:.c=.o)
#OBJS=$(SRCS:.cpp=.o)

SRCS=$(wildcard ./*.c ./z80/*.c ./utils/*.c ./win/*.c ./plat/win/*.c)
OBJS=$(SRCS:.c=.o) win/resource.o

# 如何编译各种目标的过程
# 默认目标（make 不带目标对象，或者make all）

all: $(TARGET)

$(TARGET) : $(OBJS) 
	$(LD) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LDFLAGS_SDL2)

# 指定从.c文件生成.o文件的方法，$@代表输出文件，$<代表输入文件
%.o : %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o : %.s
	$(AS) $(ASFLAGS) -o $@ $<

%.o : %.rc
	$(RC) $(RCFLAGS) $< $@

clean :
	find . -name '*.o' | xargs rm -rf
