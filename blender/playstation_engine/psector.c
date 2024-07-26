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
 *					 psector.c
 *
 */

#include "psxblend.h"
#include "psxgraph.h"


	// 81= 4096/50
#define DTIME 81
	// als boven: met correctie voor graden en 14 bits
#define RDTIME (110)

#define DYNAFIX		16384
#define SECTORFIX	4096
#define GLOBALFIX	1024
#define GLOBALSHIFT	2

extern short debuginfo;		// main01.c
extern int old_sp;			// main01.c

// exports
void del_dupli_life(pObject *ob);
int (*intersect_func)();
pObject *main_actor;

typedef struct Snijp {
	int labda, minlabda, inpspeed;
	int radius, radiusq, slen;	/* slen: lengte speed */
	pDFace *face;
	pMaterial *ma;
	MATRIX *obmat;				/* als hier iets staat: ook doen! */
	short no[3];				/* als flag==1, staat hier normaal */
	short speedn[3];			/* genormaliseerd */
	ushort ocx, ocy, ocz;		/* van de 'ray' of de life */
	char flag;					/* 0: vlak gesneden, 1: cylinder/sphere */
	char lay;
} Snijp;

void *dupallocN(void *poin)
{
	BHead *bhead;
	void *new=0;
	
	bhead= poin;
	bhead--;
	
	if(bhead->code==DATA || bhead->code==ID_OB || bhead->code==ID_LF) {
		new= mallocN(bhead->len, "dupallocN");
		memcpy(new, poin, bhead->len);
	}
	else printf("illegal dupalloc\n");
	
	return new;
}


void ApplyMatrix12(MATRIX *mat, int *old, int *new)
{
	/* in 12 bits, uit 12 bits */

	// dit is sneller: zie main01.c
	/*
	if((mat->flag & MAT_FAST)) {
		
		if(mat->flag==MAT_ONE) {
			new[0]= old[0] + (mat->t[0]<<GLOBALSHIFT);
			new[1]= old[1] + (mat->t[1]<<GLOBALSHIFT);
		}
		else if(mat->flag==MAT_270) {
			new[0]= -old[1] + (mat->t[0]<<GLOBALSHIFT);
			new[1]= old[0] + (mat->t[1]<<GLOBALSHIFT);
		}
		else if(mat->flag==MAT_180) {
			new[0]= -old[0] + (mat->t[0]<<GLOBALSHIFT);
			new[1]= -old[1] + (mat->t[1]<<GLOBALSHIFT);
		}
		else if(mat->flag==MAT_90) {
			new[0]= old[1] + (mat->t[0]<<GLOBALSHIFT);
			new[1]= -old[0] + (mat->t[1]<<GLOBALSHIFT);
		}
		else PRINT(d, mat->flag);
		
		new[2]= old[2] + (mat->t[2]<<GLOBALSHIFT);
	}
	else {

		ApplyMatrixLV(mat, (VECTOR *)old, (VECTOR *)new);

		new[0]+=  (mat->t[0]<<GLOBALSHIFT);
		new[1]+=  (mat->t[1]<<GLOBALSHIFT);
		new[2]+=  (mat->t[2]<<GLOBALSHIFT);
	}
	*/

	ApplyMatrixLV(mat, (VECTOR *)old, (VECTOR *)new);

		new[0]+=  (mat->t[0]<<GLOBALSHIFT);
		new[1]+=  (mat->t[1]<<GLOBALSHIFT);
		new[2]+=  (mat->t[2]<<GLOBALSHIFT);
	
}

void ApplyMatrix1012(MATRIX *mat, int *old, int *new)
{
	/* in 10 bits, uit 12 bits */

	ApplyMatrixLV(mat, (VECTOR *)old, (VECTOR *)new);

	new[0]= (new[0] + mat->t[0])<<GLOBALSHIFT;
	new[1]= (new[1] + mat->t[1])<<GLOBALSHIFT;
	new[2]= (new[2] + mat->t[2])<<GLOBALSHIFT;
	
}


/* ************** PORTALS ************** */


int passed_portal(int type, short *size, int *vec)
{
	
	switch(type) {
	case PO_XPOS:
		if(vec[0]>size[0]) return 1;
		break;
	case PO_XNEG:
		if(vec[0]<-size[0]) return 1;
		break;
	case PO_YPOS:
		if(vec[1]>size[1]) return 1;
		break;
	case PO_YNEG:
		if(vec[1]<-size[1]) return 1;
		break;
	case PO_ZPOS:
		if(vec[2]>size[2]) return 1;
		break;
	case PO_ZNEG:
		if(vec[2]<-size[2]) return 1;
		break;
	}
	return 0;
}

int passed_portal_safe(int type, short *size, int *vec, short safe)
{
	
	switch(type) {
	case PO_XPOS:
		if(vec[0]+safe>size[0]) return 1;
		break;
	case PO_XNEG:
		if(vec[0]-safe<-size[0]) return 1;
		break;
	case PO_YPOS:
		if(vec[1]+safe>size[1]) return 1;
		break;
	case PO_YNEG:
		if(vec[1]-safe<-size[1]) return 1;
		break;
	case PO_ZPOS:
		if(vec[2]+safe>size[2]) return 1;
		break;
	case PO_ZNEG:
		if(vec[2]-safe<-size[2]) return 1;
		break;
	}
	return 0;
}

/* ******************** LBUF ************************** */


#define LBUFSTEP	4

void add_to_lbuf(pLBuf *lbuf, pObject *ob)
{
	
	if(lbuf->max==0) {
		lbuf->ob= callocN(4*LBUFSTEP, "firstlbuf");
		lbuf->max= LBUFSTEP;
		lbuf->tot= 1;
		lbuf->ob[0]= ob;
	}
	else {
		pObject **obar;
		
		/* dubbels: gewoon toestaan */
		
		if(lbuf->tot==lbuf->max) {
			lbuf->max+= LBUFSTEP;
			obar= callocN(4*lbuf->max, "nlbuf");
			LONGCOPY(obar, lbuf->ob, lbuf->max);
			freeN(lbuf->ob);
			lbuf->ob= obar;
		}
		lbuf->ob[ lbuf->tot ]= ob;
		lbuf->tot++;
		
	}
}

void free_lbuf(pLBuf *lbuf)
{
	if(lbuf->ob) freeN(lbuf->ob);
	lbuf->ob= 0;
	lbuf->max= lbuf->tot= 0;
}

void del_from_lbuf(pLBuf *lbuf, pObject *ob)
{
	int a;
	
	if(lbuf->tot==0 || lbuf->max==0) return;
	
	for(a=0; a<lbuf->tot; a++) {
		
		/* dubbels: wel afvangen */
		
		if(lbuf->ob[a] == ob) {
			lbuf->tot--;
			if(lbuf->tot>a) {
				lbuf->ob[a] = lbuf->ob[ lbuf->tot ];
				lbuf->ob[ lbuf->tot ]= 0;				/* hoeft niet? */
			}
			else lbuf->ob[a]= 0;
		}
		
	}
	
}


/* ****************** OCTREE x:16 y:16 z:16 *************************** */



void init_snijp(pMesh *me, Snijp *sn, int *oldloc, short *speed)
{
	int vec[3];
	short *dvec;
	ushort *size;
	short a, b, min[3], max[3];
	
	/* optimalisering sphere en cyl */
	VECCOPY(sn->speedn, speed);
	sn->slen= Normalises(sn->speedn);

	sn->ocx= 0;		/* voldoende om hierop te testen */
					/* ivm. returns wel pas op 't eind berekenen */
	
//sn->ocx= 0xFFFF;
//sn->ocy= 0xFFFF;
//sn->ocz= 0xFFFF;
//return;

	if(me==0 || me->oc==0 || sn->slen==0) return;

	dvec= me->oc->dvec;
	size= me->oc->size;
	
	vec[0]= oldloc[0]-dvec[0];
	vec[1]= oldloc[1]-dvec[1];
	vec[2]= oldloc[2]-dvec[2];
	
	if(size[0]!=0) {
		a= ((vec[0]-sn->radius)<<4)/size[0];
		b= ((vec[0]-sn->radius+speed[0])<<4)/size[0];
		min[0]= MIN2(a, b);

		a= ((vec[0]+sn->radius)<<4)/size[0];
		b= ((vec[0]+sn->radius+speed[0])<<4)/size[0];
		max[0]= MAX2(a, b);

		if(max[0]<0 || min[0]>15) return;
		if(max[0]>15) max[0]= 15;
		if(min[0]< 0) min[0]=  0;
	}
	else {min[0]=0; max[0]= 15;}
	
	if(size[1]!=0) {
		a= ((vec[1]-sn->radius)<<4)/size[1];
		b= ((vec[1]-sn->radius+speed[1])<<4)/size[1];
		min[1]= MIN2(a, b);

		a= ((vec[1]+sn->radius)<<4)/size[1];
		b= ((vec[1]+sn->radius+speed[1])<<4)/size[1];
		max[1]= MAX2(a, b);

		if(max[1]<0 || min[1]>15) return;
		if(max[1]>15) max[1]= 15;
		if(min[1]< 0) min[1]=  0;
	}
	else {min[1]= 0; max[1]= 15;}
	
	if(size[2]!=0) {
		a= ((vec[2]-sn->radius)<<4)/size[2];
		b= ((vec[2]-sn->radius+speed[2])<<4)/size[2];
		min[2]= MIN2(a, b);

		a= ((vec[2]+sn->radius)<<4)/size[2];
		b= ((vec[2]+sn->radius+speed[2])<<4)/size[2];
		max[2]= MAX2(a, b);

		if(max[2]<0 || min[2]>15) return;
		if(max[2]>15) max[2]= 15;
		if(min[2]< 0) min[2]=  0;
	}
	else {min[2]= 0; max[2]= 15;}
	
	sn->ocx= BROW(min[0], max[0]);
	sn->ocy= BROW(min[1], max[1]);
	sn->ocz= BROW(min[2], max[2]);

}

/* ****************** INSIDE **************************** */


int sector_cliptest(pSector *se, int *vec)
{
	if(se->type==SE_CUBE) {
		if(vec[0]<-se->size[0]) return PO_XNEG;
		if(vec[1]<-se->size[1]) return PO_YNEG;
		if(vec[2]<-se->size[2]) return PO_ZNEG;
		if(vec[0]>se->size[0]) return PO_XPOS;
		if(vec[1]>se->size[1]) return PO_YPOS;
		if(vec[2]>se->size[2]) return PO_ZPOS;
	}
	
	return 0;
}

int sector_inside(pSector *se, int *vec, int *local)	/* vec is globale co */
{

	if(se->ob==0) return 0;
	ApplyMatrix12(&se->ob->imat, vec, local);
	if( sector_cliptest(se, local)==0 ) return 1;
	return 0;
}

pSector *find_sector(int *vec, int *local)		/* algemene find: slow */
{
	pBase *base;

	base= FIRSTBASE;
	while(base) {
		if(base->lay & G.scene->lay) {
			if(base->object->type==OB_SECTOR) {
				
				if(sector_inside(base->object->data, vec, local)) {
					return base->object->data;
				}
			}
		}
		base= base->next;
	}
	return 0;
}

void life_in_sector_test(pObject *ob)
{
	static int outcount= 0;
	pLife *lf;
	pLBuf *lb;
	pSector *se;
	pPortal *po;
	short b, out;
	
	lf= ob->data;
	if(lf->type != LF_DYNAMIC) return;

	/* apart afhandelen voor PROPS! */
	
	/* nog steeds in de huidige sector? */
	if(lf->sector) {
		
		out= sector_cliptest(lf->sector, lf->loc1);

		if(out) {
			/* welke portalen */
			
			if(lf->dflag & LF_TEMPLIFE) lb= 0;
			else if((lf->flag & LF_MAINACTOR)==0) {
				lb= &(lf->sector->lbuf);
				del_from_lbuf( lb, ob);
			}
			else lb= 0;
			
			se= lf->sector;
			b= se->totport;
			po= se->portals;
			while(b--) {
				if(po->sector && passed_portal(po->type, se->size, lf->loc1)) {

					if(sector_inside(po->sector, lf->loc, lf->loc1)) {

						lf->sector= po->sector;
						if(lb) add_to_lbuf( &(lf->sector->lbuf), ob);

						return;
					}
				}
				po++;
			}
			
			/* diagonaal ofzo? */
			se= find_sector(lf->loc, lf->loc1);
			if(se) {
				lf->sector= se;

				if(lb) add_to_lbuf( &(lf->sector->lbuf), ob);
				if(debuginfo) printf("out, but found one\n");
				outcount= 0;
				return;
			}

			/* helaas: */
			
			
			if(lf->dflag & LF_TEMPLIFE) {
				lf->timer= 0;
			}
			else {
				if(outcount==0) printf("out\n");
				outcount++;
				if(outcount>100) {
					G.f |= G_RESTART;
					printf("out: restart\n");
					outcount= 0;
				}
			}
		}
	}

	if(lf->sector==0) {
		lf->sector= find_sector(lf->loc, lf->loc1);
	
		if( (lf->dflag & LF_TEMPLIFE)==0) {
			; /* zie sector.c */
		}
	}
}


void mainactor_in_sector_test()		/* zet G.cursector */
{
	pPortal *po;
	int vec[3], local[3];
	int b, out;

	if(main_actor==0) return;

	/* van 10 naar 12 bits */
	vec[0]= main_actor->obmat.t[0]<<GLOBALSHIFT;
	vec[1]= main_actor->obmat.t[1]<<GLOBALSHIFT;
	vec[2]= main_actor->obmat.t[2]<<GLOBALSHIFT;	

	/* nog steeds in de huidige sector? */
	if(G.cursector) {

		/* lokale coord: (in 10 bits!!!) */
		ApplyMatrix12(&G.cursector->ob->imat, vec, local);

		out= sector_cliptest(G.cursector, local);
		
		if(out) {
			/* welke portalen */
			
			b= G.cursector->totport;
			po= G.cursector->portals;
			while(b--) {
				if(po->sector && passed_portal(po->type, G.cursector->size, local)) {
					
					if(sector_inside(po->sector, vec, local)) {
						G.cursector= po->sector;
						evaluate_camera_network(0, -1); /* init */
						start_mainmovie(G.cursector->name);
						return;
					}
				}
				po++;
			}

			/* helaas */
			G.cursector= 0;
		}
	}

	if(G.cursector==0) {
		G.cursector= find_sector(vec, local);
		if(G.cursector) {
			evaluate_camera_network(0, -1); /* init */
			start_mainmovie(G.cursector->name);
		}
	}
}


void normal_rot_inv(MATRIX *mat, MATRIX *inv, SVECTOR *no)
{
	VECTOR vec;
	int x, y, z;
	
	// KOLOMMEN ANDERS DAN ANDERS!!!
	x= (no->vx*mat->m[0][0] + no->vy*mat->m[0][1] + no->vz*mat->m[0][2])>>12;
	y= (no->vx*mat->m[1][0] + no->vy*mat->m[1][1] + no->vz*mat->m[1][2])>>12;
	z= (no->vx*mat->m[2][0] + no->vy*mat->m[2][1] + no->vz*mat->m[2][2])>>12;
	
	no->vx= (x*inv->m[0][0] + y*inv->m[0][1] + z*inv->m[0][2])>>12;
	no->vy= (x*inv->m[1][0] + y*inv->m[1][1] + z*inv->m[1][2])>>12;
	no->vz= (x*inv->m[2][0] + y*inv->m[2][1] + z*inv->m[2][2])>>12;
	
	// geen normalise: inpspeed bevat scale-informatie
	// VectorNormalS(&vec, no);
	
}


/* ********************** DYNA ************************* */

#define TOLER	80
#define UNSURE	(320<<10)
#define UNSURE1	(128<<8)

/*					  labdacor  labda			*/
/*                 -----x---------x-------		*/
/* 						.		  .				*/
/* geval 1         -----.---->    .				*/
/* geval 2            --.---------.-->			*/
/* geval 3              .     ----.------>		*/
/* geval 4              .  -----> .				*/


int intersect_dface(pDFace *dface, Snijp *sn, int *oldloc, short *speed)
{
	int labda, labdacor, ndist, s, t, inploc, inpspeed;
	int cox=0, coy=1;
	int a, b;

	/* als dface->v3==0 : dface->ocx==0 */
	if( (dface->ocx & sn->ocx)==0 ) return 0;
	if( (dface->ocy & sn->ocy)==0 ) return 0;
	if( (dface->ocz & sn->ocz)==0 ) return 0;

	if((sn->lay & dface->ma->lay)==0) return 0;

	inpspeed= (dface->no[0]*speed[0] + dface->no[1]*speed[1] + dface->no[2]*speed[2]);

	/* single sided! */	
	if(inpspeed < -TOLER) {
	
		inploc= (dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2]);
		
		ndist= (dface->dist - inploc);
		// test 1: mag je delen?
		if(ndist>=0 || (ndist+UNSURE)<=inpspeed) return 0;

		labdacor= ( (ndist+UNSURE)<<4 )/(inpspeed>>10);
		labda= (ndist<<4)/(inpspeed>>10);
		
		// debug test oftie nog leeft
		// if(G.qual & 1) printf("fa %x lab %d min %d\n", dface, labdacor, sn->minlabda);

		/* dit is een soort interval-wiskunde */
		// deze twee regels zijn samen nodig: geval 1 afvangen
		if(labdacor<=sn->minlabda) {
			if(labda<=sn->minlabda) return 0;
		}
		if(labdacor>=sn->labda) {
			if(labda>=sn->labda) return 0;
			// voorkomt hobbelen
		}

		if(dface->proj==1) coy= 2;
		else if(dface->proj==2) {
			cox= 1; coy= 2;
		}

		s= oldloc[cox] + S14MUL(labda, speed[cox]);
		t= oldloc[coy] + S14MUL(labda, speed[coy]);

		if( (dface->v2[cox] - s)*(dface->v2[coy] - dface->v1[coy]) < 
		    (dface->v2[coy] - t)*(dface->v2[cox] - dface->v1[cox]) -UNSURE1)
			return 0;

		if( (dface->v3[cox] - s)*(dface->v3[coy] - dface->v2[coy]) <
			(dface->v3[coy] - t)*(dface->v3[cox] - dface->v2[cox]) -UNSURE1)
			return 0;
		
		if(dface->v4==0) {
			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v3[coy]) <
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v3[cox]) -UNSURE1)
				return 0;
		}
		else {
			if( (dface->v4[cox] - s)*(dface->v4[coy] - dface->v3[coy]) <
				(dface->v4[coy] - t)*(dface->v4[cox] - dface->v3[cox]) -UNSURE1)
				return 0;

			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v4[coy]) <
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v4[cox]) -UNSURE1)
				return 0;
		}

		sn->labda= labdacor;
		sn->inpspeed= (inpspeed)>>12; 
		sn->face= dface;
		sn->flag= 0;
		
		return 1;
	}
	
	return 0;
}

int sphere_sphere_intersect(int *v1, Snijp *sn, int *oldloc, short *speed)
{
	int labda, labdacor, len, bsq, u, disc, rc[3];
	int radius, radiusq;
	
	radius= v1[3]+sn->radius;
	radiusq= radius*radius;
				
	rc[0]= oldloc[0]-v1[0];
	rc[1]= oldloc[1]-v1[1];
	rc[2]= oldloc[2]-v1[2];
	bsq= (rc[0]*sn->speedn[0] + rc[1]*sn->speedn[1] + rc[2]*sn->speedn[2])>>12; 
	u= rc[0]*rc[0] + rc[1]*rc[1] + rc[2]*rc[2] - radiusq;

	disc= bsq*bsq - u;
	
	if(disc>=0) {
		disc= SquareRoot0(disc);
		
		u= -bsq - disc;
		/* mag je delen? */
		if( u < -sn->slen || u> sn->slen) return 0;
		
		labdacor= (u<<14)/(sn->slen);		/* 14 bits */
		u= -bsq + disc;
		labda= (u<<12)/(sn->slen>>2);
		
	}
	else return 0;	

	/* twee gevallen waarbij geen snijpunt is */
	if(labdacor>=sn->labda && labda>=sn->labda) return 0;
	if(labdacor<=sn->minlabda && labda<=sn->minlabda) return 0;

	/* PATCH!!! */
	if(labdacor<0) labdacor/= 2;


	/* snijpunt en normaal */
	/* hier 12 bits */
	sn->no[0]= rc[0] + ((labdacor*speed[0])>>14) ;
	sn->no[1]= rc[1] + ((labdacor*speed[1])>>14) ;
	sn->no[2]= rc[2] + ((labdacor*speed[2])>>14) ;

	/* corrigeren: v1[3]= axsize */
	disc= (v1[3]<<12)/radius;
	sn->no[0]= SMUL(sn->no[0], disc);
	sn->no[1]= SMUL(sn->no[1], disc);
	sn->no[2]= SMUL(sn->no[2], disc);

	sn->labda= labdacor;

	/* inpspeed op lengte brengen: twee keer radius!!! (inpspeed wordt weer met normaal vermenigvuldigd)*/
	/* inspeed: 16 bits */
	radiusq= v1[3]*v1[3];
	sn->inpspeed= ( (sn->no[0]*speed[0] + sn->no[1]*speed[1] + sn->no[2]*speed[2])<<4 )/(radiusq>>12);

	sn->flag= 2;

	return 1;

}



int vertex_sphere_intersect(short *v1, Snijp *sn, int *oldloc, short *speed)
{
	int labda, labdacor, len, bsq, u, disc, rc[3];

	if(sn->slen < 10) return 0;		/* superslow */
	
	rc[0]= oldloc[0]-v1[0];
	rc[1]= oldloc[1]-v1[1];
	rc[2]= oldloc[2]-v1[2];
	bsq= (rc[0]*sn->speedn[0] + rc[1]*sn->speedn[1] + rc[2]*sn->speedn[2])>>12; 
	u= rc[0]*rc[0] + rc[1]*rc[1] + rc[2]*rc[2] - sn->radiusq;

	disc= bsq*bsq - u;
	
	if(disc>=0) {
		disc= SquareRoot0(disc);
		
		u= -bsq - disc;
		/* mag je delen? */
		if( u < -sn->slen || u> sn->slen) return 0;
		
		labdacor= (u<<14)/(sn->slen);		/* 14 bits */
		u= -bsq + disc;
		labda= (u<<12)/(sn->slen>>2);
		
	}
	else return 0;	

	/* twee gevallen waarbij geen snijpunt is */
	if(labdacor>=sn->labda && labda>=sn->labda) return 0;
	if(labdacor<=sn->minlabda && labda<=sn->minlabda) return 0;

	/* snijpunt en normaal */
	/* hier 12 bits */
	sn->no[0]= rc[0] + ((labdacor*speed[0])>>14) ;
	sn->no[1]= rc[1] + ((labdacor*speed[1])>>14) ;
	sn->no[2]= rc[2] + ((labdacor*speed[2])>>14) ;

	sn->labda= labdacor;

	/* inpspeed op lengte brengen: twee keer radius!!! (inpspeed wordt weer met normaal vermenigvuldigd)*/
	/* inspeed: 16 bits */
	sn->inpspeed= ( (sn->no[0]*speed[0] + sn->no[1]*speed[1] + sn->no[2]*speed[2])<<4 )/(sn->radiusq>>12);

	sn->flag= 1;

	return 1;

}

int cylinder_edge_intersect(short *base, short *v2, Snijp *sn, int *oldloc, short *speed)
{
	int osp, rdd, s, t, dist, len, len2, labda, labdacor, rc[3];
	short axis[3], n[3], o[3];
	
	if(sn->slen < 10) return 0;		/* superslow */
	
	axis[0]= v2[0]-base[0];
	axis[1]= v2[1]-base[1];
	axis[2]= v2[2]-base[2];
	len2= Normalises(axis);		/* op PSX:test eerst! */

	rc[0]= oldloc[0]-base[0];
	rc[1]= oldloc[1]-base[1];
	rc[2]= oldloc[2]-base[2];
	
	CrossS(n, speed, axis);
	len= Normalises(n);
	if(len==0) return 0;

	dist= abs( (rc[0]*n[0] + rc[1]*n[1] + rc[2]*n[2])>>12 );

	if( dist>=sn->radius ) return 0;
	
	o[0] = (rc[1] * axis[2] - rc[2] * axis[1])>>12;
	o[1] = (rc[2] * axis[0] - rc[0] * axis[2])>>12;
	o[2] = (rc[0] * axis[1] - rc[1] * axis[0])>>12;

	t= -((o[0]*n[0] + o[1]*n[1] + o[2]*n[2])<<2)/(len);	/* 14 bits */
	
	CrossS(o, n, axis);
	
	osp= (o[0]*speed[0] + o[1]*speed[1] + o[2]*speed[2])>>12;
	rdd= SquareRoot0(sn->radiusq-dist*dist);
	s=  abs( (rdd<<14) / osp);

	labdacor= (t-s);
	labda= (t+s);
	
	/* twee gevallen waarbij geen snijpunt is */
	if(labdacor>=sn->labda && labda>=sn->labda) return 0;
	if(labdacor<=sn->minlabda && labda<=sn->minlabda) return 0;
	
	/* normaalvector berekenen */
	/* snijpunt: */
	
	rc[0]= rc[0] + ((labdacor*speed[0])>>14);
	rc[1]= rc[1] + ((labdacor*speed[1])>>14);
	rc[2]= rc[2] + ((labdacor*speed[2])>>14);
	
	s= (rc[0]*axis[0] + rc[1]*axis[1] + rc[2]*axis[2])>>12 ;
	
	if(s<0 || s>len2) return 0;
	
	/* tot aan de laatste return niets in de sn struct invullen! */	
	
	/* 12 bits */
	sn->no[0]= rc[0] - ((s*axis[0])>>12);
	sn->no[1]= rc[1] - ((s*axis[1])>>12);
	sn->no[2]= rc[2] - ((s*axis[2])>>12);
		
	sn->labda= labdacor;
	
	/* 16 bits */
	sn->inpspeed= ( (sn->no[0]*speed[0] + sn->no[1]*speed[1] + sn->no[2]*speed[2])<<4 )/(sn->radiusq>>12);

	sn->flag= 1;
	
	return 1;
}

int intersect_dface_cyl(pDFace *dface, Snijp *sn, int *oldloc, short *speed)
{
	short *v1, *v2, *v3, *v4;
	int labda, labdacor, ndist, ndistr, s, t, inploc, inpspeed;
	int out;
	short cox=0, coy=1, ok, ed, cytest;
	int a, b;

	/* als dface->v3==0 : dface->ocx==0 */
	if( (dface->ocx & sn->ocx)==0 ) return 0;
	if( (dface->ocy & sn->ocy)==0 ) return 0;
	if( (dface->ocz & sn->ocz)==0 ) return 0;

	if((sn->lay & dface->ma->lay)==0) return 0;

	inpspeed= (dface->no[0]*speed[0] + dface->no[1]*speed[1] + dface->no[2]*speed[2]);

	/* single sided! */	
	if(inpspeed < -TOLER) {
	
		inploc= (dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2]);
		
		ndist= (dface->dist - inploc);
		ndistr= ndist+(sn->radius<<14);
		// test 1: mag je delen?
		if(ndist>=0 || (ndistr)<=inpspeed) return 0;

		labda= (ndist<<4)/(inpspeed>>10);	/* voor s-t */
		labdacor= (ndistr<<4 )/(inpspeed>>10);
		
		// debug test oftie nog leeft
		// if(G.qual & 1) printf("fa %x lab %d min %d\n", dface, labdacor, sn->minlabda);


		/* twee gevallen waarbij geen snijpunt is */
		if(labdacor>=sn->labda && labda>=sn->labda) return 0;
		if(labdacor<=sn->minlabda && labda<=sn->minlabda) return 0;
	
		if(dface->proj==1) coy= 2;
		else if(dface->proj==2) {
			cox= 1; coy= 2;
		}

		/* testen oftie exact het vlak snijdt */
		if(labdacor<0) {
			/* geen labda, wel met correctie zodat snijp binnen vlak valt */
			/* dit is in feite de loodrechte projektie */
			s= oldloc[cox] - S14MUL(sn->radius, dface->no[cox]);
			t= oldloc[coy] - S14MUL(sn->radius, dface->no[coy]);
			labdacor= 0;
		}
		else {
			/* met correctie zodat snijp binnen vlak valt */
			s= oldloc[cox] + ((labdacor*speed[cox] - sn->radius*dface->no[cox])>>14);
			t= oldloc[coy] + ((labdacor*speed[coy] - sn->radius*dface->no[coy])>>14);
		}

		v1= dface->v1;
		v2= dface->v2;
		v3= dface->v3;
		v4= dface->v4;
		
		cytest= 0;
		out= (v2[cox] - s)*(v2[coy] - v1[coy]) - (v2[coy] - t)*(v2[cox] - v1[cox]);
		if(out > -TOLER) {
			out= (v3[cox] - s)*(v3[coy] - v2[coy]) - (v3[coy] - t)*(v3[cox] - v2[cox]);
			if(out > -TOLER) {
				
				ok= 0;
				
				if(v4==0) {
					out= (v1[cox] - s)*(v1[coy] - v3[coy]) - (v1[coy] - t)*(v1[cox] - v3[cox]);
					if(out > -TOLER) ok= 1;
					else cytest= 3;
				}
				else {
					out= (v4[cox] - s)*(v4[coy] - v3[coy]) - (v4[coy] - t)*(v4[cox] - v3[cox]);

					if(out > -TOLER) {
						out= (v1[cox] - s)*(v1[coy] - v4[coy]) - (v1[coy] - t)*(v1[cox] - v4[cox]);

						if(out > -TOLER) ok= 1;
						else cytest= 5;
					}
					else cytest= 4;
				}
				if(ok) {

					sn->labda= labdacor;
					sn->inpspeed= inpspeed>>12;
					sn->face= dface;
					sn->flag= 0;
					return 1;
				}
			}
			else cytest= 2;
		}
		else cytest= 1;

		/* edges (= cylinders) testen */
		ok= 0;
		ed= dface->edcode;
		
		
		switch(cytest) {
		case 1:
			if( (ed & DF_V1V2) && cylinder_edge_intersect(v1, v2, sn, oldloc, speed) )
				ok= 1;
			break;
		case 2:
			if( (ed & DF_V2V3) && cylinder_edge_intersect(v2, v3, sn, oldloc, speed) )
				ok= 1;
			break;
		case 3:
			if( (ed & DF_V3V1) && cylinder_edge_intersect(v3, v1, sn, oldloc, speed) )
				ok= 1;
			break;
		case 4:
			if( (ed & DF_V3V4) && cylinder_edge_intersect(v3, v4, sn, oldloc, speed) )
				ok= 1;
			break;
		case 5:
			if( (ed & DF_V4V1) && cylinder_edge_intersect(v4, v1, sn, oldloc, speed) )
				ok= 1;
			break;
		}

		if(ok) sn->face= dface;
		
		if(ok==0) {
			if( (ed & DF_V1) && vertex_sphere_intersect(v1, sn, oldloc, speed)) {
				sn->face= dface;
				
				return 1;
			}
			if( (ed & DF_V2) && vertex_sphere_intersect(v2, sn, oldloc, speed)) {
				sn->face= dface;
				
				return 1;
			}
			if( (ed & DF_V3) && vertex_sphere_intersect(v3, sn, oldloc, speed)) {
				sn->face= dface;
				
				return 1;
			}
			if(v4) if( (ed & DF_V4) && vertex_sphere_intersect(v4, sn, oldloc, speed)) {
				sn->face= dface;
				
				return 1;
			}
			
		}
		
		/* PATCH: onderzoeken hoe en wat en waarom: extreem negatieve labda's */
		/* waarschijnlijk iets van doen met meerdere snijps? */
		
		if(sn->labda<sn->minlabda) sn->labda= sn->minlabda;

		return ok;


	}
	
	return 0;
}

short intersect_dynalife(pObject *ob, pLife *lf, Snijp *sn)
{
	pObject *obs;
	pLife *lfs;
	int mindist, loc1[4];		/* loc1 is incl axsize */
	short a, b, found= 0;

	/* nodig voor genormaliseerde speed */
	init_snijp(0, sn, lf->oldloc1, lf->speed1);

	if(sn->slen < 10) return 0;
	
	a= G.totlife;
	while(a--) {
	
		obs= G.lifebuf[a];
		if(obs->type!=OB_CAMERA) {
			lfs= obs->data;
			if(lf!=lfs && lfs->collision!=ob && lf->from!=obs && lfs->type==LF_DYNAMIC && (lfs->dflag & LF_TEMPLIFE)==0) {
				
				/* manhattan pre-test */
				mindist= 3*(sn->radius+lfs->axsize);
				
				if( abs(lf->loc[0]-lf->loc[0]) > mindist ) continue;
				if( abs(lf->loc[1]-lf->loc[1]) > mindist ) continue;
				if( abs(lf->loc[2]-lf->loc[2]) > mindist ) continue;
				
				/* gebruik lokale sector coords */
				ApplyMatrix12(&lf->sector->ob->imat, lfs->loc, loc1);
				loc1[3]= lfs->axsize;
				
				/* truuk: bollen met stralen r1 en r2 snijden is equiv. 
				   met lijn snijden met bol van straal r1+r2 ! */
				
				if(sphere_sphere_intersect(loc1, sn, lf->oldloc1, lf->speed1)) {
					found= 1;
					sn->ma= &G.defaultmaterial;
					sn->obmat= 0;
					sn->face= 0;
					
					lf->collision= obs;
					lfs->collision= ob;
				}
			}
		}
	}
	
	/* hier geen patch: in ruil voor labdacor delen door 2 (sphere_sphere_intersect) */
		
	return found;
}


int intersect_prop(pSector *se, pLife *lf, Snijp *sn)
{
	/* botst Life *lf tegen een van de props uit *se ? */
	/* ook se doorgeven: portals */
	pObject *ob;
	pLife *lfs;
	pDFace *dface;
	int oldloc2[3], loc2[3];
	int a, b, found= 0;
	short speed2[3];
	
	for(b=0; b<se->lbuf.tot; b++) {
		ob= se->lbuf.ob[b];
		
		if(ob->lay & G.scene->lay) {	/* ivm layer event */

			lfs= ob->data;
	
			if(lf!=lfs && lfs->dynamesh && lfs->type==LF_PROP) {
			
				/* transformeren naar life coordinaten */
				ApplyMatrix12(&ob->imat, lf->loc, loc2);
				ApplyMatrix12(&ob->imat, lf->oldloc, oldloc2);
	
				speed2[0]= (loc2[0]-oldloc2[0]);
				speed2[1]= (loc2[1]-oldloc2[1]);
				speed2[2]= (loc2[2]-oldloc2[2]);
				

				init_snijp(lfs->dynamesh, sn, oldloc2, speed2);
				if(sn->ocx) {

					dface= lfs->dynamesh->dface;
					a= lfs->dynamesh->totface;

					while(a--) {
						if( intersect_func(dface, sn, oldloc2, speed2)) {
							sn->obmat= &(ob->obmat);
							lf->collision= ob;	
							lfs->collision= ob;		/* alleen waar/nietwaar */
							found= 1;
						}
						dface++;
					}
				}
			}
		}
	}
	
	return found;
}

void force_from_prop(pLife *lf, Snijp *sn)	/* maar 1 tegelijk!!! */
{
	pObject *ob;
	pLife *lfs;
	pSector *se;
	pDFace *dface;
	int fac, oldloc2[3], loc2[3];
	short speed2[3];
	short a, b, found, *no;

	se= lf->sector;

	for(b=0; b<se->lbuf.tot; b++) {
		ob= se->lbuf.ob[b];
		
		if(G.scene->lay & ob->lay) {
			lfs= ob->data;
				
			if(lf!=lfs && (lfs->dflag & LF_DYNACHANGED)) {
				if(lfs->type==LF_PROP && lfs->dynamesh) {
					
					sn->minlabda= 0;
					sn->labda= DYNAFIX;
					found= 0;
					
					/* werken aan de hand van de 'virtuele' vorige positie van life */
					ApplyMatrix12(&ob->imat, lf->loc, loc2);
					ApplyMatrix12(&lfs->oldimat, lf->oldloc, oldloc2);
					
					speed2[0]= loc2[0]-oldloc2[0];
					speed2[1]= loc2[1]-oldloc2[1];
					speed2[2]= loc2[2]-oldloc2[2];

					init_snijp(lfs->dynamesh, sn, oldloc2, speed2);
					if(sn->ocx) {

						dface= lfs->dynamesh->dface;
						a= lfs->dynamesh->totface;
	
						while(a--) {
							if( intersect_func(dface, sn, oldloc2, speed2)) {
								found= 1;
							}
							dface++;
						}
					
						if(found) {
		
							/* de bots loc */
							oldloc2[0]+= S14MUL(sn->labda, speed2[0]);
							oldloc2[1]+= S14MUL(sn->labda, speed2[1]);
							oldloc2[2]+= S14MUL(sn->labda, speed2[2]);
							
							// nieuwe speed, hier refl.demping: -1.0: parrallel aan vlak, -2.0: zuivere botsing
							// fac wordt zo 12 bits
							fac= S14MUL( -4096-sn->face->ma->ref, sn->inpspeed);
		
							/* oplossing: hier stond +=, komt neer op min speed2 !!! */
							/* min speed2= exact de speed van het botsvlak!!! */
							
							if(sn->flag==0) no= sn->face->no; else no= sn->no;
							
							speed2[0]= S14MUL(fac, no[0]);
							speed2[1]= S14MUL(fac, no[1]);
							speed2[2]= S14MUL(fac, no[2]);
							
							/* de virtuele oldloc */
							oldloc2[0]-= S14MUL(sn->labda, speed2[0]);
							oldloc2[1]-= S14MUL(sn->labda, speed2[1]);
							oldloc2[2]-= S14MUL(sn->labda, speed2[2]);
		
							/* de nieuwe eindlocatie */
							loc2[0]= oldloc2[0]+speed2[0];
							loc2[1]= oldloc2[1]+speed2[1];
							loc2[2]= oldloc2[2]+speed2[2];
		
							ApplyMatrix12(&ob->obmat, loc2, lf->loc);
						   
							ApplyMatrixSV(&ob->obmat, (SVECTOR *)speed2, (SVECTOR *)lf->speed);
							
							/* oldloc niet meer nodig */
		
							return;
						}
					}
				}
			}
		}
	}
}

int test_visibility(int *lookat, int *from, pLife *lf, pSector *se)
{
	Snijp sn;
	pMesh *me;
	pDFace *dface;
	int fac, loc1[3], oldloc1[3];
	short speed1[3];
	short a, found;
	
	// lookat en from zijn 12 bits int 
	
	if(se==0 || se->dynamesh==0) return 0;
	
	/* transformeren naar lokale sector coords en intersecten */

	ApplyMatrix12(&se->ob->imat, from, loc1);
	ApplyMatrix12(&se->ob->imat, lookat, oldloc1);
	
	/* hier oppassen met shorts! */
	
	fac= loc1[0]-oldloc1[0];
	speed1[0]= CLAMPIS(fac, -32767, 32767);
	fac= loc1[1]-oldloc1[1];
	speed1[1]= CLAMPIS(fac, -32767, 32767);
	fac= loc1[2]-oldloc1[2];
	speed1[2]= CLAMPIS(fac, -32767, 32767);

	me= se->dynamesh;

	sn.minlabda= sn.radius= 0;	/* radius op nul voor init_snijp */
	sn.labda= DYNAFIX;
	sn.obmat= 0;
	sn.lay= lf->lay;
	found= 0;
	
	init_snijp(me, &sn, oldloc1, speed1);
	if(sn.ocx) {
	
		dface= me->dface;
		a= me->totface;
		
		while(a--) {
			if( intersect_dface(dface, &sn, oldloc1, speed1)) found= 1;
			dface++;
		}
		
		if(found) {
			
			/* de bots loc */
			oldloc1[0]+= (sn.labda*speed1[0])>>14;
			oldloc1[1]+= (sn.labda*speed1[1])>>14;
			oldloc1[2]+= (sn.labda*speed1[2])>>14;

			/* reflectie berekenen: fac en no zijn 14 bits, speed 12 bits */
			fac= -sn.inpspeed;		// 2* is hieronder gecompenseerd
			speed1[0]+= (fac*sn.face->no[0])>>15;
			speed1[1]+= (fac*sn.face->no[1])>>15;
			speed1[2]+= (fac*sn.face->no[2])>>15;
			
			fac= (DYNAFIX - sn.labda);
			
			/* endloc */
			oldloc1[0]+= (fac*speed1[0])>>14;
			oldloc1[1]+= (fac*speed1[1])>>14;
			oldloc1[2]+= (fac*speed1[2])>>14;
			
			/* blenden */
			loc1[0]= (15*loc1[0] + oldloc1[0])>>4;
			loc1[1]= (15*loc1[1] + oldloc1[1])>>4;
			loc1[2]= (15*loc1[2] + oldloc1[2])>>4;
			
			ApplyMatrix12(&se->ob->obmat, loc1, from);

			return 1;
		}

	}
	
	return 0;
}



void collision_detect(pObject *ob, pLife *lf)
{
	/* een soort reetrees routine: zoek dichtstbijzijnde snijpunt */
	/* werken met de lokale life co's */
	Snijp sn;
	pMesh *me;
	pPortal *po;
	pSector *se;
	pDFace *dface, *from=0;
	int *oldloc1, fac, len;
	short a, b, found=1, transback=0, colcount=0;
	short *speed1, *no;
	
	lf->collision= 0;

	se= lf->sector;
	me= se->dynamesh;
	if(me==0) return;

	sn.minlabda= 0;
	sn.lay= lf->lay;
	
	if(lf->flag & LF_SPHERE) {
		sn.radius= lf->axsize;
		sn.radiusq= sn.radius*sn.radius;
		intersect_func= intersect_dface_cyl;
	}
	else {
		sn.radius= TOLER;		/* voor octree */
		intersect_func= intersect_dface;
	}
	
	/* beetje speedup */
	oldloc1= lf->oldloc1;
	speed1= lf->speed1;

	while(found) {

		sn.labda= DYNAFIX;
		sn.obmat= 0;
		found= 0;
		
		init_snijp(me, &sn, oldloc1, speed1);
		if(sn.ocx) {
		
			dface= me->dface;
			a= me->totface;

			while(a--) {
				if(dface!=from) {
					if( intersect_func(dface, &sn, oldloc1, speed1)) found= 1;
				}
				dface++;
			}
			
			if(found) lf->collision= se->ob;
		}
		
		/* de prop lifes */
		/* tijdelijk lopen we ze allemaal af */
		/* toch maar vast de globale opnieuw berekenen... */
		if(transback) {
			life_from_inv_sector_co(lf);
			transback= 0;
		}
		
		if(se->lbuf.tot) found |= intersect_prop(lf->sector, lf, &sn);
		
		found |= intersect_dynalife(ob, lf, &sn);

		/* als loc buiten sector ligt, doen ook vlakken van portals mee */
		b= se->totport;
		po= se->portals;
		while(b--) {
		
				/* safe: voor krappe sectoren waar andere in steken! */ 

			if(po->sector && po->sector->dynamesh && passed_portal_safe(po->type, se->size, lf->loc1, lf->axsize)) {
				int oldloc2[3], loc2[3];
				short speed2[3];

				/* toch maar vast de globale opnieuw berekenen want... */
				if(transback) {
					life_from_inv_sector_co(lf);
					transback= 0;
				}
				
				/* ... we moeten transformeren naar nieuwe locale coordinaten */
				
				ApplyMatrix12(&po->ob->imat, lf->loc, loc2);
				ApplyMatrix12(&po->ob->imat, lf->oldloc, oldloc2);
		
				speed2[0]= (loc2[0]-oldloc2[0]);
				speed2[1]= (loc2[1]-oldloc2[1]);
				speed2[2]= (loc2[2]-oldloc2[2]);

				init_snijp(po->sector->dynamesh, &sn, oldloc2, speed2);
				if(sn.ocx) {

					dface= po->sector->dynamesh->dface;
					a= po->sector->dynamesh->totface;

					while(a--) {
						if(dface!=from) {
							if( intersect_func(dface, &sn, oldloc2, speed2)) {
								sn.obmat= &(po->ob->obmat);
								lf->collision= po->ob;
								found= 1;
							}
						}
						dface++;
					}
				}

			}
			po++;
		}
		
		if(found) {

			colcount++;

			transback= 1;
			from= sn.face;
			
			/* de bots loc */
			lf->colloc[0]= oldloc1[0]+ S14MUL(sn.labda, speed1[0]);
			lf->colloc[1]= oldloc1[1]+ S14MUL(sn.labda, speed1[1]);
			lf->colloc[2]= oldloc1[2]+ S14MUL(sn.labda, speed1[2]);
			
			if(sn.flag==0) {
				no= sn.face->no; 
				sn.ma= sn.face->ma;
			}
			else {
				no= sn.no;
				if(sn.flag==1) sn.ma= sn.face->ma;
			}

			// nieuwe speed, hier refl.demping: -1.0: parrallel aan vlak, -2.0: zuivere botsing
			// fac wordt 12 bits
			fac= S14MUL( -4096-sn.ma->ref, sn.inpspeed);

			/* pas op: als sn.obmat (in andere object) de sn.face->no[] roteren */
			if(sn.obmat) {
				SVECTOR nor;
				
				nor.vx= no[0];
				nor.vy= no[1];
				nor.vz= no[2];
				
				normal_rot_inv(sn.obmat, &lf->sector->ob->imat, &nor);
				speed1[0]+= S14MUL(fac, nor.vx);
				speed1[1]+= S14MUL(fac, nor.vy);
				speed1[2]+= S14MUL(fac, nor.vz);

			}
			else {
				speed1[0]+= S14MUL(fac, no[0]);
				speed1[1]+= S14MUL(fac, no[1]);
				speed1[2]+= S14MUL(fac, no[2]);
			}
			
			/* de virtuele oldloc */
			oldloc1[0]= lf->colloc[0] - S14MUL(sn.labda, speed1[0]);
			oldloc1[1]= lf->colloc[1] - S14MUL(sn.labda, speed1[1]);
			oldloc1[2]= lf->colloc[2] - S14MUL(sn.labda, speed1[2]);
			
			if(colcount>3) if( sn.labda <= sn.minlabda) found= 0;

			if(sn.minlabda>=DYNAFIX) found= 0;
			else sn.minlabda= sn.labda;

		}
	}
	
	/* als er gebotst is: terug transformeren naar globale coordinaten */
	if(transback) life_from_inv_sector_co(lf);

	/* als laatste: de kracht die static life uitoefenen kan */
	force_from_prop(lf, &sn);
	
	// if(colcount) {
	// 	a= abs(sn.inpspeed>>5);
	// 	if(a>5) play_sample(METALBUMP, 16, 0, a,a);
	// }
}

/* ******************* hele fh spul: *********************** */

#define FH_SECTOR	1
#define FH_PROP		2


int intersect_fh(pDFace *dface, int *oldloc, Snijp *sn)
{
	int s, t, tlab, inploc, inpspeed, ndist;
	
	/* speed: 0, 0, -1 */
	
	/* als dface->v3==0 : dface->ocx==0 */
	if( (dface->ocx & sn->ocx)==0 ) return 0;
	if( (dface->ocy & sn->ocy)==0 ) return 0;
	if( (dface->ocz & sn->ocz)==0 ) return 0;

	inpspeed= -(dface->no[2]<<12);

	/* single sided! */	
	if(inpspeed < -TOLER) {
		
		inploc= dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2];
		
		ndist= (dface->dist - inploc);
		// test 1: mag je delen?
		if(ndist>=0 || (ndist)< (inpspeed<<5) ) return 0;

		// labda is hier 12 bits!!!
		tlab= (ndist<<2)/(inpspeed>>10);

		if(tlab<0 || tlab >= sn->labda) return 0;
		
		s= oldloc[0];
		t= oldloc[1];

		if( (dface->v2[0] - s)*(dface->v2[1] - dface->v1[1]) < 
		    (dface->v2[1] - t)*(dface->v2[0] - dface->v1[0]) -UNSURE1)
			return 0;

		if( (dface->v3[0] - s)*(dface->v3[1] - dface->v2[1]) <
			(dface->v3[1] - t)*(dface->v3[0] - dface->v2[0]) -UNSURE1)
			return 0;
		
		if(dface->v4==0) {
			if( (dface->v1[0] - s)*(dface->v1[1] - dface->v3[1]) <
				(dface->v1[1] - t)*(dface->v1[0] - dface->v3[0]) -UNSURE1)
				return 0;
		}
		else {
			if( (dface->v4[0] - s)*(dface->v4[1] - dface->v3[1]) <
				(dface->v4[1] - t)*(dface->v4[0] - dface->v3[0]) -UNSURE1)
				return 0;

			if( (dface->v1[0] - s)*(dface->v1[1] - dface->v4[1]) <
				(dface->v1[1] - t)*(dface->v1[0] - dface->v4[0]) -UNSURE1)
				return 0;
		}
		
		sn->labda= tlab;
		sn->face= dface;
		if(tlab<= dface->ma->fh_dist) {
			sn->flag= FH_SECTOR;
			return 1;
		}
		return 0;
	}
	
	return 0;
}

int intersect_fh_rot(pDFace *dface, int *oldloc, Snijp *sn, short *speed)
{
	int s, t, inploc, inpspeed, ndist, tlab;
	short cox=0, coy=1;
	
	/* speed: 0, 0, -1 (maar dan geroteerd) */
	
	/* als dface->v3==0 : dface->ocx==0 */
	if( (dface->ocx & sn->ocx)==0 ) return 0;
	if( (dface->ocy & sn->ocy)==0 ) return 0;
	if( (dface->ocz & sn->ocz)==0 ) return 0;

	inpspeed= -dface->no[0]*speed[0] - dface->no[1]*speed[1] - dface->no[2]*speed[2];

	/* single sided! */	
	if(inpspeed < -TOLER) {
		
		inploc= dface->no[0]*oldloc[0] + dface->no[1]*oldloc[1] + dface->no[2]*oldloc[2];
		
		ndist= (dface->dist - inploc);
		// test 1: mag je delen?
		if(ndist>=0 || (ndist)<(inpspeed<<5)) return 0;

		// labda is hier 12 bits!!!
		tlab= (ndist<<2)/(inpspeed>>10);

		if(tlab<0 || tlab>= sn->labda) return 0;
		
		if(dface->proj==1) coy= 2;
		else if(dface->proj==2) {
			cox= 1; coy= 2;
		}		
		
		s= oldloc[cox] + ((tlab*speed[cox])>>12);
		t= oldloc[coy] + ((tlab*speed[coy])>>12);

		if( (dface->v2[cox] - s)*(dface->v2[coy] - dface->v1[coy]) < 
		    (dface->v2[coy] - t)*(dface->v2[cox] - dface->v1[cox]) -UNSURE1)
			return 0;

		if( (dface->v3[cox] - s)*(dface->v3[coy] - dface->v2[coy]) <
			(dface->v3[coy] - t)*(dface->v3[cox] - dface->v2[cox]) -UNSURE1)
			return 0;
		
		if(dface->v4==0) {
			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v3[coy]) <
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v3[cox]) -UNSURE1)
				return 0;
		}
		else {
			if( (dface->v4[cox] - s)*(dface->v4[coy] - dface->v3[coy]) <
				(dface->v4[coy] - t)*(dface->v4[cox] - dface->v3[cox]) -UNSURE1)
				return 0;

			if( (dface->v1[cox] - s)*(dface->v1[coy] - dface->v4[coy]) <
				(dface->v1[coy] - t)*(dface->v1[cox] - dface->v4[cox]) -UNSURE1)
				return 0;
		}

		sn->labda= tlab;
		sn->face= dface;
		if(tlab<= dface->ma->fh_dist) {
			sn->flag= FH_PROP;
		}
		return 1;
	}
	
	return 0;
}


void update_Fheight(pLife *lf, short *force)
{
	/* alleen anti-zwaartekracht: pas op met rotaties. Normaal vlak= F_afstoot */
	pLife *lfs;
	pDFace *dface;
	pMaterial *ma;
	pSector *se;
	pObject *propob;
	Snijp sn;
	int alpha, fac, loc2[3], oldloc2[3], loc1[3];
	short a, b, speed[3], no[3], *mat;

	// force is 14 bits

	lf->contact= 0;
	lf->floor= 0;

	/* is er wel 'n Fh vlak in mesh ? */
	if(lf->sector->dynamesh==0) return;

	sn.labda= 32*4096;
	sn.face= 0;
	sn.flag= 0;
	sn.radius= TOLER;
	mat= lf->sector->ob->obmat.m[0];
	
	speed[0]=speed[1]= 0;
	speed[2]= -7*4096;
	init_snijp(lf->sector->dynamesh, &sn, lf->loc1, speed);

	dface= lf->sector->dynamesh->dface;
	a= lf->sector->dynamesh->totface;

	while(a--) {
		if(dface->ma->fh_dist) {
			
			/* loodrechte projektie */
			if( intersect_fh(dface, lf->loc1, &sn) ) {
				
				/* geen break: beste fh! */
			}
		}
		
		dface++;
	}

	/* of mogelijk de props? */
	if(sn.flag== 0 && lf->sector->lbuf.tot) {

		se= lf->sector;
		for(b=0; b<se->lbuf.tot; b++) {
			propob= se->lbuf.ob[b];
			
			lfs= propob->data;
			
			if(lf!=lfs && lfs->dynamesh) {
				/* is er wel 'n Fh vlak in mesh ? */
				
				/* transformeren naar life coordinaten */
				ApplyMatrix12(&propob->imat, lf->loc, loc1);
				
				dface= lfs->dynamesh->dface;
				a= lfs->dynamesh->totface;

				/* psx matrix ANDERSOM!!! */
				speed[0]= -propob->obmat.m[0][2]<<3;
				speed[1]= -propob->obmat.m[1][2]<<3;
				speed[2]= -propob->obmat.m[2][2]<<3;
				
				init_snijp(lfs->dynamesh, &sn, loc1, speed);
				
				speed[0]= propob->obmat.m[0][2];
				speed[1]= propob->obmat.m[1][2];
				speed[2]= propob->obmat.m[2][2];				
				
				while(a--) {
					if(dface->ma->fh_dist) {
					
						/* loodrechte projektie */
						if( intersect_fh_rot(dface, loc1, &sn, speed) ) {
							
							/* normaal roteren volgens life */
							mat= propob->obmat.m[0];
							
							if(sn.flag == FH_PROP) break;
						}
					}
					
					dface++;
				}
				if(sn.flag == FH_PROP) break;
			}
		}
	}
	
	if(sn.face) {
		
		/* psx matrix ANDERSOM!!! */
		if(FALSE) {
			/* normaal roteren naar world coords: 'wipeout' wandjes, duwen in richting normaal vlak */
			no[0]= (sn.face->no[0]*mat[0] + sn.face->no[1]*mat[1] + sn.face->no[2]*mat[2])>>14;
			no[1]= (sn.face->no[0]*mat[3] + sn.face->no[1]*mat[4] + sn.face->no[2]*mat[5])>>14;
			no[2]= (sn.face->no[0]*mat[6] + sn.face->no[1]*mat[7] + sn.face->no[2]*mat[8])>>14;
		}
		else {
			/* alleen loodrecht */
			no[0]= mat[2];
			no[1]= mat[5];
			no[2]= mat[8];
		}

		/* floor locatie voor schaduw: altijd relatief! */
		lf->floorloc[0]= - (sn.labda*no[0])>>14;	/* naar 10 bits shiften */
		lf->floorloc[1]= - (sn.labda*no[1])>>14;
		lf->floorloc[2]= - (sn.labda*no[2])>>14;
		
		lf->floor= sn.face;
		
		if(sn.flag) {
		
			ma= sn.face->ma;
			lf->contact= ma;
			
			/* labda==0.0: exact op afstand dist */
			sn.labda= 4096 - (sn.labda<<12)/ma->fh_dist;
	
			sn.labda= SMUL(sn.labda, sn.face->ma->fh_int);
			
			/* force 14 bits */
			force[0]+= S10MUL(sn.labda, no[0]);
			force[1]+= S10MUL(sn.labda, no[1]);
			force[2]+= S10MUL(sn.labda, no[2]);
	
			/* extra fh friction, alleen in richting normaal */
			// no is hier 12 bits!
			alpha= (lf->speed[0]*no[0] + lf->speed[1]*no[1] + lf->speed[2]*no[2])>>12;
			alpha= SMUL(alpha, ma->fh_frict);
			
			lf->speed[0] -= S12MUL(alpha, no[0]);
			lf->speed[1] -= S12MUL(alpha, no[1]);
			lf->speed[2] -= S12MUL(alpha, no[2]);
			
			if( (sn.flag==FH_PROP)  && (lfs->dflag & LF_DYNACHANGED) && lfs->type==LF_PROP) {
	
				/* werken aan de hand van de 'virtuele' vorige positie van life */
				ApplyMatrix12(&propob->imat, lf->loc, loc2);
				ApplyMatrix12(&lfs->oldimat, lf->loc, oldloc2);
				
				speed[0]= oldloc2[0]-loc2[0];
				speed[1]= oldloc2[1]-loc2[1];
				speed[2]= oldloc2[2]-loc2[2];
				
				/* terug transformeren: mat3! */
				ApplyMatrixSV(&propob->obmat, (SVECTOR *)speed, (SVECTOR *)no);
				
				/* dit is de 'stilstaande' speed */
				fac= SMUL(ma->fh_xyfrict, lf->frictfac);
				alpha= 4096 - fac;
	
				lf->speed[0]= (alpha*lf->speed[0] + fac*no[0])>>12;
				lf->speed[1]= (alpha*lf->speed[1] + fac*no[1])>>12;
				lf->speed[2]= (alpha*lf->speed[2] + fac*no[2])>>12;
						
			}
			else {
				alpha= 4096 - SMUL(lf->frictfac, ma->fh_xyfrict);
				// niet shiften, voorkom driften!
				lf->speed[0]= (alpha*lf->speed[0])/4096;
				lf->speed[1]= (alpha*lf->speed[1])/4096;
			}
		}
	}
}



// ALLE globale coords 10 BITS 
// alle texmesh coords 10 bits
// FIXPOINT VARS en GTE 12 bits, 
// SNELHEID 12 bits, lf->loc, dynamesh coords 12 bits
// labda en inpr en dyna->no 14 bits

void update_dynamics(pObject *ob)
{	
	pLife *lf;
	short event, frict, speed[3], force[3], omega[3];
	
	if(ob->type==OB_CAMERA) {
		where_is_object(ob);
		return;
	}
	
	lf= ob->data;
	
	if(lf->type==LF_DYNAMIC) {
	
		// contact: ook botsingscontact zetten: voor sensors
	
		// zwaartekracht
		// force is 14 bits
		force[0]= force[1]= omega[0]=omega[1]=omega[2]= 0;
		force[2]= - SMUL(G.scene->grav, lf->mass);

		/* Fh: contact */
		if(lf->sector && (lf->flag & LF_DO_FH)) update_Fheight(lf, force);

		/* Al 2 keer een vervelnde (flipperkast) bug gehad: beide keren
		   omdat de 'bal' geen sensors heeft, en in sensor_input iets 
		   ge-initialiseerd wordt. Zoals frictfac! */

		/* toetsen */
		if(lf->flag & LF_SENSORS) sensor_input(ob, lf, force, omega);

		// speed even 14 bits
		speed[0]= 4*lf->speed[0]+  force[0] ;
		speed[1]= 4*lf->speed[1]+  force[1] ;
		speed[2]= 4*lf->speed[2]+  force[2] ;

		lf->rotspeed[0]+= omega[0];
		lf->rotspeed[1]+= omega[1];
		lf->rotspeed[2]+= omega[2];
		
		/* wrijving */
		if(lf->frict) {
			frict= 4096 - SMUL(lf->frict, lf->frictfac);

			// negatieve getallen shiften niet zo goed, dus delen: (zie timer.blend)
			lf->speed[0]= (speed[0]*frict)/16384;
			lf->speed[1]= (speed[1]*frict)/16384;
			lf->speed[2]= (speed[2]*frict)/16384;
		}
		else {
			lf->speed[0]= speed[0]/4;
			lf->speed[1]= speed[1]/4;
			lf->speed[2]= speed[2]/4;

			// voorkom driften
			if(lf->speed[0]<0) lf->speed[0]+=1;
			if(speed[1]<0) lf->speed[1]+=1;
			if(speed[2]<0) lf->speed[2]+=1;

		}

		if( lf->rotfrict ) {
			frict= 4096 - lf->rotfrict;

			lf->rotspeed[0]= (lf->rotspeed[0]*frict)/4096;
			lf->rotspeed[1]= (lf->rotspeed[1]*frict)/4096;
			lf->rotspeed[2]= (lf->rotspeed[2]*frict)/4096;
		}
		
		VECCOPY(lf->oldloc, lf->loc);
		
		lf->loc[0]+= (lf->speed[0]);
		lf->loc[1]+= (lf->speed[1]);
		lf->loc[2]+= (lf->speed[2]);

		lf->rot[0]+= lf->rotspeed[0]>>6;
		lf->rot[1]+= lf->rotspeed[1]>>6;
		lf->rot[2]+= lf->rotspeed[2]>>6;
	
		life_to_local_sector_co(lf);
		
		if(lf->sector) collision_detect(ob, lf);
		if(lf->collision) collision_sensor_input(ob, lf);

		/* floorloc is berekend t.o.v. oldloc */
		lf->floorloc[2]-= (lf->speed[2]>>2);

		ob->loc[0]= lf->loc[0]>>GLOBALSHIFT;
		ob->loc[1]= lf->loc[1]>>GLOBALSHIFT;
		ob->loc[2]= lf->loc[2]>>GLOBALSHIFT;

		VECCOPY(ob->rot, lf->rot);
		where_is_object(ob);
		mat_invert(&ob->imat, &ob->obmat);

	}
	else {
				
		if(lf->type==LF_LINK) {

			/* toetsen */
			if(lf->flag & LF_SENSORS) {

				if(lf->dflag & LF_OMEGA) {

					omega[0]=omega[1]=omega[2]= 0;
					sensor_input(ob, lf, force, omega);	
					
					lf->rotspeed[0]+= omega[0];
					lf->rotspeed[1]+= omega[1];
					lf->rotspeed[2]+= omega[2];

					if( lf->rotfrict ) {
						frict= 4096 - lf->rotfrict;
			
						lf->rotspeed[0]= (lf->rotspeed[0]*frict)/4096;
						lf->rotspeed[1]= (lf->rotspeed[1]*frict)/4096;
						lf->rotspeed[2]= (lf->rotspeed[2]*frict)/4096;
					}
	
					lf->rot[0]+= lf->rotspeed[0]>>6;
					lf->rot[1]+= lf->rotspeed[1]>>6;
					lf->rot[2]+= lf->rotspeed[2]>>6;
					
					VECCOPY(ob->rot, lf->rot);
				}
				else {
					sensor_input(ob, lf, force, omega);
				}
			}
			/* link: altijd where_is doen */
			where_is_object(ob);
			mat_invert(&ob->imat, &ob->obmat);
		}
		else {

			/* toetsen */
			if(lf->flag & LF_SENSORS) {

				/* force en omega worden hier niet gebruikt */
				if(event= sensor_input(ob, lf, force, omega)) {
					if(event & SN_ROTCHANGED) {
						VECCOPY(ob->rot, lf->rot);
					}
					lf->oldimat= ob->imat;
					
					where_is_object(ob);
					mat_invert(&ob->imat, &ob->obmat);
				}
				else {
					lf->dflag &= ~LF_DYNACHANGED;
					if(ob->parent) where_is_object(ob);
				}
			}
			lf->collision= 0;	/* is mogelijk door dynalife gezet */
		}
	}
}



/* ************* REALTIME ************** */


void view_to_piramat(MATRIX *piramat, int lens)	
{
	int xfac;
	
	/* far: altijd 16384 (OTSIZE) */
	/* maakt viewmat piramidevormig voor eenvoudige clip */
	
	lens= (lens<<12)/160;
	xfac= (lens*256)/320;

	piramat->m[0][0]=  SMUL(xfac, piramat->m[0][0]);
	piramat->m[0][1]=  SMUL(xfac, piramat->m[0][1]);
	piramat->m[0][2]=  SMUL(xfac, piramat->m[0][2]);
	// piramat->t[0]=  SMUL(xfac, piramat->t[0]);

	xfac>>= 2;
	piramat->t[0]>>= 2;
	piramat->t[0]=  (xfac*piramat->t[0])>>8;
	
	piramat->m[1][0]=  SMUL(lens, piramat->m[1][0]);
	piramat->m[1][1]=  SMUL(lens, piramat->m[1][1]);
	piramat->m[1][2]=  SMUL(lens, piramat->m[1][2]);
	// piramat->t[1]=  SMUL(lens, piramat->t[1]);

	lens>>= 2;
	piramat->t[1]>>= 2;
	piramat->t[1]=  (lens*piramat->t[1])>>8;

	//piramat->m[2][0]=  piramat->m[2][0];
	//piramat->m[2][1]=  piramat->m[2][1];
	//piramat->m[2][2]=  piramat->m[2][2];
	//piramat->t[2]=  piramat->t[2];
	
}



void init_sectors()
{
	extern int subdiv4;
	pBase *base;
	pMesh *me;
	pPortal *po;
	pSector *se;
	pCamPos *capo;
	pLife *lf;
	int a;
	short lens;
	
	G.cursector= 0;
	G.totsect= 0;
	
	/* vlaggen resetten */
	me= G.main->mesh.first;
	while(me) {
		me->flag &= ~ME_ISDONE;
		me= me->id.next;
	}

	// dit zijn globale INT tellers (gaat 10000 uren mee)
	G.dfra= G.dfrao= 1;
	
	// init_def_dyna_material();

	base= FIRSTBASE;
	while(base) {
		
		base->object->dfras= 0;
		
		if(base->object->type==OB_SECTOR) {
			
			se= base->object->data;
			se->ob= base->object;
			
			if(se->texmesh) se->texmesh->lastfra= 0;
			
			// is verplaatst naat readfile, ivm framebuffer_alloc
			/* if(se->campos) G.f |= G_NETWORK; */
			
			// moet eigenlijk op SGI
			capo= se->campos;
			for(a=0; a<se->totcam; a++, capo++) {
				where_is_object(capo->ob);
				object_to_viewmat(capo->ob, &capo->piramat);
				view_to_piramat(&capo->piramat, capo->lens);
			}
			
			// sectoren zijn statisch: op SGI berekend
			// where_is_object(base->object);
			// mat_invert(&base->object->imat, &base->object->obmat);
			
			
		}
		
		base= base->next;
	}
	

	// de portal->ob pointers (optim?)
	base= FIRSTBASE;
	while(base) {
		/* geen layertest: pointers! */
		if(base->object->type==OB_SECTOR) {
			se= base->object->data;
			po= se->portals;
			a= se->totport;
			while(a--) {
				po->ob= po->sector->ob;
				po++;
			}
		}
		base= base->next;
	}

	if(G.f & G_NETWORK) {
		printf("Camera NetWork\n");
		evaluate_camera_network(0, -1); /*init */
		subdiv4= 0;
	}
	//else subdiv4= 1000;
	else subdiv4= 0;
}

void end_sectors()
{
	pBase *base;
	pSector *se;
	
	sector_back(1);		// flush en vrijgeven

	if(G.f & G_RESTART) return;
	
	base= FIRSTBASE;
	while(base) {
		if(base->object->type==OB_SECTOR) {
			
			se= base->object->data;
			
			if(se->lbuf.ob) {
				freeN(se->lbuf.ob);
				se->lbuf.ob= 0;
				se->lbuf.tot= 0;
			}
		}

		base= base->next;
	}
}


void add_dyna_life(pObject *ob)
{
	if(G.totlife<LF_MAXBUF) {
		G.lifebuf[G.totlife]= ob;
		G.totlife++;
	}
}


void add_dupli_life(pObject *ob, pObject *from, short time)
{
	pObject *newob, *par;
	pLife *lfn, *lf;
	pSensor *sn;
	short a;

	if(G.totlife<LF_MAXBUF) {
		newob= dupallocN(ob);
		
		newob->lay= from->lay;
		
		lf= from->data;
		
		if(from->parent) {
		
			/* zeer primitieve rotatie ! */
			
			VECCOPY(newob->rot, from->rot);
			par= from->parent;
			while(par) {
				newob->rot[0]+= par->rot[0];
				newob->rot[1]+= par->rot[1];
				newob->rot[2]+= par->rot[2];
				par= par->parent;
			}

			VECCOPY(newob->loc, from->obmat.t);
			
		}
		else if(lf->type==LF_DYNAMIC && lf->collision) {
			VECCOPY(newob->loc, lf->colloc);

			newob->loc[0]= lf->colloc[0]>>GLOBALSHIFT;
			newob->loc[1]= lf->colloc[1]>>GLOBALSHIFT;
			newob->loc[2]= lf->colloc[2]>>GLOBALSHIFT;
			
			VECCOPY(newob->rot, from->rot);
		}
		else {

			VECCOPY(newob->loc, from->loc);
			VECCOPY(newob->rot, from->rot);
		}

		
		where_is_object(newob);
		add_dyna_life(newob);
		
		newob->data=lfn= dupallocN(ob->data);
		
		if(lfn->type==LF_DYNAMIC) {
			
			lfn->loc[0]= newob->loc[0]<<GLOBALSHIFT;
			lfn->loc[1]= newob->loc[1]<<GLOBALSHIFT;
			lfn->loc[2]= newob->loc[2]<<GLOBALSHIFT;
			
			VECCOPY(lfn->rot, newob->rot);
		}

		lfn->sensors= dupallocN(lfn->sensors);
		for(a=lfn->totsens, sn=lfn->sensors; a>0; a--, sn++) {
			sn->events= dupallocN(sn->events);
			sn->actions= dupallocN(sn->actions);
		}

		lfn->dflag |= LF_TEMPLIFE;
		lfn->timer= time;
		lfn->sector= lf->sector;
		lfn->collision= 0;
		while(from->parent) from= from->parent;
		lfn->from= from;
	}
}

void del_dupli_life(pObject *ob)
{
	pLife *lf;
	pSensor *sn;
	int a, b;
	
	/* opzoeken in array */
	a= G.totlife;
	while(a--) {
		if(G.lifebuf[a]==ob) {
			if(ob->type==OB_LIFE) {
				lf= ob->data;
				if(lf->dflag & LF_TEMPLIFE) {
				
					for(b=lf->totsens, sn=lf->sensors; b>0; b--, sn++) {
						freeN(sn->events);
						freeN(sn->actions);
					}
					freeN(lf->sensors);
					freeN(lf);
					freeN(ob);
					G.totlife--;
					G.lifebuf[a]= G.lifebuf[G.totlife];
				}
			}
			return;
		}
	}
}

void del_dupli_lifes()
{
	int a;
	
	a= G.totlife;
	while(a--) {
		del_dupli_life(G.lifebuf[a]);
	}
}

char clipcube[24]= {0, 1, 1,
					1, 0, 1, 
					0, 0, 0, 
					1, 1, 0, 
					1, 1, 1, 
					0, 0, 1, 
					1, 0, 0,
					0, 1, 0}; 

int cliptest_sector(int *vec, short *size, MATRIX *mat, short round)
{
	/* deze pakt de vier tetraederpunten */
	/* geen abs(new.vz). zie ook SGI code */
	VECTOR hoco, new;
	int min, max;
	short fl, fand, a, tot;
	char *ctab;
	
	/* nu de vraag hoeveel zin dit nog heeft? */
	if(round<=3) tot= 8; else tot= 4;
	ctab= clipcube;
	fand= 63;
	
	while(tot--) {
	
		if(ctab[0]) hoco.vx= vec[0] - size[0];
		else hoco.vx= vec[0] + size[0];
		if(ctab[1]) hoco.vy= vec[1] - size[1];
		else hoco.vy= vec[1] + size[1];
		if(ctab[2]) hoco.vz= vec[2] - size[2];
		else hoco.vz= vec[2] + size[2];
	
		ApplyMatrixLV(mat, &hoco, &new);
		new.vx+= mat->t[0]; new.vy+= mat->t[1]; new.vz+= mat->t[2];
		max= new.vz;
		min= -max;
		fl= 0;
		if(new.vx < min) fl+= 1; else if(new.vx > max) fl+= 2;
		if(new.vy < min) fl+= 4; else if(new.vy > max) fl+= 8;
		if(new.vz < 0) fl+= 16; else if(new.vz >16384) fl+= 32;
		
		fand &= fl;
		if(fand==0) return 1;
		
		ctab+= 3;
	}

	return 0;
}


void build_sectorlist()
{
	pSector *se;
	pBase *base;
	pPortal *po;
	pObject *ob;
	MATRIX piramat;
	short a, b, round, beforesect, startsect;
	
	startsect= 0;
	G.totsect= 0;
	G.maxsect= G.scene->maxdrawsector;
	
	/* uitzondering afhandelen */ 
	if(G.cursector==0) {
		base= FIRSTBASE;
		while(base) {
			if(base->lay & G.scene->lay) {
				if(base->object->type==OB_SECTOR) {			
					G.sectorbuf[G.totsect]= base->object->data;
					G.totsect++;
					if(G.totsect>=G.maxsect) break;
				}
			}
			base= base->next;
		}		
		return;
	}

	G.sectorbuf[0]= G.cursector;
	G.cursector->ob->dfras= G.dfras;
	G.cursector->depth= 0;
	G.totsect= 1;
	
	piramat= G.viewmat;			// copy
	view_to_piramat(&piramat, G.scene->lens);
	
	SetRotMatrix(&piramat);
	SetTransMatrix(&piramat);
	
	round= 1;
	
	while(G.totsect<G.maxsect) {
		beforesect= G.totsect;
		
		/* alle sectoren van startsect tot totsect aflopen op portals */
		for(b=G.totsect-1; b>=startsect; b--) {
			se= G.sectorbuf[b];

			a= se->totport;
			po= se->portals;
			while(a--) {
				if(po->sector && po->ob->dfras!=G.dfras) {
					po->ob->dfras= G.dfras;
					
					ob= po->ob;
					
					if(ob->lay & G.scene->lay) {

						/* in beeld */
						if(cliptest_sector((int *)ob->obmat.t, po->sector->bbsize, &piramat, round)) {
							G.sectorbuf[G.totsect]= po->sector;	
							po->sector->depth= round;
							G.totsect++;
							if(G.totsect>=G.maxsect) break;
						}
					}
				}

				po++;
			}
			if(G.totsect>=G.maxsect) break;
		}

		round++;

		/* geen meer bijgekomen */
		if(G.totsect==beforesect) return;
		startsect= beforesect;
	}
}

/* ************* REALTIME mainlus ************** */


int swapbuffers_OT()
{
	extern int sw_time;
	extern volatile char moviedone;	// sectormovie.c
	DB *dbold;
	int time=0;

	draw_text_info();

	dbold= cdb;
	cdb = (cdb==db)? db+1: db;	/* swap double buffer ID */

	DrawSync(0);	/* wait for end of drawing */
	dbold->dr_time= GetRCnt(RCntCNT1);

	VSync(0);		/* wait for the next V-BLNK */

	sw_time= GetRCnt(RCntCNT1);
	ResetRCnt(RCntCNT1);

	if(sw_time<=315) {
		VSync(0);
		sw_time= 628;
		ResetRCnt(RCntCNT1);
	}
	else if(sw_time>10000) sw_time= 10000;	// na pause of menu

	PutDispEnv(&cdb->disp); /* update display environment */	
	PutDrawEnv(&cdb->draw); /* update drawing environment */

	if(G.f & G_NETWORK) {
		sector_back(0);

		// movie?
		while( moviedone==0 ) { 
			time= GetRCnt(RCntCNT1);
			if (time > 1000) break;
		}

		// voor masks
		if((G.f & G_PLAYMOVIE)==0) {
			MoveImage(&cdb->draw.clip, 640, 0);
			DrawSync(0);
		}
	}
	
	// oude OT
	DrawOTag(dbold->ot+OTSIZE-1);

	// int cdb->ot wordt vanaf nu gewerkt:
	ClearOTagR(cdb->ot, OTSIZE);	// clear ordering table
	
	cdb->curpbuf= cdb->pbuf;
	cdb->curdbuf= cdb->dbuf;

}	

/* ************* MAIN ************** */

void update_sector_lifes(pSector *se)
{
	pObject *ob;
	pLife *lf;
	short a, b;

	if(se==0) return;
	
	for(b=0; b<se->lbuf.tot; b++) {
		ob= se->lbuf.ob[b];
		if(ob->lay & G.scene->lay) {
		
			/* lifes zitten in meerdere sectoren of sector 2x afgehandeld */
		
			if(ob->dfras!=G.dfras) {
				ob->dfras= G.dfras;
				
				lf= ob->data;
				/* beweegbare props: ook in lifebuf ivm sector wissel */
				if(lf->type==LF_DYNAMIC) {
					add_dyna_life(ob);
				}
				else {
					update_dynamics(ob);
				}
			}
		}
	}	
}

void update_lifes()
{
	pObject *ob;
	pLife *lf;
	pPortal *po;
	int a, b;
	
	/* cleanup lifebuf  */
	for(a=0; a<G.totlife; a++) {
		ob= G.lifebuf[a];
		if(ob->type==OB_LIFE) {

			lf= ob->data;
			if((lf->flag & LF_MAINACTOR)==0) {
				if( (lf->dflag & LF_TEMPLIFE)==0 ) {
					G.totlife--;
					G.lifebuf[a]= G.lifebuf[G.totlife];
					a--;
				}
			}
		}
	}
	
	a= G.totsect;
	while(a-- > 0) {
		update_sector_lifes(G.sectorbuf[a]);
	}
	
	if(G.cursector) {
		a= G.cursector->totport;
		po= G.cursector->portals;
		while(a--) {
			update_sector_lifes(po->sector);
			po++;
		}
	}	

	/* alle main+link en temp lifes: in volgorde ivm parents! */
	for(a=0; a<G.totlife; a++) {
		ob= G.lifebuf[a];
		if(ob->type==OB_LIFE) {
		
			life_in_sector_test(ob);

			update_dynamics(ob);
			
			lf= ob->data;
			for(b=0; b<lf->links.tot; b++) update_dynamics(lf->links.ob[b]);
		}
		else where_is_object(ob);		/*	camera */
	}

	/* apart doen ivm delete */
	a= G.totlife;
	while(a--) {
		ob= G.lifebuf[a];
		if(ob->type==OB_LIFE) {
			lf= ob->data;
			if(lf->dflag & LF_TEMPLIFE) {
				if(lf->timer<=0 || lf->sector==0) {
					del_dupli_life(ob);
				}
			}		
		}
	}
}

void sector_go()
{
	extern int sw_time;		// screen.c
	short a, b, dtime, padd;
	
	/* per life: sector test
	 *			 sensors
	 *			 damage
	 *			 update dynamics <- -> collision detectie
	 *			 
	 */
	
	/* hier iets doen met tijd init */
	sw_time= 628;	// 25 Hz
	
	init_sectors();	/* doet ook mesh_isdone flag */
	init_lifes();
	init_devs();

	while(TRUE) {
		
		/* dynamics lus: stapjes van 0.02 seconden */
		
		G.dfrao= G.dfra;
		G.fields= 0;
		cdb->ca_time=cdb->ga_time= GetRCnt(RCntCNT1);

		dtime= MIN2(sw_time, 1000);

		while(dtime > 0) {
		
			G.dfra++;
			G.dfras= G.dfra;

			//pad_read();
			if(G.qual & PADh) {

				if(G.qual & PADk) make_menu();
				//if(G.qual== PADk+1+8 ) make_menu(); 
				//if(G.qual & PADh) exit_func();
				//pad_read();
			}
			update_lifes();
			
			dtime-= 314;
			G.fields++;
		}

		if(G.fields) cdb->ga_time= (GetRCnt(RCntCNT1) - cdb->ga_time)/G.fields;

		if(G.f & G_NETWORK) {
			evaluate_camera_network(0, 0);
		}
		else {

			mainactor_in_sector_test();
			object_to_viewmat(G.scene->camera, &G.viewmat);
		}

		build_sectorlist();

		if( (G.f & G_PLAYMOVIE)==0) {
			//old_sp= SetSp(DCACHE_TOP-12);
			draw_all();
			//SetSp(old_sp);
		}
		
		cdb->ca_time= GetRCnt(RCntCNT1) - cdb->ca_time;

		swapbuffers_OT();

		if(G.f & (G_LOADFILE+G_RESTART+G_QUIT)) break;

	}

	end_sectors();
	end_lifes(1);	/* restore */
	//movie_exit();

}


