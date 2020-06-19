#ifndef GBLVAR_H_
#define GBLVAR_H_

/* Version info. */
#define EMU_NAME	"LASER310"
#define EMU_NAME_W	L"LASER310"
#ifdef RELEASE_BUILD
#define EMU_VERSION	"2020-06"
#define EMU_VERSION_W	L"2020-06"
#else
#define EMU_VERSION	"2020-06"
#define EMU_VERSION_W	L"2020-06"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern wchar_t	exe_path[1024];			/* path (dir) of executable */
extern wchar_t	usr_path[1024];			/* path (dir) of user data */
extern wchar_t  cfg_path[1024];			/* full path of config file */

#ifdef __cplusplus
}
#endif

#endif	//GBLVAR_H_
