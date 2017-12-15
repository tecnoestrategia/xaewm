/*
 * xaewm main.cc by Xento figal http://xinfo.sf.net
 * based in code by 2002 Frank Hale
 * frankhale@yahoo.com
 * 
 */
 
#include "xaewm.hh"

void forkExec(char *cmd)
{
    if(! (strlen(cmd)>0)) return;
    
    pid_t pid = fork();

    switch (pid) {
        case 0:
            execlp("/bin/sh", "sh", "-c", cmd, NULL);
	    cerr << "exec failed, cleaning up child" << endl;
            exit(1);
        case -1:
	    cerr << "can't fork" << endl;
    }
}

void sigHandler(int signal)
{
    switch (signal) {
        case SIGINT:
        case SIGTERM:
		wm->quitNicely(); 
	break;
	
        case SIGHUP:
		wm->restart(); 
	break;
	
	case SIGCHLD:
            wait(NULL); 
        break;
    }
}

int handleXError(Display *dpy, XErrorEvent *e)
{
    if (e->error_code == BadAccess && e->resourceid == wm->getRootWindow()) {
	cerr << "Imposible ejecutar xaewm ¿se esta ejecutando otro gestor de X?" << endl;
        exit(1);
    }
   
    return 0;
}

// Yeah, yeah I know this function is C code, but who cares!
//
// Some systems do not have setenv(). This one is modeled after 4.4 BSD, but
// is implemented in terms of portable primitives only: getenv(), putenv()
// and malloc(). It should therefore be safe to use on every UNIX system.
//  
// If clobber == 0, do not overwrite an existing variable.
// 
// Returns nonzero if memory allocation fails.
//
// Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
//
// setenv - update or insert environment (name,value) pair
#ifdef NEED_SETENV
int setenv(char *name, char *value, int clobber)
{
    char   *cp;

    if (clobber == 0 && getenv(name) != 0)
	return (0);
    if ((cp = (char *)malloc(strlen(name) + strlen(value) + 2)) == 0)
	return (1);
    sprintf(cp, "%s=%s", name, value);
    return (putenv(cp));
}
#endif


//
// START THE WINDOW MANAGER 
//
int main(int argc, char **argv)
{
	WindowManager aewm(argc, argv);
	return 0;
}
