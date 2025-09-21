/* See LICENSE file for license details. */
#define _XOPEN_SOURCE 500
#if HAVE_SHADOW_H
#include <shadow.h>
#endif

#include <ctype.h>
#include <crypt.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Imlib2.h>
#include <X11/Xft/Xft.h>
#include "arg.h"
#include "util.h"
#include <pthread.h>

static XftFont *timefont = NULL;
static XftFont *datefont = NULL;
static pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;


char *argv0;

enum {
	INIT,
	INPUT,
	FAILED,
	NUMCOLS
};

struct lock {
	int screen;
	Window root, win;
	Pixmap pmap;
	Pixmap bgmap;
	unsigned long colors[NUMCOLS];
	XftDraw *xftdraw;
	int w, h;
	XftColor tmcol, dtcol;
	GC gc;
};

struct xrandr {
	int active;
	int evbase;
	int errbase;
};

#include "config.h"

Imlib_Image image;

/* Function declarations */
static void die(const char *errstr, ...);
static const char *gethash(void);
static void readpw(Display *dpy, struct xrandr *rr, struct lock **locks, int nscreens, const char *hash);
static struct lock *lockscreen(Display *dpy, struct xrandr *rr, int screen);
static void usage(void);
static void drawmessage(Display *dpy, struct lock *lock, const char *message);
static void drawindicator(Display *dpy, struct lock *lock, int len);
#ifdef __linux__
static void dontkillme(void);
#endif

static void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

#ifdef __linux__
#include <fcntl.h>
#include <linux/oom.h>

static void
dontkillme(void)
{
	FILE *f;
	const char oomfile[] = "/proc/self/oom_score_adj";

	if (!(f = fopen(oomfile, "w"))) {
		if (errno == ENOENT)
			return;
		die("slock: fopen %s: %s\n", oomfile, strerror(errno));
	}
	fprintf(f, "%d", OOM_SCORE_ADJ_MIN);
	if (fclose(f)) {
		if (errno == EACCES)
			die("slock: unable to disable OOM killer. "
			    "Make sure to suid or sgid slock.\n");
		else
			die("slock: fclose %s: %s\n", oomfile, strerror(errno));
	}
}
#endif

static const char *
gethash(void)
{
	const char *hash;
	struct passwd *pw;

	/* Check if the current user has a password entry */
	errno = 0;
	if (!(pw = getpwuid(getuid()))) {
		if (errno)
			die("slock: getpwuid: %s\n", strerror(errno));
		else
			die("slock: cannot retrieve password entry\n");
	}
	hash = pw->pw_passwd;

#if HAVE_SHADOW_H
	if (!strcmp(hash, "x")) {
		struct spwd *sp;
		if (!(sp = getspnam(pw->pw_name)))
			die("slock: getspnam: cannot retrieve shadow entry. "
			    "Make sure to suid or sgid slock.\n");
		hash = sp->sp_pwdp;
	}
#else
	if (!strcmp(hash, "*")) {
#ifdef __OpenBSD__
		if (!(pw = getpwuid_shadow(getuid())))
			die("slock: getpwnam_shadow: cannot retrieve shadow entry. "
			    "Make sure to suid or sgid slock.\n");
		hash = pw->pw_passwd;
#else
		die("slock: getpwuid: cannot retrieve shadow entry. "
		    "Make sure to suid or sgid slock.\n");
#endif /* __OpenBSD__ */
	}
#endif /* HAVE_SHADOW_H */

	return hash;
}

static void
drawmessage(Display *dpy, struct lock *lock, const char *message)
{
	int len, x, y;
	int dir, asc, desc;
	XCharStruct overall;
	XFontStruct *font;
	
	if (!message || !*message)
		return;
		
	/* Load font - you can customize this font name */
	font = XLoadQueryFont(dpy, message_font);
	if (!font) {
		/* Fallback to fixed font if custom font fails */
		font = XLoadQueryFont(dpy, "fixed");
		if (!font)
			return;
	}
	
	len = strlen(message);
	XTextExtents(font, message, len, &dir, &asc, &desc, &overall);
	
	/* Center the text */
	x = (DisplayWidth(dpy, lock->screen) - overall.width) / 2;
	y = (DisplayHeight(dpy, lock->screen) + asc - desc) / 2;
	
	/* Set font and color for drawing */
	XSetFont(dpy, lock->gc, font->fid);
	XSetForeground(dpy, lock->gc, lock->colors[INPUT]); /* Use INPUT color for text */
	
	/* Draw the message */
	XDrawString(dpy, lock->win, lock->gc, x, y, message, len);
	
	XFreeFont(dpy, font);
}

static void
drawindicator(Display *dpy, struct lock *lock, int len)
{
	int y, i;
	// int indicator_width = 300;  /* Remove this - not needed */
	int indicator_height = 20;  /* Height of indicator */
	// int dot_size = 12;       /* Remove if not used */
	int dot_spacing = 16;       /* Space between dots */
	int max_dots = 18;          /* Maximum dots to show */
	
	/* Calculate position - center the dots directly */
	int dots_to_show = len > max_dots ? max_dots : len;
	int total_dots_width = dots_to_show * dot_spacing;
	int start_x = (DisplayWidth(dpy, lock->screen) - total_dots_width) / 2;
	y = (DisplayHeight(dpy, lock->screen)) / 2 + 150;  /* Adjust offset as needed */
	
	/* Set color for indicator */
	XSetForeground(dpy, lock->gc, lock->colors[INPUT]);
	
	/* Draw dots for each character typed */
	//XFontStruct *font = XLoadQueryFont(dpy, message_font);
	XFontStruct *font = XLoadQueryFont(dpy, "fixed");
	if (font) {
		XSetFont(dpy, lock->gc, font->fid);
		for (i = 0; i < dots_to_show; i++) {
			int char_x = start_x + (i * dot_spacing);
			int char_y = y + (indicator_height + font->ascent) / 2;
			
			/* Draw asterisk for each character */
			XDrawString(dpy, lock->win, lock->gc, char_x, char_y, "*", 1);
		}
		XFreeFont(dpy, font);
	}
	
	/* If more characters than max_dots, show "..." */
	if (len > max_dots) {
		int ellipsis_x = start_x + (max_dots * dot_spacing) - 20;
		int ellipsis_y = y + indicator_height / 2;
		
		XFontStruct *font = XLoadQueryFont(dpy, "fixed");
		if (font) {
			XSetFont(dpy, lock->gc, font->fid);
			XDrawString(dpy, lock->win, lock->gc, ellipsis_x, ellipsis_y, "...", 3);
			XFreeFont(dpy, font);
		}
	}
}

static void draw_time(Display *dpy, struct lock *lock) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestr[64];
    strftime(timestr, sizeof(timestr), time_format, tm_info);

    XGlyphInfo ext;
    XftTextExtentsUtf8(dpy, timefont, (FcChar8*)timestr, strlen(timestr), &ext);
    int time_x = (lock->w - ext.xOff) / 2;
    int time_y = lock->h/2 + time_y_off;
  // foobarski  XClearArea(dpy, lock->win, time_x, time_y - timefont->ascent, ext.xOff,
  //             timefont->ascent + timefont->descent, False);
    XftDrawStringUtf8(lock->xftdraw, &lock->tmcol, timefont, time_x, time_y,
                      (FcChar8*)timestr, strlen(timestr));
    XFlush(dpy);
}

static void draw_date(Display *dpy, struct lock *lock) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char datestr[64];
    char daystr[32];
    
    // Get the day number
    int day = tm_info->tm_mday;
    
    // Determine the ordinal suffix
    const char *suffix;
    if (day >= 11 && day <= 13) {
        suffix = "th";
    } else {
        switch (day % 10) {
            case 1: suffix = "st"; break;
            case 2: suffix = "nd"; break;
            case 3: suffix = "rd"; break;
            default: suffix = "th"; break;
        }
    }
    
    // Format: "Tuesday, September 2nd, 2025"
    strftime(daystr, sizeof(daystr), "%A, %B", tm_info);
    snprintf(datestr, sizeof(datestr), "%s %d%s, %d", 
             daystr, day, suffix, tm_info->tm_year + 1900);

    XGlyphInfo ext;
    XftTextExtentsUtf8(dpy, datefont, (FcChar8*)datestr, strlen(datestr), &ext);
    int date_x = (lock->w - ext.xOff) / 2;
    int date_y = lock->h/2 + date_y_off;
    // Removed XClearArea as discussed
    XftDrawStringUtf8(lock->xftdraw, &lock->dtcol, datefont, date_x, date_y,
                      (FcChar8*)datestr, strlen(datestr));
    XFlush(dpy);
}

struct thw_args {
        Display *dpy;
        int nscreens;
        struct lock **lock;
};

static void* thread_wrapper(void* arg) {
    struct thw_args *args = (struct thw_args*)arg;
    long long tm_tmr = 0, dt_tmr = 0;
    while (1) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < args->nscreens; ++i) {
            if (tm_tmr <= 0) {
                draw_time(args->dpy, args->lock[i]);
                tm_tmr = tm_refr_int;
            };
            if (dt_tmr <= 0) {
                draw_date(args->dpy, args->lock[i]);
                dt_tmr = dt_refr_int;
            };
        };
        pthread_mutex_unlock(&mutex);
        sleep(tw_refr_int);
        tm_tmr -= tw_refr_int;
        dt_tmr -= tw_refr_int;
    };

    return NULL;
}

static void
readpw(Display *dpy, struct xrandr *rr, struct lock **locks, int nscreens,
       const char *hash)
{
	XRRScreenChangeNotifyEvent *rre;
	char buf[32], passwd[256], *inputhash;
	int num, screen, running, failure, oldc;
	unsigned int len, color;
	KeySym ksym;
	XEvent ev;

	len = 0;
	running = 1;
	failure = 0;
	oldc = INIT;

	while (running && !XNextEvent(dpy, &ev)) {
		if (ev.type == KeyPress) {
			explicit_bzero(&buf, sizeof(buf));
			num = XLookupString(&ev.xkey, buf, sizeof(buf), &ksym, 0);
			if (IsKeypadKey(ksym)) {
				if (ksym == XK_KP_Enter)
					ksym = XK_Return;
				else if (ksym >= XK_KP_0 && ksym <= XK_KP_9)
					ksym = (ksym - XK_KP_0) + XK_0;
			}
			if (IsFunctionKey(ksym) ||
			    IsKeypadKey(ksym) ||
			    IsMiscFunctionKey(ksym) ||
			    IsPFKey(ksym) ||
			    IsPrivateKeypadKey(ksym))
				continue;
			switch (ksym) {
			case XK_Return:
				passwd[len] = '\0';
				errno = 0;
				if (!(inputhash = crypt(passwd, hash)))
					fprintf(stderr, "slock: crypt: %s\n", strerror(errno));
				else
					running = !!strcmp(inputhash, hash);
				if (running) {
					XBell(dpy, 100);
					failure = 1;
					/* Show wrong password message immediately */
					pthread_mutex_lock(&mutex);
					for (screen = 0; screen < nscreens; screen++) {
						if(locks[screen]->bgmap) {
							XSetWindowBackgroundPixmap(dpy, locks[screen]->win, locks[screen]->bgmap);
						} else {
							XSetWindowBackground(dpy, locks[screen]->win, locks[screen]->colors[FAILED]);
						}
						XClearWindow(dpy, locks[screen]->win);
						/* Draw time and date even during error */
						draw_time(dpy, locks[screen]);
						draw_date(dpy, locks[screen]);
						/* Temporarily change text color to FAILED and show wrong message */
						XSetForeground(dpy, locks[screen]->gc, locks[screen]->colors[FAILED]);
						drawmessage(dpy, locks[screen], wrong_message);
					}
					pthread_mutex_unlock(&mutex);
					XSync(dpy, 0);
					usleep(2500000); /* Show error for 2.5 seconds */
				}
				explicit_bzero(&passwd, sizeof(passwd));
				len = 0;
				break;
			case XK_Escape:
				explicit_bzero(&passwd, sizeof(passwd));
				len = 0;
				break;
			case XK_BackSpace:
				if (len)
					passwd[--len] = '\0';
				break;
			default:
				if (num && !iscntrl((int)buf[0]) &&
				    (len + num < sizeof(passwd))) {
					memcpy(passwd + len, buf, num);
					len += num;
				}
				break;
			}
			color = len ? INPUT : ((failure || failonclear) ? FAILED : INIT);
			if (running && oldc != color) {
				pthread_mutex_lock(&mutex);
				for (screen = 0; screen < nscreens; screen++) {
					if(locks[screen]->bgmap) {
						XSetWindowBackgroundPixmap(dpy, locks[screen]->win, locks[screen]->bgmap);
					} else {
						XSetWindowBackground(dpy, locks[screen]->win, locks[screen]->colors[0]);
					}
					XClearWindow(dpy, locks[screen]->win);
					/* Redraw time and date after screen cleared */
					draw_time(dpy, locks[screen]);
					draw_date(dpy, locks[screen]);
					drawmessage(dpy, locks[screen], lock_message);
					/* Draw typing indicator */
					if (len > 0) {
						drawindicator(dpy, locks[screen], len);
					}
					XFlush(dpy);
				}
				pthread_mutex_unlock(&mutex);
				oldc = color;
			} else if (running) {
				/* Update indicator even if color hasn't changed */
				pthread_mutex_lock(&mutex);
				for (screen = 0; screen < nscreens; screen++) {
					if(locks[screen]->bgmap) {
						XSetWindowBackgroundPixmap(dpy, locks[screen]->win, locks[screen]->bgmap);
					} else {
						XSetWindowBackground(dpy, locks[screen]->win, locks[screen]->colors[0]);
					}
					XClearWindow(dpy, locks[screen]->win);
					/* IMPORTANT: Redraw time and date here too! */
					draw_time(dpy, locks[screen]);
					draw_date(dpy, locks[screen]);
					drawmessage(dpy, locks[screen], lock_message);
					if (len > 0) {
						drawindicator(dpy, locks[screen], len);
					}
					XFlush(dpy);
				}
				pthread_mutex_unlock(&mutex);
			}
		} else if (rr->active && ev.type == rr->evbase + RRScreenChangeNotify) {
			rre = (XRRScreenChangeNotifyEvent*)&ev;
			pthread_mutex_lock(&mutex);
			for (screen = 0; screen < nscreens; screen++) {
				if (locks[screen]->win == rre->window) {
					if (rre->rotation == RR_Rotate_90 ||
					    rre->rotation == RR_Rotate_270)
						XResizeWindow(dpy, locks[screen]->win,
						              rre->height, rre->width);
					else
						XResizeWindow(dpy, locks[screen]->win,
						              rre->width, rre->height);
					XClearWindow(dpy, locks[screen]->win);
					/* Redraw everything after screen change */
					draw_time(dpy, locks[screen]);
					draw_date(dpy, locks[screen]);
					drawmessage(dpy, locks[screen], lock_message);
					if (len > 0) {
						drawindicator(dpy, locks[screen], len);
					}
					break;
				}
			}
			pthread_mutex_unlock(&mutex);
		} else {
			for (screen = 0; screen < nscreens; screen++)
				XRaiseWindow(dpy, locks[screen]->win);
		}
	}
}

static struct lock *
lockscreen(Display *dpy, struct xrandr *rr, int screen)
{
	char curs[] = {0, 0, 0, 0, 0, 0, 0, 0};
	int i, ptgrab, kbgrab;
	struct lock *lock;
	XColor color, dummy;
	XSetWindowAttributes wa;
	Cursor invisible;

	if (dpy == NULL || screen < 0 || !(lock = malloc(sizeof(struct lock))))
		return NULL;

	lock->screen = screen;
	lock->root = RootWindow(dpy, lock->screen);

	if(image) {
		lock->bgmap = XCreatePixmap(dpy, lock->root, DisplayWidth(dpy, lock->screen), DisplayHeight(dpy, lock->screen), DefaultDepth(dpy, lock->screen));
		imlib_context_set_image(image);
		imlib_context_set_display(dpy);
		imlib_context_set_visual(DefaultVisual(dpy, lock->screen));
		imlib_context_set_colormap(DefaultColormap(dpy, lock->screen));
		imlib_context_set_drawable(lock->bgmap);
		imlib_render_image_on_drawable(0, 0);
		imlib_free_image();
	} else {
		lock->bgmap = 0;
	}

	for (i = 0; i < NUMCOLS; i++) {
		XAllocNamedColor(dpy, DefaultColormap(dpy, lock->screen),
		                 colorname[i], &color, &dummy);
		lock->colors[i] = color.pixel;
	}

	/* Create graphics context for drawing text */
	lock->gc = XCreateGC(dpy, lock->root, 0, NULL);

	/* init */
	wa.override_redirect = 1;
	wa.background_pixel = lock->colors[INIT];
	lock->win = XCreateWindow(dpy, lock->root, 0, 0,
	                          DisplayWidth(dpy, lock->screen),
	                          DisplayHeight(dpy, lock->screen),
	                          0, DefaultDepth(dpy, lock->screen),
	                          CopyFromParent,
	                          DefaultVisual(dpy, lock->screen),
	                          CWOverrideRedirect | CWBackPixel, &wa);

	lock->xftdraw = XftDrawCreate(dpy, lock->win,
	                              DefaultVisual(dpy, lock->screen),
	                              DefaultColormap(dpy, lock->screen));
	if (!lock->xftdraw)
		return NULL;
	lock->w = DisplayWidth(dpy, lock->screen);
	lock->h = DisplayHeight(dpy, lock->screen);
	Colormap colormap = DefaultColormap(dpy, lock->screen);
	Visual *visual = DefaultVisual(dpy, lock->screen);
	if (!XftColorAllocName(dpy, visual, colormap, time_color, &lock->tmcol))
		return NULL;
	if (!XftColorAllocName(dpy, visual, colormap, date_color, &lock->dtcol))
		return NULL;

	if(lock->bgmap) {
		XSetWindowBackgroundPixmap(dpy, lock->win, lock->bgmap);
	}
	lock->pmap = XCreateBitmapFromData(dpy, lock->win, curs, 8, 8);
	invisible = XCreatePixmapCursor(dpy, lock->pmap, lock->pmap,
	                                &color, &color, 0, 0);
	XDefineCursor(dpy, lock->win, invisible);

	/* Try to grab mouse pointer *and* keyboard for 600ms, else fail the lock */
	for (i = 0, ptgrab = kbgrab = -1; i < 6; i++) {
		if (ptgrab != GrabSuccess) {
			ptgrab = XGrabPointer(dpy, lock->root, False,
			                      ButtonPressMask | ButtonReleaseMask |
			                      PointerMotionMask, GrabModeAsync,
			                      GrabModeAsync, None, invisible, CurrentTime);
		}
		if (kbgrab != GrabSuccess) {
			kbgrab = XGrabKeyboard(dpy, lock->root, True,
			                       GrabModeAsync, GrabModeAsync, CurrentTime);
		}

		/* input is grabbed: we can lock the screen */
		if (ptgrab == GrabSuccess && kbgrab == GrabSuccess) {
			XMapRaised(dpy, lock->win);
			if (rr->active)
				XRRSelectInput(dpy, lock->win, RRScreenChangeNotifyMask);

			XSelectInput(dpy, lock->root, SubstructureNotifyMask);

			
			
			/* Draw the lock message, time and date */
			drawmessage(dpy, lock, lock_message);
			
			return lock;
		}

		/* retry on AlreadyGrabbed but fail on other errors */
		if ((ptgrab != AlreadyGrabbed && ptgrab != GrabSuccess) ||
		    (kbgrab != AlreadyGrabbed && kbgrab != GrabSuccess))
			break;

		usleep(100000);
	}

	/* we couldn't grab all input: fail out */
	if (ptgrab != GrabSuccess)
		fprintf(stderr, "slock: unable to grab mouse pointer for screen %d\n",
		        screen);
	if (kbgrab != GrabSuccess)
		fprintf(stderr, "slock: unable to grab keyboard for screen %d\n",
		        screen);
	return NULL;
}

static void
usage(void)
{
	die("usage: slock [-v] [cmd [arg ...]]\n");
}

int
main(int argc, char **argv) {
	struct xrandr rr;
	struct lock **locks;
	struct passwd *pwd;
	struct group *grp;
	uid_t duid;
	gid_t dgid;
	const char *hash;
	Display *dpy;
	int s, nlocks, nscreens;

	ARGBEGIN {
	case 'v':
		puts("slock-"VERSION);
		return 0;
	default:
		usage();
	} ARGEND

	/* validate drop-user and -group */
	errno = 0;
	if (!(pwd = getpwnam(user)))
		die("slock: getpwnam %s: %s\n", user,
		    errno ? strerror(errno) : "user entry not found");
	duid = pwd->pw_uid;
	errno = 0;
	if (!(grp = getgrnam(group)))
		die("slock: getgrnam %s: %s\n", group,
		    errno ? strerror(errno) : "group entry not found");
	dgid = grp->gr_gid;

#ifdef __linux__
	dontkillme();
#endif

	hash = gethash();
	errno = 0;
	if (!crypt("", hash))
		die("slock: crypt: %s\n", strerror(errno));

    XInitThreads();

	if (!(dpy = XOpenDisplay(NULL)))
		die("slock: cannot open display\n");

	/* drop privileges */
	if (setgroups(0, NULL) < 0)
		die("slock: setgroups: %s\n", strerror(errno));
	if (setgid(dgid) < 0)
		die("slock: setgid: %s\n", strerror(errno));
	if (setuid(duid) < 0)
		die("slock: setuid: %s\n", strerror(errno));

	/*Create screenshot Image*/
	Screen *scr = ScreenOfDisplay(dpy, DefaultScreen(dpy));
	image = imlib_create_image(scr->width,scr->height);
	imlib_context_set_image(image);
	imlib_context_set_display(dpy);
	imlib_context_set_visual(DefaultVisual(dpy,0));
	imlib_context_set_drawable(RootWindow(dpy,XScreenNumberOfScreen(scr)));	
	imlib_copy_drawable_to_image(0,0,0,scr->width,scr->height,0,0,1);

#ifdef BLUR
	/*Blur function*/
	imlib_image_blur(blurRadius);
#endif // BLUR	

#ifdef DIM
	/*Dimming overlay*/
	imlib_context_set_color(0, 0, 0, dimAlpha); // Black with alpha
	imlib_image_fill_rectangle(0, 0, scr->width, scr->height);
#endif // DIM

#ifdef PIXELATION
	/*Pixelation*/
	int width = scr->width;
	int height = scr->height;
	
	for(int y = 0; y < height; y += pixelSize)
	{
		for(int x = 0; x < width; x += pixelSize)
		{
			int red = 0;
			int green = 0;
			int blue = 0;

			Imlib_Color pixel; 
			Imlib_Color* pp;
			pp = &pixel;
			for(int j = 0; j < pixelSize && j < height; j++)
			{
				for(int i = 0; i < pixelSize && i < width; i++)
				{
					imlib_image_query_pixel(x+i,y+j,pp);
					red += pixel.red;
					green += pixel.green;
					blue += pixel.blue;
				}
			}
			red /= (pixelSize*pixelSize);
			green /= (pixelSize*pixelSize);
			blue /= (pixelSize*pixelSize);
			imlib_context_set_color(red,green,blue,pixel.alpha);
			imlib_image_fill_rectangle(x,y,pixelSize,pixelSize);
			red = 0;
			green = 0;
			blue = 0;
		}
	}
#endif

	/* check for Xrandr support */
	rr.active = XRRQueryExtension(dpy, &rr.evbase, &rr.errbase);

	/* get number of screens in display "dpy" and blank them */
	nscreens = ScreenCount(dpy);
	if (!(locks = calloc(nscreens, sizeof(struct lock *))))
		die("slock: out of memory\n");
	for (nlocks = 0, s = 0; s < nscreens; s++) {
		if ((locks[s] = lockscreen(dpy, &rr, s)) != NULL)
			nlocks++;
		else
			break;
	}
	XSync(dpy, 0);

	/* did we manage to lock everything? */
	if (nlocks != nscreens)
		return 1;

	/* run post-lock command */
	if (argc > 0) {
		pid_t pid;
		extern char **environ;
		int err = posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ);
		if (err) {
			die("slock: failed to execute post-lock command: %s: %s\n",
			    argv[0], strerror(err));
		}
	}

    timefont = XftFontOpenName(dpy, DefaultScreen(dpy), time_font);
    datefont = XftFontOpenName(dpy, DefaultScreen(dpy), date_font);
    if (!timefont || !datefont) {
        fprintf(stderr, "slock: failed to load fonts\n");
        return 1;
    };

	/* everything is now blank. Wait for the correct password */
	pthread_t thread_id;
    struct thw_args args = {dpy, nscreens, locks};
	pthread_create(&thread_id, NULL, thread_wrapper, &args);

	readpw(dpy, &rr, locks, nscreens, hash);
	
	/* Cleanup */
    if (timefont) XftFontClose(dpy, timefont);
    if (datefont) XftFontClose(dpy, datefont);
    for (s = 0; s < nscreens; s++) {
        if (locks[s]->xftdraw)
            XftDrawDestroy(locks[s]->xftdraw);

        Colormap colormap = DefaultColormap(dpy, locks[s]->screen);
        Visual *visual = DefaultVisual(dpy, locks[s]->screen);
        XftColorFree(dpy, visual, colormap, &locks[s]->tmcol);
        XftColorFree(dpy, visual, colormap, &locks[s]->dtcol);
    }

	return 0;
}
