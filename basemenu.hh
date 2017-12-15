/*
 * xaewm basemenu.hh 
 * Base code by Frank Hale
 * rewrites by xento figal http://xinfo.sf.net
 */
 
#ifndef _BASEMENU_HH_
#define _BASEMENU_HH_

#include "xaewm.hh"

// Parts below borrowed from fspanel.c
static unsigned short cols[] = {
	0xd75c, 0xd75c, 0xd75c,		  /* 0. light gray */
	0xbefb, 0xbaea, 0xbefb,		  /* 1. mid gray */
	0xaefb, 0xaaea, 0xaefb,		  /* 2. dark gray */
	0xefbe, 0xefbe, 0xefbe,		  /* 3. white */
	0x8617, 0x8207, 0x8617,		  /* 4. darkest gray */
	0x0000, 0x0000, 0x0000		  /* 5. black */
};

#define PALETTE_COUNT (sizeof (cols) / sizeof (cols[0]) / 3)

extern unsigned long palette[PALETTE_COUNT];

#define DEFAULT_FOREGROUND_COLOR 	"black"
#define DEFAULT_BACKGROUND_COLOR	"slategrey"
#define DEFAULT_BORDER_COLOR		"black"

// Not all have been implemented yet (for basemenu)
enum { 	
	SEND_TO_DESKTOP=0,
//	EXECUTE_COMMAND=1, 
//	MAXIMIZE=2,
//	ICONIFY=3,
 	UNICONIFY=4
//	CLOSE=5, 
//	LOWER=6,
//	SHADE=7
//
};

class BaseMenu;

class BaseMenuItem 
{
public:	
	BaseMenuItem() {
		client		= NULL;
		icon		= None;
		name		= "";
		exec		= "";
		function	= 0;
		is_selected	= false;
		item_x		= 0;
		item_y		= 0;
		index		= 0;
		sub		= NULL; 
	}
	
	Client *client; 
	Window icon;   // this menu item may be pointing to an icon for use
		       // in an icon list or something.
	
	std::string name; // name showing on menu.
	std::string exec; // command to execute when clicked.
	
	int item_x;
	int item_y;

	int function;
	int index;

	bool is_selected;
	
	BaseMenu *sub; // submenu this item points to.
};

class BaseMenu
{
protected:	
	list<BaseMenuItem*> mi;
	
	Display *dpy;
	Window  root;
	Visual  *visual;
	int	depth;
	unsigned int     screen;
	
	// Menu specific stuff ==========================
	Window item_window;
	int x, y, x_move, y_move;
	unsigned int width,height, total_item_height;
	bool is_visible;
	// ==============================================
	
	XSetWindowAttributes attrib;
	unsigned long create_mask;	

	XColor xforeground_color, xbackground_color, xborder_color;

	Cursor curs;
	GC gc;
	GC select_gc;
	XFontStruct *font;
	XCharStruct overall;
	
	int direction;
	int ascent;
	int descent;
	int counter;
	
	bool bottom_edge;
	bool right_edge;
	
	unsigned int item_width,item_height;
	
	unsigned int xres, yres;
	
	// Used to know which item to paint.
	BaseMenuItem *curr;
	
	// This is for our synthetic enter event and we only want this to happen once.
	// This is set to true once we detect the mouse has entered the item window.
	bool enterOnce;	

public:		
	BaseMenu();
	virtual ~BaseMenu();

	inline list<BaseMenuItem*> getMenuItemList() const { return mi; }
	inline int getItemCount() const { return mi.size(); }
	inline bool isVisible() const { return is_visible; }
					
	void setMenuPos(int x, int y);
	void show();
	void show(int x, int y);
	void showSub(BaseMenu *sub, int x, int y);
	void hide(BaseMenu *sub);
	void hideAllVisibleSubmenus();
	
	void updateMenu();

	virtual void insert(std::string n, BaseMenu *sub);
	virtual void insert(std::string n, std::string exec, int func);
	virtual void insert(std::string n, std::string exec, BaseMenu *sub, int func);
	virtual void insert(BaseMenuItem *item);

	int remove(BaseMenuItem *element);
	void removeAll();

	BaseMenuItem *findMenuItem(int x, int y);

	virtual void handleButtonPressEvent(XButtonEvent *e);
	virtual void handleButtonReleaseEvent(XButtonEvent *e);
	void handleEnterNotify(XCrossingEvent *e);
	void handleLeaveNotify(XCrossingEvent *e);
	void handleExposeEvent(XExposeEvent *e);
	void handleMotionNotifyEvent(XMotionEvent *e);

	// The menu item behavoir is defined with these
	// virtual functions in a derived class.
	virtual void handleButton1Press(BaseMenuItem *curr){} 
	virtual void handleButton2Press(BaseMenuItem *curr){} 
	virtual void handleButton3Press(BaseMenuItem *curr){}
	virtual void handleButton1Release(BaseMenuItem *curr){}
	virtual void handleButton2Release(BaseMenuItem *curr){}
	virtual void handleButton3Release(BaseMenuItem *curr){}

	void execute(std::string s);

	inline Window getMenuWindow() 	    const { return item_window; }

	virtual void redraw();
	void hide();

private:
	void initializeMenu(Display *display);

	virtual void redraw(BaseMenuItem *m);
		
	void selectMenuItem(int col);
	void getMousePosition(int *x, int *y);
	void testMenuEdgeDetect(BaseMenu *sub);		

	// Not meant to be called directly by subclasses! Used internally.
	void hideSubmenus(); 
	
	void setForeground (int index);
	void draw3DLine(Window win, int x1, int y1, int x2, int y2);
};

#endif
