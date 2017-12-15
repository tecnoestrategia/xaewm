/*
 * windowmanager.cc
 * Copyleft 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 * Modified by xento figal http://xinfo.sf.net
 */
 
#include "xaewm.hh"

WindowManager* wm;

#define AEWM_KEY_ALT_COUNT 4
KeySym WindowManager::alt_keys[]=
{
			XK_Delete,
			XK_End,
			XK_Page_Up,
			XK_Page_Down,
			XK_1,
			XK_2,
			XK_3,
			XK_4,
			XK_5,
			XK_6,
			XK_7,
			XK_8,
			XK_9
};

WindowManager::WindowManager(int argc, char** argv)
{
	wm = this; 
	
	version = new Version();

   	current_desktop=0;

	focused_client=NULL;

	master_strut = new Strut;
	master_strut->east	= 0;
	master_strut->west	= 0;
	master_strut->north	= 0;
	master_strut->south	= 0;

	parseCommandLine(argc, argv);

    	if(max_desktops <= 0) max_desktops=MAX_DESKTOPS;

	setupSignalHandlers();
	setupDisplay();
	scanWins();
	
        // If there are any iconified clients, add them to the icon menu.
	updateIconMenu(); 
    	
	doEventLoop();
}

void WindowManager::parseCommandLine(int argc, char** argv)
{
	// Make the default options equal something
    	opt_font = DEF_FONT;
	opt_fm = DEF_FM;
    	opt_fg = DEF_FG;
    	opt_fc = DEF_FC;
    	opt_bg = DEF_BG;
    	opt_bd = DEF_BD;
    	opt_tj = TEXT_JUSTIFY;
	opt_wm = WIRE_MOVE;
	opt_es = EDGE_SNAP;
    	opt_new1 = DEF_NEW1;
    	opt_new2 = DEF_NEW2;
    	opt_bw = DEF_BW;
    	opt_display=NULL;
	max_desktops=MAX_DESKTOPS;

#define OPT_STR(name, variable)                                      \
    if (strcmp(argv[i], name) == 0 && i+1<argc) {                    \
        variable = argv[++i];                                        \
        continue;                                                    \
    }
#define OPT_INT(name, variable)                                      \
    if (strcmp(argv[i], name) == 0 && i+1<argc) {                    \
        variable = atoi(argv[++i]);                                  \
        continue;                                                    \
    }

	// Create a command line string, i.e. the command line used to 
	// run this iteration of the window manager. If the user restarts
	// the window manager while running we will restart it with the same
	// options used to start it the first time.
	for (int i = 0; i < argc; i++) command_line = command_line + argv[i] + " ";

	// Get the args and test for different options
	for (int i = 1; i < argc; i++) 
    	{
        	OPT_STR("-fn", opt_font)
	        OPT_STR("-fg", opt_fg)
        	OPT_STR("-bg", opt_bg)
		OPT_STR("-fc", opt_fc)
		OPT_STR("-fm", opt_fm)
        	OPT_STR("-bd", opt_bd)
        	OPT_STR("-new1", opt_new1)
	        OPT_STR("-new2", opt_new2)
		OPT_STR("-display", opt_display)
		OPT_STR("-tj", opt_tj)
		OPT_STR("-wm", opt_wm)
		OPT_STR("-es", opt_es)
		
		OPT_INT("-bw", opt_bw)
		OPT_INT("-md", max_desktops)

	       	if (strcmp(argv[i], "-version") == 0) 
                {
			cout << version->getVersionString() << endl;
            		exit(0);
        	}
        	
		if(strcmp(argv[i], "-usage")==0) 
                {
        		cerr << "usage: " << version->getWindowManagerName() << " [options]" << endl;
	        	cerr << "   options are: -display <display>, -fn <font>, -fg|-bg|-bd <color>, " << endl;
        		cerr << "   -bw <width>, -md <max desktops>, -tj <left|center|right>, -wm <true|false>," << endl;
			cerr << "    -new1|-new2 <cmd>, -fm (follow|sloppy|click), -usage, -help" << endl;
			exit(0);
		}
                
                if(strcmp(argv[i], "-help")==0) 
                {
                        cerr << "help: " << version->getWindowManagerName() << endl << endl;
                        cerr << "-display specifies a display to start the window manager on, The default is display :0." << endl;
                        cerr << "-fn is the font which to use for displaying window title names, icon names and such." << endl;
                        cerr << "-fg, -bg and -bd are colors you wish the foreground, background and border to be for window titlebars." << endl;
                        cerr << "-bw is the border width of the window." << endl;
                        cerr << "-md is the number of maximum virtual desktops, the default is 4." << endl;
                        cerr << "-tj is the text justify variable, its default is center, but you can specify left or right also." << endl;
                        cerr << "-new1 and -new2 are commands you wish the first and second mouse buttons to execute when pressed on the root window." << endl;
                        cerr << "-fm is the focus model you want to use, the default is click to focus." << endl;
                        cerr << "-es is for edge snapping, pass either true or false here." << endl;
			cerr << "-usage prints a reduced version of this information." << endl;
                        cerr << "-help prints this message." << endl << endl;
                        
                        exit(0);
                }
	}

	// Set the focus model based on user defined option
	if (strcmp(opt_fm, "follow")==0) setFocusModel(FOCUS_FOLLOW);
	else if (strcmp(opt_fm, "sloppy")==0) setFocusModel(FOCUS_SLOPPY);
	else if (strcmp(opt_fm, "click")==0) setFocusModel(FOCUS_CLICK);
	else setFocusModel(FOCUS_SLOPPY);	

	// Set up the window title justification per user defined option
	if(strcmp(opt_tj, "left")==0) opt_text_justify = LEFT_JUSTIFY;
	else if(strcmp(opt_tj, "center")==0) opt_text_justify = CENTER_JUSTIFY;
	else if(strcmp(opt_tj, "right")==0) opt_text_justify = RIGHT_JUSTIFY;
	else opt_text_justify = LEFT_JUSTIFY;

	// Set wire move based on user defined option
	if(strcmp(opt_wm, "true")==0) wire_move=true;
	else if(strcmp(opt_wm, "false")==0) wire_move=false;
	else wire_move=false;

	// Set edge snapping based on user defined option
	if(strcmp(opt_es, "true")==0) edge_snap=true;
	else if(strcmp(opt_es, "false")==0) edge_snap=false;
	else edge_snap=false;
}

void WindowManager::setupSignalHandlers()
{
    	struct sigaction act;

    	act.sa_handler = sigHandler;
    	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;
	
    	sigaction(SIGTERM, &act, NULL);
    	sigaction(SIGINT,  &act, NULL);
    	sigaction(SIGHUP, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);
}

void WindowManager::setCurrentDesktop(int desk)
{
	if ( (desk < max_desktops) && (desk > 0) ) current_desktop = desk;
	
	updateIconMenu();
}

void WindowManager::addClientToIconMenu(Client *c)
{
	icon_menu->hide();
	
	if(c->belongsToWhichDesktop() == current_desktop)
		icon_menu->addThisClient(c);
	
	icon_menu->updateMenu();
}

void WindowManager::removeClientFromIconMenu(Client *c)
{
	icon_menu->hide();
	icon_menu->removeClientFromIconMenu(c);
	icon_menu->updateMenu();
}

void WindowManager::updateIconMenu()
{
	icon_menu->hide();
	icon_menu->removeAll();
	
	list<Client*>::iterator it;
	for(it = client_list.begin(); it != client_list.end(); it++)
	{
		if((*it)->belongsToWhichDesktop() == current_desktop)
		{
			if((*it)->isIconified())
			{
				icon_menu->addThisClient((*it));
			}
		}
	}
	
	icon_menu->updateMenu();
}

void WindowManager::goToDesktop(int d)
{
	unsigned int nwins, i;
	Window dummyw1, dummyw2, *wins;
	Client* c;

	if( (d < max_desktops) && (d >= 0) )
	{
		current_desktop = d;

		updateIconMenu();

		setGnomeHint(root, atom_gnome_win_workspace, current_desktop);
		setExtendedWMHint(root, atom_extended_net_current_desktop, current_desktop);

		XSetInputFocus(dpy, gnome_button_proxy_win, RevertToNone, CurrentTime);
		
		// Preserve stacking order
		XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);
		for (i = 0; i < nwins; i++) 
		{
			c = findClient(wins[i]);
				
			if(c)
			{
				if(! c->isSticky())
				{
					if(c->belongsToWhichDesktop() == current_desktop)
					{
						if(! (c->isIconified())) 
							c->unhide();
					}
					else {
						if(! (c->isIconified())) 
							c->hide();												
					}
				}
			}
		}
		XFree(wins);
	}
}

void WindowManager::scanWins(void)
{
	unsigned int nwins, i;
    	Window dummyw1, dummyw2, *wins;
    	XWindowAttributes attr;
	Client *c=NULL;

	XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);
	for(i = 0; i < nwins; i++) 
	{
        	XGetWindowAttributes(dpy, wins[i], &attr);
	        if (!attr.override_redirect && attr.map_state == IsViewable)
		{
    			client_window_list.push_back(wins[i]);
			c = new Client(dpy, wins[i]);
		}
    	}
    	XFree(wins);
    
	XMapWindow(dpy, gnome_button_proxy_win);
	grabKeys(gnome_button_proxy_win);
	XSetInputFocus(dpy, gnome_button_proxy_win, RevertToNone, CurrentTime);
}

void WindowManager::setupDisplay()
{
    	XColor dummyc;
    	XGCValues gv;
    	XSetWindowAttributes sattr;

#ifdef SHAPE
    	int dummy;
#endif

	if (opt_display)
        	setenv("DISPLAY", opt_display, 1);
	else
        	opt_display = getenv("DISPLAY");

    	dpy = XOpenDisplay(opt_display);

    	if (!dpy) {
		cerr << "can't open display! check your DISPLAY variable." << endl;
	        exit(1);
	}

    	screen = DefaultScreen(dpy);
    	root = RootWindow(dpy, screen);

	xres = WidthOfScreen(ScreenOfDisplay(dpy, screen));
	yres = HeightOfScreen(ScreenOfDisplay(dpy, screen));

	XSetErrorHandler(handleXError);

	// ICCCM atoms
	atom_wm_state 		= XInternAtom(dpy, "WM_STATE", False);
	atom_wm_change_state 	= XInternAtom(dpy, "WM_CHANGE_STATE", False);
	atom_wm_protos 		= XInternAtom(dpy, "WM_PROTOCOLS", False);
	atom_wm_delete 		= XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	atom_wm_cmapwins 	= XInternAtom(dpy, "WM_COLORMAP_WINDOWS", False);
	atom_wm_takefocus 	= XInternAtom(dpy, "WM_TAKE_FOCUS", False);

 	// Motif hints 
    	atom_mwm_hints 		= XInternAtom(dpy, "_MOTIF_WM_HINTS", False);

	// GNOME atoms
	atom_gnome_win_client_list	= XInternAtom(dpy, "_WIN_CLIENT_LIST", False);
	atom_gnome_win_state		= XInternAtom(dpy, "_WIN_STATE", False);
	atom_gnome_win_hints		= XInternAtom(dpy, "_WIN_HINTS", False);
	//atom_gnome_win_layer		= XInternAtom(dpy, "_WIN_LAYER", False);
	atom_gnome_win_supporting_wm_check = XInternAtom(dpy, "_WIN_SUPPORTING_WM_CHECK", False); 
	atom_gnome_win_desktop_button_proxy= XInternAtom(dpy, "_WIN_DESKTOP_BUTTON_PROXY", False); 
	atom_gnome_win_workspace	= XInternAtom(dpy, "_WIN_WORKSPACE", False); 
	atom_gnome_win_workspace_count	= XInternAtom(dpy, "_WIN_WORKSPACE_COUNT", False); 
	atom_gnome_win_protocols	= XInternAtom(dpy, "_WIN_PROTOCOLS", False);

	gnome_check_win=XCreateSimpleWindow(dpy, root, -200, -200, 5, 5, 0, 0, 0);
	gnome_button_proxy_win=XCreateSimpleWindow(dpy, root, -80, -80, 24, 24,0,0,0);
	
	XChangeProperty(dpy, gnome_button_proxy_win, atom_gnome_win_desktop_button_proxy, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&gnome_button_proxy_win, 1);
	
  	/* Set up GNOME properties */ 
  	setGnomeHint(root, atom_gnome_win_supporting_wm_check, gnome_check_win); 
	setGnomeHint(gnome_check_win, atom_gnome_win_supporting_wm_check, gnome_check_win); 

	setGnomeProtocols();
  		
  	setGnomeHint(gnome_check_win, atom_gnome_win_desktop_button_proxy, gnome_check_win); 
  	setGnomeHint(root, atom_gnome_win_desktop_button_proxy, gnome_check_win); 
  	setGnomeHint(root, atom_gnome_win_workspace_count, max_desktops); 
  	setGnomeHint(root, atom_gnome_win_workspace, 0); 

	// Extended WM Hints
	atom_extended_net_supported 			= XInternAtom(dpy, "_NET_SUPPORTED", False); 
	atom_extended_net_client_list 			= XInternAtom(dpy, "_NET_CLIENT_LIST", False); 
	atom_extended_net_client_list_stacking 		= XInternAtom(dpy, "_NET_CLIENT_LIST_STACKING", False);
	atom_extended_net_number_of_desktops 		= XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False); 
	atom_extended_net_desktop_geometry 		= XInternAtom(dpy, "_NET_DESKTOP_GEOMETRY", False); 
	atom_extended_net_desktop_viewport 		= XInternAtom(dpy, "_NET_DESKTOP_VIEWPORT", False); 
	atom_extended_net_current_desktop 		= XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False); 
	//atom_extended_net_desktop_names 		= XInternAtom(dpy, "_NET_DESKTOP_NAMES", False); 
	atom_extended_net_active_window 		= XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False); 
	atom_extended_net_workarea 			= XInternAtom(dpy, "_NET_WORKAREA", False); 
	atom_extended_net_supporting_wm_check 		= XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False); 
	//atom_extended_net_virtual_roots 		= XInternAtom(dpy, "_NET_VIRTUAL_ROOTS", False); 
	atom_extended_net_close_window 			= XInternAtom(dpy, "_NET_CLOSE_WINDOW", False); 
	//atom_extended_net_wm_moveresize 		= XInternAtom(dpy, "_NET_WM_MOVERESIZE", False); 
	atom_extended_net_wm_name 			= XInternAtom(dpy, "_NET_WM_NAME", False); 
	//atom_extended_net_wm_visible_name 		= XInternAtom(dpy, "_NET_WM_VISIBLE_NAME", False); 
	//atom_extended_net_wm_icon_name 		= XInternAtom(dpy, "_NET_WM_ICON_NAME", False); 
	//atom_extended_net_wm_visible_icon_name 	= XInternAtom(dpy, "_NET_WM_VISIBLE_ICON_NAME", False); 
	atom_extended_net_wm_desktop 			= XInternAtom(dpy, "_NET_WM_DESKTOP", False); 
	//atom_extended_net_wm_window_type 		= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False); 
	//atom_extended_net_wm_window_type_desktop 	= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DESKTOP", False); 
	//atom_extended_net_wm_window_type_dock   	= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False); 
	//atom_extended_net_wm_window_type_toolbar 	= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_TOOLBAR", False); 
	//atom_extended_net_wm_window_type_menu   	= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_MENU", False); 
	//atom_extended_net_wm_window_type_dialog 	= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False); 
	//atom_extended_net_wm_window_type_normal 	= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", False); 
	atom_extended_net_wm_state 		  	= XInternAtom(dpy, "_NET_WM_STATE", False); 
	atom_extended_net_wm_state_modal 	  	= XInternAtom(dpy, "_NET_WM_STATE_MODAL", False); 
	atom_extended_net_wm_state_sticky 	  	= XInternAtom(dpy, "_NET_WM_STATE_STICKY", False); 
	atom_extended_net_wm_state_maximized_vert 	= XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False); 
	atom_extended_net_wm_state_maximized_horz 	= XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False); 
	atom_extended_net_wm_state_shaded 	  	= XInternAtom(dpy, "_NET_WM_STATE_SHADED", False); 
	atom_extended_net_wm_state_skip_taskbar   	= XInternAtom(dpy, "_NET_WM_STATE_SKIP_TOOLBAR", False); 
	atom_extended_net_wm_state_skip_pager 	  	= XInternAtom(dpy, "_NET_WM_STATE_PAGER", False); 
	atom_extended_net_wm_strut 		  	= XInternAtom(dpy, "_NET_WM_STRUT", False); 
	//atom_extended_net_wm_icon_geometry 	  	= XInternAtom(dpy, "_NET_WM_ICON_GEOMETRY", False); 
	//atom_extended_net_wm_icon 		  	= XInternAtom(dpy, "_NET_WM_ICON", False); 
	//atom_extended_net_wm_pid 		  	= XInternAtom(dpy, "_NET_WM_PID", False); 
	//atom_extended_net_wm_handled_icons 	  	= XInternAtom(dpy, "_NET_WM_HANDLED_ICONS", False); 
	//atom_extended_net_wm_ping 		  	= XInternAtom(dpy, "_NET_WM_PING", False); 

	// This window is does nothing more than store properties which let
	// other apps know we are supporting the extended wm hints
	extended_hints_win=XCreateSimpleWindow(dpy, root, -200, -200, 5, 5, 0, 0, 0);

	XSetWindowAttributes pattr;
	pattr.override_redirect=True;
	XChangeWindowAttributes(dpy, extended_hints_win, CWOverrideRedirect, &pattr);
	XChangeWindowAttributes(dpy, gnome_check_win, CWOverrideRedirect, &pattr);

	setExtendedWMHintString(extended_hints_win, atom_extended_net_wm_name, (char*)version->getWindowManagerName());
	setExtendedWMHint(extended_hints_win, atom_extended_net_supporting_wm_check, extended_hints_win); 
	setExtendedWMHint(root, atom_extended_net_supporting_wm_check, extended_hints_win); 
	setExtendedNetSupported();
	setExtendedWMHint(root, atom_extended_net_number_of_desktops, max_desktops);
	setExtendedNetDesktopGeometry();
	setExtendedNetDesktopViewport();
	setExtendedWMHint(root, atom_extended_net_current_desktop, 0);

    	XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_fg, &fg, &dummyc);
    	XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_bg, &bg, &dummyc);
    	XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_bd, &bd, &dummyc);
    	XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_fc, &fc, &dummyc);
    
    	font = XLoadQueryFont(dpy, opt_font);
    	if (!font) font = XLoadQueryFont(dpy, DEF_FONT);
    	if (!font) { cerr << "DEF_FONT not found, aborting." << endl; exit(1); }

#ifdef SHAPE
    	shape = XShapeQueryExtension(dpy, &shape_event, &dummy);
#endif

    	move_curs = XCreateFontCursor(dpy, XC_fleur);
    	resize_curs = XCreateFontCursor(dpy, XC_plus);
	arrow_curs = XCreateFontCursor(dpy, XC_left_ptr);

	XDefineCursor(dpy, root, arrow_curs);

    	gv.function = GXcopy;
    	gv.foreground = fg.pixel;
    	gv.font = font->fid;
    	string_gc = XCreateGC(dpy, root, GCFunction|GCForeground|GCFont, &gv);

    	gv.foreground = bd.pixel;
    	gv.line_width = opt_bw;
    	border_gc = XCreateGC(dpy, root, GCFunction|GCForeground|GCLineWidth, &gv);

	gv.foreground = fg.pixel;
    	gv.function = GXinvert; 
    	gv.subwindow_mode = IncludeInferiors;
    	invert_gc = XCreateGC(dpy, root, GCForeground|GCFunction|GCSubwindowMode|GCLineWidth|GCFont, &gv);

	sattr.event_mask = SubstructureRedirectMask	|
			   SubstructureNotifyMask  	|
			   ColormapChangeMask		|
			   ButtonPressMask		|
			   ButtonReleaseMask		|
			   FocusChangeMask		|
			   EnterWindowMask		|
			   LeaveWindowMask		|
			   PropertyChangeMask   	|
			   ButtonMotionMask		;
			   
    	XChangeWindowAttributes(dpy, root, CWEventMask, &sattr);

	grabKeys(root);
	
	window_menu = new WindowMenu();
	icon_menu = new IconMenu();  
}

void WindowManager::doEventLoop()
{
 	XEvent ev;

     	for (;;) 
	{
		XNextEvent(dpy, &ev);

        	switch (ev.type) 
		{		
		    	case KeyPress:
		    		handleKeyPressEvent(&ev);
			break;
			
            		case ButtonPress:
				handleButtonPressEvent(&ev);
			break;
 
			case ButtonRelease:
				handleButtonReleaseEvent(&ev);
			break;
 	
			case ConfigureRequest:
				handleConfigureRequestEvent(&ev);
			break;
      
      	    		case MotionNotify:
				handleMotionNotifyEvent(&ev);
			break;

			case MapRequest:
				handleMapRequestEvent(&ev);
	    		break;

            		case UnmapNotify:
				handleUnmapNotifyEvent(&ev);
	    		break;
	    
	    		case DestroyNotify:
				handleDestroyNotifyEvent(&ev);
			break;

            		case EnterNotify:
				handleEnterNotifyEvent(&ev);
			break;

	    		case LeaveNotify:
				handleLeaveNotifyEvent(&ev);
			break;

	    		case FocusIn:
				handleFocusInEvent(&ev);
	    		break;
	    
	    		case FocusOut:
				handleFocusOutEvent(&ev);
	    		break;
      
            		case ClientMessage:
				handleClientMessageEvent(&ev);
			break;
      
           		case ColormapNotify:
				handleColormapNotifyEvent(&ev);
			break;
      
            		case PropertyNotify:
				handlePropertyNotifyEvent(&ev);
			break;
   
            		case Expose:
				handleExposeEvent(&ev);
			break;
		
#ifdef SHAPE
            		default:
				handleDefaultEvent(&ev);
	    		break;
#endif
        }
    }
}

void WindowManager::grabKeys(Window w)
{
	int max_desktop_keys=max_desktops;
	if (max_desktops>9)
	{
		max_desktop_keys=9;
	}
      	for(int i=0;i<AEWM_KEY_ALT_COUNT;i++)
		XGrabKey(dpy,XKeysymToKeycode(dpy,alt_keys[i]), (Mod1Mask|ControlMask),
			 w,True,GrabModeAsync,GrabModeAsync);
      	for(int i=AEWM_KEY_ALT_COUNT;i<AEWM_KEY_ALT_COUNT+max_desktop_keys;i++)
		XGrabKey(dpy,XKeysymToKeycode(dpy,alt_keys[i]), (Mod1Mask),
			 w,True,GrabModeAsync,GrabModeAsync);
}

void WindowManager::ungrabKeys(Window w)
{
	int max_desktop_keys=max_desktops;
	if (max_desktops>9)
	{
		max_desktop_keys=9;
	}
    	for(int i=0;i<AEWM_KEY_ALT_COUNT;i++)
		XUngrabKey(dpy,XKeysymToKeycode(dpy,alt_keys[i]),
			  (Mod1Mask|ControlMask),w);
    	for(int i=AEWM_KEY_ALT_COUNT;i<AEWM_KEY_ALT_COUNT+max_desktop_keys;i++)
		XUngrabKey(dpy,XKeysymToKeycode(dpy,alt_keys[i]),
			  (Mod1Mask),w);
}

void WindowManager::handleKeyPressEvent(XEvent *ev) 
{
	KeySym ks;

	ks=XKeycodeToKeysym(dpy,ev->xkey.keycode,0);
	if (ks==NoSymbol) return;

	switch(ks) 
	{
		case XK_Delete:
			cerr << "aewm++ is restarting..." << endl;
			restart();
		break;

		case XK_End:
			cerr << "aewm++ is quitting." << endl;
			quitNicely();
		break;
	
		case XK_Page_Up: 
			if( current_desktop < max_desktops - 1 )
			{
				current_desktop++;
				goToDesktop(current_desktop);
			}
		break;
		
		case XK_Page_Down: 
                        if( current_desktop > 0 )
			{
				current_desktop--;
				goToDesktop(current_desktop);
			}			
		break;
	}
	if (ks >= XK_1 && ks <= XK_1+(unsigned)max_desktops && ks - XK_1 <= (unsigned)9)  /* no two digit keys */
 	{
                if( (unsigned)current_desktop != ks - XK_1 )
		{
			(unsigned)current_desktop = ks - XK_1;
			goToDesktop(current_desktop);
		}
	}
}

void WindowManager::handleButtonPressEvent(XEvent *ev)
{
	if (ev->xbutton.window == root) 
	{
		switch (ev->xbutton.button) 
		{
			case Button1: 
				forkExec(opt_new1); 

				if(icon_menu->isVisible())
					icon_menu->hide();
			break;
							
			case Button2: 
				if(icon_menu->getItemCount())
				{
					if(icon_menu->isVisible())
						icon_menu->hide();
					else 
						icon_menu->show();
				}
			break;
							
			case Button3: 
				forkExec(opt_new2); 
						
				if(icon_menu->isVisible())
					icon_menu->hide();
			break;
	        }
	}
	else
	{
		Client* c = findClient(ev->xbutton.window);
		
		if(c && c->hasWindowDecorations())
		{
			if( (ev->xbutton.button == Button1) 
			    && 
			    (ev->xbutton.type==ButtonPress) 
			    && 
			    (ev->xbutton.state==Mod1Mask) 
			    &&
			    (c->getFrameWindow() == ev->xbutton.window)
			   )
			{
				if (!Grab(c->getFrameWindow(), PointerMotionMask|ButtonReleaseMask, wm->getMoveCursor())) return;
			}			
		}
		
		switch (focus_model) 
		{
			case FOCUS_FOLLOW:
			case FOCUS_SLOPPY:
				if(c)
				{
					c->handleButtonEvent(&ev->xbutton);
					focused_client = c;
				}
			break;

			case FOCUS_CLICK:
				// if this is the first time the client window's clicked, focus it
				if(c && c != focused_client)
				{
					XSetInputFocus(dpy, c->getAppWindow(), RevertToNone, CurrentTime);
					focused_client = c;
				}

				// otherwise, handle the button click as usual
				if(c && c == focused_client)
					c->handleButtonEvent(&ev->xbutton);
			break;
		}
					
		BaseMenu* mu = window_menu->findMenu(ev->xbutton.window);

		if(!mu)
			mu = icon_menu->findMenu(ev->xbutton.window);

		if(mu)
			mu->handleButtonPressEvent(&ev->xbutton);
	}

	if(ev->xbutton.window==root)
		XSendEvent(dpy, gnome_button_proxy_win, False, SubstructureNotifyMask, ev);

}

void WindowManager::handleButtonReleaseEvent(XEvent *ev)
{
	Client* c = findClient(ev->xbutton.window);
				
	if(c) {
		Ungrab();
		
		c->handleButtonEvent(&ev->xbutton); 
	}
	else 
	{
		BaseMenu* mu = window_menu->findMenu(ev->xbutton.window);
			
		if(!mu)
			mu = icon_menu->findMenu(ev->xbutton.window);
			
		if(mu) 
		{
			mu->hide();
			mu->handleButtonReleaseEvent(&ev->xbutton);
		}
	}
				
	if(ev->xbutton.window==root)
		XSendEvent(dpy, gnome_button_proxy_win, False, SubstructureNotifyMask, ev);
}

void WindowManager::handleConfigureRequestEvent(XEvent *ev)
{
	Client* c = findClient(ev->xconfigurerequest.window);
		
	if(c)
		c->handleConfigureRequest(&ev->xconfigurerequest); 
	else 
	{
		// Since this window isn't yet a client lets delegate
		// the configure request back to the window so it can
		// use it.
	
		XWindowChanges wc;	
			
	        wc.x = ev->xconfigurerequest.x;
		wc.y = ev->xconfigurerequest.y;
		wc.width = ev->xconfigurerequest.width;
		wc.height = ev->xconfigurerequest.height;
	    	wc.sibling = ev->xconfigurerequest.above;
	    	wc.stack_mode = ev->xconfigurerequest.detail;
		XConfigureWindow(dpy, ev->xconfigurerequest.window, ev->xconfigurerequest.value_mask, &wc);
	}
}

void WindowManager::handleMotionNotifyEvent(XEvent *ev)
{
	Client* c = findClient(ev->xmotion.window);
		
	if(c)
		c->handleMotionNotifyEvent(&ev->xmotion); 
	else
	{
		BaseMenu* mu = window_menu->findMenu(ev->xmotion.window);
		
		if(!mu)
			mu = icon_menu->findMenu(ev->xmotion.window);
				
		if(mu)
			mu->handleMotionNotifyEvent(&ev->xmotion);
	}
}

void WindowManager::handleMapRequestEvent(XEvent *ev)
{
	Client* c = findClient(ev->xmaprequest.window);
		
	if(c) 
		c->handleMapRequest(&ev->xmaprequest);
	else {
		client_window_list.push_back(ev->xmaprequest.window);
		c = new Client(dpy, ev->xmaprequest.window);
	}
}

void WindowManager::handleUnmapNotifyEvent(XEvent *ev)
{
	Client* c = findClient(ev->xunmap.window);
	
	if(c) 
	{
		c->handleUnmapEvent(&ev->xunmap); 
		// if unmapping it, note that it's no longer focused
		focused_client = NULL;
	}
}

void WindowManager::handleDestroyNotifyEvent(XEvent *ev)
{
	Client* c = findClient(ev->xdestroywindow.window);

	if(c)
	{
		c->handleDestroyEvent(&ev->xdestroywindow);
		// if destroying it, note that it's no longer focused
		focused_client = NULL;
	}     
}

void WindowManager::handleEnterNotifyEvent(XEvent *ev)
{
	BaseMenu* mu = window_menu->findMenu(ev->xcrossing.window);

	if(!mu)
		mu = icon_menu->findMenu(ev->xcrossing.window);			

	if(mu) 
		mu->handleEnterNotify(&ev->xcrossing);	
	else
	{
		Client* c = findClient(ev->xcrossing.window);
		
		switch (focus_model) 
		{
			case FOCUS_FOLLOW:
				if(c)
				{
 					c->handleEnterEvent(&ev->xcrossing);
					focused_client = c;
				}
				else
					XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
			break;

			case FOCUS_SLOPPY:
				// if the pointer's not on a client now, don't change focus
				if(c)
				{
					c->handleEnterEvent(&ev->xcrossing);
					focused_client = c;
				}
			break;
		}
	}
}

void WindowManager::handleLeaveNotifyEvent(XEvent *ev)
{
	BaseMenu* mu = window_menu->findMenu(ev->xcrossing.window);
		
	if(!mu)
		mu = icon_menu->findMenu(ev->xcrossing.window);			
		
	if(mu) mu->handleLeaveNotify(&ev->xcrossing);	
}

void WindowManager::handleFocusInEvent(XEvent *ev)
{
/* Notify modes */

//#define NotifyNormal		0
//#define NotifyGrab		1
//#define NotifyUngrab		2
//#define NotifyWhileGrabbed	3

/* Notify detail */

//#define NotifyAncestor	0
//#define NotifyVirtual		1
//#define NotifyInferior	2
//#define NotifyNonlinear	3
//#define NotifyNonlinearVirtual4
//#define NotifyPointer		5
//#define NotifyPointerRoot	6
//#define NotifyDetailNone	7

	if((ev->xfocus.mode==NotifyGrab) || (ev->xfocus.mode==NotifyUngrab)) return;
	
	list<Window>::iterator iter;
	
	for(iter=client_window_list.begin(); iter != client_window_list.end(); iter++)
	{
		if(ev->xfocus.window == (*iter))
		{
			Client *c = findClient( (*iter) );
			
			if(c) 
			{
				unfocusAnyStrayClients();
				c->handleFocusInEvent(&ev->xfocus);
				focused_client = c;
				grabKeys( (*iter) );
			}
		}
		else 
		{
			if(ev->xfocus.window==root && focus_model==FOCUS_FOLLOW)
				unfocusAnyStrayClients();
		}
	}
}

void WindowManager::handleFocusOutEvent(XEvent *ev)
{	
	list<Window>::iterator iter;
	for(iter=client_window_list.begin(); iter != client_window_list.end(); iter++)
	{
		if(ev->xfocus.window == (*iter))
		{
			Client *c = findClient( (*iter) );
			
			if(c) 
			{
				c->handleFocusOutEvent(&ev->xfocus);
				focused_client = NULL;
				ungrabKeys( (*iter) );
				return;
			}
		}
	}

	if(focus_model == FOCUS_CLICK) 
		focusPreviousWindowInStackingOrder();
}

void WindowManager::handleClientMessageEvent(XEvent *ev)
{
	Client* c = findClient(ev->xclient.window);
		
	if(c)
		c->handleClientMessage(&ev->xclient);
		
	if(ev->xclient.window == root)
	{
		if(ev->xclient.message_type== atom_gnome_win_workspace && ev->xclient.format==32)
			goToDesktop(ev->xclient.data.l[0]);

		if(ev->xclient.message_type== atom_extended_net_current_desktop && ev->xclient.format==32)
			goToDesktop(ev->xclient.data.l[0]);

		if(ev->xclient.message_type== atom_gnome_win_workspace_count && ev->xclient.format==32)
			max_desktops=ev->xclient.data.l[0];

		if(ev->xclient.message_type== atom_extended_net_number_of_desktops && ev->xclient.format==32)
			max_desktops=ev->xclient.data.l[0];
	}
}

void WindowManager::handleColormapNotifyEvent(XEvent *ev)
{
	Client* c = findClient(ev->xcolormap.window);
		
	if(c)
		c->handleColormapChange(&ev->xcolormap); 
}

void WindowManager::handlePropertyNotifyEvent(XEvent *ev)
{
	Client* c = findClient(ev->xproperty.window);
		
	if(c)
		c->handlePropertyChange(&ev->xproperty); 
}

void WindowManager::handleExposeEvent(XEvent *ev)
{
	BaseMenu* mu = window_menu->findMenu(ev->xexpose.window);

	if(!mu)
		mu = icon_menu->findMenu(ev->xexpose.window);					
		
	if(mu) 
		mu->handleExposeEvent(&ev->xexpose); 
	else
	{
		Client* c = findClient(ev->xexpose.window);
		
		if(c)
			c->handleExposeEvent(&ev->xexpose);
	}
}

void WindowManager::handleDefaultEvent(XEvent *ev)
{
	Client* c = findClient(ev->xany.window);
				
	if(c)
	{	
		if (shape && ev->type == shape_event)
			c->handleShapeChange((XShapeEvent *)&ev);
	} 
}

void WindowManager::unfocusAnyStrayClients()
{
	// To prevent two windows titlebars from being painted with the focus color we
	// will prevent that from happening by setting all windows to false.
	
	list<Client*>::iterator iter;
	for(iter=client_list.begin(); iter != client_list.end(); iter++)
		(*iter)->setFocus(false);
}

void WindowManager::focusPreviousWindowInStackingOrder()
{
	unsigned int nwins, i;
	Window dummyw1, dummyw2, *wins;
	Client* c=NULL;

	XSetInputFocus(dpy, gnome_button_proxy_win, RevertToNone, CurrentTime);

	XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);

	if(client_list.size())
	{
		list<Client*> client_list_for_current_desktop;

		for (i = 0; i < nwins; i++)
		{
			c = findClient(wins[i]);
			
			if(c)
			{
				if((c->belongsToWhichDesktop()==current_desktop 
			   && c->hasWindowDecorations() && (c->isIconified() == false) ))
									client_list_for_current_desktop.push_back(c);
			}
		}
		
		if(client_list_for_current_desktop.size())
		{
			list<Client*>::iterator iter = client_list_for_current_desktop.end();
			
			iter--;
			
			if( (*iter) )
			{
				XSetInputFocus(dpy, (*iter)->getAppWindow(), RevertToNone, CurrentTime);
			
				client_list_for_current_desktop.clear();
				
				XFree(wins);
						
				return;
			}
		} 
	} 
	
	XFree(wins);
}

void WindowManager::getMousePosition(int *x, int *y)
{
	Window mouse_root, mouse_win;
	int win_x, win_y;
	unsigned int mask;

	XQueryPointer(dpy, root, &mouse_root, &mouse_win, x, y, &win_x, &win_y, &mask);
}

void WindowManager::restackOnTopWindows()
{
	list<Client*>::iterator iter;
	for(iter=client_list.begin(); iter!= client_list.end(); iter++)
		if ((*iter)->isAlwaysOnTop()) (*iter)->raise();
}

void WindowManager::addClient(Client *c)
{
	client_list.push_back(c);
}

void WindowManager::removeClient(Client* c)
{
	removeClientFromIconMenu(c);
	client_window_list.remove(c->getAppWindow());
	client_list.remove(c);

	updateClientList();	
}

Client* WindowManager::findClient(Window w)
{
	if(client_list.size())
	{
		list<Client*>::iterator iter = client_list.begin();

		for(; iter != client_list.end(); iter++)
		{
			if (w == (*iter)->getTitleWindow()  ||
			    w == (*iter)->getFrameWindow()  ||
			    w == (*iter)->getAppWindow())
			    return (*iter);
		}
	}
	return NULL;
}

void WindowManager::findTransientsToMapOrUnmap(Window win, bool hide)
{
	list<Client*>::iterator iter;
	
	if(client_list.size()) 
	{
		for(iter=client_list.begin(); iter!= client_list.end(); iter++)
		{
			if((*iter)->getTransientWindow() == win)
			{
				if(hide)
				{
					if(! (*iter)->isIconified())
						(*iter)->iconify(); 
				}
				else 
				{
					if((*iter)->isIconified())
						(*iter)->unhide();
				}
			}
		}
	}	
}

void WindowManager::setGnomeProtocols()
{
  	Atom gnome_protocols[5];

  	gnome_protocols[0] = atom_gnome_win_state;
	gnome_protocols[1] = atom_gnome_win_hints;
	gnome_protocols[2] = atom_gnome_win_client_list;
	gnome_protocols[3] = atom_gnome_win_workspace;
	gnome_protocols[4] = atom_gnome_win_workspace_count;
  
  	XChangeProperty(dpy, root, atom_gnome_win_protocols, XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)gnome_protocols, 5);
}

void WindowManager::setGnomeHint(Window w, Atom a, long value) 
{
	XChangeProperty(dpy, 
			w, 
			a, 
			XA_CARDINAL, 
			32, 
			PropModeReplace, 
			(unsigned char *) &value, 1); 
} 
 
long WindowManager::getHint(Window w, int a) 
{ 
	Atom real_type; 
  	int real_format; 
  	unsigned long items_read, items_left; 
  	long *data=NULL, value=0; 
 
  	if(XGetWindowProperty(dpy, w, a, 0L, 1L, False, 
			      XA_CARDINAL, &real_type, 
			      &real_format, &items_read, 
			      &items_left, 
			      (unsigned char **)&data)==Success && items_read) 
  	{ 
    		value=*data; 
    		XFree(data); 
  	}
	
  	return value; 
}

void WindowManager::updateClientList() 
{

	int i=0, client_count=0; 
  	CARD32 *wins=NULL; 
	
	Window  *extended_wins=NULL;
 	
	list<Client*>::iterator iter;
	for(iter=client_list.begin(); iter!= client_list.end(); iter++)
	{
		// We don't want to include transients in our client list
		if(! ((*iter)->getTransientWindow())) client_count++;
	}
  
  	wins = new CARD32[client_count];
	
	extended_wins = new Window[client_count];
 	
  	if(wins==NULL)
  	{ 
    		cerr << "Memory allocation failed in function update_gnome_client." << endl; 
    		exit(1);
  	}
	
	for(iter=client_list.begin(); iter!= client_list.end(); iter++)
	{
		// We don't want to include transients in our client list
		if(! ((*iter)->getTransientWindow()))
		{
			Window t = (*iter)->getAppWindow();
			
			wins[i]=t;
			
			extended_wins[i]=t;
						
			i++;
		}
	}
 
  	XChangeProperty(dpy, root, atom_gnome_win_client_list, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)wins, client_count); 
	XChangeProperty(dpy, root, atom_extended_net_client_list, XA_WINDOW, 32, PropModeReplace, (unsigned char*)extended_wins, client_count);
	XChangeProperty(dpy, root, atom_extended_net_client_list_stacking, XA_WINDOW, 32, PropModeReplace, (unsigned char*)extended_wins, client_count);
	
  	delete [] wins;
	delete [] extended_wins;

} 

int WindowManager::sendExtendedHintMessage(Window w, Atom a, long mask, long int data[])
{
    	XEvent e;

    	e.type = ClientMessage;
    	e.xclient.window = w;
    	e.xclient.message_type = a;
    	e.xclient.format = 32;

	// xclient.data.l is a long int[5]
	e.xclient.data.l[0] = data[0];
	e.xclient.data.l[1] = data[1];
	e.xclient.data.l[2] = data[2];
	//e.xclient.data.l[3] = data[3];
	//e.xclient.data.l[4] = data[4];

    	return XSendEvent(dpy, 
			  w, 
			  False, 
			  SubstructureNotifyMask|SubstructureRedirectMask, 
			  &e);
}

void WindowManager::setExtendedWMHint(Window w, int a, long value)
{ 
  	XChangeProperty(dpy, w, a, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &value, 1); 
} 

void WindowManager::setExtendedWMHintString(Window w, int a, char* value)
{
	XChangeProperty(
		dpy, 
		w, 
		a,	
		XA_STRING,
		8, 
		PropModeReplace, 
		(unsigned char*) value, 
		strlen (value));
}

// Borrowed from:
//
// http://capderec.udg.es:81/ebt-bin/nph-dweb/dynaweb/SGI_Developer/XLib_PG/%40Generic__BookTextView/45827
Status WindowManager::getExtendedWMHintString(Window w, int a, char** name)
{
    	Atom actual_type;
    	int actual_format;
	unsigned long nitems;
	unsigned long leftover;
	unsigned char *data = NULL;
    
    	if (XGetWindowProperty(dpy, w, a, 0L, (long)BUFSIZ,
            False, XA_STRING, &actual_type, &actual_format,
            &nitems, &leftover, &data) != Success) 
	{
        	*name = NULL;
        	return (0);
    	}
	
	if ( (actual_type == XA_STRING) && (actual_format == 8) ) 
	{
	        // The data returned by XGetWindowProperty is guaranteed
        	// to contain one extra byte that is null terminated to
	        // make retrieving string properties easy 
        	*name = (char *)data;
	
        	return(1);
        }
    	
	if (data) XFree ((char *)data);
    	
	*name = NULL;
    	
	return(0);
}

void WindowManager::setExtendedNetSupported()
{
	int total_net_supported = 22;

	Atom net_supported_list[] = {
		atom_extended_net_supported,
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
		atom_extended_net_wm_strut,
		//atom_extended_net_wm_icon_geometry,
		//atom_extended_net_wm_icon,
		//atom_extended_net_wm_pid,
		//atom_extended_net_wm_handled_icons,
		//atom_extended_net_wm_ping
	};
	
	XChangeProperty(dpy, root, atom_extended_net_supported, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)net_supported_list, total_net_supported);
}

void WindowManager::setExtendedNetDesktopGeometry()
{
	CARD32 geometry[] = { xres, yres };
	
	XChangeProperty(dpy, root, atom_extended_net_desktop_geometry, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)geometry, 2);
}

void WindowManager::setExtendedNetDesktopViewport()
{
	CARD32 viewport[] = { 0, 0 };
	
	XChangeProperty(dpy, root, atom_extended_net_desktop_viewport, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)viewport, 2);
}

void WindowManager::setExtendedNetActiveWindow(Window w)
{
	setExtendedWMHint(root, atom_extended_net_active_window, w);
}

void WindowManager::setExtendedNetWorkArea()
{
	int work_x, work_y, work_width, work_height;
		
	work_x = master_strut->west;
	work_y = master_strut->north;
	work_width = xres - master_strut->east - work_x;
	work_height = yres - master_strut->south - work_y;
	
	CARD32 workarea[] = { work_x, work_y, work_width, work_height };
		
	XChangeProperty(dpy, root, atom_extended_net_workarea, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)workarea, 4);
}

void* WindowManager::getExtendedNetPropertyData(Window win, Atom prop, Atom type, int *items)
{
	Atom type_ret = None;
	int format_ret = 0;
	unsigned long items_ret = 0;
	unsigned long after_ret = 0;
	unsigned char *prop_data = 0;

	XGetWindowProperty (dpy, win, prop, 0, 0x7fffffff, False,
				  type, &type_ret, &format_ret, &items_ret,
				  &after_ret, &prop_data);
	if (items)
		*items = items_ret;

	return prop_data;
}

NetWMStates* WindowManager::getExtendedNetWMStates(Window win)
{
	NetWMStates *win_state;

	win_state = new NetWMStates;

	win_state->modal=false;
	win_state->sticky=false;
 	win_state->max_vert=false;
 	win_state->max_horz=false;
 	win_state->shaded=false;
 	win_state->skip_taskbar=false;
	win_state->skip_pager=false;
 	
	int num=0;
 	Atom* states;
 	states = (Atom*) getExtendedNetPropertyData(win,
					atom_extended_net_wm_state,	
					XA_ATOM,
					&num);
 
 	if(states!=NULL)
	{
		if(states[0]==atom_extended_net_wm_state_modal) win_state->modal=true;
		if(states[1]==atom_extended_net_wm_state_sticky) win_state->sticky=true;
		if(states[2]==atom_extended_net_wm_state_maximized_vert) win_state->max_vert=true;
		if(states[3]==atom_extended_net_wm_state_maximized_horz) win_state->max_horz=true;
		if(states[4]==atom_extended_net_wm_state_shaded) win_state->shaded=true;
		if(states[5]==atom_extended_net_wm_state_skip_taskbar) win_state->skip_taskbar=true;
		if(states[6]==atom_extended_net_wm_state_skip_pager) win_state->skip_pager=true;
	}
	
	XFree(states);
	
	return win_state;
}

void WindowManager::setExtendedNetWMState(
				Window win,
				bool modal,
				bool sticky,
				bool max_vert,
				bool max_horz,
				bool shaded,
				bool skip_taskbar,
				bool skip_pager
				)
{
	if(modal) 	
		net_wm_states[0] = atom_extended_net_wm_state_modal;
	else
		net_wm_states[0] = 0;
	
	if(sticky) 	
		net_wm_states[1] = atom_extended_net_wm_state_sticky;
	else 
		net_wm_states[1] = 0;
		
	if(max_vert)	
		net_wm_states[2] = atom_extended_net_wm_state_maximized_vert;
	else
		net_wm_states[2] = 0;

	if(max_horz)	
		net_wm_states[3] = atom_extended_net_wm_state_maximized_horz;
	else
		net_wm_states[3] = 0;
			
	if(shaded)	
		net_wm_states[4] = atom_extended_net_wm_state_shaded;
	else
		net_wm_states[4] = 0;	
	
	if(skip_taskbar) 
		net_wm_states[5] = atom_extended_net_wm_state_skip_taskbar;
	else
		net_wm_states[5] = 0;
	
	if(skip_pager)	
		net_wm_states[6] = atom_extended_net_wm_state_skip_pager;
	else
		net_wm_states[6] = 0;
	
	XChangeProperty(dpy, win, atom_extended_net_wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *) net_wm_states, NET_WM_STATE_MAX_STATES);
}

void WindowManager::addStrut(Strut *new_strut)
{
	client_struts.push_back(new_strut);
	
	if(client_struts.size())
	{
		list<Strut*>::iterator struts_it;
		for(struts_it = client_struts.begin(); struts_it != client_struts.end() ; struts_it++)
		{
			if(master_strut->east < (*struts_it)->east) master_strut->east = (*struts_it)->east;
			
			if(master_strut->west < (*struts_it)->west) master_strut->west = (*struts_it)->west;
			
			if(master_strut->north < (*struts_it)->north) master_strut->north = (*struts_it)->north;

			if(master_strut->south < (*struts_it)->south) master_strut->south = (*struts_it)->south;
		}
	} 
	
	setExtendedNetWorkArea();
}

void WindowManager::removeStrut(Strut *rem_strut)
{
	if(client_struts.size())
		client_struts.remove(rem_strut);

	master_strut->east = 0;
	master_strut->west = 0;
	master_strut->north = 0;	
	master_strut->south = 0;

	if(client_struts.size())
	{
		list<Strut*>::iterator struts_it;
		for(struts_it = client_struts.begin(); struts_it != client_struts.end() ; struts_it++)
		{
			if(master_strut->east < (*struts_it)->east) master_strut->east = (*struts_it)->east;
			
			if(master_strut->west < (*struts_it)->west) master_strut->west = (*struts_it)->west;
			
			if(master_strut->north < (*struts_it)->north) master_strut->north = (*struts_it)->north;

			if(master_strut->south < (*struts_it)->south) master_strut->south = (*struts_it)->south;
		}
	} 
	
	setExtendedNetWorkArea();
}

int WindowManager::findExtendedDesktopHint(Window win)
{
	int desktop=-1;
	
	desktop = getDesktopHint(win, atom_extended_net_wm_desktop);

	return desktop;
}

int WindowManager::findGnomeDesktopHint(Window win)
{
	int desktop=-1;
	
	desktop = getDesktopHint(win, atom_gnome_win_workspace);

	return desktop;	
}

long WindowManager::getDesktopHint(Window win, int a)
{
	Atom real_type; 
  	int real_format; 
  	unsigned long items_read, items_left; 
  	long *data=NULL, value=-1; 
 
  	if(XGetWindowProperty(dpy, win, a, 0L, 1L, False, 
			      XA_CARDINAL, &real_type, 
			      &real_format, &items_read, 
			      &items_left, 
			      (unsigned char **)&data)==Success && items_read) 
  	{ 
    		value=*data; 
    		XFree(data); 
  	}
	
	return value;
}

void WindowManager::restart()
{
	cleanup();
	
	execl("/bin/sh", "sh", "-c", command_line.c_str(), 0);
}

void WindowManager::quitNicely()
{
	cleanup();
	exit(0);
}

void WindowManager::cleanup()
{
        cerr << "xaewm++ is cleaning up.... " << endl;
	
	unsigned int nwins, i;
    	Window dummyw1, dummyw2, *wins;
    	Client* c;

	delete version;

	XDestroyWindow(dpy, gnome_button_proxy_win);
	XDestroyWindow(dpy, gnome_check_win);
	XDestroyWindow(dpy, extended_hints_win);

	ungrabKeys(root);

    	// Preserve stacking order when removing the clients
    	// from the list.
    	XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);
    	for (i = 0; i < nwins; i++) 
    	{
		c = findClient(wins[i]);
		
		if(c)
		{
			XMapWindow(dpy, c->getAppWindow());
			
			delete c;
		}
    	}
    	XFree(wins);
    
    	delete window_menu;
	delete icon_menu;
	delete master_strut;

	list<Strut*>::iterator iter;
	for(iter=client_struts.begin(); iter != client_struts.end(); iter++)
	{
		client_struts.remove(*iter);
		delete(*iter);
	}

	client_struts.clear();
	client_window_list.clear();
	client_list.clear();

    	XFreeFont(dpy, font);
    	
	XFreeCursor(dpy, move_curs);
    	XFreeCursor(dpy, resize_curs);
	
	XFreeGC(dpy, invert_gc);
    	XFreeGC(dpy, border_gc);
    	XFreeGC(dpy, string_gc);

	XInstallColormap(dpy, DefaultColormap(dpy, screen));
    	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    	XCloseDisplay(dpy);
}

/* If we can't find a wm->wm_state we're going to have to assume
 * Withdrawn. This is not exactly optimal, since we can't really
 * distinguish between the case where no WM has run yet and when the
 * state was explicitly removed (Clients are allowed to either set the
 * atom to Withdrawn or just remove it... yuck.) */
long WindowManager::getWMState(Window window)
{
    	Atom real_type; int real_format;
    	unsigned long items_read, items_left;
    	long *data, state = WithdrawnState;

    	if (XGetWindowProperty(dpy, window, atom_wm_state, 0L, 2L, False,
            wm->atom_wm_state, &real_type, &real_format, &items_read, &items_left,
            (unsigned char **) &data) == Success && items_read) {
        	state = *data;
        	XFree(data);
    	}
    
    	return state;
}

/* Attempt to follow the ICCCM by explicity specifying 32 bits for
 * this property. Does this goof up on 64 bit systems? */
void WindowManager::setWMState(Window window, int state)
{
    	CARD32 data[2];

    	data[0] = state;
    	data[1] = None; // Icon? We don't need no steenking icon. 

    	XChangeProperty(dpy, window, atom_wm_state, atom_wm_state,
        	32, PropModeReplace, (unsigned char *)data, 2);
}

// The name of this function is a bit misleading: if the client
// doesn't listen to WM_DELETE then we just terminate it with extreme
// prejudice. 
void WindowManager::sendWMDelete(Window window)
{
    	int i, n, found = 0;
    	Atom *protocols;

    	if (XGetWMProtocols(dpy, window, &protocols, &n)) {
        	for (i=0; i<n; i++) if (protocols[i] == atom_wm_delete) found++;
        	XFree(protocols);
    	}
    	if (found) 
		sendXMessage(window, atom_wm_protos, NoEventMask, atom_wm_delete);
    	else XKillClient(dpy, window);
}

// Currently, only sendWMDelete uses this one...
int WindowManager::sendXMessage(Window w, Atom a, long mask, long x)
{
    	XEvent e;

    	e.type = ClientMessage;
    	e.xclient.window = w;
    	e.xclient.message_type = a;
    	e.xclient.format = 32;
    	e.xclient.data.l[0] = x;
    	e.xclient.data.l[1] = CurrentTime;

    	return XSendEvent(dpy, w, False, mask, &e);
}

// This one does -not- free the data coming back from Xlib; it just
// sends back the pointer to what was allocated.
MwmHints* WindowManager::getMWMHints(Window w)
{
    	Atom real_type; int real_format;
    	unsigned long items_read, items_left;
    	MwmHints *data;

    	if (XGetWindowProperty(dpy, w, atom_mwm_hints, 0L, 20L, False,
            wm->atom_mwm_hints, &real_type, &real_format, &items_read, &items_left,
            (unsigned char **) &data) == Success && items_read >= PropMwmHintsElements) 
	{
        	return data;
	} 
	else return NULL;
}
