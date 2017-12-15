/*
 * client.cc
 * Copyleft 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 * Modified by xento figal http://xinfo.sf.net
 */
 
#include "xaewm.hh"

Client::Client(Display *d, Window new_client)
{
	initialize(d);
	wm->addClient(this);
	makeNewClient(new_client);
}

Client::~Client()
{
	removeClient();
}

void Client::initialize(Display *d)
{
	dpy 			= d;

	name			= NULL;
	    	
	window 			= None;
	frame  			= None;
	title			= None;
	trans  			= None;

	window_menu 		= NULL;
	
    	x      			= 1; 
	y      			= 1;
	width  			= 1;
	height 			= 1;
    	ignore_unmap		= 0;

	pointer_x		= 0;
	pointer_y		= 0;

	old_cx			= 0;
	old_cy			= 0;

	wire_move		= wm->getWireMove();

#ifdef SHAPE
   	has_been_shaped 	= false;
#endif
	
	has_title		= true;
	has_border		= true;
	has_focus 		= false;

	is_iconified 		= false;

	// Extra Window States
	is_shaded		= false;
	is_maximized		= false;
	is_maximized_vertical	= false;
	is_maximized_horizontal	= false;
	is_sticky		= false;
	is_always_on_top	= false;
	is_visible		= false;
	
	is_being_dragged 	= false;
	do_drawoutline_once	= false;
	is_being_resized 	= false;
	
	client_strut 		= new Strut;
		
	client_strut->east	= 0;
	client_strut->west	= 0;
	client_strut->north	= 0;
	client_strut->south	= 0;
		
	has_strut=false;
		
	has_extended_net_name	= false;
	skip_taskbar		= false;
	skip_pager		= false;
	
	last_button1_time 	= 0;
	old_x			= 0;
	old_y	   		= 0;
	old_width  		= 1;
	old_height 		= 1;
	
	direction		= 0;
	ascent			= 0;
	descent 		= 0;
	text_width 		= 0;
	text_justify		= 0;
	justify_style 		= wm->getTextJustify();
	
	screen 			= DefaultScreen(dpy);
	root			= wm->getRootWindow();
	
	xres			= wm->getXRes();
	yres			= wm->getYRes();
}

void Client::getXClientName()
{
	if(name) XFree(name);

	wm->getExtendedWMHintString(window, wm->atom_extended_net_wm_name, &name);
	
	// _NET_WM_NAME isn't set fallback to XA_WM_NAME
	if(name==NULL)
	{
    		XFetchName(dpy, window, &name);
		
		if(name==NULL) 
		{
			XStoreName(dpy, window, "no name");
			XFetchName(dpy, window, &name);
		}
	} 
	else 
		has_extended_net_name=true;

	if(name!=NULL)
	{
		XTextExtents(wm->getFont(), name , strlen(name), 
			&direction, &ascent, 
            		&descent, &overall);
	
		text_width = overall.width;
	}
}

// Set up a client structure for the new (not-yet-mapped) window. The
// confusing bit is that we have to ignore 2 unmap events if the
// client was already mapped but has IconicState set (for instance,
// when we are the second window manager in a session).  That's
// because there's one for the reparent (which happens on all viewable
// windows) and then another for the unmapping itself. 
void Client::makeNewClient(Window w)
{
  	XWindowAttributes attr;
    	XWMHints *hints;
    	MwmHints *mhints;

    	long dummy;

    	XGrabServer(dpy);

	window = w;

	getXClientName();

	XGetTransientForHint(dpy, window, &trans);

    	XGetWindowAttributes(dpy, window, &attr);

    	x = attr.x;
    	y = attr.y;
    	width = attr.width;
    	height = attr.height;
    	cmap = attr.colormap;
    	size = XAllocSizeHints();
    	XGetWMNormalHints(dpy, window, size, &dummy);

    	old_x		= x;
    	old_y		= y;
    	old_width 	= width;
    	old_height 	= height;

    	if ((mhints = wm->getMWMHints(window))) 
	{
        	if (mhints->flags & MwmHintsDecorations && !(mhints->decorations & MwmDecorAll)) 
		{
            		has_title  = mhints->decorations & MwmDecorTitle;
            		has_border = mhints->decorations & MwmDecorBorder;
		}
        
		XFree(mhints);
    	}

    	if (attr.map_state == IsViewable) ignore_unmap++;
	{
		initPosition();
		
        	if ((hints = XGetWMHints(dpy, w))) 
		{
            		if (hints->flags & StateHint) 
				wm->setWMState(window, hints->initial_state);
			else wm->setWMState(window, NormalState);

            		XFree(hints);
        	}
    	}

	window_menu = wm->getWindowMenu();

    	gravitate(APPLY_GRAVITY);
    	reparent();

	NetWMStates *win_states;
	win_states = wm->getExtendedNetWMStates(window);

	if(win_states->sticky) is_sticky=true;
	if(win_states->shaded) shade();
	if(win_states->max_vert && win_states->max_horz) maximize();
	if(win_states->skip_taskbar) skip_taskbar=true;
	if(win_states->skip_pager) skip_pager=true;
	
	delete win_states;

    	if(!trans) wm->updateClientList();

	belongs_to_desktop = wm->findGnomeDesktopHint(window);
	
	if(belongs_to_desktop == -1)
	{
		belongs_to_desktop = wm->findExtendedDesktopHint(window);	
	}

	if(belongs_to_desktop == -1)
        {
		belongs_to_desktop = wm->getCurrentDesktop();
        }
	
	wm->setGnomeHint(window, wm->atom_gnome_win_workspace, belongs_to_desktop); 

	wm->setExtendedWMHint(window, wm->atom_extended_net_wm_desktop, belongs_to_desktop);

    	if(wm->getHint(window, wm->atom_gnome_win_state)&WIN_STATE_STICKY) is_sticky=true;
	if(wm->getHint(window, wm->atom_gnome_win_hints)&WIN_HINTS_DO_NOT_COVER) is_always_on_top=true;	

	// Do we want a strut?
	int num=0;
	CARD32 *temp_strut=NULL;
	temp_strut = (CARD32*) wm->getExtendedNetPropertyData(window, wm->atom_extended_net_wm_strut, XA_CARDINAL, &num);

	if(temp_strut) 
	{	
		client_strut->west = temp_strut[0];
		client_strut->east = temp_strut[1];
		client_strut->north = temp_strut[2];
		client_strut->south = temp_strut[3];
			
		wm->addStrut(client_strut);
			
		has_strut=true;	
		
		XFree(temp_strut);								
	}
	
	// Actually updates both gnome and net client lists.
	updateNetWMStates();

	if (wm->getWMState(window) == NormalState || wm->getWMState(window) == WithdrawnState)
	{
		unhide();
	
		if(wm->getFocusModel() == FOCUS_CLICK) 
			XSetInputFocus(dpy, window, RevertToNone, CurrentTime);
       	} 
	else if (wm->getWMState(window) == IconicState) iconify();
	
	XSync(dpy, False);
    	XUngrabServer(dpy);
}

void Client::removeClient()
{
    	XGrabServer(dpy);

	if(trans) XSetInputFocus(dpy, trans, RevertToNone, CurrentTime);

	XUngrabButton(dpy, AnyButton, AnyModifier, frame);

	gravitate(REMOVE_GRAVITY);
	XReparentWindow(dpy, window, root, x, y);
	
	XRemoveFromSaveSet(dpy,window);

	XDestroyWindow(dpy, title);
	XDestroyWindow(dpy, frame);
	
	if (name) XFree(name);
	if (size) XFree(size);

    	XSync(dpy, False);
    	XUngrabServer(dpy);

	if(has_strut)
		wm->removeStrut(client_strut);
			
	delete client_strut;

    	window_menu->hide();
		
	wm->removeClient(this);
}

// For a regular window, trans is None (false), and we include
// enough space to draw the title. For a transient window we just make
// a tiny strip. 
int Client::theight()
{
   	if (!has_title) return 0;
	
	int title_size = wm->getFont()->ascent + wm->getFont()->descent + SPACE;
	
	if(trans) return 4; // size for transient titlebar heights not to big, not too small
	else
		return title_size;
	
}

// This is called whenever we update our Client stuff. 
void Client::sendConfig()
{
    	XConfigureEvent ce;

    	ce.type = ConfigureNotify;
    	ce.event = window;
    	ce.window = window;
    	ce.x = x;
    	ce.y = y;
    	ce.width = width;
    	ce.height = height;
    	ce.border_width = 0;
    	ce.above = (is_always_on_top) ? Above : Below;
    	ce.override_redirect = False;

    	XSendEvent(dpy, window, False, StructureNotifyMask, (XEvent *)&ce);
}

void Client::redraw()
{
	if (!has_title) return;
  
    	if( has_focus ) XSetForeground(dpy, wm->getStringGC(), wm->getFGColor().pixel);
	else XSetForeground(dpy, wm->getStringGC(), wm->getBDColor().pixel);
    
    	XDrawLine(dpy, title, wm->getBorderGC(), 0, theight() - BW + BW/2, width, theight() - BW + BW/2);
	
    	if(has_focus)
	{
		XDrawLine(dpy, title, wm->getBorderGC(),
        		width - theight()+ BW/2, 0,
	        	width - theight()+ BW/2, theight());
	}
	
    	if (!trans && name)
    	{
		switch(justify_style)
		{
			case LEFT_JUSTIFY:
				text_justify = SPACE;
			break;
		
			case CENTER_JUSTIFY:
				text_justify = ( (width / 2) - (text_width / 2) );
			break;
		
			case RIGHT_JUSTIFY:
				text_justify = width - text_width - 25;
			break;
		}		
	
		if(name!=NULL)
		{
			XDrawString(dpy, title, wm->getStringGC(),
				text_justify, wm->getFont()->ascent+1,
				name, strlen(name));
		}
	}
}

// Window gravity is a mess to explain, but we don't need to do much
// about it since we're using X borders. For NorthWest et al, the top
// left corner of the window when there is no WM needs to match up
// with the top left of our fram once we manage it, and likewise with
// SouthWest and the bottom right (these are the only values I ever
// use, but the others should be obvious.) Our titlebar is on the top
// so we only have to adjust in the first case. 
void Client::gravitate(int multiplier)
{
    	int dy = 0;
    	int gravity = (size->flags & PWinGravity) ?
        	size->win_gravity : NorthWestGravity;

    	switch (gravity) {
        	case NorthWestGravity:
        	case NorthEastGravity:
        	case NorthGravity: dy = theight(); break;
        	case CenterGravity: dy = theight()/2; break;
    	}

   	y += multiplier * dy;
}

/* Well, the man pages for the shape extension say nothing, but I was
 * able to find a shape.PS.Z on the x.org FTP site. What we want to do
 * here is make the window shape be a boolean OR (or union, if you
 * prefer) of the client's shape and our titlebar. The titlebar
 * requires both a bound and a clip because it has a border -- the X
 * server will paint the border in the region between the two. (I knew
 * that using X borders would get me eventually... ;-)) */
#ifdef SHAPE
void Client::setShape()
{
	int n=0, order=0;
    	XRectangle temp, *dummy;

    	dummy = XShapeGetRectangles(dpy, window, ShapeBounding, &n, &order);
	
    	if (n > 1) {
        	XShapeCombineShape(dpy, frame, ShapeBounding,
	            0, theight(), window, ShapeBounding, ShapeSet);

        	temp.x = -BW;
	        temp.y = -BW;
        	temp.width = width + 2*BW;
	        temp.height = theight() + BW;
        	
		XShapeCombineRectangles(dpy, frame, ShapeBounding,
	            0, 0, &temp, 1, ShapeUnion, YXBanded);
        	
		temp.x = 0;
	        temp.y = 0;
        	temp.width = width;
	        temp.height = theight() - BW;
        	
		XShapeCombineRectangles(dpy, frame, ShapeClip,
	            0, theight(), &temp, 1, ShapeUnion, YXBanded);
        	
		has_been_shaped = 1;
	} 
	else 
	if (has_been_shaped) 
	{
	        // I can't find a 'remove all shaping' function... 
	        temp.x = -BW;
        	temp.y = -BW;
	        temp.width = width + 2*BW;
        	temp.height = height + theight() + 2*BW;
	        
		XShapeCombineRectangles(dpy, frame, ShapeBounding,
        	    0, 0, &temp, 1, ShapeSet, YXBanded);
    	}
    
    	XFree(dummy);
}
#endif

void Client::iconify()
{
	if (!ignore_unmap) ignore_unmap++; 

	if(has_focus) setFocus(false);

	XUnmapWindow(dpy, window);
	XUnmapWindow(dpy, title);
	XUnmapWindow(dpy, frame);
		
	is_iconified=true;
	
	wm->setWMState(window, IconicState);

	if(!trans) 
	{
		wm->addClientToIconMenu(this);
		wm->findTransientsToMapOrUnmap(window, true);
	}
	
	is_visible=false;
}

void Client::hide()
{
	if (!ignore_unmap) ignore_unmap++; 

	if(has_focus) setFocus(false);

	if(window_menu->isVisible()) window_menu->hide();

	XUnmapWindow(dpy, window);
	XUnmapWindow(dpy, title);
	XUnmapWindow(dpy, frame);
	
	wm->setWMState(window, WithdrawnState);

	is_visible=false;
}

void Client::unhide()
{
	if(belongs_to_desktop == wm->getCurrentDesktop())
	{
		XMapRaised(dpy, window);
		XMapRaised(dpy, title);
		XMapRaised(dpy, frame);

		if(is_iconified)
		{
			is_iconified=false;
			wm->removeClientFromIconMenu(this);
		} 

		wm->setWMState(window, NormalState);

		if(!trans) wm->findTransientsToMapOrUnmap(window, false);

		wm->restackOnTopWindows();	
	
		if(wm->getFocusModel() == FOCUS_CLICK)
			XSetInputFocus(dpy, window, RevertToNone, CurrentTime);
		
		is_visible=true;
	}
}

// This function sets up the initial position of windows when they are mapped
// for the first time. It still needs some work done to it to make it more 
// aware of _NET_WM_STRUT's
// 
//
//	XSizeHints structure definition
// 
// 	typedef struct {
//            long flags;
//            int x, y;
//            int width, height;
//            int min_width, min_height;
//            int max_width, max_height;
//            int width_inc, height_inc;
//            struct {
//                   int x;
//                   int y;
//            } min_aspect, max_aspect;
//            int base_width, base_height;
//            int win_gravity;
//
//       } XSizeHints;
//
void Client::initPosition()
{
    	int mouse_x, mouse_y;

	Strut *temp_strut = wm->getMasterStrut();

	unsigned int w, h;
	unsigned int border_width, depth;

  	XWindowAttributes attr;

    	XGetWindowAttributes(dpy, window, &attr);

	// If the window is mapped already leave we want
	// to leave it alone!
	if (attr.map_state == IsViewable) return;

	XGetGeometry(dpy, window, &root, &x, &y, &w, &h, &border_width, &depth);
		
	width = (int)w;
	height = (int)h;

	// Lets be strut concious here, hehe =)
	fixupPositionBasedOnStruts(temp_strut);

    	if (size->flags & PPosition) 
	{
        	if(!x) x = size->x;
        	if(!y) y = size->y;
		
		// Lets be strut concious here, hehe =)
		fixupPositionBasedOnStruts(temp_strut);
    	} 
	else 
	{
		if (size->flags & USPosition) 
		{
			if(!x) x = size->x;
            		if(!y) y = size->y;
			
			// Lets be strut concious here, hehe =)
			fixupPositionBasedOnStruts(temp_strut);
		}
		else
		if ( (x==0) || (y==0)  )
    		{
			if( width>=xres && height>=yres	)
			{
				is_always_on_top=true;
				
				x=0;
				y=0;
				width=xres;
				height=yres-theight();
				
				if(!trans) wm->updateClientList();
			}
			else 
			{
			
				wm->getMousePosition(&mouse_x, &mouse_y);

				if(mouse_x && mouse_y)
				{
					x = (int) (((long) (xres - width) 
		      				* (long) mouse_x) / (long) xres);
	 				y = (int) (((long) (yres - height - theight()) 
		      				* (long) mouse_y) / (long) yres);
	 				y = (y<theight()) ? theight() : y;

					// Lets be strut concious here, hehe =)
					// This isn't the end all and be all of 
					// being strut conscious. This doesn't account
					// for some windows.
				
					fixupPositionBasedOnStruts(temp_strut);
										
	         			gravitate(REMOVE_GRAVITY);	
				}
			}
			
			
    		} 
    	}
}

void Client::fixupPositionBasedOnStruts(Strut* temp_strut)
{
	bool size_changed=false;
		
	if(x < (int) temp_strut->west)
		x = temp_strut->west;
	else
	if( 
		( (x + width) > xres ) 
		|| 
		( (x+width) > (xres - (int) temp_strut->east) )
	)
		x = ((xres-width) - temp_strut->east);
	
	if(y < (int) temp_strut->north)
		y = temp_strut->north;
	else
	if( 
		( (y + height) > yres )
		||
		( (y+height) > (yres - (int) temp_strut->south) )
	)
		y = ((yres-height) - temp_strut->south);

	if(x < 0) x = 0;
	if(y < 0) y = 0;
	
	if(width > xres)
	{
		width = xres - temp_strut->east - x;
		size_changed=true;
	}
	
	if(height > yres)
	{
		height = yres - temp_strut->south - y;
		size_changed=true;
	}
	
	if(size_changed) sendConfig();
}

void Client::maximize()
{
	if(trans) return;

	if(is_shaded) 
	{
		shade();
		return;
	}

	if(! is_maximized)
	{
		old_x=x;
		old_y=y;
		old_width=width;
		old_height=height;

		// Check to see if this client sets
		// its max size property. If so don't
		// maximize it past that size.
		if (size->flags & PMaxSize) {
			
			width = size->max_width;
	        	height = size->max_height;
			
			XMoveResizeWindow(dpy, frame, x, y-theight(), width, height+theight());
			
	    	} else {
	
			x=0;
			y=0;
			width=xres;
			height=yres;
		
			// MAKE SURE WE DON'T MAXIMIZE OVER A STRUT
			Strut *temp_strut = wm->getMasterStrut();
			
			x = temp_strut->west;
			y = temp_strut->north;
			width = xres - temp_strut->east - x;
			height = yres - temp_strut->south - y;

			XMoveResizeWindow(dpy, frame, x, y, width, height);

			y = theight();
			height -= theight();
		}
		
		is_maximized=true;
		
		is_maximized_vertical=true;
		is_maximized_horizontal=true;
		
	} else {
	
		x=old_x;
		y=old_y;
		width=old_width;
		height=old_height;
	
		XMoveResizeWindow(dpy, frame,
        		old_x, old_y - theight(), old_width, old_height + theight());
	
		is_maximized=false;

		is_maximized_vertical=false;
		is_maximized_horizontal=false;
	
		if(is_shaded) is_shaded=false;
	}

	XResizeWindow(dpy, title, width, theight());
	XResizeWindow(dpy, window, width, height);
	
	sendConfig();
	
	updateNetWMStates();
}

void Client::handleMotionNotifyEvent(XMotionEvent *ev)
{
	int nx=0, ny=0;

	if(ev->state & Button1Mask)
	{
			if(! do_drawoutline_once && wire_move) 
			{
				XGrabServer(dpy);
				drawOutline();
				do_drawoutline_once=true;
				is_being_dragged=true;
			}
		
			if(wire_move) drawOutline();

			nx = old_cx + (ev->x_root - pointer_x);
			ny = old_cy + (ev->y_root - pointer_y);
		
			if(wm->getEdgeSnap())
			{
				// Move beyond edges of screen
				if(nx == xres - width) nx = xres - width + 1;		
				else if(nx == 0) nx = -1;
		
				if(ny == yres - SNAP) ny = yres - SNAP - 1;
				else if(ny == theight()) ny = theight() - 1;
				
				// Snap to edges of screen
				if( (nx + width >= xres - SNAP) && (nx + width <= xres) ) nx = xres - width;
				else if( (nx <= SNAP) && (nx >= 0) ) nx = 0;

				if(is_shaded)
				{
		 			if( (ny  >= yres - SNAP) && (ny  <= yres) ) ny = yres;
					else if( (ny - theight() <= SNAP) && (ny - theight() >= 0)) ny = theight();
				}
				else
				{
					if( (ny + height >= yres - SNAP) && (ny + height <= yres) ) ny = yres - height;
					else if( (ny - theight() <= SNAP) && (ny - theight() >= 0)) ny = theight();
				}
			}
		
			x=nx; y=ny;
			
			if(!wire_move)
			{
				XMoveWindow(dpy, frame, nx, ny-theight());
				sendConfig();
			}
		
			if(wire_move) drawOutline();
	}
	else 
	if(ev->state & Button2Mask)
	{
		if(! is_being_resized)
		{
		     	int in_box = (ev->x >= width - theight()) && (ev->y <= theight());
			
			if(! in_box) return;
		}
		
		if(! do_drawoutline_once) 
		{ 
			XGrabServer(dpy);
			is_being_resized=true;
			do_drawoutline_once=true;
			drawOutline();
			setmouse(frame, width, height+theight());
		}
   		else
		{
			if((ev->x > 50) && (ev->y > 50))
			{
				drawOutline();

				width =  ev->x;
				height = ev->y - theight();

    				getIncsize(&width, &height, PIXELS);
        
    				if (size->flags & PMinSize) 
				{
	        			if (width < size->min_width) width = size->min_width;
        				if (height < size->min_height) height = size->min_height;
					
					if(width<100) width=100;
					if(height<50) height=50;
				}

				if (size->flags & PMaxSize) 
				{
       					if (width > size->max_width) width = size->max_width;
       					if (height > size->max_height) height = size->max_height;
				
					if(width>xres) width=xres;
					if(height>yres) height=yres;
				} 
			
				drawOutline();			
			}
		}
	}
}

void Client::drawOutline()
{
    	char buf[32];
 	
    	if(! is_shaded)
    	{
    		XDrawRectangle(dpy, root, wm->getInvertGC(),
        		x + BW/2, y - theight() + BW/2,
        		width + BW, height + theight() + BW);

		XDrawRectangle(dpy, root, wm->getInvertGC(),
        		x + BW/2 + 4, y - theight() + BW/2 + 4,
        		width + BW - 8, height + theight() + BW - 8);			
			
    		//XDrawLine(dpy, root, wm->getInvertGC(), x + BW, y + BW/2,
        	//	x + width + BW, y + BW/2);
    
    		gravitate(REMOVE_GRAVITY);
    		snprintf(buf, sizeof buf, "%dx%d+%d+%d", width, height, x, y);
		gravitate(APPLY_GRAVITY);
    		XDrawString(dpy, root, wm->getInvertGC(),
	        	x + width - XTextWidth(wm->getFont(), buf, strlen(buf)) - SPACE - 4,
	        	y + height - SPACE - 4,
        		buf, strlen(buf));
    	} 
	else 
	{
		XDrawRectangle(dpy, root, wm->getInvertGC(),
        			x + BW/2, 
				y - theight() + BW/2,
        			width + BW, 
				theight() + BW);
    	}
}

// If the window in question has a ResizeInc int, then it wants to be
// resized in multiples of some (x,y). Here we set x_ret and y_ret to
// the number of multiples (if mode == INCREMENTS) or the correct size
// in pixels for said multiples (if mode == PIXELS). 
int Client::getIncsize(int *x_ret, int *y_ret, int mode)
{
    	int basex, basey;

    	if (size->flags & PResizeInc) 
	{
        	basex = (size->flags & PBaseSize) ? size->base_width :
            	(size->flags & PMinSize) ? size->min_width : 0;
        	
		basey = (size->flags & PBaseSize) ? size->base_height :
            	(size->flags & PMinSize) ? size->min_height : 0;
                                
		if (mode == PIXELS) 
		{
            		*x_ret = width - ((width - basex) % size->width_inc);
            		*y_ret = height - ((height - basey) % size->height_inc);   
        	} 
		else // INCREMENTS 
		{
                        *x_ret = (width - basex) / size->width_inc;
            		*y_ret = (height - basey) / size->height_inc;
        	}
        
		return 1;
    	} 

    return 0;
}

// This function makes it so only the titlebar shows.
void Client::shade()
{
	raise();
	
	wm->restackOnTopWindows();	

	if(! is_shaded)
	{
		XResizeWindow(dpy, frame, width, theight() - 1);
		is_shaded=true;
	} else {
		XResizeWindow(dpy, frame, width, height + theight());
		is_shaded=false;
	}
	
	updateNetWMStates();
}

// Because we are redirecting the root window, we get ConfigureRequest
// events from both clients we're handling and ones that we aren't.
// For clients we manage, we need to fiddle with the frame and the
// client window, and for unmanaged windows we have to pass along
// everything unchanged. Thankfully, we can reuse (a) the
// XWindowChanges struct and () the code to configure the client
// window in both cases.
//
// Most of the assignments here are going to be garbage, but only the
// ones that are masked in by e->value_mask will be looked at by the X
// server. 
void Client::handleConfigureRequest(XConfigureRequestEvent *e)
{
      	XWindowChanges wc;

        gravitate(REMOVE_GRAVITY);
        if (e->value_mask & CWX) x = e->x;
        if (e->value_mask & CWY) y = e->y;
        if (e->value_mask & CWWidth) width = e->width;
        if (e->value_mask & CWHeight) height = e->height;
        gravitate(APPLY_GRAVITY);

	Strut *temp_strut = wm->getMasterStrut();
		
	if((! has_strut) && (has_title))
	{
		fixupPositionBasedOnStruts(temp_strut);
	}
	
	wc.x = x;
        wc.y = y - theight();
        wc.width = width;
        wc.height = height + theight();
        wc.border_width = BW;
        wc.sibling = e->above;
        wc.stack_mode = e->detail;
        XConfigureWindow(dpy, frame, e->value_mask, &wc);

	if(! is_shaded) 
	{
		XMoveResizeWindow(dpy, frame,x,y-theight(), width, height+theight());
		XResizeWindow(dpy, title, width, theight());
		XMoveResizeWindow(dpy, window,0,theight(),width,height);
	}

	// If an app wants to place his window in a bogus position
	// like offscreen then fix its position. Lets see if this
	// works out okay. Warez and Porn sites have a bad habit
	// of trying to place the Mozilla browser window titlebar
	// just offscreen so you can't close the window. 
	// The bastards!
	if( (x + width > xres) 		|| 
	    (height + theight() > yres) ||
	    (x > xres)	 		|| 
	    (y > yres)			||
	    (x < 0)			||
	    (y < 0) 
	    )
		initPosition();
	
#ifdef SHAPE
        if (e->value_mask & (CWWidth|CWHeight)) setShape();
#endif

	sendConfig();

	if (e->value_mask & CWY) wc.x = 0;
	if (e->value_mask & CWY) wc.y = theight();
	if (e->value_mask & CWWidth) wc.width = e->width;
	if (e->value_mask & CWHeight) wc.height = e->height;

    	wc.sibling = e->above;
    	wc.stack_mode = e->detail;
    	XConfigureWindow(dpy, e->window, e->value_mask, &wc);

	wm->restackOnTopWindows();
}

void Client::handleMapRequest(XMapRequestEvent *e)
{
	unhide();
}

void Client::handleUnmapEvent(XUnmapEvent *e)
{
	if (! ignore_unmap) 
		delete this;
}

// This happens when a window is iconified and destroys itself. An
// Unmap event wouldn't happen in that case because the window is
// already unmapped. 
void Client::handleDestroyEvent(XDestroyWindowEvent *e)
{
	delete this;
}

// If a client wants to iconify itself (boo! hiss!) it must send a
// special kind of ClientMessage. We might set up other handlers here
// but there's nothing else required by the ICCCM. 
void Client::handleClientMessage(XClientMessageEvent *e)
{
	bool state_remove=false;
	bool state_add=false;
	bool state_toggle=false;
	
	if(e->message_type == wm->atom_extended_net_wm_state)
	{
		if(e->data.l[0]==_NET_WM_STATE_REMOVE) 	state_remove=true;
		if(e->data.l[0]==_NET_WM_STATE_ADD)	state_add=true;
		if(e->data.l[0]==_NET_WM_STATE_TOGGLE)	state_toggle=true;

		if(
			(e->data.l[1] == (long) wm->atom_extended_net_wm_state_sticky)
			||
		    	(e->data.l[2] == (long) wm->atom_extended_net_wm_state_sticky)
		)
		{
			if(state_add) 		is_sticky=true;	
			if(state_remove)	is_sticky=false;
			if(state_toggle) 	is_sticky = (is_sticky) ? true : false;
		}
		
		if(
			(e->data.l[1] == (long) wm->atom_extended_net_wm_state_maximized_vert)
			||
		    	(e->data.l[2] == (long) wm->atom_extended_net_wm_state_maximized_vert)
		)
		{
			if(state_add) 		is_maximized_vertical=true;	
			if(state_remove)	is_maximized_vertical=false;
			if(state_toggle) 	is_maximized_vertical = (is_maximized_vertical) ? true : false;
		}

		if(
			(e->data.l[1] == (long) wm->atom_extended_net_wm_state_maximized_horz)
			||
		    	(e->data.l[2] == (long) wm->atom_extended_net_wm_state_maximized_horz)
		)
		{
			if(state_add) 		is_maximized_horizontal=true;	
			if(state_remove)	is_maximized_horizontal=false;
			if(state_toggle) 	is_maximized_horizontal = (is_maximized_horizontal) ? true : false;
		}
			
		if(is_maximized_vertical || is_maximized_horizontal) maximize();

		if(
			(e->data.l[1] == (long) wm->atom_extended_net_wm_state_shaded)
			||
		    	(e->data.l[2] == (long) wm->atom_extended_net_wm_state_shaded)
		)
		{
			if(state_add) 		is_maximized_horizontal=true;	
			if(state_remove)	is_maximized_horizontal=false;
			if(state_toggle) 	shade();
		}

		if(
			(e->data.l[1] == (long) wm->atom_extended_net_wm_state_skip_taskbar)
			||
		    	(e->data.l[2] == (long) wm->atom_extended_net_wm_state_skip_taskbar)
		)
		{
			if(state_add) 		skip_taskbar=true;	
			if(state_remove)	skip_taskbar=false;
			if(state_toggle) 	skip_taskbar = (skip_taskbar) ? true : false;
		}

		if(
			(e->data.l[1] == (long) wm->atom_extended_net_wm_state_skip_pager)
			||
		    	(e->data.l[2] == (long) wm->atom_extended_net_wm_state_skip_pager)
		)
		{
			if(state_add) 		skip_pager=true;	
			if(state_remove)	skip_pager=false;
			if(state_toggle) 	skip_pager = (skip_pager) ? true : false;
		}

		updateNetWMStates();
	}
	
	if(e->message_type == wm->atom_extended_net_active_window &&
		e->data.l[0] == 0)
	{
		wm->sendXMessage(window, wm->atom_wm_protos, SubstructureRedirectMask, wm->atom_wm_takefocus );
		XSetInputFocus(dpy, window, RevertToNone, CurrentTime);
		XInstallColormap(dpy, cmap);
	}
			
	if(e->message_type == wm->atom_extended_net_close_window 
		&& e->data.l[0] == 0)			
			delete this;
	
	if (e->message_type == wm->atom_wm_change_state &&
        	e->format == 32 && e->data.l[0] == IconicState) 
			iconify();
}

void Client::handlePropertyChange(XPropertyEvent *e)
{
	long dummy;

	if(e->atom == wm->atom_gnome_win_state)
	{
		if(wm->getHint(window, wm->atom_gnome_win_state)&WIN_STATE_STICKY) is_sticky=true;
		else is_sticky=false; 
	}

	if(e->atom == wm->atom_gnome_win_hints)
	{
		if(wm->getHint(window, wm->atom_gnome_win_hints)&WIN_HINTS_DO_NOT_COVER) is_always_on_top=true;	
		else is_always_on_top=false;
	}

	if(e->atom == wm->atom_extended_net_wm_desktop)	
	{	
			belongs_to_desktop = wm->findExtendedDesktopHint(window);
			if(belongs_to_desktop != wm->getCurrentDesktop()) hide();
	}

	if(e->atom == wm->atom_extended_net_wm_strut)
	{
		// Do we want to update our strut?
		int num=0;
		CARD32 *temp_strut = NULL;
		temp_strut = (CARD32*) wm->getExtendedNetPropertyData(window, wm->atom_extended_net_wm_strut, XA_CARDINAL, &num);

		if(has_strut)
		{
			wm->removeStrut(client_strut);
			has_strut=false;
		}

		if(temp_strut)
		{
			client_strut->west = temp_strut[0];
			client_strut->east = temp_strut[1];
			client_strut->north = temp_strut[2];
			client_strut->south = temp_strut[3];
		
			wm->addStrut(client_strut);
			
			has_strut=true;
			
			XFree(temp_strut);
		}
	}
	
	if(has_extended_net_name)
	{
		if(e->atom == wm->atom_extended_net_wm_name)
		{ 
			getXClientName();
			
			// Yup we totally ignore UTF-8 strings here. Don't 
			// know how to implement it. And I can't find any 
			// docs on how to implement it with respect to window
			// managers.
			
			XClearWindow(dpy, title);
 			redraw();
		}
	}
	
	switch (e->atom) 
	{
        	case XA_WM_NAME:
			if(! has_extended_net_name)
			{
				getXClientName();
	    
       				XClearWindow(dpy, title);
       				redraw();
			}
            	break;
		
        	case XA_WM_NORMAL_HINTS:
            		XGetWMNormalHints(dpy, window, size, &dummy);
	    	break;
	    
		case XA_WM_TRANSIENT_FOR:
			if(!trans) XGetTransientForHint(dpy, window, &trans);
		break;
	}
}

void Client::reparent()
{
    	XSetWindowAttributes pattr;

	XGrabServer(dpy);

    	pattr.background_pixel = wm->getFCColor().pixel;
    	pattr.border_pixel = wm->getBDColor().pixel;
	pattr.do_not_propagate_mask = ButtonPressMask|ButtonReleaseMask|ButtonMotionMask;
	pattr.override_redirect=False;
	pattr.event_mask = ButtonMotionMask		|
			   SubstructureRedirectMask	|
			   SubstructureNotifyMask	|	
			   ButtonPressMask		|
			   ButtonReleaseMask		|
			   ExposureMask			|
			   EnterWindowMask 		|
			   LeaveWindowMask		;
    
    	frame = XCreateWindow(dpy, 
				root,
        			x, 
				y - theight(), 
				width, 
				height + theight(), 
				BW,
        			DefaultDepth(dpy, wm->getScreen()), 
				CopyFromParent, 
				DefaultVisual(dpy, wm->getScreen()),
        			CWOverrideRedirect|CWDontPropagate|CWBackPixel|CWBorderPixel|CWEventMask, 
				&pattr);

	// This window is used to house the window title and title bar button
	title = XCreateWindow(dpy, 
				frame,
        			0, 
				0, 
				width, 
				theight(), 
				0,
        			DefaultDepth(dpy, wm->getScreen()), 
				CopyFromParent, 
				DefaultVisual(dpy, wm->getScreen()),
				CWOverrideRedirect|CWDontPropagate|CWBackPixel|CWBorderPixel|CWEventMask, 
				&pattr);

	#ifdef SHAPE
    	if (wm->getShape()) {
        	XShapeSelectInput(dpy, window, ShapeNotifyMask);
        	setShape();
    	}
	#endif

	// We don't want these masks to be propagated down to the frame
	XChangeWindowAttributes(dpy, window, CWDontPropagate, &pattr);

	XSelectInput(dpy, window, FocusChangeMask|PropertyChangeMask);

    	XAddToSaveSet(dpy, window);
    	XSetWindowBorderWidth(dpy, window, 0);
    	XReparentWindow(dpy, window, frame, 0, theight());
	XResizeWindow(dpy,window,width,height);
	
	sendConfig();

	XGrabButton(dpy, 
		Button1, 
		AnyModifier, 
		frame,
		1, 
		ButtonPressMask|ButtonReleaseMask,
		GrabModeSync, 
		GrabModeAsync, None, None);	
	

	XSync(dpy, false);
	XUngrabServer(dpy);
}


// This function handles the button events.
// Remember this is designed for a 3 button mouse and to the way I like
// the buttons laid out. If you want something different then edit
// this until your heart is content.
void Client::handleButtonEvent(XButtonEvent *e)
{
	int in_box;
	
	// Formula to tell if the pointer is in the little
	// box on the right edge of the window. This box is
	// the iconify button, resize button and close button.
     	in_box = (e->x >= width - theight()) && (e->y <= theight());

	// Used to compute the pointer position on click
	// used in the motion handler when doing a window move.
	old_cx = x;
	old_cy = y;
	pointer_x = e->x_root;
	pointer_y = e->y_root;

	// Allow us to get clicks from anywhere on the window
	// so click to raise works.
	XAllowEvents(dpy, ReplayPointer, CurrentTime);

	window_menu->hide();

	switch (e->button) 
	{
		case Button4:
		case Button5:
		{
			if(is_being_resized)
			{
				drawOutline();
				do_drawoutline_once=false;
				is_being_resized=false;
				
				XResizeWindow(dpy, frame, width, height + theight());
				XResizeWindow(dpy, title, width, theight());
				XResizeWindow(dpy, window, width, height);

				sendConfig(); 

				//XClearWindow(dpy, title);
				//redraw();		
				XUngrabServer(dpy);
				XSync(dpy, False);				
				
				return;
			}
		}
		break;
	
	    case Button1:
	    {
		if (e->type == ButtonPress) 
		{		
			if(e->window == window || e->subwindow == window)
			{
				raise();
				wm->restackOnTopWindows();
			}
			
			if (e->window == title) 
			{
				window_menu->hideAllVisibleSubmenus();

				if (in_box) 
				{
					if(!trans)
					{
						window_menu->hide();
						iconify();
					}
				}
				else
				{
					raise();
					wm->restackOnTopWindows();
	      			}
			}			
     		}

		if (e->type == ButtonRelease) 
		{
			if(is_being_dragged)
			{
				is_being_dragged=false;
				do_drawoutline_once=false;
				drawOutline(); 
				XMoveWindow(dpy, frame, x, y-theight());
				sendConfig();
				//XClearWindow(dpy, title);
				//redraw();
				XUngrabServer(dpy);
					XSync(dpy, False);
			}

			if (e->window == title) 
			{
				// Check for a double click then maximize 
				// the window.
				if(e->time-last_button1_time<250) 
				{
					maximize();
			
					last_button1_time=0;
				
					return;
				} else 
					last_button1_time=e->time;
			}
		}
	      
	    }
	    break;
	      
            case Button2:
	    {	
			if(e->window == title)
			{
				if(in_box) 
				{
					if(e->type == ButtonPress) 
					{
						if(is_shaded) shade();

						raise();
						wm->restackOnTopWindows();
					}
				}
	      		}

			if(e->type == ButtonRelease)
			{
				if(is_being_resized)
				{
					drawOutline();
					do_drawoutline_once=false;
					is_being_resized=false;

					XResizeWindow(dpy, frame, width, height + theight());
					XResizeWindow(dpy, title, width, theight());
					XResizeWindow(dpy, window, width, height);
					
					sendConfig(); 

					//XClearWindow(dpy, title);
					//redraw();		
					
					XUngrabServer(dpy);
					XSync(dpy, False);
					
					return;
				} 
				
				if( (e->window == title) && (!in_box) ) shade();
			}	
	    } 
	    break;
	      
		case Button3:
		{
			if(e->window == title)
			{
				if (e->type == ButtonRelease)
				{
					if (in_box) 
						wm->sendWMDelete(window);
					else {
						if(! trans)
						{
							window_menu->setThisClient(this);
							window_menu->show();
						}
					}
				}
			}
	      }
	      break;
	      	      
	      
	  }
}

void Client::handleEnterEvent(XCrossingEvent *e)
{
	XSetInputFocus(dpy, window, RevertToNone, CurrentTime);
}

void Client::handleFocusInEvent(XFocusChangeEvent *e)
{
	wm->sendXMessage(window, wm->atom_wm_protos, SubstructureRedirectMask, wm->atom_wm_takefocus );
	XInstallColormap(dpy, cmap);
	wm->setExtendedNetActiveWindow(window);
	setFocus(true);
}

void Client::handleFocusOutEvent(XFocusChangeEvent *e)
{
	if(e->window == window)
		wm->setExtendedNetActiveWindow(None);
}

void Client::setFocus(bool focus)
{
	has_focus=focus;
	
	if (has_title)
	{
		if(has_focus)
			XSetWindowBackground(dpy, title, wm->getBGColor().pixel);
		else 
			XSetWindowBackground(dpy, title, wm->getFCColor().pixel);
		
		XClearWindow(dpy, title);
		redraw();
	} 		
}

void Client::handleColormapChange(XColormapEvent *e)
{
    	if (e->c_new) {
        	cmap = e->colormap;
        	XInstallColormap(dpy, cmap);
    	}
}

// If we were covered by multiple windows, we will usually get
// multiple expose events, so ignore them unless e->count (the number
// of outstanding exposes) is zero.
void Client::handleExposeEvent(XExposeEvent *e)
{
    	if (e->count == 0) redraw();
}

#ifdef SHAPE
void Client::handleShapeChange(XShapeEvent *e)
{
    	setShape();
}
#endif

void Client::setDesktop(int desk)
{
	belongs_to_desktop=desk;
	
	if(belongs_to_desktop != wm->getCurrentDesktop())
	{
		hide();
		
		wm->setExtendedWMHint(window, wm->atom_extended_net_wm_desktop, belongs_to_desktop);
		wm->setGnomeHint(window, wm->atom_gnome_win_workspace, belongs_to_desktop);
	}
}

void Client::raise()
{
	XWindowChanges wc;
	wc.stack_mode = Above;
	XConfigureWindow(dpy, frame, CWStackMode, &wc);
}

void Client::lower()
{
	XWindowChanges wc;
	wc.stack_mode = Below;
	XConfigureWindow(dpy, window, CWStackMode, &wc);
}

void Client::updateNetWMStates()
{
	wm->setExtendedNetWMState(
				window,
				false, 	// false because xaewm++ doesn't support modal	
				is_sticky, 		
				is_maximized_vertical, 	
				is_maximized_horizontal,
				is_shaded,		
				skip_taskbar, 		
				skip_pager  		
				);
}
