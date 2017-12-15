/*
 * windowmanager.hh
 * Copyleft 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 * Modified by xento figal http://xinfo.sf.net
 */

#ifndef _WINDOWMANAGER_HH_
#define _WINDOWMANAGER_HH_

#include "xaewm.hh"

class WindowManager
{
private: /* member variables */

	Version *version;		

	list<Client*> client_list;

	list<Window> client_window_list;

	WindowMenu *window_menu;
	IconMenu   *icon_menu;
	
	Client* focused_client;
	XFontStruct *font;
	GC invert_gc, string_gc, border_gc;
	XColor fg, bg, bd, fc;
	Cursor move_curs, resize_curs, arrow_curs;

	Display *dpy;	
	Window 	root;   
	Window  gnome_check_win;		
	Window  gnome_button_proxy_win;  
	Window	extended_hints_win;

	int 	screen; 
	int current_desktop;
	
	// The screen max resolutions (x,y)
	int xres;	
	int yres;	

	#ifdef SHAPE
		int shape, shape_event;
	#endif

	Strut *master_strut;
	list<Strut*> client_struts;

	string 	command_line;
	int 	max_desktops;
	int 	focus_model;	
	char 	*opt_display,	
		*opt_font, 
		*opt_fc, 
		*opt_fg,
		*opt_fm, 
		*opt_bg, 
		*opt_bd, 
		*opt_tj,
		*opt_wm,
		*opt_es,
		*opt_new1, 
		*opt_new2;
	int	opt_bw;
	int 	opt_text_justify;
	bool 	wire_move;	
	bool 	edge_snap;	

	static KeySym alt_keys[];
	
public: /* member variables */

 	Atom 	atom_wm_state;
	Atom	atom_wm_change_state;
	Atom	atom_wm_protos; 
	Atom	atom_wm_delete; 
	Atom	atom_wm_cmapwins;
	Atom	atom_wm_takefocus;

	// Gnome stuff
	Atom 	atom_gnome_win_client_list,
	     	atom_gnome_win_state,
	     	atom_gnome_win_hints,
	     	//atom_gnome_win_layer,
	     	atom_gnome_win_supporting_wm_check,
	     	atom_gnome_win_desktop_button_proxy,
	     	atom_gnome_win_workspace,
	     	atom_gnome_win_workspace_count,
		atom_gnome_win_protocols;

	// Extended WM Hints
	Atom	atom_extended_net_supported,
		atom_extended_net_client_list,
		atom_extended_net_client_list_stacking,
		atom_extended_net_number_of_desktops,
		atom_extended_net_desktop_geometry,
		atom_extended_net_desktop_viewport,
		atom_extended_net_current_desktop,
		//atom_extended_net_desktop_names,
		atom_extended_net_active_window,
		atom_extended_net_workarea,  
		atom_extended_net_supporting_wm_check,
		//atom_extended_net_virtual_roots,
		atom_extended_net_close_window,
		//atom_extended_net_wm_moveresize,
		atom_extended_net_wm_name,
		//atom_extended_net_wm_visible_name,
		//atom_extended_net_wm_icon_name,
		//atom_extended_net_wm_visible_icon_name,
		atom_extended_net_wm_desktop,
		//atom_extended_net_wm_window_type,
		//atom_extended_net_wm_window_type_desktop,
		//atom_extended_net_wm_window_type_dock,
		//atom_extended_net_wm_window_type_toolbar,
		//atom_extended_net_wm_window_type_menu,
		//atom_extended_net_wm_window_type_dialog,
		//atom_extended_net_wm_window_type_normal,
		atom_extended_net_wm_state,
		atom_extended_net_wm_state_modal,
		atom_extended_net_wm_state_sticky,
		atom_extended_net_wm_state_maximized_vert,
		atom_extended_net_wm_state_maximized_horz,
		atom_extended_net_wm_state_shaded,
		atom_extended_net_wm_state_skip_taskbar,
		atom_extended_net_wm_state_skip_pager,
		atom_extended_net_wm_strut;
		//atom_extended_net_wm_icon_geometry,
		//atom_extended_net_wm_icon,
		//atom_extended_net_wm_pid,
		//atom_extended_net_wm_handled_icons,
		//atom_extended_net_wm_ping;
	
	Atom net_wm_states[NET_WM_STATE_MAX_STATES];

	// Atom for motif hints
	Atom 	atom_mwm_hints;

private: /* Member Functions */
	
	void setupSignalHandlers();
	void setupDisplay();	
	void cleanup();
	void doEventLoop();
	void scanWins();
	
	void handleKeyPressEvent(XEvent *ev);
	void handleButtonPressEvent(XEvent *ev);
	void handleButtonReleaseEvent(XEvent *ev);
	void handleConfigureRequestEvent(XEvent *ev);
	void handleMotionNotifyEvent(XEvent *ev);
	void handleMapRequestEvent(XEvent *ev);
	void handleUnmapNotifyEvent(XEvent *ev);
	void handleDestroyNotifyEvent(XEvent *ev);
	void handleEnterNotifyEvent(XEvent *ev);
	void handleLeaveNotifyEvent(XEvent *ev);
	void handleFocusInEvent(XEvent *ev);
	void handleFocusOutEvent(XEvent *ev);
	void handleClientMessageEvent(XEvent *ev);
	void handleColormapNotifyEvent(XEvent *ev);
	void handlePropertyNotifyEvent(XEvent *ev);
	void handleExposeEvent(XEvent *ev);
	void handleDefaultEvent(XEvent *ev);

public: /* Member Functions */

	WindowManager(int argc, char** argv);

	void parseCommandLine(int argc, char** argv);
	void quitNicely();
	void restart();

	// Functions pertaining to the client list.
	inline list<Client*> getClientList() const { return client_list; }
	
	void addClient(Client *c);
	void removeClient(Client* c);
	Client* findClient(Window w);

	void focusPreviousWindowInStackingOrder();
	void unfocusAnyStrayClients();
	void restackOnTopWindows();
	void findTransientsToMapOrUnmap(Window win, bool hide); 
	
	inline WindowMenu* getWindowMenu() 	const { return window_menu; }
	inline IconMenu*   getIconMenu() 	const { return icon_menu; }
	
	void updateIconMenu();
	void addClientToIconMenu(Client *c);
	void removeClientFromIconMenu(Client *c);
	
	inline XFontStruct* getFont() 	const { return font; 		}
	inline GC getInvertGC() 	const { return invert_gc; 	}
	inline GC getStringGC() 	const { return string_gc; 	}
	inline GC getBorderGC() 	const { return border_gc; 	}
	inline Cursor getMoveCursor() 	const { return move_curs;	}
	inline Cursor getResizeCursor() const { return resize_curs;	}
	inline Cursor getArrowCursor()  const { return arrow_curs; 	}
	inline XColor getFGColor() 	const { return fg; 		}
	inline XColor getFCColor() 	const { return fc; 		}
	inline XColor getBGColor() 	const { return bg; 		}
	inline XColor getBDColor() 	const { return bd; 		}
	
	#ifdef SHAPE
		inline int getShape() 		const { return shape; }
		inline int getShapeEvent() 	const { return shape_event; }
	#endif 

	long getWMState(Window window);
	void setWMState(Window window, int state);
	void sendWMDelete(Window window);
	int sendXMessage(Window w, Atom a, long mask, long x);
	MwmHints* getMWMHints(Window w);

	void getMousePosition(int *x, int *y);
	void goToDesktop(int d);
	inline int getCurrentDesktop() const { return current_desktop; }
	void setCurrentDesktop(int desk);

	// Returns a number corresponding to the current focus model.
	inline int getFocusModel() const { return focus_model; }

	// Accepts a number corresponding to a new focus model.
	inline void setFocusModel(int new_fm)
	{
		if (new_fm == FOCUS_FOLLOW || new_fm == FOCUS_SLOPPY || new_fm == FOCUS_CLICK) focus_model = new_fm;
	}

	inline int getTextJustify()	const { return opt_text_justify; }
	inline bool getWireMove() 	const { return wire_move; }
	inline bool getEdgeSnap() 	const { return edge_snap; }
	inline int getMaxDesktops() 	const { return max_desktops; }
	void setMaxDesktops(int max_desks) { max_desktops=max_desks; }

	inline int getOptBW() 	const { return opt_bw; 		}
	
	inline Display* getDisplay()  	const { return dpy; 	}
	inline Window 	getRootWindow()	const { return root; 	}
	inline int 	getScreen() 	const { return screen; 	}
	inline int 	getXRes() 	const { return xres; 	}
	inline int 	getYRes() 	const { return yres; 	}

	// Gnome hint functions
	void setGnomeProtocols();
	void setGnomeHint(Window w, Atom a, long value);
	int findGnomeDesktopHint(Window win);
	Window getGnomeCheckWin() const { return gnome_check_win; }
	long getHint(Window w, int a);

	// Extended Window Manager hints function prototypes
	void addStrut(Strut *new_strut);
	void removeStrut(Strut *rem_strut);
	Strut* getMasterStrut() const { return master_strut; }
	int sendExtendedHintMessage(Window w, Atom a, long mask, long int data[]);
	void setExtendedWMHint(Window w, int a, long value);
	void setExtendedWMHintString(Window w, int a, char* value);
	void setExtendedNetSupported();
	void setExtendedNetDesktopGeometry();
	void setExtendedNetDesktopViewport();
	void setExtendedNetActiveWindow(Window w);
	void setExtendedNetWorkArea();
	Status getExtendedWMHintString(Window w, int a, char** name);
	NetWMStates* getExtendedNetWMStates(Window win);
	void setExtendedNetWMState(Window win, bool modal, bool sticky, bool max_vert, bool max_horz, bool shaded, bool skip_taskbar, bool skip_pager);
	void *getExtendedNetPropertyData(Window win, Atom prop, Atom type, int *items);					
	int findExtendedDesktopHint(Window win);
	long getDesktopHint(Window win, int a);
	void updateClientList();

	void grabKeys(Window w);
	void ungrabKeys(Window w);
};

extern WindowManager *wm;

#endif
