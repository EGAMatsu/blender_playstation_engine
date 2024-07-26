/*
 * $PSLibId: Runtime Library Version 3.3$
 */
/*			tuto3: simplest sample
 *
 *	
 *		Copyright (C) 1993 by Sony Corporation
 *			All rights Reserved
 */


#include <sys/types.h>
#include <sys/file.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpress.h>
#include "movie.h"
#include <ctype.h>
#include <kernel.h>
#include <libcd.h>

static DISPENV	Disp[2];			/* display area */

cls(uchar r, uchar g, uchar b)
{
	RECT	rect;
	
	/* clear image */
	rect.w = 1024;
	rect.h = 512;
	rect.x = 0;
	rect.y = 0;
	ClearImage(&rect, r, g, b);
}

init_graph(int width, int height, int use24)
{
	int bpp;
 	 	
	if (use24) bpp = 3;
	else bpp = 2;
	
	SetVideoMode(MODE_PAL);
	ResetGraph(0);		/* reset graphic subsystem (0:cold,1:warm) */
	SetGraphDebug(0);	/* set debug mode (0:off, 1:monitor, 2:dump) */
	SetDispMask(1);		/* enable to display (0:inhibit, 1:enable) */
	
	cls(0, 0, 0);

	if (height <= 256) {
		height = 256;
		SetDefDispEnv(&Disp[0], 0,				0,		320, height);
		SetDefDispEnv(&Disp[1], 0, 		   		256,	320, height);
	} else {
		SetDefDispEnv(&Disp[0], 0,				0,		320, height);
		SetDefDispEnv(&Disp[1], 320 * bpp / 2, 	0,		320, height);
	}
		
	
	Disp[0].screen.h = Disp[0].disp.h;
 	Disp[1].screen.h = Disp[1].disp.h;
	Disp[1].screen.y = Disp[0].screen.y = (288 - height) / 2;

	Disp[0].screen.w = 256;
	Disp[1].screen.w = 256;
	
	Disp[0].isrgb24 = use24;
	Disp[1].isrgb24 = use24;
	
	Disp[0].isinter = 1;
	Disp[1].isinter = 1;
	
	PutDispEnv(&Disp[0]);
}

main2(int argc, char ** argv)
{ 		
	uchar result[8];
	
	ResetCallback();

	
	// dit loopt vast op de pc
	// printf("%d\n", open("cdrom:/system.cnf", O_RDONLY));

	
	init_graph(320, 256, 0);	/* reset graphic system */
		
	sleep(5);
	cls(255, 255, 255);
	sleep(5);


 	CdInit();
	CdSetDebug(0);

	if (CdControl(CdlNop, 0, result)) cls(0, 255, 0);
	else cls(255, 0, 0);
	
	return(0);
}

