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
 *					 pobject.c
 *
 */

#include "psxblend.h"
#include "psxgraph.h"


void do_obipo(pObject *ob, short cur, short delta, short *speed)	/* delta is ook flag! */
{
	/* alleen de realtime versies */
	pLife *lf;
	pIpo *pipo=0;
	CVECTOR *cvec;
	short icur, *fpoin, *first, *rotpoin;
	
	if(ob->type==OB_SECTOR) {
		pSector *se= ob->data;
		pipo= se->ipo.first;
	}
	else if(ob->type==OB_LIFE) {
		lf= ob->data;
		pipo= lf->ipo.first;
	}

	/* dit is het geval bij dyna's */
	if(speed==0) {
		rotpoin= ob->rot;
	}
	else rotpoin= lf->rot;

	while(pipo) {
		if(pipo->type<=IP_FROMOB) {
			
			icur= cur - pipo->sta;
			
			if(delta==0) {		/* standaard ipo */
			
				CLAMP(icur, 0, pipo->nr_elems-1);

				fpoin= ((short *)(pipo+1))+icur*pipo->elemsize;
				
				if(pipo->type & IP_OBCOL) {
					ob->obcol.r= fpoin[0];
					ob->obcol.g= fpoin[1];
					ob->obcol.b= fpoin[2];
					fpoin+= 3;
				}
				if(pipo->type & IP_LOC) {
					VECCOPY(ob->loc, fpoin);
					fpoin+= 3;
				}
				if(pipo->type & IP_ROT) {
					VECCOPY(ob->rot, fpoin);
					fpoin+= 3;
				}
				if(pipo->type & IP_SIZE) {
					VECCOPY(ob->size, fpoin);
				}
			}
			else {				/* delta ipo */

				/*  deze test is niet overbodig: interval sensors != interval ipo */
				if(delta==1 && (icur<=0 || icur>= pipo->nr_elems));
				else if(delta== -1 && (icur<0 || icur>= pipo->nr_elems-1));
				else {
					
					fpoin= ((short *)(pipo+1))+icur*pipo->elemsize;
					first= fpoin - delta*pipo->elemsize;
				
					if(pipo->type & IP_OBCOL) {
						
						fpoin+= 3; first+= 3;
					}
					if(pipo->type & IP_LOC) {
						if(speed) {	
							speed[0]+= (fpoin[0] - first[0])<<2;
							speed[1]+= (fpoin[1] - first[1])<<2;
							speed[2]+= (fpoin[2] - first[2])<<2;
						}
						else {
							ob->loc[0]+= fpoin[0] - first[0];
							ob->loc[1]+= fpoin[1] - first[1];
							ob->loc[2]+= fpoin[2] - first[2];
						}
						fpoin+= 3; first+= 3;
					}
					if(pipo->type & IP_ROT) {
						ob->rot[0]+= fpoin[0] - first[0];
						ob->rot[1]+= fpoin[1] - first[1];
						ob->rot[2]+= fpoin[2] - first[2];
						
						ob->rot[0] %= 4096;
						ob->rot[1] %= 4096;
						ob->rot[2] %= 4096;
						
						fpoin+= 3; first+= 3;
					}
					if(pipo->type & IP_SIZE) {
						ob->size[0]+= fpoin[0] - first[0];
						ob->size[1]+= fpoin[1] - first[1];
						ob->size[2]+= fpoin[2] - first[2];
					}
				
				}
				
			}	
		}
		pipo= pipo->next;
	}
	
}

void do_force_obipo(pObject *ob, short cur, short *force, short *omega)
{
	/* alleen de realtime versies */
	pIpo *pipo=0;
	CVECTOR *cvec;
	short icur, *fpoin, *first;
	
	if(ob->type==OB_LIFE) {
		pLife *lf= ob->data;
		pipo= lf->ipo.first;
	}

	while(pipo) {
		if(pipo->type<=IP_FROMOB) {
			
			icur= cur - pipo->sta;

			/*  deze test is niet overbodig: interval sensors != interval ipo */
			if(icur<=0 || icur>= pipo->nr_elems);
			else {

				fpoin= ((short *)(pipo+1))+icur*pipo->elemsize;
				first= fpoin - pipo->elemsize;
			
				if(pipo->type & IP_OBCOL) {
					
					fpoin+= 3; first+= 3;
				}
				if(pipo->type & IP_LOC) {
					force[0]+= 16*(fpoin[0] - first[0]);
					force[1]+= 16*(fpoin[1] - first[1]);
					force[2]+= 16*(fpoin[2] - first[2]);
					fpoin+= 3; first+= 3;
				}
				if(pipo->type & IP_ROT) {
					omega[0]+= 64*(fpoin[0] - first[0]); // let op rotspeed in psector.c
					omega[1]+= 64*(fpoin[1] - first[1]);
					omega[2]+= 64*(fpoin[2] - first[2]);
				}
			}
		
		}
		pipo= pipo->next;
	}
	
}

/* return 1: einde bereikt */
int set_k2k_interval(short mode, pAction *ac, pLife *lf)
{
	pIpo *pipo;
	short a, *sp;

	if(ac->action==SN_K2K_OBIPO) {
		pipo= lf->ipo.first;
		while(pipo) {
			if(pipo->type<=IP_FROMOB) {
				if(pipo->nr_keys) {
					sp=  ((short *)(pipo+1))+pipo->nr_elems*pipo->elemsize;
					
					if(mode==0) {		/* forw first */
						ac->sta= sp[0];
						ac->end= sp[1];
						lf->cfra= ac->sta;
					}
					else if(mode==2) {		/* backw last */
						sp += (pipo->nr_keys-1);
						ac->sta= sp[0];
						ac->end= sp[-1];
						lf->cfra= ac->sta;
					}
					else if(mode==1) {	/* next */
						a= pipo->nr_keys-1;
						while(a--) {
							if(sp[0]==lf->cfra) {
								ac->sta= sp[0];
								ac->end= sp[1];
								return 0;
							}
							sp++;
						}
						/* we zijn hier: eind bereikt */
						if(lf->cfra==sp[0]) return 1;
						/* of niets te vinden */
						return 2;
					}
					else if(mode== -1) {	/* prev */

						a= pipo->nr_keys-1;

						/* kan niet verder terug */
						if(lf->cfra==sp[0]) return 1;

						sp++;
						while(a--) {
							if(sp[0]==lf->cfra) {
								ac->sta= sp[0];
								ac->end= sp[-1];
								return 0;
							}
							sp++;
						}
						/* we zijn hier: eind bereikt */
						return 2;
					}
				}
			}
			pipo= pipo->next;
		}
	}
	
	return 0;
}



void where_is_object(pObject *ob)
{
	pObject *par;
	MATRIX tmat, slowmat, *obmat;
	VECTOR vec;
	short a, ok, *sp;
	
	if(ob==0) return;

	if(ob->parent && (ob->partype & PARSLOW)) slowmat= ob->obmat;
	
	eul_to_matrix(ob->rot, &ob->obmat);
  	if(ob->size[0] != 4096) {
		vec.vx= ob->size[0];
		vec.vy= ob->size[1];
		vec.vz= ob->size[2];
  		ScaleMatrix(&ob->obmat, &vec);
  	}
  
   	VECCOPY(ob->obmat.t, ob->loc);
	
	par= ob->parent;
	
	if(par) {
		
		switch(ob->partype & 15) {
		case PAROBJECT:
			tmat= ob->obmat;
			CompMatrix(&par->obmat, &tmat, &ob->obmat); // tmat: translatiestuk moet in 16 bits passen
			break;
			
		case PARVERT1:
			ob->obmat.t[0]+= par->obmat.t[0];
			ob->obmat.t[1]+= par->obmat.t[1];
			ob->obmat.t[2]+= par->obmat.t[2];
			break;
		}
		
	}
	
	if(ob->parent && (ob->partype & PARSLOW)) {
		extern int sw_time;		// screen.c
		int fac1, fac2;

		/* deze is getest met wapen in orcus */
		fac1= (328 + 6*sw_time)/(1 + (ob->sf));	// 6x628=3768
		if(fac1<4096) {
			fac2= 4096 - fac1;
			obmat= &ob->obmat;
			
			for(a=0; a<3; a++) {
			
				obmat->m[0][a]= (fac1*obmat->m[0][a] + fac2*slowmat.m[0][a])>>12;
				obmat->m[1][a]= (fac1*obmat->m[1][a] + fac2*slowmat.m[1][a])>>12;
				obmat->m[2][a]= (fac1*obmat->m[2][a] + fac2*slowmat.m[2][a])>>12;
				
				obmat->t[a]= (fac1*obmat->t[a] + fac2*slowmat.t[a])>>12;
			}
		}
	}
	
	//ob->obmat.flag= MAT_CALC;
}

