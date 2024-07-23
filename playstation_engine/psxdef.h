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
 *  psxdef.h    juli 96
 * 
 *  alle defines en standaard includes
 * 
 */


#ifndef PSXDEF_H
#define PSXDEF_H


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include <libetc.h>
#include <kernel.h>

#include "psxutil.h"
#include <libgte.h>		// blendpsx.h moet de MATRIX struct hebben
#include <libgpu.h>
#include <libgs.h>
#include "../joeri/button.h"
#include "../joeri/psxfont.h"
#include "../rob/psxsound.h"




/* **************** ALGEMEEN ********************* */

#define VECCOPY(v1,v2) 		{*(v1)= *(v2); *(v1+1)= *(v2+1); *(v1+2)= *(v2+2);}
#define QUATCOPY(v1,v2) 	{*(v1)= *(v2); *(v1+1)= *(v2+1); *(v1+2)= *(v2+2); *(v1+3)= *(v2+3);}

#define S10MUL(a, b)		( ((a)*(b))>>10 )
#define S12MUL(a, b)		( ((a)*(b))>>12 )
#define SMUL(a, b)			( ((a)*(b))>>12 )
#define S14MUL(a, b)		( ((a)*(b))>>14 )
#define S16MUL(a, b)		( ((a)*(b))>>16 )

#define INPR(v1, v2)		( (v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2] )
#define CLAMP(a, b, c)		if((a)<(b)) (a)=(b); else if((a)>(c)) (a)=(c)
#define CLAMPIS(a, b, c)	((a)<(b) ? (b) : (a)>(c) ? (c) : (a))
#define CLAMPTEST(a, b, c)	if((b)<(c)) {CLAMP(a, b, c);} else {CLAMP(a, c, b);}

#define	FLT_EPSILON	1.19209290E-07
#define IS_EQ(a,b) ((fabs((double)(a)-(b)) >= (double) FLT_EPSILON) ? 0 : 1)

#define INIT_MINMAX(min, max) (min)[0]= (min)[1]= (min)[2]= 1.0e30; (max)[0]= (max)[1]= (max)[2]= -1.0e30;
#define DO_MINMAX(vec, min, max) if( (min)[0]>(vec)[0] ) (min)[0]= (vec)[0]; \
							  if( (min)[1]>(vec)[1] ) (min)[1]= (vec)[1]; \
							  if( (min)[2]>(vec)[2] ) (min)[2]= (vec)[2]; \
							  if( (max)[0]<(vec)[0] ) (max)[0]= (vec)[0]; \
							  if( (max)[1]<(vec)[1] ) (max)[1]= (vec)[1]; \
							  if( (max)[2]<(vec)[2] ) (max)[2]= (vec)[2]; \

#define DO_MINMAX2(vec, min, max) if( (min)[0]>(vec)[0] ) (min)[0]= (vec)[0]; \
							  if( (min)[1]>(vec)[1] ) (min)[1]= (vec)[1]; \
							  if( (max)[0]<(vec)[0] ) (max)[0]= (vec)[0]; \
							  if( (max)[1]<(vec)[1] ) (max)[1]= (vec)[1];

#define MINSIZE(val, size)	( ((val)>=0.0) ? (((val)<(size)) ? (size): (val)) : ( ((val)>(-size)) ? (-size) : (val)))

#define BTST(a,b)	( ( (a) & 1<<(b) )!=0 )
#define BCLR(a,b)	( (a) & ~(1<<(b)) )
#define BSET(a,b)	( (a) | 1<<(b) )

#define MAKE_ID2(c, d)		( (c)<<8 | (d) )


#define ISPOIN(a, b, c)			( (a->b) && (a->c) )
#define ISPOIN3(a, b, c, d)		( (a->b) && (a->c) && (a->d) )
#define ISPOIN4(a, b, c, d, e)	( (a->b) && (a->c) && (a->d) && (a->e) )

#define LONGCOPY(a, b, c)	{int lcpc=c, *lcpa=(int *)a, *lcpb=(int *)b; while(lcpc-->0) *(lcpa++)= *(lcpb++);}



#endif /* PSXDEF_H */

