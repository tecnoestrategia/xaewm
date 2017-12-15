/*
 * windowmenu.cc
 * Copyleft 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 * Modified by xento figal http://xinfo.sf.net
 */
 
#include "xaewm.hh"

WindowMenu::WindowMenu() : GenericMenu()
{
	updateWindowMenu();
}

void WindowMenu::updateWindowMenu()
{
	char* temp = new char[wm->getMaxDesktops()];
	for(int i=0; i<wm->getMaxDesktops(); i++)
	{
		sprintf(temp, "%d", i);
		
		insert(temp,"", SEND_TO_DESKTOP);
	}		
	delete [] temp;

	updateMenu();
	
	addToMenuList(this);
}

void WindowMenu::handleButtonReleaseEvent(XButtonEvent *e)
{
	int desktop=0;
	
	GenericMenu::handleButtonReleaseEvent(e);

	switch (e->button) 
	{
		case Button1:
			if (curr) 
			{
				switch(	curr->function )
				{
					case SEND_TO_DESKTOP:
						desktop = atoi(curr->name.c_str());
						if(client) client->setDesktop(desktop);
						hideAllVisibleSubmenus();
					break;
				
				}										
			}	
		break;
	} 
}
