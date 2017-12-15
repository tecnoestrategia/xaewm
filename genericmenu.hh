/*
 * genericmenu.hh
 * Copyleft 2002 Frank Hale
 * frankhale@yahoo.com
 * http://sapphire.sourceforge.net/
 */

class GenericMenu : public BaseMenu
{
public:
	list<BaseMenu*> menuList;
		
public:
	GenericMenu();
	virtual ~GenericMenu();

	BaseMenu* findMenu(Window w);
	
	void addToMenuList(BaseMenu* m) { menuList.push_back(m); }
};
