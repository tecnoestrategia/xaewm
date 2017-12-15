# XAEWM
Minimalist window manager for X-window writen in C++, with a little menu writen in TCL & TK for the front-end on you can put your prefer applications made easy, very light and fast.

It's based on aewm++ from Frank Hale <frankhale@yahoo.com> and 9Wm by David Hogan and the original Decklin foster proyect <decklin@red-bean.com> in C (1998).

#### This's a VERY OLD PROJECT (2003) Rescue for my personal interest in not losing one of my first public projects. 
### Installation

xaewm requires X11 to run.

Make sure you have C++ compiler 

```sh
$ tar xvzf xaewm.x.tar.gz
$ cd xaewm.x
$ make
$ make install
$ make clear
```
Edit your ~/.xinitrc and add:
    exec xaewm
  
### Uninstall
```sh
$ rm -rf /bin/xaewm.x
$ rm -rf /man/man1/xaewm.1x
$ rm -rf /usr/share/xaewm
```
### Usage
Simply press the right mouse button and you can see the little menu, in this menu you can edit (add or remove) some entrys like this: 

*%name in menu*
*command*
