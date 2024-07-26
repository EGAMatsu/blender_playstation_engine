/*
 *	PSX FONTS
 *
 *	{ kruis button		< left button
 *  } vierkant button	> right button
 *  [ driehoek button   & up button
 *	] cirkel button		* down button
 *
 */
 
#include "psxutil.h"
#include "psxgraph.h"
#include "../joeri/psxfont.h"



char *PPBuffer[36]; 	// iets te groot: TON
int   PPBLine = 0 ;

struct PsxFont *set_font(char *name, int sx, int sy)
{
	RECT			clutrect;
	struct PsxFont  *font;
	int				i;
	int				file, size, fbp;
	ulong			*rect;
	ulong			clut[128];
	char            magic[16];

	font = mallocN(sizeof(struct PsxFont),"Font");
	if (!font) {
		printf("No memory for font '%s'.\n", name); 
		return(0);
	}
	
	file = open(name, 0);
	if (file == -1) {
		printf("Could not open '%s' for input\n", name); 
		return(0);
	}
	
	read(file, magic,12);
	magic[12]= 0;
	
	read(file, font, sizeof(struct PsxFont) );
	
	size = font->rectsize[0] * font->rectsize[1] / 2;
	
	rect = mallocN(size, "FontPicture");
	
	read(file, rect, size );
	read(file, clut, 32 );
	
	font->fb_allocpage = -1;
	
	if ((sx == 0) && (sy == 0)){
		fbp = fb_alloc(&sx, &sy, 256, font->rectsize[1]+2, "Font");
		if (fbp == -1){
			printf("Error in fb_alloc (no space in fb)\n");
			return(0);
		} 
		if (fbp == -2){
			printf("Error in fb_alloc (no blocks free)\n");
			return(0);
		} 
		if (fbp == -3){
			printf("Error in fb_alloc (size y to big)\n");
			return(0);
		} 
		font->fb_allocpage = fbp; 	
	}
	if (sy == 512) { sy = 510 - font->rectsize[1]; }
	if (sy ==  -1) { sy = 250 - font->rectsize[1]; }
	
	setRECT (&clutrect, sx, sy + font->rectsize[1] + 1, 16, 1);
	LoadImage (&clutrect, clut);
	font->clut  = GetClut  (sx, sy + font->rectsize[1] + 1);
	
	font->addtpage = LoadTPage (rect, 0, 1, sx, sy , font->rectsize[0], font->rectsize[1]);
	font->subtpage = GetTPage  (0, 2, sx, sy );
	font->mixtpage = GetTPage  (0, 0, sx, sy );
	
	font->proportional = 1;
	font->spacing      = 0; 
	font->drmode= F_ADD;
	
	if (sy > 255) sy -= 256;
	for (i=0;i<128;i++){
		font->character[i].uv[1] += sy;
	}
	
	freeN(rect);
	close(file);
	return(font);
}

int free_font(struct PsxFont *font)
{
	int i, j;
	
	if (font) {
		if (font->fb_allocpage > -1){
			fb_free(font->fb_allocpage);
			font->fb_allocpage = -1;	
		}
		freeN(font);
		font = 0;
	}
	for (i=0; i < PPBLine ; i++){
		freeN(PPBuffer[i]);
	}
	PPBLine = 0;
	return(0);	
}


int font_height(struct PsxFont *font)
{	 return (font->character['m'].wh[1] * 2); }

int strfontlen(struct PsxFont *font, char *txt)
{
	int x = 0, i, l;
	char ch;
	
	l = strlen(txt);
	for(i=0; i<l; i++) {
		ch = txt[i];
		x +=  font->character[ch].wh[0];
		x +=  font->character[ch].cc[0];
	}
	return(x);
}

void set_font_prop(struct PsxFont *font, char on_off)
{	font->proportional = on_off;	}

void set_font_space(struct PsxFont *font, short val)
{	font->spacing = val;        }

void set_font_tint(struct PsxFont *font, int r, int g, int b)
{	font->tint[0] = r; font->tint[1] = g; font->tint[2] = b;}


void psxprint(struct PsxFont *font, short x, short y, char *txt)
{
	RECT rect;
	SPRT sp;
	DR_MODE dr;
	int i,l, castline;
	char ch;
	
	SetSprt (&sp);
	
	if(font->drmode) SetSemiTrans(&sp, 1);
	
	setRECT(&rect, 0,0,255,255);
	
	if(font->drmode==F_SUB) SetDrawMode (&dr, 1, 1, font->subtpage, &rect);
	else if(font->drmode==F_ADD) SetDrawMode (&dr, 1, 1, font->addtpage, &rect);
	else SetDrawMode (&dr, 1, 1, font->mixtpage, &rect);

	DrawPrim(&dr);

	sp.clut = font->clut;
	sp.r0 = font->tint[0];
	sp.g0 = font->tint[1];
	sp.b0 = font->tint[2];
		
	castline = font->unit[0];
	
	l = strlen(txt);
	for(i=0; i<l; i++) {
		ch = txt[i];

		
		sp.x0 = x + font->character[ch].cc[0];
		sp.y0 = y - castline + font->character[ch].cc[1];
	
		sp.u0 = font->character[ch].uv[0];
		sp.v0 = font->character[ch].uv[1];
		sp.w  = font->character[ch].wh[0]; 
		sp.h  = font->character[ch].wh[1];
	  	
if(PadRead(1) & 1) {
	if(ch=='p') {
		PRINT4(d,d,d,d, sp.u0, sp.v0, sp.w, sp.h);
		PRINT2(d, d, font->addtpage, font->drmode);
	}
}
		DrawPrim(&sp);
		
		x += font->spacing;
		if (font->proportional == 1){
			x +=  font->character[ch].wh[0];
			x +=  font->character[ch].cc[0];
		}else{
			x +=  font->character['w'].wh[0];
		}
	}
}

void addprint(char *txt)
{
	if (PPBLine == 32) return;
	
	PPBuffer[PPBLine] = mallocN(strlen(txt)+1, "Line");
	strcpy(PPBuffer[PPBLine],txt);
	PPBLine++;
}

void flushprint(struct PsxFont *font, short x, short y)
{
	int h, i;
	
	h = font_height(font);
	
	for (i=0 ; i<PPBLine; i++){
		y += h;
		psxprint(font, x,y, PPBuffer[i] );
		freeN(PPBuffer[i]);
	}
	PPBLine = 0;
}


void psxprint_ot(struct PsxFont *font, short x, short y, char *txt, ulong *ot, char **primbuf)
{
	// wel van te voren testen of primbuf nog groot genoeg is
	RECT rect;		// wel nodig!!!
	SPRT sp, *spp;
	DR_MODE *dr;
	int i,l, castline;
	char ch;
		
	// sp is werkbuffertje
	SetSprt (&sp);
	
	if(font->drmode) SetSemiTrans(&sp, 1);

	sp.clut = font->clut;
	sp.r0 = font->tint[0];
	sp.g0 = font->tint[1];
	sp.b0 = font->tint[2];
		
	castline = font->unit[0];
	
	l = strlen(txt);
	for(i=0; i<l; i++) {
		ch = txt[i];
		
		sp.x0 = x + font->character[ch].cc[0];
		sp.y0 = y - castline + font->character[ch].cc[1];
	
		sp.u0 = font->character[ch].uv[0];
		sp.v0 = font->character[ch].uv[1];
		sp.w  = font->character[ch].wh[0]; 
		sp.h  = font->character[ch].wh[1];
	  	
		spp= (SPRT *)(*primbuf); (*primbuf) += sizeof(SPRT);
		*spp= sp;
		AddPrim(ot, spp);
		
		x += font->spacing;
		if (font->proportional == 1){
			x +=  font->character[ch].wh[0];
			x +=  font->character[ch].cc[0];
		}else{
			x +=  font->character['w'].wh[0];
		}
	}

	// OT is backwards!!
	setRECT(&rect, 0,0,255,255);
	dr= (DR_MODE *)(*primbuf); (*primbuf) += sizeof(DR_MODE);

	if(font->drmode==F_MIX) SetDrawMode (dr, 1, 1, font->mixtpage, &rect);
	else if(font->drmode==F_ADD) SetDrawMode (dr, 1, 1, font->addtpage, &rect);
	else SetDrawMode (dr, 1, 1, font->subtpage, &rect);

	AddPrim(ot, dr);
}
