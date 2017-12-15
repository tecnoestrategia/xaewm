/*
 * windowmenu.hh 
 * Copyleft 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 * Modified by xento figal http://xinfo.sf.net
 */
 
#ifndef _WINDOWMENU_HH
#define _WINDOWMENU_HH

#include "xaewm.hh"

class WindowMenu : public GenericMenu
{
private:
	Client *client;
	
public:
	WindowMenu();

	virtual void handleButtonReleaseEvent(XButtonEvent *e);

	void updateWindowMenu();
	
	void setThisClient(Client *c) { client = c; }
};

#endif 
