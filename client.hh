/*
 * xaewm client.hh by xento figal http://xinfo.sf.net 
 * based in code by 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 */
 
#ifndef _CLIENT_HH_
#define _CLIENT_HH_

#include "xaewm.hh"

class Client
{
private: /* Member Variables */

    	Display 	*dpy;
	Window 		root;
	XSizeHints	*size;
    	Colormap	cmap;
	int 		screen;

	// Screen resolution
	int 	xres;
	int	yres;

    	char	*name;	// Name used to display in titlebar

	Window	window;	// actual client window
	Window  frame;	// parent window which we reparent the client to
	Window  title;	// window which holds title
	Window  trans;	// window id for which this client is transient for

	WindowMenu *window_menu; // menu which lets us change the clients
	    		 	 // virtual desktop.

	// The position and dimensions of the client window
	int  x;	
	int  y;
	int  width;
	int  height;

	// The old position and dimensions of the client, used 
	// in the maximize function.
	int  old_x; 
	int  old_y;
	int  old_width;
	int  old_height;   

    	bool has_focus;
   	bool has_title;
	bool has_border;

	bool is_being_dragged;
	bool is_being_resized;
	bool do_drawoutline_once; // used for wire move
	bool wire_move; // Do we wanna move clients by wire or opaque?

    	bool is_shaded;
    	bool is_iconified;
	bool is_maximized;
	bool is_maximized_vertical;
	bool is_maximized_horizontal;
	bool is_sticky;
	bool is_always_on_top;
	bool is_visible;

	#ifdef SHAPE
    		bool has_been_shaped;
	#endif

	Strut *client_strut;		
	bool has_strut;
	bool has_extended_net_name;
	bool skip_taskbar;
	bool skip_pager;

    	int belongs_to_desktop;
    	Time last_button1_time;
    	int ignore_unmap;
 	
	// For window title placement
	XCharStruct overall;
	int direction;
	int ascent;
	int descent;
	int text_width;
	int text_justify;
	int justify_style;
	
	// Used in client move
	int pointer_x, pointer_y;
	int old_cx, old_cy;

private: /* Member Functions */

	void initialize(Display *d);
	void updateNetWMStates();
	void getXClientName();

	void redraw();
	void sweep();
	void drawOutline();
	int  getIncsize(int *, int *, int);
	void initPosition();
	void fixupPositionBasedOnStruts(Strut* temp_strut);
	void reparent();
	int  theight();
	void sendConfig();
	void gravitate(int);

	#ifdef SHAPE
		void setShape();
	#endif

public: /* Member Functions */

	Client(Display *d, Window new_client);
	~Client();
		
	void makeNewClient(Window);
	void removeClient();
	
	char* getClientName() const { return name; }
	char* getClientIconName() const { return name; } // for now just return name
	
	Window getFrameWindow() const	{ return frame; }
	Window getAppWindow() 	const { return window; }
	Window getTitleWindow()	const { return title;	}
	Window getTransientWindow() const { return trans; }

	bool hasWindowDecorations() 	const { return has_title; }
	bool hasFocus()			const { return has_focus; }
	
	int belongsToWhichDesktop() 	const { return belongs_to_desktop;}
	bool isAlwaysOnTop()		const { return is_always_on_top;}
	bool isIconified() 		const { return is_iconified; 	}
	bool isSticky() 		const { return is_sticky; 	}
	bool isVisible()		const { return is_visible;	}

	void raise();
	void lower();

	void setFocus(bool focus); // (decieving name) Only paints the titlebar in the focus color

	void hide();
	void unhide();
	void iconify();
	void shade();
	void maximize();

	void setDesktop(int desk);

	void handleButtonEvent(XButtonEvent *);
	void handleConfigureRequest(XConfigureRequestEvent *);
	void handleMapRequest(XMapRequestEvent *);
	void handleUnmapEvent(XUnmapEvent *);
	void handleDestroyEvent(XDestroyWindowEvent *);
	void handleClientMessage(XClientMessageEvent *);
	void handlePropertyChange(XPropertyEvent *);
	void handleEnterEvent(XCrossingEvent *);
	void handleColormapChange(XColormapEvent *);
	void handleExposeEvent(XExposeEvent *);
	void handleFocusInEvent(XFocusChangeEvent *);
	void handleFocusOutEvent(XFocusChangeEvent *);
	void handleMotionNotifyEvent(XMotionEvent *);

	#ifdef SHAPE
		void handleShapeChange(XShapeEvent *);
	#endif
};

#endif
