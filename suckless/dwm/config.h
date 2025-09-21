#include <X11/XF86keysym.h>
#include "fibonacci.c"
//#include "movestack.c"
/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int gappx     = 15;        /* gap pixel between windows */
static const unsigned int snap      = 0;       /* snap pixel */
static const int scalepreview       = 4;        /* preview scaling (display w and h / scalepreview) */
static const int previewbar         = 1;        /* show the bar in the preview window */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 0;        /* 0 means bottom bar */
static const int splitstatus        = 1;
static const char *splitdelim        = ";";
static const char *fonts[]          = {
	"CozetteCrossedSeven:size=14",
	"Noto Sans Gujarati:size=10", 
};
static const char dmenufont[]       = "CozetteCrossedSeven:size=14";
static const char col_black[]	    = "#ffffff";
static const char col_gray1[]       = "#000000";	/* #222222 */
static const char col_gray2[]       = "#000000";	/* #444444 */
static const char col_gray3[]       = "#ffffff";	/* #ffffff prior white */
static const char col_gray4[]       = "#ffffff";	/* #eeeeee */
static const char col_cyan[]        = "#000000";	/* #005577 - workspace highlight color */
static const char col_transparent[] = "#000000"; /* color for transparent titlebar background */
static const char col_black_text[]  = "#000000";
static const char col_white_text[]  = "#ffffff";
static const unsigned int baralpha = 0x00;       /* transparent bar background */
static const unsigned int borderalpha = OPAQUE;    /* transparent borders 0x00 */
static const unsigned int titlealpha = 0x00;     /* transparent titlebar for unselected windows */

static const char *colors[][3]      = {
	/*               fg         bg                border   */
	[SchemeNorm] = { col_black_text, col_transparent, col_white_text },  /* unselected windows - transparent bg */
	[SchemeSel]  = { col_white_text, col_cyan,        col_black_text },  /* selected window - black bg (workspace highlight) */
};

static const unsigned int alphas[][3]      = {
    /*               fg      bg           border*/
    [SchemeNorm] = { OPAQUE, titlealpha,  borderalpha }, /* transparent titlebar for unselected */
    [SchemeSel]  = { OPAQUE, OPAQUE,      borderalpha }, /* opaque black for selected workspace */
};

/* gaps */

/* tagging */
static const char *tags[] = { "૧", "૨", "૩", "૪", "૫", "૬", "૭", "૮", "૯" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            0,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 }, 
	{ "mpv",      NULL,       NULL,       0,            1,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function    []=tile ><>float */
	{ "[]=",      tile },    /* first entry is default, prior tile */
	{ "><>",      NULL },    /* no layout function means floating behavior */
/*	{ "[M]",      monocle }, */
/*        { "[@]",      spiral }, */
/*        { "[\\]",      dwindle }, */
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
//static const char *roficmd[] = { "rofi", "-show", "drun", NULL };
static const char *roficmd[] = { "rofi", "-show", "drun", "-theme", "/home/user/.config/rofi/styles/dmenu-2.rasi", NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *floorpcmd[] = {"floorp", "--new-window", NULL };
static const char *privatefloorpcmd[] = { "floorp", "--private-window", NULL };
static const char *thunarcmd[] = {"thunar", NULL };
static const char *playpausecmd[] = { "smart-toggle", "toggle", NULL };
static const char *nextcmd[] = { "smart-toggle", "next", NULL };
static const char *prevcmd[] = { "smart-toggle", "prev", NULL };
static const char *stopcmd[] = { "smart-toggle", "stop", NULL };
/* static const char *screenshot_full[] = { "sh", "-c", "maim | xclip -selection clipboard -t image/png", NULL }; */
/* static const char *screenshot_select[] = { "sh", "-c", "maim -s | xclip -selection clipboard -t image/png", NULL }; */

static const Key keys[] = {
	/* modifier                     key        function        argument */
/*	{ MODKEY|ShiftMask,             XK_d,      spawn,          {.v = dmenucmd } }, */
	{ MODKEY|ShiftMask, 		XK_d, 	   spawn, 	   {.v = roficmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_i,      spawn,          {.v = floorpcmd } },
	{ MODKEY|ShiftMask,		XK_x,	   spawn, 	   {.v = privatefloorpcmd } }, 
	{ MODKEY|ShiftMask,		XK_m,	   spawn,	   {.v = thunarcmd } },
	{ 0, XF86XK_AudioPlay,        spawn,          {.v = playpausecmd } },
    	{ 0, XF86XK_AudioPause,       spawn,          {.v = playpausecmd } },
    	{ 0, XF86XK_AudioNext,        spawn,          {.v = nextcmd } },
    	{ 0, XF86XK_AudioPrev,        spawn,          {.v = prevcmd } },
	{ 0, XF86XK_AudioStop,	      spawn,	      {.v = stopcmd } },
/*        { MODKEY,                       XK_b,      togglebar,      {0} }, */
	{ MODKEY|ShiftMask,             XK_b,      spawn,          {.v = (const char*[]){ "rofi-bluetooth", NULL } } },
	{ MODKEY|ShiftMask, 		XK_r, 	   spawn, 	   SHCMD("~/.local/bin/record_screen.sh") },
	{ MODKEY|ShiftMask,             XK_grave,  spawn,          SHCMD("~/.local/bin/mpd_control.sh") },
	{ MODKEY,                       XK_Left,   focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_Right,  focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	{ MODKEY|ShiftMask, 		XK_Left,   rotatestack,    {.i = +1} },  // rotate down
        { MODKEY|ShiftMask, 		XK_Right,  rotatestack,    {.i = -1} },  // rotate up
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
/*	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
*	{ MODKEY,                       XK_r,      setlayout,      {.v = &layouts[3]} },
*	{ MODKEY|ShiftMask,             XK_r,      setlayout,      {.v = &layouts[4]} },
*/
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },
/*	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
*	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
*/
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY,                       XK_backslash,  viewnext,       {0} },
	{ MODKEY,                       XK_slash,   viewprev,       {0} },
//	{ MODKEY|ShiftMask,             XK_backslash,  tagtonext,      {0} },
//	{ MODKEY|ShiftMask,             XK_BackSpace,   tagtoprev,      {0} },
	{ MODKEY,                       XK_minus,  setgaps,        {.i = -1 } },
	{ MODKEY,                       XK_equal,  setgaps,        {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_equal,  setgaps,        {.i = 0  } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_e,      spawn,           SHCMD("~/.local/bin/power-menu.sh") },
//	{ MODKEY|ShiftMask,		XK_e,	   quit,           {1) },
        { MODKEY,                       XK_bracketleft, scratchpad_show, {0} },
        { MODKEY|ShiftMask,             XK_bracketleft, scratchpad_hide, {0} },
        { MODKEY,                       XK_bracketright, scratchpad_remove,{0} },
	{ MODKEY|ShiftMask,             XK_s,      spawn,          SHCMD("~/.local/bin/screenshot.sh select") },
	{ MODKEY|ShiftMask,             XK_p,  spawn,          SHCMD("~/.local/bin/screenshot.sh full") },
	{ MODKEY|ShiftMask,             XK_w,      spawn,          SHCMD("~/.local/bin/wifi-menu.sh") },
	{ 0,                       	XF86XK_AudioRaiseVolume,  spawn,      SHCMD("~/.local/bin/volume.sh up") },
        { 0,                       	XF86XK_AudioLowerVolume,  spawn,      SHCMD("~/.local/bin/volume.sh down") },
        { 0,                       	XF86XK_AudioMute,         spawn,      SHCMD("~/.local/bin/volume.sh mute") },
        { 0,                       	XF86XK_AudioMicMute,      spawn,      SHCMD("~/.local/bin/volume.sh micmute") },
        { 0,                       	XF86XK_MonBrightnessUp,   spawn,      SHCMD("~/.local/bin/brightness.sh up") },
        { 0,                       	XF86XK_MonBrightnessDown, spawn,      SHCMD("~/.local/bin/brightness.sh down") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

