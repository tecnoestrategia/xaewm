/*
 * iconmenu.cc
 * Copyleft 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 * Modified by xento figal http://xinfo.sf.net
 */
 
#include "xaewm.hh"

IconMenu::IconMenu() : GenericMenu()
{
	updateMenu();
	addToMenuList(this);
}

void IconMenu::handleButtonReleaseEvent(XButtonEvent *e)
{
	GenericMenu::handleButtonReleaseEvent(e);

	switch (e->button) 
	{
		case Button1:
			if (curr) 
			{
				switch(	curr->function )
				{
					case UNICONIFY:
						curr->client->unhide();
						hideAllVisibleSubmenus();
					break;
				
				}										
			}	
		break;
	} 
}

void IconMenu::addThisClient(Client *c)
{
	if(c)
	{
		BaseMenuItem *item = new BaseMenuItem();

		item->client = c;
		item->name=c->getClientIconName(); // name showing on menu.
		item->function=UNICONIFY;
	
		insert(item);
	}
}

void IconMenu::removeClientFromIconMenu(Client *c)
{
	list<BaseMenuItem*> menuItemList = getMenuItemList();
	list<BaseMenuItem*>::iterator mit;
	
	for(mit = menuItemList.begin(); mit != menuItemList.end(); mit++)
	{
		if(*mit)
		{
			if((*mit)->client == c) 
			{
				remove(*mit);
				delete *mit;
				curr=NULL;
				break;
			}
		}
	}
}
