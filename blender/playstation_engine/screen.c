/**
 * $Id:$
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * The contents of this file may be used under the terms of either the GNU
 * General Public License Version 2 or later (the "GPL", see
 * http://www.gnu.org/licenses/gpl.html ), or the Blender License 1.0 or
 * later (the "BL", see http://www.blender.org/BL/ ) which has to be
 * bought from the Blender Foundation to become active, in which case the
 * above mentioned GPL option does not apply.
 *
 * The Original Code is Copyright (C) 1997 by Ton Roosendaal, Frank van Beek and Joeri Kassenaar.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

/*

 *					 screen.c
 *					 MY FIRST SONY!
 *
 *					 - init graphics
 *					 - screen_examin
 *
 */

#include "psxdef.h"
#include "psxgraph.h"

DB db[2];
DB *cdb= db;

int sw_time;

void frontbuffer(int val)
{
	DB *rt;
	char isbg;

	if(val) {
		if(cdb==db) rt= db+1;
		else rt= db;
		
		// automatische clear afzetten
		isbg= rt->draw.isbg;
		rt->draw.isbg= 0;
		PutDrawEnv(&rt->draw); /* update  environment!!! */
		rt->draw.isbg= isbg;
	}
	else {
		PutDrawEnv(&cdb->draw); /* update  environment */	
	}

}	

void viewmove()
{
	DISPENV mem;
	ulong padd;
	short dx,dy;
	DB *rt;
	// single buffer: dispenv opschuiven

	
	if(cdb==db) rt= db;
	else rt= db+1;

	mem= (rt->disp);

	while(1)  {
		
		padd = PadRead(1);
		if(padd & (PADk)) break;
		
		dx=dy= 0;
		if (padd & PADLleft) dx= -4;
		if (padd & PADLright) dx= 4;

		if (padd & PADLup) dy= -2;
		if (padd & PADLdown) dy= 2;
		
		if(dx || dy)  {

			rt->disp.disp.x+= dx;
			CLAMP(rt->disp.disp.x,0,704);
			rt->disp.disp.y+= dy;
			CLAMP(rt->disp.disp.y,0,256);
			
			PutDispEnv(&rt->disp); /* update display environment */
			VSync(0);
		}
		
	}
	
	(rt->disp)= mem;
	PutDispEnv(&rt->disp); /* update display environment */	
	
}	

void init_display()
{
	/* altijd doublebuffer */
	DB *db0, *db1;
	
	/* reset graphic system */
	PadInit(0);			/* reset PAD */
	ResetGraph(0);		/* reset graphic subsystem (0:cold,1:warm) */
	SetGraphDebug(0);	/* set debug mode (0:off, 1:monitor, 2:dump) */
	InitGeom();			/* initialize geometry subsystem */
 	SetDispMask(1);		/* enable to display (0:inhibit, 1:enable) */

	SetRCnt(RCntCNT1,0xffff,RCntMdINTR|RCntMdFR);
	StartRCnt(RCntCNT1);

	db0= &db[0];
	db1= &db[1];

   	if(HIRES && LACE && PAL) 
	{	
		SetDefDrawEnv(&db0->draw, 0, 0, 640, 512);
		SetDefDrawEnv(&db1->draw, 0, 0, 640, 512);
		SetDefDispEnv(&db0->disp, 0, 0, 640, 512);
		SetDefDispEnv(&db1->disp, 0, 0, 640, 512);

		db0->disp.isinter= db1->disp.isinter= 1;
		SetGeomOffset(320, 256);

		fb_add(  0,   0,  640, 256, "Screen1");
		fb_add(  0, 256,  640, 256, "Screen2");

	}		
   	else if(HIRES && PAL) 
	{	
		SetDefDrawEnv(&db0->draw, 0, 0, 512, 256);
		SetDefDrawEnv(&db1->draw, 512, 0, 512, 256);
		SetDefDispEnv(&db0->disp, 512, 0, 512, 256);
		SetDefDispEnv(&db1->disp, 0, 0, 512, 256);

		db0->disp.isinter= db1->disp.isinter= 0;
		SetGeomOffset(256, 128);

		fb_add(  0,   0, 512, 256, "Screen1");
		fb_add(  512,   0, 512, 256, "Screen2");
	}		
	else if(PAL) 
	{	
		SetDefDrawEnv(&db0->draw, 0,   0, 320, 256);
		SetDefDrawEnv(&db1->draw, 320, 0, 320, 256);
		SetDefDispEnv(&db0->disp, 320, 0, 320, 256);
		SetDefDispEnv(&db1->disp, 0, 0,   320, 256);

		db0->disp.isinter= db1->disp.isinter= 0;
		SetGeomOffset(160, 128); 

		fb_add(  0,   0, 320, 256, "Screen1");
		fb_add(  320, 0, 320, 256, "Screen2");

	}		
	
	if(PAL) 
	{
		SetVideoMode(MODE_PAL);
		db0->disp.screen.x= db1->disp.screen.x= 0;
		db0->disp.screen.y= db1->disp.screen.y= 16;
		db0->disp.screen.h= db1->disp.screen.h= 256;
		db0->disp.screen.w= db1->disp.screen.w= 256;
	}
	
	db0->draw.dtd= db1->draw.dtd= 1;	/* dither flag */
	db0->draw.isbg= db1->draw.isbg= 1;	/* clear drawing env */
	db0->draw.r0= 0;
	db1->draw.r0= 0;
	
	PutDrawEnv(&db0->draw); /* update drawing environment */
	PutDispEnv(&db0->disp); /* update display environment */
	
	db0->pbuf= malloc(PBUFSIZE);
	db1->pbuf= malloc(PBUFSIZE);
	db0->dbuf= malloc(DBUFSIZE);
	db1->dbuf= malloc(DBUFSIZE);

	db0->maxpbuf= db0->pbuf + PBUFSIZE;
	db1->maxpbuf= db1->pbuf + PBUFSIZE;
	db0->curpbuf= db0->pbuf;
	db1->curpbuf= db1->pbuf;

	db0->maxdbuf= db0->dbuf + DBUFSIZE-16*sizeof(POLY_GT4);
	db1->maxdbuf= db1->dbuf + DBUFSIZE-16*sizeof(POLY_GT4);
	db0->curdbuf= db0->dbuf;
	db1->curdbuf= db1->dbuf;

	db0->ot= malloc(4*OTSIZE);
	db1->ot= malloc(4*OTSIZE);

	ClearOTagR(db0->ot, OTSIZE);	// clear ordering table
	ClearOTagR(db1->ot, OTSIZE);	// clear ordering table

	cdb= db0;
}	

void end_display()
{
	DB *db0, *db1;
	RECT rt;

	db0= &db[0];
	db1= &db[1];
	if(db0->pbuf) freeN(db0->pbuf); db0->pbuf= 0;
	if(db1->pbuf) freeN(db1->pbuf); db1->pbuf= 0;
	if(db0->dbuf) freeN(db0->dbuf); db0->dbuf= 0;
	if(db1->dbuf) freeN(db1->dbuf); db1->dbuf= 0;
	
	if(db0->ot) freeN(db0->ot);	db0->ot= 0;
	if(db1->ot) freeN(db1->ot);	db1->ot= 0;

	PadStop();
	ResetGraph(3);
	StopCallback();
		
}	

void init_ride_display()
{
	/* altijd doublebuffer */
	DB *db0, *db1;
	
	/* reset graphic system */
	PadInit(0);			/* reset PAD */
	ResetGraph(0);		/* reset graphic subsystem (0:cold,1:warm) */
	SetGraphDebug(0);	/* set debug mode (0:off, 1:monitor, 2:dump) */
	InitGeom();			/* initialize geometry subsystem */
 	SetDispMask(1);		/* enable to display (0:inhibit, 1:enable) */

	SetRCnt(RCntCNT1,0xffff,RCntMdINTR|RCntMdFR);
	StartRCnt(RCntCNT1);

	db0= &db[0];
	db1= &db[1];

   	SetDefDrawEnv(&db0->draw, 16,   	0, 	320, 256);
	SetDefDrawEnv(&db1->draw, 320+32, 	0, 	320, 256);
	SetDefDispEnv(&db0->disp, 320+32, 	0, 	320, 256);
	SetDefDispEnv(&db1->disp, 16, 		0,  320, 256);

	db0->disp.isinter= db1->disp.isinter= 0;
	SetGeomOffset(160, 128); 
			
	
	if(PAL) {
		SetVideoMode(MODE_PAL);
		db0->disp.screen.x= db1->disp.screen.x= 0;
		db0->disp.screen.y= db1->disp.screen.y= 16;
		db0->disp.screen.h= db1->disp.screen.h= 256;
		db0->disp.screen.w= db1->disp.screen.w= 256;
	}
	
	db0->draw.dtd= db1->draw.dtd= 1;	/* dither flag */
	db0->draw.isbg= db1->draw.isbg= 1;	/* clear drawing env */
	db0->draw.r0= 0;
	db1->draw.r0= 0;
	
	PutDispEnv(&db1->disp); /* update display environment */	
	PutDrawEnv(&db1->draw); /* update drawing environment */
	PutDrawEnv(&db0->draw); /* update drawing environment */
	PutDispEnv(&db0->disp); /* update display environment */
	
	db0->pbuf= mallocN(R_PBUFSIZE, "pbuf");
	db1->pbuf= mallocN(R_PBUFSIZE, "pbuf");
 
	db0->maxpbuf= db0->pbuf + R_PBUFSIZE;
	db1->maxpbuf= db1->pbuf + R_PBUFSIZE;
	db0->curpbuf= db0->pbuf;
	db1->curpbuf= db1->pbuf;

	db0->ot= mallocN(4*R_OTSIZE, "ot");
	db1->ot= mallocN(4*R_OTSIZE, "ot");
	
	ClearOTagR(db0->ot, R_OTSIZE);	// clear ordering table
	ClearOTagR(db1->ot, R_OTSIZE);	// clear ordering table

	cdb= db0;
}	


void set_swapclear(int val)
{
	if(val) {
		db[0].draw.isbg= 1;
		db[1].draw.isbg= 1;
	}
	else {
		db[0].draw.isbg= 0;
		db[1].draw.isbg= 0;
	}
}

int swapbuffers()
{
	int time;

	cdb = (cdb==db)? db+1: db;	/* swap double buffer ID */

	DrawSync(0);	/* wait for end of drawing */

	VSync(0);		/* wait for the next V-BLNK */

	PutDispEnv(&cdb->disp); /* update display environment */	
	PutDrawEnv(&cdb->draw); /* update drawing environment */

	sw_time= GetRCnt(RCntCNT1);
	ResetRCnt(RCntCNT1);

}	

int swapbuffers25Hz()		
{
	int time;

	cdb = (cdb==db)? db+1: db;	/* swap double buffer ID */

	DrawSync(0);	/* wait for end of drawing */

	VSync(0);		/* wait for the next V-BLNK */

	PutDispEnv(&cdb->disp); /* update display environment */	
	PutDrawEnv(&cdb->draw); /* update drawing environment */

	sw_time= GetRCnt(RCntCNT1);
	if(sw_time<400) {
		VSync(0);
	}

	ResetRCnt(RCntCNT1);

}	


ulong tempar[16][16];


void fade_to_black()
{
	POLY_F4 po;
	int a;
	short tp;

	SetPolyF4(&po);
	SetSemiTrans(&po, 1);

	setXYWH(&po, 0,0, 320, 256);
	setRGB0(&po, 16, 16, 16);
	
	tp= GetTPage(2, 2, 320, 0);
	
	db[0].draw.tpage= tp;
	db[1].draw.tpage= tp;
	
	frontbuffer(1);

	for(a=0; a<16; a++) {
		DrawSync(0);
		VSync(0);
		DrawPrim(&po);
	}
	
	frontbuffer(0);
}


