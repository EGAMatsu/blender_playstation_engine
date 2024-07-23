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
 *					 peffect.c
 *
 */

#include "psxblend.h"
#include "psxgraph.h"


void do_effect(pObject *ob, pMesh *me)
{
	static int ang= 0, index=0;
	SVECTOR *svec;
	int a, datalen, packets;
	short *sp, type, nr_elems, ang1, ang2, ang3, ang4;
	
	if(me->packetdata==0) return;

	/* nog doen: tellertje in mesh */
	if(me->flag & ME_UVEFFECT) {
		ang++;
		ang1= (1000+11*ang) % 4096;
		ang2= (1000+17*ang) % 4096;
		ang3= (1000+23*ang) % 4096;
		ang4= (1000+27*ang) % 4096;
	}
	else if(me->flag & ME_VCOLEFFECT) {
		index++;
		index &= 0xFF;
	}

	svec= (SVECTOR *)me->mvert;
	packets= me->totpacket;
	sp= me->packetdata;

	while(packets-- > 0)  {
	
		type= sp[0];
		nr_elems= sp[1];
		sp+= 2;
		
		switch(type)  {
/* FLAT */
		case P_F3:
		case P_OBF3:
			{
				PrimF3 *pr= (PrimF3 *)sp;
								
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_F4:
		case P_OBF4:
			{
				PrimF4 *pr= (PrimF4 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;

		case P_LF3:
			{
				PrimLF3 *pr= (PrimLF3 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_LF4:
			{
				PrimLF4 *pr= (PrimLF4 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;
/* GOUR */
		case P_G3:
		case P_OBG3:
			{
				PrimG3 *pr= (PrimG3 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_G4:
		case P_OBG4:
			{
				PrimG4 *pr= (PrimG4 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;

		case P_LG3:
			{
				PrimLG3 *pr= (PrimLG3 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_LG4:
			{
				PrimLG4 *pr= (PrimLG4 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;

/* FLAT TEX */			
		case P_FT3:
		case P_MASK3:
		case P_OBFT3:
			{
				PrimFT3 *pr= (PrimFT3 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
		case P_FT4:
		case P_SHADOW:
		case P_MASK4:
		case P_OBFT4:
			{
				PrimFT4 *pr= (PrimFT4 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;
		case P_LFT3:
			{
				PrimLFT3 *pr= (PrimLFT3 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
		case P_LFT4:
			{
				PrimLFT4 *pr= (PrimLFT4 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;

/* GOUR TEX */
		case P_2GT3:
		case P_OBGT3:
		case P_GT3:
			{
				PrimGT3 *pr= (PrimGT3 *)sp;
				
				while(nr_elems-- > 0)  {
					pr->po.u0= 128 + SMUL(120, rsin(ang1));
					pr->po.u0= 128 + SMUL(120, rcos(ang1));
					
					pr->po.v1= 128 + SMUL(120, rsin(ang2));
					pr->po.u1= 128 + SMUL(120, rcos(ang2));
					
					pr->po.v2= 128 + SMUL(120, rsin(ang3));
					pr->po.v2= 128 + SMUL(120, rcos(ang3));
					
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
		case P_2GT4:
		case P_OBGT4:
		case P_GT4:
			{
				PrimGT4 *pr= (PrimGT4 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;
		case P_LGT3:
			{
				PrimLGT3 *pr= (PrimLGT3 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
		case P_LGT4:
			{
				PrimLGT4 *pr= (PrimLGT4 *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;

/* SPECIALS */
		case P_BILLB:
		case P_OBBILLB:
			{
				PrimBillb *pr= (PrimBillb *)sp;
				
				while(nr_elems-- > 0)  {
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;

		default:
			printf("error in initmesh type %d aantal %d\n", type, nr_elems);
			return;
		}
	}	
}


void do_twin_anims()
{
	/* loop de images af: testen op twin anim, uitvoeren */
	pImage *ima;
	RECT rect, clutrect;
	short a, dx, *sp;
	
	ima= G.main->image.first;
	while(ima) {
		if(ima->tpageflag & IMA_COLCYCLE) {
			if(ima->clut) {
				setRECT(&clutrect, ima->xrep, ima->yrep , ima->clutlen, 1);

				sp= ima->clut;
				sp[ima->twsta]= sp[ima->twend];
				a= ima->twend;
				while(a > ima->twsta) {
					sp[a]= sp[a-1];				
					a--;
				}
  				LoadImage(&clutrect, ima->clut);
				
			}
		}
		else if(ima->tpageflag & IMA_TWINANIM) {
			
			/* next frame */
			if(ima->twcur < ima->twend) ima->twcur++;
			else ima->twcur= ima->twsta;
			
			/* rect berekenen */
			dx= (256/ima->xrep);
			rect.h= (256/ima->yrep);

			if(ima->type==0) rect.w= dx/4;
			else if(ima->type==1) rect.w= dx/2;
			else rect.w= dx;

			rect.y= ima->twcur / ima->xrep;
			rect.x= ima->twcur - rect.y*ima->xrep;
	
			rect.x= ima->sx + rect.x*rect.w;
			rect.y= ima->sy + rect.y*rect.h;


// PRINT4(d,d,d,d, rect.x,rect.y,rect.w,rect.h);

			/* rectcopy */
			MoveImage(&rect, ima->sx, ima->sy);
			
		}
		ima= ima->id.next;
	}
	
	
}

void init_twin_anims()
{
	/* loop de images af: testen op twin anim, G.f zetten */
	pImage *ima;
	
	G.f &= ~G_TWINANIM;
	
	ima= G.main->image.first;
	while(ima) {
		if(ima->tpageflag & IMA_TWINANIM) {

			G.f |= G_TWINANIM;
			ima->twcur= ima->twsta;
			
		}
		if(ima->tpageflag & IMA_COLCYCLE) {
			G.f |= G_TWINANIM;
		}
		ima= ima->id.next;
	}
}

