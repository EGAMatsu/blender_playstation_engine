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
 *  psxblend.h    juli 96
 * 
 *  GEEN MIXED TYPE!!!
 *  NIET VERWARREN MET BLENDPSX.H !!!
 * 
 */


#ifndef PSXBLEND_H
#define PSXBLEND_H


#include "psxdef.h"
#include "blendpsx.h"


/* ************ EXTERN ********** */

extern pGlobal G;
extern MATRIX matone;	// arithp.c

#define SELECT			1
#define ACTIVE			2


#define FIRSTBASE		G.scene->base.first
#define LASTBASE		G.scene->base.last
#define BASACT			(G.scene->basact)
#define OBACT			(BASACT? BASACT->object: 0)

#define BROW(min, max)	(((max)>=31? 0xFFFFFFFF: (1<<(max+1))-1) - ((min)? ((1<<(min))-1):0) )


/* ******************* FILES ******************** */

#define FORM MAKE_ID('F','O','R','M')
#define DDG1 MAKE_ID('3','D','G','1')
#define DDG2 MAKE_ID('3','D','G','2')
#define DDG3 MAKE_ID('3','D','G','3')
#define DDG4 MAKE_ID('3','D','G','4')

#define GOUR MAKE_ID('G','O','U','R')

#define BLEN MAKE_ID('B','L','E','N')
#define DER_ MAKE_ID('D','E','R','_')
#define DPSX MAKE_ID('D','P','S','X')
#define V100 MAKE_ID('V','1','0','0')

#define DATA MAKE_ID('A','T','A','D')
#define GLOB MAKE_ID('B','O','L','G')
#define IMAG MAKE_ID('G','A','M','I')

#define DNA1 MAKE_ID('1','A','N','D')
#define TEST MAKE_ID('T','S','E','T')
#define REND MAKE_ID('D','N','E','R')

#define ENDB MAKE_ID('B','D','N','E')


typedef struct BHead {
	int code, len;
	void *old;
	int SDNAnr, nr;
} BHead;


/* ************************************** */

/* ID */
#define ID_SCE	MAKE_ID2('S', 'C')
#define ID_LI	MAKE_ID2('L', 'I')
#define ID_OB	MAKE_ID2('O', 'B')
#define ID_ME	MAKE_ID2('M', 'E')
#define ID_MA	MAKE_ID2('M', 'A')
#define ID_CU	MAKE_ID2('C', 'U')
#define ID_IM	MAKE_ID2('I', 'M')
#define ID_IK	MAKE_ID2('I', 'K')
#define ID_WV	MAKE_ID2('W', 'V')
#define ID_LT	MAKE_ID2('L', 'T')
#define ID_SE	MAKE_ID2('S', 'E')
#define ID_LF	MAKE_ID2('L', 'F')
#define ID_LA	MAKE_ID2('L', 'A')
#define ID_CA	MAKE_ID2('C', 'A')
#define ID_IP	MAKE_ID2('I', 'P')
#define ID_KE	MAKE_ID2('K', 'E')
#define ID_WO	MAKE_ID2('W', 'O')
#define ID_ID	MAKE_ID2('I', 'D')


#define LIB_LOCAL		0
#define LIB_EXTERN		1
#define LIB_INDIRECT	2
#define LIB_TEST		8
#define LIB_TESTEXT		9
#define LIB_TESTIND		10
#define LIB_READ		16
#define LIB_NEEDLINK	32
#define LIB_MALLOC		64

#define LIB_NEW			256
#define LIB_FAKEUSER	512



/* **************** OBJECT ********************* */

/* type */
#define OB_EMPTY		0
#define OB_MESH			1
#define OB_CURVE		2
#define OB_SURF			3
#define OB_FONT			4
#define OB_MBALL		5

#define OB_LAMP			10
#define OB_CAMERA		11

#define OB_IKA			20
#define OB_WAVE			21
#define OB_LATTICE		22
#define OB_SECTOR		23
#define OB_LIFE			24

/***************** GLOBAL ************************* */

/* G.f */
#define G_TWINANIM	1
#define G_LOADFILE	2
#define G_RESTART	4
#define G_QUIT		8
#define G_NETWORK	16
#define G_PLAYMOVIE	32






#endif /* BLENDPSX_H */


