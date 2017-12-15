/*
 * xaewm genericmenu.cc by xento figal http://xinfo.sf.net
 * based in code by 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 */
 
#include "xaewm.hh"

GenericMenu::GenericMenu() : BaseMenu()
{
}

GenericMenu::~GenericMenu()
{
	menuList.clear();
}

BaseMenu* GenericMenu::findMenu(Window w)
{
	if (w && w != DefaultRootWindow(dpy)) {
		if(menuList.size()) 
		{
			list<BaseMenu*>::iterator menu_it;

			for(menu_it = menuList.begin(); menu_it != menuList.end(); menu_it++)
			{
				if (w == (*menu_it)->getMenuWindow())
				{
					return (*menu_it);
				}				
			}
		}
	}

	return NULL;	
}
