#include "MyErrorsHandlers.h"

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#if defined(DARWIN)
#include <sys/wait.h>
#else
#include <wait.h>
#endif

#include "wbx.h"
#include "erbi.h"
#include "py.h"

#if defined(DARWIN)
#define SIGUNUSED 32
#endif

XErrorHandler   oldXErrorHandler;

extern BYTE     iWBChanged;
extern BYTE	iEBChanged;
extern BYTE     iNewPYPhraseCount;
extern BYTE     iOrderCount;
extern BYTE     iNewFreqCount;
extern BYTE	iWBOrderChanged;

void SetMyExceptionHandler (void)
{
    int             signo;

    for (signo = SIGHUP; signo < SIGUNUSED; signo++)
	signal (signo, OnException);
}

void OnException (int signo)
{
    fprintf (stderr, "\nFCITX -- Get Signal No.: %d\n", signo);
	
    if (iWBChanged || iWBOrderChanged)
	SaveWubiDict ();
    if (iEBChanged)
	SaveErbiDict ();
    if (iNewPYPhraseCount)
	SavePYUserPhrase ();
    if (iOrderCount)
	SavePYIndex ();
    if (iNewFreqCount)
	SavePYFreq ();

    if (signo != SIGCHLD && signo != SIGQUIT && signo != SIGWINCH) {
	fprintf (stderr, "FCITX -- Exit Signal No.: %d\n\n", signo);
	exit (0);
    }
}

void SetMyXErrorHandler (void)
{
    oldXErrorHandler = XSetErrorHandler (MyXErrorHandler);
}

int MyXErrorHandler (Display * dpy, XErrorEvent * event)
{
    char            str[1025];

    if (iWBChanged)
	SaveWubiDict ();
	if (iEBChanged)
	SaveErbiDict ();
    if (iNewPYPhraseCount)
	SavePYUserPhrase ();
    if (iOrderCount)
	SavePYIndex ();
    if (iNewFreqCount)
	SavePYFreq ();

    XGetErrorText (dpy, event->error_code, str, 1024);
    fprintf (stderr, "fcitx: %s\n", str);

    if (event->error_code != 3 && event->error_code != BadMatch)	// xterm will generate 3
	oldXErrorHandler (dpy, event);

    return 0;
}
