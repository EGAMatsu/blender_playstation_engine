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


	psxgraphics.h

*/



#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>


#define SCR_Z		(256)		/* screen depth (h) */
#define OTLEN		12			/* ordering table bit width */
#define	OTSIZE		(1<<OTLEN)	/* ordering table size */
#define	PBUFSIZE	(105000)	/* doen: test waarom deze bij overflow bugt (Rough translation: Do: Test why this one causes overflow bug)*/
#define	DBUFSIZE	(35000)	

#define R_OTSIZE	(32)		/* ride otsize */
#define R_PBUFSIZE	(32000)		/* ride buffers */

#define	PAL	1
#define HIRES 0
#define LACE 0
#define XSCR	(HIRES?512:320)
#define YSCR	(256<<LACE)


typedef struct SVECT {
	short x,y,z;
} SVECT;

typedef struct {		
	DRAWENV		draw;			/* drawing environment */
	DISPENV		disp;			/* display environment */
	u_long		*ot;			/* ordering table */
	short 		dr_time, ca_time, ga_time, rtime;
	char		moviedone, rt;
	short		rtt;
	char 		*pbuf;			/* voor doublebuf polys */
	char 		*maxpbuf, *curpbuf;
	char 		*dbuf;			/* voor subdiv */
	char 		*maxdbuf, *curdbuf;
} DB;


extern DB db[2], *cdb; 			/* screen.c */





/* restantjes:

#define myRotAverageNclip4( a, b, c, d,  e, f, g, h,  p, otz, flag)  RotAverageNclip4( (SVECTOR *)(a), (SVECTOR *)(b), (SVECTOR *)(c), (SVECTOR *)(d), (long *)(e), (long *)(f), (long *)(g), (long *)(h), p, otz, flag)

#define myRotAverageNclip3( a, b, c,  e, f, g,  p, otz, flag)    	 RotAverageNclip3( (SVECTOR *)(a), (SVECTOR *)(b), (SVECTOR *)(c), (long *)(e), (long *)(f), (long *)(g), p, otz, flag)

#define myClip4GP(a,b,c,d, e, f, g, h, evmx)					Clip4GP(a,b,c,d, (CVECTOR *)e, (CVECTOR *)f, (CVECTOR *)g, (CVECTOR *)h, evmx)

// niet net, maar voor snelheid: (kan bugs geven als EVECTOR verandert)
typedef struct EVECT {
	SVECTOR v;
	VECTOR sxyz;
	long sxy;
	long rgb;
	short txuv, pad;
	long chx, chy;
} EVECT;

*/





