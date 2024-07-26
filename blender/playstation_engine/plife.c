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
 *					 plife.c
 *
 */

#include "psxblend.h"
#include "psxgraph.h"

#define GLOBALSHIFT	2


void life_to_local_sector_co(pLife *lf)
{

	if(lf->sector) {
		/* transformeren naar locale coordinaten */
		
		ApplyMatrix12(&lf->sector->ob->imat, lf->loc, lf->loc1);
		ApplyMatrix12(&lf->sector->ob->imat, lf->oldloc, lf->oldloc1);
		
		lf->speed1[0]= (lf->loc1[0]-lf->oldloc1[0]);
		lf->speed1[1]= (lf->loc1[1]-lf->oldloc1[1]);
		lf->speed1[2]= (lf->loc1[2]-lf->oldloc1[2]);
	}
}


void life_from_inv_sector_co(pLife *lf)
{

	if(lf->sector) {
		/* transformeren vanuit locale coordinaten naar globale */
		
		/* eerst de (nieuwe) eindlocatie */
		lf->loc1[0]= lf->oldloc1[0]+lf->speed1[0];
		lf->loc1[1]= lf->oldloc1[1]+lf->speed1[1];
		lf->loc1[2]= lf->oldloc1[2]+lf->speed1[2];

		ApplyMatrix12(&lf->sector->ob->obmat, lf->loc1, lf->loc);
		ApplyMatrix12(&lf->sector->ob->obmat, lf->oldloc1, lf->oldloc);
		if(lf->collision) ApplyMatrix12(&lf->sector->ob->obmat, lf->colloc, lf->colloc);
		
		lf->speed[0]= (lf->loc[0]-lf->oldloc[0]);
		lf->speed[1]= (lf->loc[1]-lf->oldloc[1]);
		lf->speed[2]= (lf->loc[2]-lf->oldloc[2]);
	}
}

void init_lifes()
{
	extern pObject *main_actor;
	pLife *lf;
	pBase *base;
	pObject *ob, *par;
	pSensor *sn;
	pEvent *ev;
	pAction *ac;
	short *sp;
	int a, b;
	
	G.totlife= 0;
	main_actor= 0;	
	
	lf= G.main->life.first;
	while(lf) {

		lf->timer= -1;
		lf->contact= 0;
		lf->collision= 0;
		lf->floor= 0;
		lf->dflag= 0;
		lf->frictfac= 4096;
		
		lf= lf->id.next;
	}

	base= FIRSTBASE;
	while(base) {
		ob= base->object;
		if(ob==G.scene->camera) {
			if(ob->parent && ob->parent->type==OB_LIFE) {
				/* is al in psx file gebeurd */
			}
			else add_dyna_life(base->object);	/* uitzondering */
		}
		else if(base->object->type==OB_LIFE) {

			lf= ob->data;
			
			if(lf->type==LF_DYNAMIC && (lf->flag & LF_MAINACTOR)) {
				if(main_actor==0) main_actor= ob;
				else if(lf->flag & LF_CAMERA_ACTOR) main_actor= ob;
			}

			if(lf->texmesh) lf->texmesh->lastfra= 0;
			
			lf->speed[0]= lf->speed[1]= lf->speed[2]= 0;
			lf->rotspeed[0]= lf->rotspeed[1]= lf->rotspeed[2]= 0;
			
			do_obipo(ob, lf->sfra, 0);
			
			sn= lf->sensors;
			a= lf->totsens;
			while(a--) {
				
				/* init events */
				sn->event= sn->evento= 0;
				
				ev= sn->events;
				b= sn->totevent;
				while(b--) {
					
					if(ev->flag & SN_NOT) ev->shiftval= -1;
					else ev->shiftval= 0;

					if(ev->event==SN_NEAR) {
						/* near flag */
						ev->var= 0;	
					}
					else if(ev->event==SN_COLLISION) {
						ev->var= 0;
					}
					
					ev++;
				}
				
				/* init actions (restart?) */
				ac= sn->actions;
				b= sn->totaction;
				while(b--) {
				
					if(ac->action<100) {
						ac->cur= 0;
						if ELEM3(ac->action, SN_X_ROT, SN_Y_ROT, SN_Z_ROT) lf->dflag |= LF_OMEGA;
					}
					else if(ac->action<200) {
						
						/* ac->go  bepaalt of er aktie ondernomen wordt */ 
						ac->go= 0;

						if(ac->action == SN_DELTA_OBIPO) {
							ac->cur= ac->sta;
						}
						else if(ac->action == SN_PLAY_OBIPO) {
							ac->cur= ac->sta;
						}
						else if ELEM(ac->action, SN_LOOPSTOP_OBIPO, SN_LOOPEND_OBIPO) {
							ac->cur= ac->end;
						}
						else if ELEM(ac->action, SN_PINGPONG_OBIPO, SN_FLIPPER_OBIPO) {
							ac->cur= ac->sta;
						}
						else if(ac->action==SN_K2K_OBIPO) {
							if(ac->var & SN_K2K_PREV) set_k2k_interval(2, ac, lf);
							else set_k2k_interval(0, ac, lf);
							
							ac->cur= ac->sta;
						}

						if(ac->flag & SN_IPO_SETVAR) {
							if(ac->poin) *((short *)ac->poin)= ac->cur/2;
						}
						
					}
					else if(ac->action<300) {
						// divers
						ac->go= 0;
						
						if(ac->action==SN_VISIBLE) {
							ac->cur= 0;
							lf->dflag |= LF_DONTDRAW;
						}
						else if(ac->action==SN_ROBBIE_M) {
							ac->fac= 0;	/* corr */
						}
					}
					else if(ac->action<400) {
						// vars: init
						if(ac->poin) *((short *)ac->poin)= ac->cur;
					}
					ac++;
				}
				sn++;
			}

			where_is_object(ob);
			mat_invert(&ob->imat, &ob->obmat);
			lf->oldimat= ob->imat;
			
			// conversie van 10 bits naar 12 bits 
			lf->startloc[0]= ob->loc[0]<<GLOBALSHIFT;
			lf->startloc[1]= ob->loc[1]<<GLOBALSHIFT;
			lf->startloc[2]= ob->loc[2]<<GLOBALSHIFT;

			VECCOPY(lf->startrot, ob->rot);
			
			VECCOPY(lf->loc, lf->startloc);
			VECCOPY(lf->rot, lf->startrot);
			
			VECCOPY(lf->oldloc, lf->startloc);
			
			/* alleen zichtbare, ivm ADDLIFE */
			if(base->lay & G.scene->lay) {
				
				/* de lbuf's zijn in psx file weggeschreven */
				
				if(lf->type==LF_DYNAMIC) {
					if(lf->flag & LF_MAINACTOR) add_dyna_life(ob);

					/* alleen findsector bij dynalife: props zijn al gezet */
					lf->sector= (pSector *)find_sector(lf->loc, lf->loc1);
				}
				else if(lf->type==LF_LINK) {
					/* o.a. voor robbie */
					if(lf->flag & LF_MAINACTOR) add_dyna_life(ob);
				}
			}

			life_to_local_sector_co(lf);
			
		}
		base= base->next;
	}
	
}


void end_lifes(int restore)
{
	pLife *lf;
	pBase *base;

	base= FIRSTBASE;
	while(base) {
		
		if(base->object->type==OB_LIFE) {

			base->object->lay= base->lay;	/* kan verzet zijn */
		
			lf= base->object->data;
			
			if(restore || lf->type==LF_PROP) {
				// conversie van 10 bits naar 12 bits 
				base->object->loc[0]= lf->startloc[0]>>GLOBALSHIFT;
				base->object->loc[1]= lf->startloc[1]>>GLOBALSHIFT;
				base->object->loc[2]= lf->startloc[2]>>GLOBALSHIFT;

				VECCOPY(base->object->rot, lf->startrot);
			}
		}
		base= base->next;
	}

	del_dupli_lifes();
}

/* *********************SENSORS ******************* */

/*
 * 
 * 
 */

#define TOTKEY	19
int padstate=0;
short simuldevs[32], simulvals[32];

void init_devs()
{
	/* indexnummers zijn gelijk aan sensor-eventnummers */
	
	simuldevs[0]= 0;

	// simuldevs[1]= (1<<4);
	// simuldevs[2]= (1<<7);
	simuldevs[3]= (1<<8);
	// simuldevs[4]= (1<<5);
	simuldevs[5]= 1<<11;
	simuldevs[6]= 0;

	simuldevs[7]= (1<<15);
	simuldevs[8]= (1<<13);	
	simuldevs[9]= (1<<12);
	simuldevs[10]= (1<<14);

	simuldevs[11]= (1<<7);
	simuldevs[12]= (1<<5);	
	simuldevs[13]= (1<<4);
	simuldevs[14]= (1<<6);

	simuldevs[15]= (1<<3);
	simuldevs[16]= (1<<1);	
	simuldevs[17]= (1<<2);
	simuldevs[18]= (1<<0);
	
	simuldevs[31]= 1;
	
	bzero((char *)simulvals, 32*2);
	
	simulvals[31]= 1;	/* always */
}


/* iets soortgelijks is op SGI: combinatie van getbutton en qread */
/* het geheugen (hold, vorige stand) zit in sensors zelf */
void pad_read()
{
	ulong qual;
	short a;
	
	/* events wissen, getbutton */
	
	qual = PadRead(1);
	G.qual= qual & 0xFFFF;
	qual= qual>>16;
	
	for(a=0; a<TOTKEY; a++) {
		simulvals[a]= 0;
		if(G.qual & simuldevs[a]) simulvals[a]  = 1;
		if(  qual & simuldevs[a]) simulvals[a] |= 2;
	}
	
	/* queue lezen? */

}

void Mat3ToEulFast(MATRIX *mat, short *eul)
{
	int cy;

	/* geen shift! */
	cy= (mat->m[0][0]*mat->m[0][0] + mat->m[0][1]*mat->m[0][1]);	
	cy= SquareRoot0(cy);

	if (cy > 30) {	/* ? */
		eul[0] = ratan2(mat->m[1][2], mat->m[2][2]);
		eul[1] = ratan2(-mat->m[0][2], cy);
		eul[2] = ratan2(mat->m[0][1], mat->m[0][0]);
	} else {
		eul[0] = ratan2(-mat->m[2][1], mat->m[1][1]);
		eul[1] = ratan2(-mat->m[0][2], cy);
		eul[2] = 0;
	}
}

void camera_behaviour(pObject *cam, pLife *lfcam, pAction *ac)
{
	pObject *actor;
	pLife *lfactor;
	pPortal *po;
	MATRIX mat;
	short *fp1, *fp2, rc[3];
	int mindistsq, maxdistsq;
	int inp, fac, distsq, lookat[3], from[3];
	short a, ok;
	
	actor= ac->poin;
	lfactor= actor->data;
	mindistsq= ac->min*ac->min;
	maxdistsq= ac->max*ac->max;

	/* init */
	lookat[0]= lfactor->loc[0];
	lookat[1]= lfactor->loc[1];
	lookat[2]= lfactor->loc[2];
	inp= MAX2( (lfactor->floorloc[2]<<2), -5*lfactor->axsize );
	lookat[2] += inp;
	
	// 12 bits maken
	from[0]= cam->loc[0]<<2;
	from[1]= cam->loc[1]<<2;
	from[2]= cam->loc[2]<<2;

	/* CONSTRAINT 1: staat camera goed geroteerd in sector (90 graden grid)? */

	/* CONSTRAINT 2: kan cam actor zien? */
	/*				 niet vanuit schaduw!!! */
	ok= test_visibility(lfactor->loc, from, lfcam, lfactor->sector);

	/* if(ok==0 && lfactor->sector) { */
	/* 	a= lfactor->sector->totport; */
	/* 	po= lfactor->sector->portals; */
	/* 	while(a--) { */
	/* 		if( test_visibility(lfactor->loc, from, po->sector) ) break; */
	/* 		po++; */
	/* 	} */
	/* } */

	/* CONSTRAINT 3: vaste hoogte boven schaduw */
	if(ok==0) from[2]= (15*from[2] + lookat[2] + ac->force)>>4;
	
	/* CONSTRAINT: achterliggende camera */
	if(ok==0 && G.qual==0) {
		fp1= actor->obmat.m[0];
		fp2= cam->obmat.m[0];
		
		inp= (fp1[0]*fp2[0] + fp1[3]*fp2[3] + fp1[6]*fp2[6]);
		fac= (-4096*4096 + inp)>>18;

		from[0]+= SMUL(fac, fp1[0]);
		from[1]+= SMUL(fac, fp1[3]);
		from[2]+= SMUL(fac, fp1[6]);
		
		/* alleen alstie ervoor ligt: cross testen en loodrechte bijtellen */
		if(inp<0) {
			if(fp1[0]*fp2[3] - fp1[3]*fp2[0] > 0) {
				from[0]-= SMUL(fac, fp1[3]);
				from[1]+= SMUL(fac, fp1[0]);
			}
			else {
				from[0]+= SMUL(fac, fp1[3]);
				from[1]-= SMUL(fac, fp1[0]);
			}
		}
	}

	/* CONSTRAINT 4: minimum / maximum afstand */
	rc[0]= (lookat[0]-from[0]);
	rc[1]= (lookat[1]-from[1]);
	rc[2]= (lookat[2]-from[2]);
	distsq= rc[0]*rc[0] + rc[1]*rc[1] + rc[2]*rc[2];

	if(distsq > maxdistsq) {
		distsq = ( (distsq-maxdistsq))/(distsq>>10);
		
		from[0] += SMUL(distsq, rc[0]);
		from[1] += SMUL(distsq, rc[1]);
		from[2] += SMUL(distsq, rc[2]);
	}
	else if(distsq < mindistsq) {
		distsq = ( (mindistsq-distsq))/(mindistsq>>10);
		
		from[0] -= SMUL(distsq, rc[0]);
		from[1] -= SMUL(distsq, rc[1]);
		from[2] -= SMUL(distsq, rc[2]);
	}

	/* CONSTRAINT 4a: nog eens vaste hoogte boven schaduw */
	if(ok==0) from[2]= (15*from[2] + lookat[2] + ac->force)>>4;

	/* CONSTRAINT 5: track naar schaduw */
	rc[0]= (lookat[0]-from[0]);
	rc[1]= (lookat[1]-from[1]);
	rc[2]= (lookat[2]-from[2]);
	VecUpMat3(rc, &mat, 1);	/* Z up Track x */
	

	/* CONSTRAINT: klein beetje met aktie meekijken: projecteer x-vec op scherm? */

	fp1= actor->obmat.m[0];
	fp2= G.viewmat.m[0];		// uitlezen alsof dit viewinv is
		
	inp= (fp2[0]*fp1[0] + fp2[1]*fp1[3] + fp2[2]*fp1[6])>>16;

	ac->fac= (15*ac->fac + inp)>>4;

	Mat3ToEulFast(&mat, cam->rot);
	cam->rot[2]-= ac->fac;
	
	// terug naar 10 bits */	
	cam->loc[0]= from[0]>>2;
	cam->loc[1]= from[1]>>2;
	cam->loc[2]= from[2]>>2;

}

void compatible_eulFast(short *eul, short *oldrot)
{
	short dx, dy, dz;
	
	/* verschillen van ong 360 graden corrigeren */

	dx= eul[0] - oldrot[0];
	dy= eul[1] - oldrot[1];
	dz= eul[2] - oldrot[2];

	if( abs(dx) > 3500) {
		if(dx > 0) eul[0] -= 4096; else eul[0]+= 4096;
	}
	if( abs(dy) > 3500) {
		if(dy > 0) eul[1] -= 4096; else eul[1]+= 4096;
	}
	if( abs(dz) > 3500 ) {
		if(dz > 0) eul[2] -= 4096; else eul[2]+= 4096;
	}
}


void track_life_to_life(pLife *this, pLife *to, short fac, short mode)
{
	short vec[3];
	MATRIX mat;
	
	vec[0]= to->loc[0] - this->loc[0];
	vec[1]= to->loc[1] - this->loc[1];
	vec[2]= to->loc[2] - this->loc[2];
	
	if(mode==0) {	/* alleen z-rot */
		vec[2]= ratan2(vec[1], vec[0]);
		vec[0]= vec[1]= 0.0;
	}
	else {
		VecUpMat3(vec, &mat, 1);	/* Z up Track x */
		Mat3ToEulFast(&mat, vec);
	}
	
	if(fac==0) {
		VECCOPY(this->rot, vec);
	}
	else {
		/* this rot aanpassen !!!*/
		compatible_eulFast(this->rot, vec);
	
		this->rot[0]= (fac*this->rot[0] + vec[0])/(1+fac);
		this->rot[1]= (fac*this->rot[1] + vec[1])/(1+fac);
		this->rot[2]= (fac*this->rot[2] + vec[2])/(1+fac);
	}
}

int dyna_near_life(pObject *prob, int mindist, pObject *actor)
{
	pLife *lf;
	pObject *ob;
	long *vec, *test, dist;
	int a;
	
	/* alle dyna lifes */
	test= prob->obmat.t;
	
	if(actor) {
		vec= actor->obmat.t;
		dist= abs(vec[0]-test[0]) + abs(vec[1]-test[1]) + abs(vec[2]-test[2]);
		if(dist<mindist) return 1;
	}
	else {
		a= G.totlife;
		while(a--) {
			ob= G.lifebuf[a];
			if( prob != ob && ob->type==OB_LIFE) {
				lf= ob->data;
				if(lf->type==LF_DYNAMIC && (lf->dflag & LF_TEMPLIFE)==0) {
					vec= ob->obmat.t;
					dist= abs(vec[0]-test[0]) + abs(vec[1]-test[1]) + abs(vec[2]-test[2]);
					if(dist<mindist) return 1;
				}
			}
		}
	}
	return 0;
}

/* retval: */
#define SN_CHANGED		1
#define SN_DOFORCE		2
#define SN_DOSPEED		4
#define SN_DOOMEGA		8


int sensor_input(pObject *ob, pLife *lf, short *forcetot, short *omegatot)
{
	pSensor *sn;
	pEvent *ev;
	pAction *ac;
	pObject *pob, *victim=0;
	pLife *plf;
	SVECTOR temp;
	short omega[3], force[3], speed[3];
	short event, a, b, pulseval, mode, retval=0, eventlock;
	short *sp, cox;

	
	if(lf->type==LF_DYNAMIC) {
		omega[0]=omega[1]=omega[2]= 0;
		force[0]=force[1]=force[2]= 0;
		speed[0]=speed[1]=speed[2]= 0;
		lf->frictfac= 4096;
	}
	else if(lf->type==LF_LINK) {
		omega[0]=omega[1]=omega[2]= 0;
	}

	if(lf->timer>=0) lf->timer--;
	
	sn= lf->sensors;
	a= lf->totsens;
	while(a--) {
		
		/* EVENTS */
		
		sn->evento= sn->event;
		
		b= sn->totevent;
		ev= sn->events;
		sn->event= 1;

		while(b--) {
			
			ev->shiftval <<= 1;
			event= 1;
			
			if(ev->event<32) {	/* globale events */
				event= simulvals[ev->event];

				if(ev->var==0) {	// controller 1
					if(event & 1) event= 1;
					else event= 0;
				}
				else {				// controller 2
					if(event & 2) event= 1;
					else event= 0;
				}
				
				if(ev->event==31) ev->flag= SN_HOLD;
				
				/* if( (event & 2) && ev->var==0) event= 0; */
				/* else if( (event & 1) && ev->var==1) event= 0; */
				
			}
			else {
				switch(ev->event) {
				case SN_NEAR:
					/* ev->var is memory */

					if(ev->var) event= dyna_near_life(ob, ev->sfac1, ev->poin);
					else event= dyna_near_life(ob, ev->sfac, ev->poin);
					
					if(event) {
						ev->var= 1;
						victim= ev->poin;
					}
					else ev->var= 0;
					
					break;
					
				case SN_CONTACT:
					if(lf->contact==0) event= 0;
					else if(ev->poin && ev->poin!=lf->contact) event= 0;
					break;
					
				case SN_COLLISION:

					if(lf->collision==0 || ev->var>0) {
						event= 0;
						if(ev->var>0) ev->var--;
					}
					else if(ev->poin && lf->collision!=ev->poin) {
						event= 0;
						if(ev->var>0) ev->var--;
					}
					else {
						ev->var= ev->sfac;
						victim= lf->collision;
					}
					
					break;
					
				case SN_TIMER:
					/* timer in fields */
					if( (lf->timer>>1) != ev->var) event= 0;
					break;
				
				case SN_VAR_EQUAL:
					sp= ev->poin;
					if(sp==0 || *sp!=ev->var) event= 0;
					break;
				case SN_VAR_INTERVAL:
					sp= ev->poin;
					if(sp==0 || *sp<ev->sfac || *sp>ev->sfac1) event= 0;
					break;
				case SN_VAR_CHANGED:
					sp= ev->poin;
					if(sp) {
						if(*sp==ev->var) event= 0;
						else ev->var= *sp;
					}
					else event= 0;
					break;
				}
				
			}
			
			if(ev->flag & SN_NOT) event= 1-event;

			if(event) {
				ev->shiftval |= 1;
			
				if(ev->flag & SN_HOLD) {
					if(ev->pulse) {
						pulseval= (1<< (ev->pulse+1))-1;
						
						if( (ev->shiftval & pulseval)==pulseval ) ev->shiftval &= ~1;
						if((ev->shiftval & 3)!=1) sn->event= 0;
						
					}
				}
				else {
					if((ev->shiftval & 6)) sn->event= 0;	/* klein beetje fuzz */
				}
			}
			else sn->event= 0;

			ev++;
		}
		
		/* ACTIONS STARTEN */
		
		if(sn->event) {
		
			eventlock= 0;

			ac= sn->actions;
			b= sn->totaction;
			while(b--) {
			
				/* waarop: */
				switch(ac->action) {
				case SN_X_TRANS:
					force[0] += ac->force;
					if(lf->frictfac>ac->fac) lf->frictfac= ac->fac;
					retval |= SN_CHANGED+SN_DOFORCE;
					break;
				case SN_Y_TRANS:
					force[1] += ac->force;
					if(lf->frictfac>ac->fac) lf->frictfac= ac->fac;
					retval |= SN_CHANGED+SN_DOFORCE;
					break;
				case SN_Z_TRANS:
					force[2] += ac->force;
					if(lf->frictfac>ac->fac) lf->frictfac= ac->fac;
					retval |= SN_CHANGED+SN_DOFORCE;
					break;

				case SN_GOTO:
					if(ac->poin) {
						pObject *ob= ac->poin;
						ob->loc[0]= ac->force;
						ob->loc[1]= ac->min;
						ob->loc[2]= ac->max;
					}
					else {	

						lf->speed[0]= lf->speed[1]= lf->speed[2]= 0;
						force[0]= force[1]= force[2]= 0;
						// 10 bits naar 12 bits
						lf->loc[0]= 4*ac->force;
						lf->loc[1]= 4*ac->min;
						lf->loc[2]= 4*ac->max;
					}
					break;

				case SN_X_ROT:
					omega[0] += ac->force;
					retval |= SN_CHANGED+SN_DOOMEGA;
					if(ac->flag & SN_LIMITS) {
						if( lf->rot[0] < ac->min || lf->rot[0]>ac->max) {
							ac->go= 1;
							ac->cur= 50;
						}
					}
					break;
				case SN_Y_ROT:
					omega[1] += ac->force;
					retval |= SN_CHANGED+SN_DOOMEGA;
					if(ac->flag & SN_LIMITS) {
						if( lf->rot[1] < ac->min || lf->rot[1]>ac->max) {
							ac->go= 1;
							ac->cur= 50;
						}
					}
					break;
				case SN_Z_ROT:
					omega[2] += ac->force;
					retval |= SN_CHANGED+SN_DOOMEGA;
					if(ac->flag & SN_LIMITS) {
						if( lf->rot[2] < ac->min || lf->rot[2]>ac->max) {
							ac->go= 1;
							ac->cur= 50;
						}
					}
					break;

				case SN_TRACKTO:
					if(ac->poin) {
						pObject *to=ac->poin;
						retval |= SN_CHANGED+SN_ROTCHANGED;
						track_life_to_life(lf, to->data, ac->sta, ac->cur);
					}
					break;

				case SN_ADD_DAMAGE:
					if(victim) {
						if(victim->type==OB_LIFE) {
							pLife *lfs= victim->data;
							/* wel 'add health' toestaan */
							if(ac->var<0 || (lfs->dflag & LF_NO_DAMAGE)==0) 
								lfs->state[LF_DAMAGE-1] += ac->var;
						}
					}
					break;

				case SN_NO_DAMAGE:
					lf->dflag |= LF_NO_DAMAGE;
					ac->go= 1;
					break;

				case SN_ROBBIE_M:
					if(ac->poin) {
						camera_behaviour(ob, lf, ac);
						retval |= SN_CHANGED+SN_ROTCHANGED;
					}
					break;

				case SN_DELTA_OBIPO:
				case SN_PLAY_OBIPO:
					
					if( sn->evento==0) {	/* KEY_IN */
					
						if(ac->go==0) {
							ac->cur= ac->sta;
							if(ac->end > ac->sta) ac->go= 1;
							else ac->go= -1;
						}
						else eventlock= 1;
					}
					break;
					
				case SN_PINGPONG_OBIPO:
					
					if( sn->evento==0) {	/* KEY_IN */
						if(ac->go) {
							ac->go= -ac->go;
						}
						else if(ac->cur==ac->sta) { /* sta== ook initwaarde en rustwaarde */
							if(ac->end > ac->sta) ac->go= 1;
							else ac->go= -1;
						}
						else if(ac->cur==ac->end) {/* end==rustwaarde */
							if(ac->sta > ac->end) ac->go= 1;
							else ac->go= -1;
						}
					}
					break;
					
				case SN_FLIPPER_OBIPO:

					if( sn->evento==0) {	/* KEY_IN */
						/* niet cur zetten: is flipper! */
						if(ac->end > ac->sta) ac->go= 1;
						else ac->go= -1;
					}
					break;
					
				case SN_LOOPSTOP_OBIPO:
				case SN_LOOPEND_OBIPO:
					
					if( sn->evento==0) {	/* KEY_IN */
						
						if(ac->cur==ac->end) {	/* end== ook initwaarde en rustwaarde */
							ac->cur= ac->sta;
						}
						if(ac->end > ac->sta) ac->go= 1;
						else ac->go= -1;
					}
					break;
				case SN_K2K_OBIPO:
					
					if( sn->evento==0 || (sn->event && (ac->var & SN_K2K_HOLD))) {	/* KEY_IN */
					
						if(ac->go==0) {
							
							if(ac->var & SN_K2K_PREV) mode= set_k2k_interval(-1, ac, lf);
							else mode= set_k2k_interval(1, ac, lf);

							if(mode==1) {	/* extrema bereikt */
								if(ac->var & SN_K2K_CYCLIC) {
									if(ac->var & SN_K2K_PREV) set_k2k_interval(2, ac, lf);
									else set_k2k_interval(0, ac, lf);
									mode= 0;
								}
								else if(ac->var & SN_K2K_PINGPONG) {
									if(ac->var & SN_K2K_PREV) ac->var &= ~SN_K2K_PREV;
									else ac->var |= SN_K2K_PREV;
										
									SWAP(short, ac->end, ac->sta);
									mode= 0;
								}
							}

							if(mode==0) {
								ac->cur= ac->sta;
								if(ac->end > ac->sta) ac->go= 1;
								else ac->go= -1;
							}

							if(mode==2) {
								eventlock= 1;	
							}
							
						}
						else eventlock= 1;
					}
					break;
					
				case SN_LAYERMOVE:
					ob->lay= 1<<(ac->end-1);
					break;
				case SN_STARTTIMER:
					if(lf->timer<0) lf->timer= ac->sta;
					break;
				case SN_ADDLIFE:
					if(ac->poin) add_dupli_life(ac->poin, ob, ac->sta);
					break;
				case SN_REPLACELIFE:
					break;
				case SN_ENDLIFE:
					lf->timer= 0;
					break;
				case SN_LOADFILE:
					strcpy(G.main->name, ac->poin);
					G.f |= G_LOADFILE;
					break;
				case SN_RESTART:
					G.f |= G_RESTART;
					break;
				case SN_QUIT:
					G.f |= G_QUIT;
					break;
				case SN_VISIBLE:
					lf->dflag &= ~LF_DONTDRAW;
					ac->go= 1;
					ac->cur= ac->sta;
					break;
				case SN_REPLACEMESH:
					lf->texmesh= ac->poin;
					break;
				case SN_SETCAMERA:
					/* geen KEY_IN: voor o.a. near sensors en vars */
					evaluate_camera_network(ac->poin, 0);
					break;
					
				case SN_PLAYMOVIE:
					if( sn->evento==0) {	/* KEY_IN */
						extern pAction *playmovie;
						
						playmovie= ac;
						ac->cur= ac->sta;
						start_mainmovie(ac->poin);
					}
					break;
					
				
				case SN_ADD_VARIABLE:
					if( ac->poin) {
						if(eventlock) break;
						sp= ac->poin;
						*sp+= ac->var;
						if(ac->flag & SN_VAR_CYCLIC) {
							if(*sp < ac->sta) *sp= ac->end;
							else if(*sp > ac->end) *sp= ac->sta;
						}
						else {
							CLAMP( *sp , ac->sta, ac->end);
						}
					}
					break;
				case SN_SET_VARIABLE:
					if( ac->poin ) {
						*((short *)ac->poin)= ac->var;
					}
					break;
				case SN_HOLD_VARIABLE:
					if( ac->poin) {
						*((short *)ac->poin)= 1;
						ac->go= 1;
					}
					break;
				case SN_TOG_VARIABLE:
					if( ac->poin) {
						if(eventlock) break;
						sp= ac->poin;
						*sp= *sp?0:1;
					}
					break;
				case SN_IPO_VARIABLE:
					if(ac->poin) {
						retval |= SN_CHANGED;
						cox= *((short *)ac->poin);
						do_obipo(ob, 2*cox, 0);
					
					}
					break;
				}	
				
				ac++;
			}
		}
		
		/* LOPENDE ACTIONS */
		
		ac= sn->actions;
		b= sn->totaction;
		while(b--) {

			if(ac->go) {		/* niet alleen bij ipoos!!! */

				pob= ob;

				switch(ac->action) {

				case SN_X_ROT:
				case SN_Y_ROT:
				case SN_Z_ROT:
					
					ac->cur--;
					if(ac->cur>=0) {
						
						retval |= SN_CHANGED;

						if(ac->action==SN_X_ROT) cox= 0;
						else if(ac->action==SN_Y_ROT) cox= 1;
						else cox= 2;
					
						if( lf->rot[cox] < ac->min) {
							lf->rot[cox]= ( 3686*lf->rot[cox] + 410*ac->min )>>12;
							lf->rotspeed[cox]= (3200*lf->rotspeed[cox])>>12;
						}
						else if( lf->rot[cox] > ac->max) {
							lf->rot[cox]= ( 3686*lf->rot[cox] + 410*ac->max )>>12;
							lf->rotspeed[cox]= (3200*lf->rotspeed[cox])>>12;
						}

					}
					else ac->go= 0;		
					break;

				case SN_NO_DAMAGE:
					if(sn->event==0) {
						lf->dflag &= ~LF_NO_DAMAGE;
						ac->go= 0;
					}
					break;

				case SN_DELTA_OBIPO:
					
					retval |= SN_CHANGED;
	
					/* ipo afhandelen als FLIPPER_OBIPO */
					ac->cur+= ac->go;
					CLAMPTEST(ac->cur, ac->sta, ac->end);
					
					if(lf->type==LF_DYNAMIC) {
							if(ac->flag & SN_IPOFORCE) {
							do_force_obipo(pob, ac->cur, force, omega);
							retval |= SN_DOFORCE+SN_DOOMEGA;
						}
						else {
							do_obipo(pob, ac->cur, 1, speed);
							retval |= SN_DOSPEED;
						}
					}
					else do_obipo(pob, ac->cur, ac->go, 0);
					
					/*  zolang 'hold', dx niet op nul zetten */
					if(ac->cur==ac->end) {
						if(sn->event) ;	/* hold */
						else ac->go= 0;
					}
					break;

				case SN_PLAY_OBIPO:
				
					retval |= SN_CHANGED;
					ac->cur+= ac->go;
					CLAMPTEST(ac->cur, ac->sta, ac->end);
					
					if(lf->type==LF_DYNAMIC) {
						if(ac->flag & SN_IPOFORCE) {
							do_force_obipo(pob, ac->cur, force, omega);
							retval |= SN_DOFORCE+SN_DOOMEGA;
						}
						else {
							do_obipo(pob, ac->cur, ac->go, speed);
							retval |= SN_DOSPEED;
						}
					}
					else do_obipo(pob, ac->cur, 0, 0);

	
					if(ac->cur==ac->end) ac->go= 0;
					break;
				
				case SN_PINGPONG_OBIPO:
				
					retval |= SN_CHANGED;
				
					ac->cur+= ac->go;
					CLAMPTEST(ac->cur, ac->sta, ac->end);
					
					if(lf->type==LF_DYNAMIC) {
						if(ac->flag & SN_IPOFORCE) {
							do_force_obipo(pob, ac->cur, force, omega);
							retval |= SN_DOFORCE+SN_DOOMEGA;
						}
						else {
							do_obipo(pob, ac->cur, ac->go, speed);
							retval |= SN_DOSPEED;
						}
					}
					else do_obipo(pob, ac->cur, 0, 0);
					
					if(ac->cur==ac->sta || ac->cur==ac->end) ac->go= 0;
					break;

				case SN_FLIPPER_OBIPO:
				
					if(ac->cur==ac->end && sn->event==sn->evento ) ;	/* hold */
					else {
					
						retval |= SN_CHANGED;
				
						if(sn->event)  ac->cur+= ac->go;	/* start en hold */
						else ac->cur-= ac->go;

						CLAMPTEST(ac->cur, ac->sta, ac->end);
						
						if(lf->type==LF_DYNAMIC) {
							if(ac->flag & SN_IPOFORCE) {
								do_force_obipo(pob, ac->cur, force, omega);
								retval |= SN_DOFORCE+SN_DOOMEGA;
							}
							else {
								if(sn->event) do_obipo(pob, 1, ac->go, speed);
								else do_obipo(pob, ac->cur, -1, speed);
								retval |= SN_DOSPEED;
							}
						}
						else do_obipo(pob, ac->cur, 0, 0);

						if(ac->cur==ac->sta) ac->go= 0;
					}
					break;
					
				case SN_LOOPSTOP_OBIPO:
				
					retval |= SN_CHANGED;
					
					ac->cur+= ac->go;
					CLAMPTEST(ac->cur, ac->sta, ac->end);
					
					if(lf->type==LF_DYNAMIC) {
						if(ac->flag & SN_IPOFORCE) {
							do_force_obipo(pob, ac->cur, force, omega);
							retval |= SN_DOFORCE+SN_DOOMEGA;
						}
						else {
							do_obipo(pob, ac->cur, ac->go, speed);
							retval |= SN_DOSPEED;
						}
					}
					else do_obipo(pob, ac->cur, 0, 0);
					
					if( sn->event==0) ac->go= 0;		/* no hold */
					else if(ac->cur==ac->end) {	/* end== ook initwaarde en rustwaarde */
						ac->cur= ac->sta;
					}
					break;
				
				case SN_LOOPEND_OBIPO:
				
					retval |= SN_CHANGED;
					
					ac->cur+= ac->go;
					CLAMPTEST(ac->cur, ac->sta, ac->end);
					
					if(lf->type==LF_DYNAMIC) {
						if(ac->flag & SN_IPOFORCE) {
							do_force_obipo(pob, ac->cur, force, omega);
							retval |= SN_DOFORCE+SN_DOOMEGA;
						}
						else {
							do_obipo(pob, ac->cur, ac->go, speed);
							retval |= SN_DOSPEED;
						}
					}
					else do_obipo(pob, ac->cur, 0, 0);
					
					if(sn->event==0) {	/* no hold */
						if(ac->cur==ac->end) ac->go= 0;
					}
					else if(ac->cur==ac->end) {	/* end== ook initwaarde en rustwaarde */
						ac->cur= ac->sta;
					}
					break;	
				
				case SN_K2K_OBIPO:
					retval |= SN_CHANGED;
					ac->cur+= ac->go;
					CLAMPTEST(ac->cur, ac->sta, ac->end);
					lf->cfra= ac->cur;
					
					if(lf->type==LF_DYNAMIC) {
						if(ac->flag & SN_IPOFORCE) {
							do_force_obipo(pob, ac->cur, force, omega);
							retval |= SN_DOFORCE+SN_DOOMEGA;
						}
						else {
							do_obipo(pob, ac->cur, ac->go, speed);
							retval |= SN_DOSPEED;
						}
					}
					else do_obipo(pob, ac->cur, 0, 0);
	
					if(ac->cur==ac->end) ac->go= 0;				
					break;

				case SN_VISIBLE:
					if(sn->event==0) {		/* KEY OUT */
						if(ac->cur<=0 ) {		
							lf->dflag |= LF_DONTDRAW;
							ac->go= 0;
						}
						else ac->cur--;
					}
					break;
					
				case SN_HOLD_VARIABLE:
					if(ac->poin) {
						if(sn->event==0 ) {
							*((short *)ac->poin)= 0;
							ac->go= 0;
						}
					}
					break;
				}
				
				if(ac->action>=100 && ac->action<200) {		/*  ipoos */
					if(ac->flag & SN_IPO_SETVAR) {
						if(ac->poin) *((short *)ac->poin)= ac->cur/2;
					}
					/* pob= ac->poin?ac->poin:ob; */
				}
				
			}
			ac++;
		}
		
		sn++;
	}
	
	/* dyna's twee soorten speed: van force en van ipo */
	/* beide moeten lokaal bij dyna's */

	if(retval) {
		lf->dflag |= LF_DYNACHANGED;
	
		if(retval & SN_DOFORCE) {
			/* force: lokaal */
			ApplyMatrixSV(&ob->obmat, (SVECTOR *)force, &temp);  
	
			forcetot[0]+= temp.vx;
			forcetot[1]+= temp.vy;
			forcetot[2]+= temp.vz;
		}
		if(retval & SN_DOOMEGA) {
			/* eigenlijk ook lokaal, hoe? */
			omegatot[0]+= omega[0];
			omegatot[1]+= omega[1];
			omegatot[2]+= omega[2];
		}
		if(retval & SN_DOSPEED) {
			/* speed: lokaal */
			ApplyMatrixSV(&ob->obmat, (SVECTOR *)speed, &temp);  
	
			lf->speed[0]= temp.vx;
			lf->speed[1]= temp.vy;
			lf->speed[2]= temp.vz;
		}
	}

	return retval;
}


void collision_sensor_input(pObject *ob, pLife *lf)
{
	/* voor snelheid:
	 *  alleen actions die maar 1 x worden afgehandeld
	 */
	pSensor *sn;
	pEvent *ev;
	pAction *ac;
	int event, a, b;
	short *sp;
	
	a= lf->totsens;
	if(a==0) return;
	
	sn= lf->sensors;
	while(a--) {
		
		if(sn->totaction==1) {

			/* EVENTS */
			b= sn->totevent;
			ev= sn->events;
			event= 1;
			while(b--) {
				
				switch(ev->event) {
				case SN_COLLISION:
					if(lf->collision==0 || ev->var>0) {
						event= 0;
					}
					else if(ev->poin && lf->collision!=ev->poin) {
						event= 0;
					}
					break;
	
				case SN_TIMER:
					/* timer in fields */
					if( (lf->timer>>1) != ev->var) event= 0;
					break;
				
				case SN_VAR_EQUAL:
					sp= ev->poin;
					if(sp==0 || *sp!=ev->var) event= 0;
					break;
				case SN_VAR_INTERVAL:
					sp= ev->poin;
					if(sp==0 || *sp<ev->sfac || *sp>ev->sfac1) event= 0;
					break;
				case SN_VAR_CHANGED:
					sp= ev->poin;
					if(sp) {
						if(*sp==ev->var) event= 0;
						else ev->var= *sp;
					}
					else event= 0;
				}
				
				if(event==0) break;
				ev++;
			}
			
			/* ACTIONS STARTEN */
			
			if(event) {
				
				ac= sn->actions;
				b= sn->totaction;
				while(b--) {
				
					/* waarop: */
					switch(ac->action) {
					case SN_LAYERMOVE:
						ob->lay= 1<<(ac->end-1);
						break;
	
					case SN_ADDLIFE:
						if(ac->poin) add_dupli_life(ac->poin, ob, ac->sta);
						break;
	
					case SN_REPLACELIFE:
						break;
	
					case SN_ENDLIFE:
						lf->timer= 0;					
						break;
	
					}
					ac++;
				}
			}
		}
		sn++;
	}
}

