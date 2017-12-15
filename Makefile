CC       = g++ 
CFLAGS   = -g -O2 -Wall # -fpermissive Usar esta opcion si da error en versiones 3.4.X o superiores 

prefix   = /usr
INCLUDES = -I$(prefix)/include
LDPATH   = -L/usr/X11R6/lib
LIBS     = -lXext -lX11

# SHAPE = Shape Extension
# NEED_SETENV = Does your Unix not support the setenv function? Use this!
DEFINES  = -DSHAPE #-DNEED_SETENV 
HEADERS  = xaewm.hh 		\
	   client.hh 		\
	   windowmanager.hh  	\
   	   basemenu.hh  	\
	   windowmenu.hh	\
	   genericmenu.hh 	\
	   iconmenu.hh  	

OBJS     = windowmanager.o 	\
	   client.o 		\
	   main.o 		\
	   basemenu.o   	\
	   genericmenu.o	\
	   iconmenu.o   	\
	   windowmenu.o 	

all: xaewm

xaewm: $(OBJS)
	$(CC) $(OBJS) $(LDPATH) $(LIBS) -o $@

$(OBJS): %.o: %.cc $(HEADERS)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

install: all
	mkdir -p $(DESTDIR)$(prefix)/bin
	mkdir -p $(DESTDIR)$(prefix)/man/man1
	install -s xaewm $(DESTDIR)$(prefix)/bin
	install -m 644 xaewm.1x  $(DESTDIR)$(prefix)/man/man1
	mkdir -p /usr/share/xaewm
	cp ./xaewmenu /usr/share/xaewm
	cp ./xaewmedit /usr/share/xaewm
	cp ./menu.data /usr/share/xaewm
	
clean:
	rm -f xaewm $(OBJS) core
