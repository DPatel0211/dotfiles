/* user and group to drop privileges to */
static const char *user  = "nobody";
static const char *group = "nobody";

static const char *colorname[NUMCOLS] = {
	[INIT] =   "black",     /* after initialization */
	[INPUT] =  "black",   /* during input */
	[FAILED] = "#CC3333",   /* wrong password */
};

static const char *lock_message = "";
static const char *wrong_message = "";
static const char *message_font = "Cozette:pixelsize=11:antialias=false:autohint=false";

/* treat a cleared input like a wrong password (color) */
static const int failonclear = 1;

/*Enable blur*/
#define BLUR
/*Set blur radius*/
static const int blurRadius=5;

/*Enable dimming*/
#define DIM
/*Set dim factor (0-255, higher = more dim)*/
static const int dimAlpha = 24; // 90 was prior
/*Enable Pixelation*/
//#define PIXELATION
/*Set pixelation radius*/
static const int pixelSize=0;

/*Font settings for the time text*/
//static const float textsize=30;
//static const char* textfamily="JetBrains Maple Mono";
//static const double textcolorred=255; // prior 255 for all three
//static const double textcolorgreen=255;
//static const double textcolorblue=255;

// Time display settings
static const char *time_font = "JetBrains Maple Mono:style=Bold:size=65";
static const char *time_color = "#000000";
//static const char *time_format = "%H:%M";
static const char *time_format = "%I:%M %p";
static const int time_y_off = 0; // pixels relative to center

// Date display settings
static const char *date_font = "JetBrains Maple Mono:style=Regular:size=25";
static const char *date_color = "#000000";
//static const char *date_format = "%a %d.%m.%Y";
static const int date_y_off = 47; // pixels relative to center

// Refresh intervals (in seconds)
static const int tw_refr_int = 10;   // thread_wrapper() interval. Should be the lowest one.
static const int tm_refr_int = 10;   // interval for draw_time()
static const int dt_refr_int = 3600; // interval for draw_date()
