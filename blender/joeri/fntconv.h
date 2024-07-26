#ifndef DEFFNT
#define DEFFNT

struct FontChar
{
	char uv[2];
	char wh[2];
	char cc[2];
};
struct PsxFont
{
	short rectsize[2];
	short unit[2];
	short curpos[2];
	
	struct FontChar character[128];
};

struct BBSprite
{
	short  xy[2];
	short  grow;
	
	char uv[2];
	char wh[2];
	char cc[2];
	
	ulong *rect;
};

struct SGIFont
{
	short rectsize[2];
	short unit[2];
	short curpos[2];
	
	struct BBSprite *letter[128];
};


extern set_character(struct PsxFont *font, short x, short y, char ch);
extern struct PsxFont *set_font(char *name);



#endif /*  DEFFNT  */
