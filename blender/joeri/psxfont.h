#ifndef DEFFNT
#define DEFFNT


#define F_SOLID		0
#define F_MIX		1
#define F_ADD		2
#define F_SUB		3



struct FontChar{
	char uv[2];
	char wh[2];
	char cc[2];
	char dm[2];
};
struct PsxFont
{
	short rectsize[2];
	short unit[2];
	short curpos[2];
	
	struct FontChar character[128];
	
	short addtpage, subtpage;
	short clut, tint[3];
	char  proportional, dummy;
	short spacing;
	
	short fb_allocpage, drmode, mixtpage;
	short rt[7];
};

struct SGIBBSprite
{
	short  xy[2];
	short  grow;
	
	char uv[2];
	char wh[2];
	char cc[2];
	char dm[2];
	
	unsigned long *rect;
};

struct BBSprite
{
	short  xy[2];
	short  grow;
	
	char uv[2];
	char wh[2];
	char cc[2];
	char dm[2];
	
	unsigned long *rect;
};

struct SGIFont
{
	short rectsize[2];
	short unit[2];
	short curpos[2];
	
	struct BBSprite *letter[128];
};

extern void set_character(struct PsxFont *font, short x, short y, char ch);
extern struct PsxFont *set_font(char *name, int x, int y);
extern void psxprint_ot(struct PsxFont *font, short x, short y, char *txt, unsigned long *ot, char **primbuf);


struct BBSprite *load_sprite(char *name);

#endif /*  DEFFNT  */
