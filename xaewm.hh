/*
 * xaewm.hh by xento figal http://xinfo.sf.net/
 * Based form Frank Hale frankhale@yahoo.com
 * a part off xaewm
 */
 
#ifndef _AEWM_HH_
#define _AEWM_HH_

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <list>

#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

using namespace std;

/* Here are the default settings. Change to suit your taste.  If you
 * aren't sure about DEF_FONT, change it to "fixed"; almost all X
 * installations will have that available. En la version xaewm aqui se incluye
 * la orden al boton 2 de lanzar el menu
 */
#define DEF_FONT	"Fixed"
#define DEF_FG		"#ffffff"
#define DEF_BG		"#646a8c"
#define DEF_FC		"#d1d1d1"
#define DEF_BD		"#000000"
#define DEF_NEW1	"" 
#define DEF_NEW2	"/usr/share/xaewm/xaewmenu"
#define DEF_BW		1
#define SPACE		3
#define MINSIZE		15
#define EDGE_SNAP	"true"
#define SNAP		10
#define TEXT_JUSTIFY	"center"
#define WIRE_MOVE	"true"
#define MAX_DESKTOPS	4
#define DEF_FM		"click"

// MOTIF hints
//#define MwmHintsFunctions     		(1l << 0)
#define MwmHintsDecorations   		(1l << 1)
//#define MwmFuncAll            		(1l << 0)
//#define MwmFuncResize         		(1l << 1)
#define MwmFuncMove           		(1l << 2)
//#define MwmFuncIconify        		(1l << 3)
//#define MwmFuncMaximize       		(1l << 4)
//#define MwmFuncClose          		(1l << 5)
#define MwmDecorAll           		(1l << 0)
#define MwmDecorBorder        		(1l << 1)
//#define MwmDecorHandle        		(1l << 2)
#define MwmDecorTitle         		(1l << 3)
//#define MwmDecorMenu          		(1l << 4)
//#define MwmDecorIconify       		(1l << 5)
//#define MwmDecorMaximize      		(1l << 6)

#define PropMwmHintsElements 3

typedef struct MwmHints {
  unsigned long flags, functions, decorations;
} MwmHints;

// aewm++ doesn't provide full GNOME support, enough support has been added
// to make fspanel work properly. More support will be added in the future
// perhaps.

// GNOME hints
//#define WIN_HINTS_SKIP_FOCUS		(1<<0) 
//#define WIN_HINTS_SKIP_TASKBAR	(1<<2) 
#define WIN_HINTS_DO_NOT_COVER  	(1<<5)
#define WIN_STATE_STICKY		(1<<0) 
//#define WIN_STATE_MAXIMIZED_VERT	(1<<2) 
//#define WIN_STATE_MAXIMIZED_HORIZ	(1<<3) 
//#define WIN_STATE_MAXIMIZED		(WIN_STATE_MAXIMIZED_VERT|WIN_STATE_MAXIMIZED_HORIZ) 
//#define WIN_STATE_SHADED		(1<<5) 
//#define WIN_LAYER_DESKTOP		0 
//#define WIN_LAYER_BELOW		2 
//#define WIN_LAYER_NORMAL		4 
//#define WIN_LAYER_ONTOP		6 

// Extended Net Hints stuff 
struct NetWMStates
{
	bool modal;
	bool sticky;
	bool max_vert;
	bool max_horz;
	bool shaded;
	bool skip_taskbar;
	bool skip_pager; 
};

struct Strut
{
	// Size of strut per border
	CARD32 east;
	CARD32 west;
	CARD32 north;
	CARD32 south;
};

#define NET_WM_STATE_MAX_STATES 7

// Someday maybe we will have support for this
// net spec hint. =)
//#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
//#define _NET_WM_MOVERESIZE_SIZE_TOP          1
//#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
//#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
//#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
//#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
//#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
//#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
//#define _NET_WM_MOVERESIZE_MOVE              8   /* Movement only */

#define NET_WM_STICKY_WINDOW	0xffffffff

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

// Text Justify for window titlebars
enum { LEFT_JUSTIFY, CENTER_JUSTIFY, RIGHT_JUSTIFY };

// Shorthand for wordy function calls
#define setmouse(w, x, y) XWarpPointer(dpy, None, w, 0, 0, 0, 0, x, y)
#define Ungrab() XUngrabPointer(dpy, CurrentTime)
#define Grab(w, mask, curs) (XGrabPointer(dpy, w, False, mask, \
    GrabModeAsync, GrabModeAsync, None, curs, CurrentTime) == GrabSuccess)

// Border width accessor to handle hints/no hints
#define BW (has_border ? wm->getOptBW() : 0)

// Multipliers for calling gravitate 
#define APPLY_GRAVITY 1
#define REMOVE_GRAVITY -1

// Modes to call get_incsize with 
#define PIXELS 0
#define INCREMENTS 1

// defined in main.cc
void forkExec(char *);
void sigHandler(int);
int handleXError(Display *, XErrorEvent *);
#ifdef NEED_SETENV
int setenv(char *name, char *value, int clobber);
#endif

enum { FOCUS_FOLLOW, FOCUS_SLOPPY, FOCUS_CLICK };

class Version
{
private:
	string window_manager_name;
	string version;
	string release_date;
	string version_string;
	
public:
	Version() 
	{
		window_manager_name	= "xaewm";
		version			= "0.0.3";
		release_date		= "Dec 14, 2003";
		
		version_string = window_manager_name + " " +
				 version	     + " " +
				 release_date;
	}	
	
	const char* getWindowManagerName()	{ return window_manager_name.c_str();}
	const char* getVersion()		{ return version.c_str();}
	const char* getReleasedate() 	 	{ return release_date.c_str();}
	const char* getVersionString() 		{ return version_string.c_str();}
};

class Client;

#include "basemenu.hh"
#include "genericmenu.hh"
#include "windowmenu.hh"
#include "iconmenu.hh"
#include "client.hh"
#include "windowmanager.hh"

#endif // _AEWM_HH_ 
