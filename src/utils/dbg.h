#ifndef _DBG_H_
#define _DBG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define	DBG_ENABLED	1
#define DBG_FILE	"dbg.txt"
static FILE* dbg_file;

inline static __attribute__((constructor))
void dbg_file_init(void)
{
	dbg_file = stderr;
	if(DBG_ENABLED) {
		dbg_file = fopen(DBG_FILE, "w");
		if(!dbg_file) {
			dbg_file = stderr;
			fprintf(stderr, "Cannot open dbg file '%s'\n", DBG_FILE);
		}
	}
}

/*
#define DBG_PRINTF(...)	while(DBG_ENABLED){fprintf(dbg_file, __VA_ARGS__);fflush(dbg_file);break;}
*/

static void fmtprint(FILE *stream, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stream, fmt, ap);
  fprintf(stream, "\n");
}

static void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

static char *format(char *fmt, ...) {
  char buf[2048];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  return strdup(buf);
}


//#define DEBUG 0

/// util.c

#define DBG
#define DBGF

#ifdef DEBUG

//#define DBG_OSTREAM stderr
#define DBG_OSTREAM stdout

#undef DBG
#define DBG(fmt,arg...) fmtprint(DBG_OSTREAM,"%s %s %d:"fmt,__FILE__,__FUNCTION__,__LINE__,##arg)

#undef DBGF
#define DBGF(flag,fmt,arg...) if(DEBUG&&flag)fmtprint(DBG_OSTREAM,"%s %s %d:"fmt,__FILE__,__FUNCTION__,__LINE__,##arg)

#endif


#define DBG1(arg...) DBGF(1,##arg)
#define DBG2(arg...) DBGF(3,##arg)



#define DBGF_1		1
#define DBGF_2		2
#define DBGF_FLAG3	4
#define DBGF_FLAG4	8


#ifdef DEBUG

#undef DEBUG
#define DEBUG		(DBGF_1|DBGF_2)
#endif


#define LOG(fmt,arg...) fmtprint(DBG_OSTREAM,"%s %s %d:"fmt,__FILE__,__FUNCTION__,__LINE__,##arg)

#define INFO(fmt,arg...) fmtprint(DBG_OSTREAM,fmt,##arg)


#endif /* dbg.h */
