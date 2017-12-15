/*
 * iconmenu.hh
 * Copyleft 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 * Modified by xento figal http://xinfo.sf.net
 */
 
#ifndef _ICONMENU_HH
#define _ICONMENU_HH

#include "xaewm.hh"

class IconMenu : public GenericMenu
{
private:
	Client *client;

public:
	IconMenu();

	virtual void handleButtonReleaseEvent(XButtonEvent *e);

	void setThisClient(Client *c) { client = c; }

	void addThisClient(Client *c);
	void removeClientFromIconMenu(Client *c);
};

#endif 
