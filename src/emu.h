#ifndef EMU_H_
#define EMU_H_


/* Configuration values. */
//#define SERIAL_MAX	2
//#define PARALLEL_MAX	1
#define SCREEN_RES_W	256
#define SCREEN_RES_H	192

/* Version info. */
/*
#define EMU_NAME	"LASER310"
#define EMU_NAME_W	L"LASER310"

#ifdef RELEASE_BUILD
#define EMU_VERSION	"1.1"
#define EMU_VERSION_W	L"1.1"
#else
#define EMU_VERSION	"1.1"
#define EMU_VERSION_W	L"1.1"
#endif
*/

/* Filename and pathname info. */
#define CONFIG_FILE	L"laser310.cfg"
#define SCREENSHOT_PATH L"screenshots"


#if defined(ENABLE_LOG_BREAKPOINT) || defined(ENABLE_VRAM_DUMP)
# define ENABLE_LOG_COMMANDS	1
#endif

#define MIN(a, b)             ((a) < (b) ? (a) : (b))
#define ABS(x)		      ((x) > 0 ? (x) : -(x))

#ifdef __cplusplus
extern "C" {
#endif

/* Global variables. */
extern int	dump_on_exit;			/* (O) dump regs on exit*/
extern int	do_dump_config;			/* (O) dump cfg after load */
extern int	start_in_fullscreen;		/* (O) start in fullscreen */
#ifdef _WIN32
extern int	force_debug;			/* (O) force debug output */
#endif
#ifdef USE_WX
extern int	video_fps;			/* (O) render speed in fps */
#endif
extern int	settings_only;			/* (O) show only the settings dialog */
#ifdef _WIN32
extern uint64_t	unique_id;
extern uint64_t	source_hwnd;
#endif
extern wchar_t	log_path[1024];			/* (O) full path of logfile */


extern int	window_w, window_h,		/* (C) window size and */
		window_x, window_y,		/*     position info */
		window_remember,
		vid_resize,			/* (C) allow resizing */
		invert_display,			/* (C) invert the display */
		suppress_overscan;		/* (C) suppress overscans */
extern int	scale;				/* (C) screen scale factor */
extern int	vid_api;			/* (C) video renderer */
extern int	vid_cga_contrast,		/* (C) video */
		video_fullscreen,		/* (C) video */
		video_fullscreen_first,		/* (C) video */
		video_fullscreen_scale,		/* (C) video */
		enable_overscan,		/* (C) video */
		force_43,			/* (C) video */
		gfxcard;			/* (C) graphics/video card */
extern int	serial_enabled[],		/* (C) enable serial ports */
		bugger_enabled,			/* (C) enable ISAbugger */
		isamem_type[],			/* (C) enable ISA mem cards */
		isartc_type;			/* (C) enable ISA RTC card */
extern int	sound_is_float,			/* (C) sound uses FP values */
		GAMEBLASTER,			/* (C) sound option */
		GUS, GUSMAX,			/* (C) sound option */
		SSI2001,			/* (C) sound option */
		voodoo_enabled;			/* (C) video option */
extern uint32_t	mem_size;			/* (C) memory size */
extern int	cpu_manufacturer,		/* (C) cpu manufacturer */
		cpu,				/* (C) cpu type */
		cpu_use_dynarec,		/* (C) cpu uses/needs Dyna */
		enable_external_fpu;		/* (C) enable external FPU */
extern int	time_sync;			/* (C) enable time sync */
extern int	network_type;			/* (C) net provider type */
extern int	network_card;			/* (C) net interface num */
extern char	network_host[522];		/* (C) host network intf */
extern int	hdd_format_type;		/* (C) hard disk file format */

extern int	is_pentium;			/* TODO: Move back to cpu/cpu.h when it's figured out,
							 how to remove that hack from the ET4000/W32p. */


#ifdef ENABLE_LOG_TOGGLES
extern int	buslogic_do_log;
extern int	cdrom_do_log;
extern int	d86f_do_log;
extern int	fdc_do_log;
extern int	ide_do_log;
extern int	serial_do_log;
extern int	nic_do_log;
#endif

#ifndef USE_NEW_DYNAREC
extern FILE	*stdlog;			/* file to log output to */
#endif
extern int	scrnsz_x,			/* current screen size, X */
		scrnsz_y;			/* current screen size, Y */
extern int	efscrnsz_y;
extern int	config_changed;			/* config has changed */


/* Function prototypes. */
#ifdef HAVE_STDARG_H
extern void	pclog_ex(const char *fmt, va_list);
#endif
extern void	pclog_toggle_suppr(void);
extern void	pclog(const char *fmt, ...);
extern void	fatal(const char *fmt, ...);
extern void	set_screen_size(int x, int y);
extern void	set_screen_size_natural(void);
#if 0
extern void	pc_reload(wchar_t *fn);
#endif
extern int	pc_init_modules(void);
extern int	pc_init(int argc, wchar_t *argv[]);
extern void	pc_close(void *threadid);
extern void	pc_reset_hard_close(void);
extern void	pc_reset_hard_init(void);
extern void	pc_reset_hard(void);
extern void	pc_reset(int hard);
extern void	pc_full_speed(void);
extern void	pc_speed_changed(void);
extern void	pc_send_cad(void);
extern void	pc_send_cae(void);
extern void	pc_send_cab(void);
extern void	pc_thread(void *param);
extern void	pc_start(void);
extern void	pc_onesec(void);

extern uint16_t	get_last_addr(void);

/* This is for external subtraction of cycles;
   should be in cpu.c but I put it here to avoid
   having to include cpu.c everywhere. */
extern void	sub_cycles(int c);

extern double	isa_timing;
extern int	io_delay;

#ifdef __cplusplus
}
#endif


#endif	//EMU_H_

