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
 *					 drawmesh.c
 *
 */

#include "psxblend.h"
#include "psxgraph.h"

#define SCO3		sco, sco+1, sco+2
#define SCO4		sco, sco+1, sco+2, sco+3

#define COPY_SCO3	*((long *)&po->x0)= sco[0];*((long *)&po->x1)= sco[1];*((long *)&po->x2)= sco[2];
#define COPY_SCO4	*((long *)&po->x0)= sco[0];*((long *)&po->x1)= sco[1];*((long *)&po->x2)= sco[2];*((long *)&po->x3)= sco[3];

#define UV3			&po->u0, &po->u1, &po->u2
#define UV4			&po->u0, &po->u1, &po->u2, &po->u3
#define COL3		&po->r0, &po->r1, &po->r2
#define COL4		&po->r0, &po->r1, &po->r2, &po->r3

/*
#define PVERT3		mvert+pr->v1, mvert+pr->v2, mvert+pr->v3
#define PVERT4		mvert+pr->v1, mvert+pr->v2, mvert+pr->v3, mvert+pr->v4
*/
#define PVERT3 pr->v1, pr->v2, pr->v3			// Incorrect
#define PVERT4 pr->v1, pr->v2, pr->v3, pr->v4	// But makes the compiler happy.

#define GET_BF3		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_F3); *po= blackpf3;}
#define GET_BF4		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_F4); *po= blackpf4;}
#define GET_F3		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_F3); *po= pr->po;}
#define GET_F4		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_F4); *po= pr->po;}
#define GET_G3		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_G3); *po= pr->po;}
#define GET_G4		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_G4); *po= pr->po;}
#define GET_FT3		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_FT3); *po= pr->po;}
#define GET_FT4		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_FT4); *po= pr->po;}
#define GET_GT3		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_GT3); *po= pr->po;}
#define GET_GT4		{po= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(POLY_GT4); *po= pr->po;}

#define UNGET_FT3	{cdb->curpbuf-= sizeof(POLY_FT3);}
#define UNGET_FT4	{cdb->curpbuf-= sizeof(POLY_FT4);}
#define UNGET_GT3	{cdb->curpbuf-= sizeof(POLY_GT3);}
#define UNGET_GT4	{cdb->curpbuf-= sizeof(POLY_GT4);}

#define GET_DR		{dr= (void *)cdb->curpbuf; cdb->curpbuf+= sizeof(DR_TWIN); *dr= pr->dr;}

extern int old_sp;			// main01.c
extern short debuginfo;		// main01.c

int subdiv4= 1000, subdiv16= 640;
int tots4=0, tots16= 0;

DIVPOLYGON4 divpoly;
extern short tpagenumbers[32];	// readbpsx.c
POLY_F3	blackpf3;
POLY_F4	blackpf4;

void init_blackpoly()
{
	SetPolyF3(&blackpf3);
	SetPolyF4(&blackpf4);
	setRGB0(&blackpf3, 0,0,0);
	setRGB0(&blackpf4, 0,0,0);
}

void set_divbuffer()
{
	divpoly.ndiv= 2;
    divpoly.pih=XSCR;			/* horizontal resolution */
    divpoly.piv=YSCR;			/* vertical resolution */
}	


// return 0: niet afbeelden

int cliptest3(short *v1)
{

	if( v1[0]<0 && v1[2]<0 && v1[4]<0 ) return 0;
	if( v1[1]<0 && v1[3]<0 && v1[5]<0 ) return 0;

	if( v1[0]>XSCR && v1[2]>XSCR && v1[4]>XSCR) return 0;
	if( v1[1]>YSCR && v1[3]>YSCR && v1[5]>YSCR) return 0;

	return 1;
}

int cliptest3mask(short *v1)
{
	short ret1, ret2;
	
	ret1=ret2= 0;
	if( v1[0]<0) { v1[0]= 0; ret1++;} else if(v1[0]>=XSCR) {v1[0]= XSCR; ret2++;}
	if( v1[2]<0) { v1[2]= 0; ret1++;} else if(v1[2]>=XSCR) {v1[2]= XSCR; ret2++;}
	if( v1[4]<0) { v1[4]= 0; ret1++;} else if(v1[4]>=XSCR) {v1[4]= XSCR; ret2++;}
	if(ret1==3 || ret2==3) return 0;
	
	ret1=ret2= 0;
	if( v1[1]<0) { v1[1]= 0; ret1++;} else if(v1[1]>=YSCR) {v1[1]= YSCR-1; ret2++;}
	if( v1[3]<0) { v1[3]= 0; ret1++;} else if(v1[3]>=YSCR) {v1[3]= YSCR-1; ret2++;}
	if( v1[5]<0) { v1[5]= 0; ret1++;} else if(v1[5]>=YSCR) {v1[5]= YSCR-1; ret2++;}
	if(ret1==3 || ret2==3) return 0;

	return 1;
}


int cliptest4mask(short *v1)
{
	short ret1, ret2;
	
	ret1=ret2= 0;
	if( v1[0]<0) { v1[0]= 0; ret1++;} else if(v1[0]>=XSCR) {v1[0]= XSCR; ret2++;}
	if( v1[2]<0) { v1[2]= 0; ret1++;} else if(v1[2]>=XSCR) {v1[2]= XSCR; ret2++;}
	if( v1[4]<0) { v1[4]= 0; ret1++;} else if(v1[4]>=XSCR) {v1[4]= XSCR; ret2++;}
	if( v1[6]<0) { v1[6]= 0; ret1++;} else if(v1[6]>=XSCR) {v1[6]= XSCR; ret2++;}
	if(ret1==4 || ret2==4) return 0;
	
	ret1=ret2= 0;
	if( v1[1]<0) { v1[1]= 0; ret1++;} else if(v1[1]>=YSCR) {v1[1]= YSCR-1; ret2++;}
	if( v1[3]<0) { v1[3]= 0; ret1++;} else if(v1[3]>=YSCR) {v1[3]= YSCR-1; ret2++;}
	if( v1[5]<0) { v1[5]= 0; ret1++;} else if(v1[5]>=YSCR) {v1[5]= YSCR-1; ret2++;}
	if( v1[7]<0) { v1[7]= 0; ret1++;} else if(v1[7]>=YSCR) {v1[7]= YSCR-1; ret2++;}
	if(ret1==4 || ret2==4) return 0;

	return 1;
}

int cliptest4(short *v1)
{

	if( v1[0]<0 && v1[2]<0 && v1[4]<0 && v1[6]<0) return 0;
	if( v1[1]<0 && v1[3]<0 && v1[5]<0 && v1[7]<0) return 0;

	if( v1[0]>XSCR && v1[2]>XSCR && v1[4]>XSCR && v1[6]>XSCR) return 0;
	if( v1[1]>YSCR && v1[3]>YSCR && v1[5]>YSCR && v1[7]>YSCR) return 0;

	return 1;
}


int cliptest4o(short *v1, short *v2, short *v3, short *v4)
{

	if( v1[0]<0 && v2[0]<0 && v3[0]<0 && v4[0]<0) return 0;
	if( v1[1]<0 && v2[1]<0 && v3[1]<0 && v4[1]<0) return 0;

	if( v1[0]>XSCR && v2[0]>XSCR && v3[0]>XSCR && v4[0]>XSCR) return 0;
	if( v1[1]>YSCR && v2[1]>YSCR && v3[1]>YSCR && v4[1]>YSCR) return 0;

	return 1;
}

void draw_mesh(pObject *ob, pMesh *me, CVECTOR obcol)
{
	SVECTOR *mvert;
	DR_TWIN *dr;
	long b, p, flag, datalen, otz, sco[4];
	short packets, mist, frombuf;
	short *sp, type, nr_elems;
	
	if(me==0 || me->packetdata==0) return;
	
	packets= me->totpacket;
	sp= me->packetdata;


	mvert= (SVECTOR *)me->mvert;
	if(mvert==0) return;
	
	mist= G.scene->mist;

	G.totface+= me->totface;

	// voor doublebuf
	if(me->lastfra!=G.dfrao && me->lastfra!=G.dfra) {
		me->lastfra= G.dfra;
		frombuf= 1;
	} else {
		frombuf= 1;
	}

	if(frombuf) {
		if( cdb->curpbuf + me->polysize > cdb->maxpbuf ) {
			printf("pbuffer overflow cur %d size %d\n", cdb->curpbuf-cdb->pbuf, me->polysize);
			return;
		}
	}

	while(packets-- > 0)  {
	
		type= sp[0];
		nr_elems= sp[1];
		sp+= 2;

		switch(type)  {
/* FLAT */
		case P_OBF3:
			{
				PrimF3 *pr= (PrimF3 *)sp;
				short nr= nr_elems;
				while(nr-- > 0)  {
					pr->po.r0= obcol.r;
					pr->po.g0= obcol.g;
					pr->po.b0= obcol.b;
					pr++;
				}
			}
		case P_F3:
			{
				PrimF3 *pr= (PrimF3 *)sp;
				POLY_F3 *po;
				
				while(nr_elems-- > 0)  {
					
					b= RotAverageNclip3( PVERT3, SCO3, &p, &otz, &flag);

					if (b>0 && otz>0 && otz<OTSIZE) {
						
						if(frombuf) GET_F3
						else po= &pr->po;
						COPY_SCO3
						
						AddPrim(cdb->ot+otz, po);
					}
					pr++;
				}
			   	sp= (short *)pr;	
			}
	
			break;
		
		case P_OBF4: 
			{
				PrimF4 *pr= (PrimF4 *)sp;
				short nr= nr_elems;
				while(nr-- > 0)  {
					pr->po.r0= obcol.r;
					pr->po.g0= obcol.g;
					pr->po.b0= obcol.b;
					pr++;
				}
			}
		case P_F4: 
			{
				PrimF4 *pr= (PrimF4 *)sp;
				POLY_F4 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip4( PVERT4, SCO4, &p, &otz, &flag);

					if(b>0 && otz>0 && otz<OTSIZE)  {
						
						if( cliptest4((short *)sco) ) {
							// test op subdiv
							
							if(frombuf) GET_F4
							else po= &pr->po;
							COPY_SCO4
							
							if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
								if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
								else { tots4++; divpoly.ndiv= 1;}

								cdb->curdbuf= (char *)DivideF4(PVERT4, (CVECTOR *)&po->r0, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
							}
							else  {
								AddPrim(cdb->ot+otz, po);
							}
						}  
					}
					
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_LF3:
			{
				PrimLF3 *pr= (PrimLF3 *)sp;
				POLY_F3 *po;
				
				while(nr_elems-- > 0)  {
					
					b= RotAverageNclip3( PVERT3, SCO3, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE) {

						if(frombuf) GET_F3
						else po= &pr->po;
						COPY_SCO3

						if(mist) NormalColorDpq(&pr->no, &pr->c0, p, (CVECTOR *)&po->r0);
						else NormalColorCol(&pr->no, &pr->c0, (CVECTOR *)&po->r0);

						AddPrim(cdb->ot+otz, po);
					}
					
					pr++;
				}
			   	sp= (short *)pr;	
			}
	
			break;
		
		case P_LF4: 
			{
				PrimLF4 *pr= (PrimLF4 *)sp;
				POLY_F4 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip4( PVERT4, SCO4, &p, &otz, &flag);

					if(b>0 && otz>0 && otz<OTSIZE)  {
						
						if( cliptest4((short *)sco)) {

							if(frombuf) GET_F4
							else po= &pr->po;
							COPY_SCO4

							if(mist) NormalColorDpq(&pr->no, &pr->c0, p, (CVECTOR *)&po->r0);
							else NormalColorCol(&pr->no, &pr->c0, (CVECTOR *)&po->r0);

							// test op subdiv
							if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
								if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
								else { tots4++; divpoly.ndiv= 1;}

								cdb->curdbuf= (char *)DivideF4(PVERT4, (CVECTOR *)&po->r0, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
							}
							else  {
								AddPrim(cdb->ot+otz, po);
							}
						}  
					}
					
					pr++;
				}
			   	sp= (short *)pr;	
			}

/* GOUR */
		case P_OBG3: 
		case P_G3: 
			{
				PrimG3 *pr= (PrimG3 *)sp;
				POLY_G3 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip3( PVERT3, SCO3, &p, &otz, &flag);
								
					if (b>0 && otz>0 && otz<OTSIZE)  {

						if(frombuf) GET_G3
						else po= &pr->po;
						COPY_SCO3
						
						AddPrim(cdb->ot+otz, po);
					}
					
					pr++;
				}
				sp= (short *)pr;
			}
			break;
		case P_OBG4:
		case P_G4:
			{
				PrimG4 *pr= (PrimG4 *)sp;
				POLY_G4 *po;

				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip4( PVERT4, SCO4, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE && cliptest4((short *)sco))  {
						
						if(frombuf) GET_G4
						else po= &pr->po;
						COPY_SCO4

						//test op subdiv
						if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
							else { tots4++; divpoly.ndiv= 1;}

							cdb->curdbuf= (char *)DivideG4(PVERT4, COL4, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
						}
					}

					pr++;
				}
				sp= (short *)pr;
			}
			break;
		
		case P_LG3:
		case P_LPG3:
			{
				PrimLG3 *pr= (PrimLG3 *)sp;
				POLY_G3 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip3( PVERT3, SCO3, &p, &otz, &flag);
								
					if (b>0 && otz>0 && otz<OTSIZE)  {
						
						if(frombuf) GET_G3
						else po= &pr->po;
						COPY_SCO3

						if(mist) {
							if(type==P_LPG3) {
								NormalColorDpq( pr->v1+1, &pr->c0, p, (CVECTOR *)&po->r0);
								NormalColorDpq( pr->v2+1, &pr->c1, p, (CVECTOR *)&po->r1);
								NormalColorDpq( pr->v3+1, &pr->c2, p, (CVECTOR *)&po->r2);
							}
							else {
								NormalColorDpq(&pr->no, &pr->c0, p, (CVECTOR *)&po->r0);
								NormalColorDpq(&pr->no, &pr->c1, p, (CVECTOR *)&po->r1);
								NormalColorDpq(&pr->no, &pr->c2, p, (CVECTOR *)&po->r2);
							}
						}
						else {
							if(type==P_LPG3) {
								NormalColorCol( pr->v1+1, &pr->c0, (CVECTOR *)&po->r0);
								NormalColorCol( pr->v2+1, &pr->c1, (CVECTOR *)&po->r1);				
								NormalColorCol( pr->v3+1, &pr->c2, (CVECTOR *)&po->r2);				
							}
							else {
								NormalColorCol(&pr->no, &pr->c0, (CVECTOR *)&po->r0);				
								NormalColorCol(&pr->no, &pr->c1, (CVECTOR *)&po->r1);				
								NormalColorCol(&pr->no, &pr->c2, (CVECTOR *)&po->r2);				
							}
						}
						AddPrim(cdb->ot+otz, po);
					}
					
					pr++;
				}
				sp= (short *)pr;
			}
			break;
			
		case P_LG4:
		case P_LPG4:
			{
				PrimLG4 *pr= (PrimLG4 *)sp;
				POLY_G4 *po;

				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip4( PVERT4, SCO4, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE && cliptest4((short *)sco))  {

						if(frombuf) GET_G4
						else po= &pr->po;
						COPY_SCO4

						if(mist) {
							if(type==P_LPG4) {
								NormalColorDpq( pr->v1+1, &pr->c0, p, (CVECTOR *)&po->r0);
								NormalColorDpq( pr->v2+1, &pr->c1, p, (CVECTOR *)&po->r1);
								NormalColorDpq( pr->v3+1, &pr->c2, p, (CVECTOR *)&po->r2);
								NormalColorDpq( pr->v4+1, &pr->c3, p, (CVECTOR *)&po->r3);
							}
							else {
								NormalColorDpq(&pr->no, &pr->c0, p, (CVECTOR *)&po->r0);
								NormalColorDpq(&pr->no, &pr->c1, p, (CVECTOR *)&po->r1);
								NormalColorDpq(&pr->no, &pr->c2, p, (CVECTOR *)&po->r2);
								NormalColorDpq(&pr->no, &pr->c3, p, (CVECTOR *)&po->r3);
							}
						}
						else {
							if(type==P_LPG4) {
								NormalColorCol( pr->v1+1, &pr->c0, (CVECTOR *)&po->r0);
								NormalColorCol( pr->v2+1, &pr->c1, (CVECTOR *)&po->r1);				
								NormalColorCol( pr->v3+1, &pr->c2, (CVECTOR *)&po->r2);				
								NormalColorCol( pr->v4+1, &pr->c3, (CVECTOR *)&po->r3);
							}
							else {
								NormalColorCol(&pr->no, &pr->c0, (CVECTOR *)&po->r0);
								NormalColorCol(&pr->no, &pr->c1, (CVECTOR *)&po->r1);
								NormalColorCol(&pr->no, &pr->c2, (CVECTOR *)&po->r2);
								NormalColorCol(&pr->no, &pr->c3, (CVECTOR *)&po->r3);
							}
						}
						
						//test op subdiv
						if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
							else { tots4++; divpoly.ndiv= 1;}

							cdb->curdbuf= (char *)DivideG4(PVERT4, COL4, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
						}
					}

					pr++;
				}
				sp= (short *)pr;
			}
			break;
/* FLAT TEX */

		case P_OBFT3:
			{
				PrimFT3 *pr= (PrimFT3 *)sp;
				short nr= nr_elems;
				if(mist) {
					while(nr-- > 0)  {
						pr->c0.r= obcol.r;
						pr->c0.g= obcol.g;
						pr->c0.b= obcol.b;
						pr++;
					}
				}
				else {
					while(nr-- > 0)  {
						pr->po.r0= obcol.r;
						pr->po.g0= obcol.g;
						pr->po.b0= obcol.b;
						pr++;
					}
				}
			}
			
		case P_FT3:
			{
				PrimFT3 *pr= (PrimFT3 *)sp;
				POLY_FT3 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip3(PVERT3, SCO3, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE)  {

						if(frombuf) {
							GET_FT3
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO3
	
						if(mist) DpqColor(&pr->c0, p, (CVECTOR *)&po->r0);
						
						//test op subdiv
						if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
							else { tots4++; divpoly.ndiv= 1;}

							cdb->curdbuf= (char *)DivideFT3(PVERT3, UV3, (CVECTOR *)&po->r0, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
							AddPrim(cdb->ot+otz, dr);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}
					}
					
				   	pr++;

				}
			   	sp= (short *)pr;	
			}
			break;

		case P_OBFT4:
			{
				PrimFT4 *pr= (PrimFT4 *)sp;
				short nr= nr_elems;
				if(mist) {
					while(nr-- > 0)  {
						pr->c0.r= obcol.r;
						pr->c0.g= obcol.g;
						pr->c0.b= obcol.b;
						pr++;
					}
				}
				else {
					while(nr-- > 0)  {
						pr->po.r0= obcol.r;
						pr->po.g0= obcol.g;
						pr->po.b0= obcol.b;
						pr++;
					}
				}
			}
		case P_SHADOW:
		case P_FT4:
			{
				PrimFT4 *pr= (PrimFT4 *)sp;
				POLY_FT4 *po;
				
				if(type==P_SHADOW) {
					pLife *lf;
					MATRIX tempmat;
					VECTOR vec;

					if(ob->type!=OB_LIFE) break;
					lf= ob->data;
				
					vec.vx= ob->obmat.t[0];
					vec.vy= ob->obmat.t[1];
					vec.vz= ob->obmat.t[2]+lf->floorloc[2];
					ApplyMatrixLV(&G.viewmat, &vec, (VECTOR *)&tempmat.t);
				
					tempmat.t[0]+= G.viewmat.t[0];
					tempmat.t[1]+= G.viewmat.t[1];
					tempmat.t[2]+= G.viewmat.t[2];
				
					SetTransMatrix(&tempmat);
				
					SetMulMatrix( &G.viewmat, &ob->obmat);
				
				}
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip4(PVERT4, SCO4, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE && cliptest4((short *)sco))  {
						
						if(frombuf) {
							GET_FT4
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO4

						if(mist) DpqColor(&pr->c0, p, (CVECTOR *)&po->r0);

						if(type==P_SHADOW) {
							if(otz>128) otz -= 64;
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}
						//test op subdiv
						else if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
							else { tots4++; divpoly.ndiv= 1;}

							cdb->curdbuf= (char *)DivideFT4(PVERT4, UV4, (CVECTOR *)&po->r0, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
							AddPrim(cdb->ot+otz, dr);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}
					}

					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_LFT3:
			{
				PrimLFT3 *pr= (PrimLFT3 *)sp;
				POLY_FT3 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip3(PVERT3, SCO3, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE)  {

						if(frombuf) {
							GET_FT3
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO3

						if(mist) NormalColorDpq(&pr->no, &pr->c0, p, (CVECTOR *)&po->r0);
						else NormalColorCol(&pr->no, &pr->c0, (CVECTOR *)&po->r0);

						AddPrim(cdb->ot+otz, po);
						AddPrim(cdb->ot+otz, dr);
					}
					
				   	pr++;

				}
			   	sp= (short *)pr;	
			}
			break;

		case P_LFT4:
			{
				PrimLFT4 *pr= (PrimLFT4 *)sp;
				POLY_FT4 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip4(PVERT4, SCO4, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE && cliptest4((short *)sco))  {
						
						if(frombuf) {
							GET_FT4
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO4

						if(mist) NormalColorDpq(&pr->no, &pr->c0, p, (CVECTOR *)&po->r0);
						else NormalColorCol(&pr->no, &pr->c0, (CVECTOR *)&po->r0);

						//test op subdiv
						if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
							else { tots4++; divpoly.ndiv= 1;}

							cdb->curdbuf= (char *)DivideFT4(PVERT4, UV4, (CVECTOR *)&po->r0, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
							AddPrim(cdb->ot+otz, dr);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}
					}

					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

/* GOUR TEX */
		case P_2GT3:
			b= 1;
		case P_OBGT3:
		case P_GT3:
			{
				PrimGT3 *pr= (PrimGT3 *)sp;
				POLY_GT3 *po;
				
				while(nr_elems-- > 0)  {
				
					if(type==P_2GT3) otz= RotAverage3( PVERT3, SCO3, &p, &flag);
					else b= RotAverageNclip3( PVERT3, SCO3, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE && cliptest3((short *)sco))  {

						if(frombuf) {
							GET_GT3
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO3

						// DpqColor3 aanroepen met c0 als eerste is funest!? (code?)
						if(mist) DpqColor3(&pr->c2, &pr->c1, &pr->c0, p, (CVECTOR *)&po->r2, (CVECTOR *)&po->r1, (CVECTOR *)&po->r0);

						//test op subdiv
						if(b!=1 && otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) divpoly.ndiv= 2;
							else divpoly.ndiv= 1;

							cdb->curdbuf= (char *)DivideGT3(PVERT3, UV3, COL3, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
							// REVERSE OT!!!
							AddPrim(cdb->ot+otz, dr);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}
					}
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
			
		case P_2GT4:
			b= 1;
		case P_OBGT4:
		case P_GT4:
			{
				PrimGT4 *pr= (PrimGT4 *)sp;
				POLY_GT4 *po;
				
				while(nr_elems-- > 0)  {
				
					if(type==P_2GT4) otz= RotAverage4( PVERT4, SCO4, &p, &flag);
					else b= RotAverageNclip4( PVERT4, SCO4, &p, &otz, &flag);

					if (b>0 && otz>0 && otz<OTSIZE && cliptest4((short *)sco))  {
						
						if(frombuf) {
							GET_GT4
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO4

						if(mist) {
							// DpqColor3 aanroepen met c0 als eerste is funest!?
							DpqColor3(&pr->c1, &pr->c2, &pr->c3, p, (CVECTOR *)&po->r1, (CVECTOR *)&po->r2, (CVECTOR *)&po->r3);
							DpqColor(&pr->c0, p, (CVECTOR *)&po->r0);
						}

						//test op subdiv
						if(b!=1 && otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
							else { tots4++; divpoly.ndiv= 1;}

							cdb->curdbuf= (char *)DivideGT4(PVERT4, UV4, COL4, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);

							// add head!!
							AddPrim(cdb->ot+otz, dr);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}
					}
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
			
		case P_LPGT3:
		case P_LGT3:
			{
				PrimLGT3 *pr= (PrimLGT3 *)sp;
				POLY_GT3 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip3( PVERT3, SCO3, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE && cliptest3((short *)sco))  {

						if(frombuf) {
							GET_GT3
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO3

						if(mist) {
							if(type==P_LPGT3) {
								NormalColorDpq( pr->v1+1, &pr->c0, p, (CVECTOR *)&po->r0);
								NormalColorDpq( pr->v2+1, &pr->c1, p, (CVECTOR *)&po->r1);
								NormalColorDpq( pr->v3+1, &pr->c2, p, (CVECTOR *)&po->r2);
							}
							else {
								NormalColorDpq(&pr->no, &pr->c0, p, (CVECTOR *)&po->r0);
								NormalColorDpq(&pr->no, &pr->c1, p, (CVECTOR *)&po->r1);
								NormalColorDpq(&pr->no, &pr->c2, p, (CVECTOR *)&po->r2);
							}
						}
						else {
							if(type==P_LPGT3) {
								NormalColorCol( pr->v1+1, &pr->c0, (CVECTOR *)&po->r0);
								NormalColorCol( pr->v2+1, &pr->c1, (CVECTOR *)&po->r1);				
								NormalColorCol( pr->v3+1, &pr->c2, (CVECTOR *)&po->r2);				
							}
							else {
								NormalColorCol(&pr->no, &pr->c0, (CVECTOR *)&po->r0);				
								NormalColorCol(&pr->no, &pr->c1, (CVECTOR *)&po->r1);				
								NormalColorCol(&pr->no, &pr->c2, (CVECTOR *)&po->r2);				
							}
						}

						//test op subdiv
						if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) divpoly.ndiv= 2;
							else divpoly.ndiv= 1;

							cdb->curdbuf= (char *)DivideGT3(PVERT3, UV3, COL3, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);

							AddPrim(cdb->ot+otz, dr);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}
					}
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
			
		case P_LPGT4:
		case P_LGT4:
			{
				PrimLGT4 *pr= (PrimLGT4 *)sp;
				POLY_GT4 *po;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip4( PVERT4, SCO4, &p, &otz, &flag);
			
					if (b>0 && otz>0 && otz<OTSIZE && cliptest4((short *)sco))  {
						
						if(frombuf) {
							GET_GT4
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO4

						if(mist) {
							if(type==P_LPGT4) {
								NormalColorDpq( pr->v1+1, &pr->c0, p, (CVECTOR *)&po->r0);
								NormalColorDpq( pr->v2+1, &pr->c1, p, (CVECTOR *)&po->r1);
								NormalColorDpq( pr->v3+1, &pr->c2, p, (CVECTOR *)&po->r2);
								NormalColorDpq( pr->v4+1, &pr->c3, p, (CVECTOR *)&po->r3);
							}
							else {
								NormalColorDpq(&pr->no, &pr->c0, p, (CVECTOR *)&po->r0);
								NormalColorDpq(&pr->no, &pr->c1, p, (CVECTOR *)&po->r1);
								NormalColorDpq(&pr->no, &pr->c2, p, (CVECTOR *)&po->r2);
								NormalColorDpq(&pr->no, &pr->c3, p, (CVECTOR *)&po->r3);
							}
						}
						else {
							if(type==P_LPGT4) {
								NormalColorCol( pr->v1+1, &pr->c0, (CVECTOR *)&po->r0);
								NormalColorCol( pr->v2+1, &pr->c1, (CVECTOR *)&po->r1);				
								NormalColorCol( pr->v3+1, &pr->c2, (CVECTOR *)&po->r2);				
								NormalColorCol( pr->v4+1, &pr->c3, (CVECTOR *)&po->r3);
							}
							else {
								NormalColorCol(&pr->no, &pr->c0, (CVECTOR *)&po->r0);
								NormalColorCol(&pr->no, &pr->c1, (CVECTOR *)&po->r1);
								NormalColorCol(&pr->no, &pr->c2, (CVECTOR *)&po->r2);
								NormalColorCol(&pr->no, &pr->c3, (CVECTOR *)&po->r3);
							}
						}
						

						//test op subdiv
						if(otz<subdiv4 && cdb->curdbuf<=cdb->maxdbuf)  {
							
							if(otz<subdiv16) { divpoly.ndiv= 2; tots16++;}
							else { tots4++; divpoly.ndiv= 1;}

							cdb->curdbuf= (char *)DivideGT4(PVERT4, UV4, COL4, (ulong *)cdb->curdbuf, cdb->ot+otz, &divpoly);
							// add head!!
							AddPrim(cdb->ot+otz, dr);
						}
						else  {
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}
					}
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
		
		case P_MASK3:
			{
				PrimFT3 *pr= (PrimFT3 *)sp;
				POLY_FT3 *po;
				short ofsx, minx;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip3(PVERT3, SCO3, &p, &otz, &flag);
				
					if(G.qual & 1) b= 0;
					
					if (b>0 && otz>0 && otz<OTSIZE && cliptest3mask((short *)sco))  {
						
						if(frombuf) {
							GET_FT3
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO3

						// komt de truuk: tpage berekenen
						minx= MIN3(po->x0, po->x1, po->x2);
						ofsx= minx & (~63);
						
						po->tpage= GetTPage(2, 0, 640+ofsx, 0);
						
						po->u0= po->x0-ofsx;
						po->v0= po->y0;
						po->u1= po->x1-ofsx;
						po->v1= po->y1;
						po->u2= po->x2-ofsx;
						po->v2= po->y2;						
						
						AddPrim(cdb->ot+otz, po);
						AddPrim(cdb->ot+otz, dr);

						// black
						{
							POLY_F3 *po;

							GET_BF3
							COPY_SCO3
							AddPrim(cdb->ot+otz, po);
						}

					}

					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
		case P_MASK4:
			{
				PrimFT4 *pr= (PrimFT4 *)sp;
				POLY_FT4 *po;
				short ofsx, minx;
				
				while(nr_elems-- > 0)  {
				
					b= RotAverageNclip4(PVERT4, SCO4, &p, &otz, &flag);
				
					if(G.qual & 1) b= 0;
					
					if (b>0 && otz>0 && otz<OTSIZE && cliptest4mask((short *)sco))  {
						
						if(frombuf) {
							GET_FT4
							GET_DR
						} else {
							po= &pr->po;
							dr= &pr->dr;
						}
						COPY_SCO4

						// komt de truuk: tpage berekenen
						minx= MIN4(po->x0, po->x1, po->x2, po->x3);
						ofsx= minx & (~63);
						
						po->tpage= GetTPage(2, 0, 640+ofsx, 0);
						
						po->u0= po->x0-ofsx;
						po->v0= po->y0;
						po->u1= po->x1-ofsx;
						po->v1= po->y1;
						po->u2= po->x2-ofsx;
						po->v2= po->y2;
						po->u3= po->x3-ofsx;
						po->v3= po->y3;
						
						AddPrim(cdb->ot+otz, po);
						AddPrim(cdb->ot+otz, dr);
						
						// black
						{
							POLY_F4 *po;

							GET_BF4
							COPY_SCO4
							AddPrim(cdb->ot+otz, po);
						}
						
					}

					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_BILLB:		// halo-billboard
		case P_OBBILLB:		
			{
				PrimBillb *pr= (PrimBillb *)sp;
				POLY_FT4 *po;
				short sxy[2], sizey, sizex;
				
				while(nr_elems-- > 0)  {
				
					otz= RotTransPers(&pr->cent, (long *)sxy, &p, &flag);
					
					if(flag & (1<<17));
					else if( otz>50 && otz<OTSIZE )  {
						
						if(frombuf) GET_FT4
						else po= &pr->po;

						sizey= (pr->cent.pad*G.scene->lens)/otz;
						sizey= SMUL( ob->size[0], sizey);
						
						if(HIRES) {
							sizex= SMUL(sizey, (512<<12)/320);
						}
						else sizex= sizey;
						
						sxy[0]+= (sizex/2);
						sxy[1]+= (sizey/2);
						
						po->x0= sxy[0]; 		po->y0= sxy[1];
						po->x1= sxy[0]-sizex; po->y1= po->y0;
						po->x2= po->x0; 	 	po->y2= sxy[1]-sizey;
						po->x3= po->x1; 	 	po->y3= po->y2;
						
						if(cliptest4o(&po->x0, &po->x1, &po->x2, &po->x3)) {

							if(frombuf) GET_DR
							else dr= &pr->dr;

							otz -= sizex/2;
							if(otz<=0) otz= 1;
							
							if(type==P_OBBILLB) {
								po->r0= obcol.r;
								po->g0= obcol.g;
								po->b0= obcol.b;
							}
							
							AddPrim(cdb->ot+otz, po);
							AddPrim(cdb->ot+otz, dr);
						}

					}
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		default:
			printf("error in drawmesh type %d aantal %d\n", type, nr_elems);
			return;
		}	
	}
	
}	




void draw_object(pObject *ob)
{
	MATRIX tempmat, tmat;
	pLife *lf;

	if(ob->type==OB_CAMERA) return;

//if(debuginfo) sprintf(gurustr, "ob %x %c%c%c%c\n", ob, ob->id.name[0], ob->id.name[1], ob->id.name[2], ob->id.name[3]);
//if(debuginfo) guru(gurustr);


	// lighting
	if(ob->type==OB_LIFE) {
		lf= ob->data;
		if(lf->dflag & LF_DONTDRAW) return;
		templfunc(ob);
	}
	else SetLightMatrix( (MATRIX *)ob->loclight[0]);
	
	SetColorMatrix( (MATRIX *)ob->lcolor[0]);

	// transleer: this destroys current rotmatrix!!!
	// geen compmatrix gebruiken: 2e matrix translatie <16 bits!
	ApplyMatrixLV(&G.viewmat, (VECTOR *)ob->obmat.t, (VECTOR *)&tempmat.t);

	tempmat.t[0]+= G.viewmat.t[0];
	tempmat.t[1]+= G.viewmat.t[1];
	tempmat.t[2]+= G.viewmat.t[2];

	SetTransMatrix(&tempmat);

	// roteer
	SetMulMatrix( &G.viewmat, &ob->obmat);

	if(ob->type==OB_SECTOR) {
		pSector *se;

		se= ob->data;
		
		if(se->texmesh->flag & ME_EFFECT) do_effect(ob, se->texmesh);
		draw_mesh(ob, se->texmesh, ob->obcol);

	}
	else if(ob->type==OB_LIFE) {
		
		if(lf->texmesh->flag & ME_EFFECT) {
			do_effect(ob, lf->texmesh);
		}
		draw_mesh(ob, lf->texmesh, ob->obcol);
	}
}


void draw_all()
{
	MATRIX mat;
	pSector *se;
	pObject *ob;
	pLife *lf;
	int a, b;

	// in theorie zit ie hier op de goede plek: pas NA drawOT!
	if(G.f & G_TWINANIM) do_twin_anims();

	/* update matrices*/

	SetTransMatrix(&G.viewmat);
	SetRotMatrix(&G.viewmat); 

	G.totface= 0;

	a= G.totsect;
	while(a--) {
		se= G.sectorbuf[a];
		if(se) {

			draw_object(se->ob);
			
			if(se->depth<4 && se->lbuf.tot) {
				for(b=0; b<se->lbuf.tot; b++) {
					ob= se->lbuf.ob[b];
					if(ob->lay & G.scene->lay) {
						if(ob->dfras==G.dfras) {
							draw_object(ob);
							ob->dfras= 0;
						}
					}
				}
			}
		}
	}

	// dyna lifes (als pbuf vol zit: zie je 'm flipperen)
	a= G.totlife;
	while(a--) {
	
		ob= G.lifebuf[a];
		draw_object(ob);
	
		if(ob->type==OB_LIFE) {
			lf= G.lifebuf[a]->data;
			for(b=0; b<lf->links.tot; b++) {
				draw_object(lf->links.ob[b]);
			}
		}
	}
}


