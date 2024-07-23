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

/*  readblendpsx.c   juli 96  

 * 
 * 
 */


#include "psxblend.h"
#include "psxgraph.h"


short tpagenumbers[32];
short clutnumbers[32];



/*
	
-	(voorlopig) 1 blenderfile inlezen: interne data 
	wordt niet opniew gemallocd

*/



#define NEWVERTS3	if(me->flag & ME_PUNO) {	\
						pr->v1= svec + 2*(int)pr->v1; \
						pr->v2= svec + 2*(int)pr->v2; \
						pr->v3= svec + 2*(int)pr->v3; \
					} \
					else { \
						pr->v1= svec + (int)pr->v1; \
						pr->v2= svec + (int)pr->v2; \
						pr->v3= svec + (int)pr->v3; \
					}
#define NEWVERTS4	if(me->flag & ME_PUNO) {	\
						pr->v1= svec + 2*(int)pr->v1; \
						pr->v2= svec + 2*(int)pr->v2; \
						pr->v3= svec + 2*(int)pr->v3; \
						pr->v4= svec + 2*(int)pr->v4; \
					} \
					else { \
						pr->v1= svec + (int)pr->v1; \
						pr->v2= svec + (int)pr->v2; \
						pr->v3= svec + (int)pr->v3; \
						pr->v4= svec + (int)pr->v4; \
					}

#define TEXPAD1		pr->po.clut= clutnumbers[ pr->po.tpage ]; \
					pr->po.tpage= tpagenumbers[ pr->po.tpage ] + pr->po.pad1; \
					SetTexWindow(&pr->dr, (RECT *)pr->dr.code);
#define TEXPAD2		pr->po.clut= clutnumbers[ pr->po.tpage ]; \
					pr->po.tpage= tpagenumbers[ pr->po.tpage ] + pr->po.pad2; \
					SetTexWindow(&pr->dr, (RECT *)pr->dr.code);

void init_mesh(pMesh *me)
{
	SVECTOR *svec;
	int a, datalen, packets;
	short *sp, type, nr_elems;
	
	if(me->packetdata==0) return;

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
					NEWVERTS3
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
					NEWVERTS4 
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;

		case P_LF3:
			{
				PrimLF3 *pr= (PrimLF3 *)sp;
				
				while(nr_elems-- > 0)  {
					NEWVERTS3
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_LF4:
			{
				PrimLF4 *pr= (PrimLF4 *)sp;
				
				while(nr_elems-- > 0)  {
					NEWVERTS4 
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
					NEWVERTS3
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
					NEWVERTS4 
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;

		case P_LPG3:
		case P_LG3:
			{
				PrimLG3 *pr= (PrimLG3 *)sp;
				
				while(nr_elems-- > 0)  {
					NEWVERTS3
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;

		case P_LPG4:
		case P_LG4:
			{
				PrimLG4 *pr= (PrimLG4 *)sp;
				
				while(nr_elems-- > 0)  {
					NEWVERTS4 
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
					NEWVERTS3
					TEXPAD1
					if(type==P_MASK3) SetShadeTex(&pr->po, 1);
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
					NEWVERTS4
					TEXPAD2
					if(type==P_MASK4) SetShadeTex(&pr->po, 1);
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;
		case P_LFT3:
			{
				PrimLFT3 *pr= (PrimLFT3 *)sp;
				
				while(nr_elems-- > 0)  {
					NEWVERTS3
					TEXPAD1
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
		case P_LFT4:
			{
				PrimLFT4 *pr= (PrimLFT4 *)sp;
				
				while(nr_elems-- > 0)  {
					NEWVERTS4
					TEXPAD2
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
					NEWVERTS3
					TEXPAD2
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
					NEWVERTS4
					TEXPAD2
					pr++;
				}
			   	sp= (short *)pr;
			}
			break;
		case P_LPGT3:
		case P_LGT3:
			{
				PrimLGT3 *pr= (PrimLGT3 *)sp;
				
				while(nr_elems-- > 0)  {
					NEWVERTS3
					TEXPAD2
					pr++;
				}
			   	sp= (short *)pr;	
			}
			break;
		case P_LPGT4:
		case P_LGT4:
			{
				PrimLGT4 *pr= (PrimLGT4 *)sp;
				
				while(nr_elems-- > 0)  {
					NEWVERTS4
					TEXPAD2
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
					TEXPAD2
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


ListBase *wich_libbase(pMain *main, short type)
{
	switch( type ) {
	
		case ID_SCE:
			return &(main->scene);
		case ID_LI:
			return &(main->library);
		case ID_OB:
			return &(main->object);
		case ID_ME:
			return &(main->mesh);
		case ID_IM:
			return &(main->image);
		case ID_IK:
			return &(main->ika);
		case ID_SE:
			return &(main->sector);
		case ID_LF:
			return &(main->life);
		case ID_LA:
			return &(main->lamp);
		case ID_CA:
			return &(main->camera);
		case ID_IP:
			return &(main->ipo);
		case ID_KE:
			return &(main->key);
		case ID_WO:
			return &(main->world);
		case ID_MA:
			return &(main->mat);
		deafult:
			printf("error in wich_libbase\n");
	}
	return 0;
}

/* ************** AFHANDELING OUDE POINTERS ******************* */

typedef struct OldNew {
	void *old, *new;
	int nr;
} OldNew;

OldNew *datablocks=0;
OldNew *libblocks=0;
int datacount= 0, maxdatacount=1024;
int libcount= 0, maxlibcount=1024;
void *disable_newlibadr;


void add_data_adr(void *old, void *new)
/* met dynamische malloc
 * (0, 1) doorgeven herinitialiseert en geeft ongebruikte blokken vrij
 * (0, 0) doorgeven geeft alles vrij 
 */
{
	OldNew *temp;
	int a;
	
	if(old==0) {	/* ongebruikte vrijgeven */
		temp= datablocks;
		for(a=0; a<datacount; a++, temp++) {
			// if(temp->nr==0) freeN(temp->new);
		}
		if(new==0 && datablocks) {
			freeN(datablocks);
			datablocks= 0;
			maxdatacount= 1024;
		}
		datacount= 0;
	}
	else {
		if(datablocks==0) {
			datablocks= mallocN(maxdatacount*sizeof(OldNew), "OldNew");
		}
		else if(datacount==maxdatacount) {
			maxdatacount*= 2;
			temp= mallocN(maxdatacount*sizeof(OldNew), "OldNew");
			memcpy(temp, datablocks, maxdatacount*sizeof(OldNew)/2);
			freeN(datablocks);
			datablocks= temp;
		}
		
		temp= datablocks+datacount;
		temp->old= old;
		temp->new= new;
		temp->nr= 0;
		
		datacount++;
	}
}

void add_lib_adr(void *old, void *new)
/* met dynamische malloc
 * (0, 0) doorgeven geeft alles vrij
 * Zet aantal users al op 1!!!
 */
{
	OldNew *temp=0;
	int a;
	
	/* onderscheid tussen twee gevallen:
	 * 
	 * 1. lib obj's in locale scene, ob->parent		(old is uit library)
	 * 2. lib obj's in locale scene: base->object   (old is uit locale scene)
	 * 
	 */
	
	if(disable_newlibadr) {
		disable_newlibadr= new;
		
	}
	
	if(old==0) {	/* alles vrijgeven */
		if(libblocks) freeN(libblocks);
		libblocks= 0;
		maxlibcount= 1024;
		libcount= 0;
	}
	else {
		if(libblocks==0) {
			libblocks= mallocN(maxlibcount*sizeof(OldNew), "OldNew");
		}
		else if(libcount==maxlibcount) {
			maxlibcount*= 2;
			temp= mallocN(maxlibcount*sizeof(OldNew), "OldNew");
			memcpy(temp, libblocks, maxlibcount*sizeof(OldNew)/2);
			freeN(libblocks);
			libblocks= temp;
		}
		
		temp= libblocks+libcount;
		temp->old= old;
		temp->new= new;
		temp->nr= 1;
		
		libcount++;
	}
	
}

void *newadr(void *adr)		/* alleen direkte datablokken */
{
	static int lastone= 0;
	struct OldNew *onew;

	if(adr) {
		/* op goed geluk: eerst het volgende blok doen */
		if(lastone<datacount-1) {
			lastone++;
			onew= datablocks+lastone;
			if(onew->old==adr) {
				onew->nr++;
				return onew->new;
			}
		}
		
		lastone= 0;
		onew= datablocks;
		while(lastone<datacount) {
			if(onew->old==adr) {
				onew->nr++;
				return onew->new;
			}
			onew++;
			lastone++;
		}
	}
	
	return 0;
}

void *newlibadr(void *lib, void *adr)		/* alleen Lib datablokken */
{
	static int lastone= 0;
	struct OldNew *onew;
	pID *id;
	
	/* ook testen op lib: lib==id->lib) */
	
	if(adr) {
	
		/* op goed geluk: eerst hetzelfde en volgende blok doen */
		if(lastone<libcount-1) {

			onew= libblocks+lastone;
			if(onew->old==adr) {
				id= onew->new;
				if(id->lib==0 && lib);
				else {
					onew->nr++;
					return onew->new;
				}
			}
			lastone++;
			onew= libblocks+lastone;
			if(onew->old==adr) {
				id= onew->new;
				if(id->lib==0 && lib);
				else {
					onew->nr++;
					return onew->new;
				}
			}
		}
		
		lastone= 0;
		onew= libblocks;
		while(lastone<libcount) {
			if(onew->old==adr) {
				id= onew->new;
				
				if(id && id->lib==0 && lib);
				else {
					onew->nr++;
					return onew->new;
				}
			}
			onew++;
			lastone++;
		}

	}

	return 0;
}

void *newlibadr_us(void *lib, void *adr)		/* hoogt usernummer op */
{
	pID *id;
	
	id= newlibadr(lib, adr);
	if(id) {
		id->us++;
	}
	
	return id;
}

void change_libadr(void *old, void *new)
{
	struct OldNew *onew;
	pID *id;
	int lastone= 0;
	
	onew= libblocks;
	while(lastone<libcount) {
		
		id= onew->new;
		if(id && id->lib) {
			if(onew->new==old) {
				onew->new= new;
				return;
				/* return; kunnen er meer zijn? */
			}
		}
		
		onew++;
		lastone++;
	}
	
}



/* ********** END OUDE POINTERS ****************** */
/* ********** LINK FUNCTIES ****************** */

void link_list(ListBase *lb)		/* alleen direkte data */
{
	Link *ln, *prev;
	
	if(lb->first==0) return;

	lb->first= newadr(lb->first);
	lb->last= newadr(lb->last);
	
	ln= lb->first;
	prev= 0;
	while(ln) {
		
		ln->next= newadr(ln->next);
		ln->prev= prev;
		
		prev= ln;
		ln= ln->next;
	}
}


void lib_link_scene(pMain *main)
{
	pScene *sce;
	pBase *base, *next;
	
	sce= main->scene.first;
	while(sce) {

		if(sce->id.flag & LIB_NEEDLINK) {
			sce->id.us= 1;
			sce->camera= newlibadr(sce->id.lib, sce->camera);
			/* sce->world= newlibadr_us(sce->id.lib, sce->world); */
			sce->set= newlibadr(sce->id.lib, sce->set);

			base= sce->base.first;
			while(base) {
				next= base->next;
				base->object= newlibadr_us(sce->id.lib, base->object);
				if(base->object==0) {
					// printf("LIB ERROR: base removed\n");
					remlink(&sce->base, base);
					if(base==sce->basact) sce->basact= 0;
				}

				base= next;
			}
			sce->id.flag -= LIB_NEEDLINK;
		}
		sce= sce->id.next;
	}
}

void direct_link_scene(sce)
pScene *sce;
{
	
	link_list( &(sce->base) );

}



void lib_link_object(pMain *main)
{
	pObject *ob;
	pNetLink *nl;
	void *poin;
	int a;
	
	ob= main->object.first;
	while(ob) {
	
		if(ob->id.flag & LIB_NEEDLINK) {

			ob->parent= newlibadr(ob->id.lib, ob->parent);
			ob->track= newlibadr(ob->id.lib, ob->track);
			
			poin= ob->data;
		
			ob->data= newlibadr_us(ob->id.lib, ob->data);
			if(ob->id.us) ob->id.flag -= LIB_NEEDLINK;
			/* als us==0 wordt verderop nog een base gemaakt */
	
			nl= ob->network.first;
			while(nl) {
				nl->ob= newlibadr(ob->id.lib, nl->ob);
				nl= nl->next;
			}
		}
		ob= ob->id.next;
	}
}


void direct_link_object(ob)
pObject *ob;
{
	
	link_list( &(ob->network) );

}

void lib_link_sector(pMain *main)
{
	pSector *se;
	pPortal *po;
	pCamPos *capo;
	int a;
	
	se= main->sector.first;
	while(se) {
		if(se->id.flag & LIB_NEEDLINK) {
		
			// se->ob: in init_sectors, wordt niet weggeschreven
			
			se->dynamesh= newlibadr_us(se->id.lib, se->dynamesh);
			se->texmesh= newlibadr_us(se->id.lib, se->texmesh);
			
			po= se->portals;
			a= se->totport;
			while(a--) {
				po->sector= newlibadr(se->id.lib, po->sector);
				po++;
			}

			a= se->lbuf.tot;
			while(a--) {
				se->lbuf.ob[a]= newlibadr(se->id.lib, se->lbuf.ob[a]);
				
			}
			
			a= se->totcam;
			capo= se->campos;
			while(a--) {
				capo->ob= newlibadr(se->id.lib, capo->ob);
				capo++;
			}
			
			se->id.flag -= LIB_NEEDLINK;
		}
		se= se->id.next;
	}
	
}

void direct_link_sector(se)
pSector *se;
{
	pObject **obar;

	/* portals */
	se->portals= newadr(se->portals);
	
	if(se->lbuf.tot) {
		obar= newadr(se->lbuf.ob);
		se->lbuf.ob= mallocN( 4*se->lbuf.max, "lbuf");
		LONGCOPY(se->lbuf.ob, obar, se->lbuf.tot);
	}
	
	se->campos= newadr(se->campos);
	if(se->campos) G.f |= G_NETWORK;
	
	se->name= newadr(se->name);
	
	link_list(&se->ipo);
	
}


void lib_link_life(pMain *main)
{
	pLife *lf;
	pSensor *sn;
	pEvent *ev;
	pAction *ac;
	int a, b, c;
	
	lf= main->life.first;
	while(lf) {
		if(lf->id.flag & LIB_NEEDLINK) {
			
			lf->sector= newlibadr(lf->id.lib, lf->sector);
			lf->dynamesh= newlibadr_us(lf->id.lib, lf->dynamesh);
			lf->texmesh= newlibadr_us(lf->id.lib, lf->texmesh);

			a= lf->links.tot;
			while(a--) {
				lf->links.ob[a]= newlibadr(lf->id.lib, lf->links.ob[a]);
			}

			lf->id.flag -= LIB_NEEDLINK;
			
			sn= lf->sensors;
			a= lf->totsens;
			while(a--) {
				
				/* set events: pointers */
				ev= sn->events;
				b= sn->totevent;
				while(b--) {
					
					if ELEM4(ev->event, SN_COLLISION, SN_NEAR, SN_CONTACT, SN_TRIPFACE) {
						ev->poin= newlibadr(lf->id.lib, ev->poin);
					}
					else if ELEM3(ev->event, SN_VAR_EQUAL, SN_VAR_INTERVAL, SN_VAR_CHANGED) {
						c= (int)ev->poin;
						if(c<0 && c>-5) ev->poin= lf->state-c-1;
						else if(c>=0) ev->poin= G.actvarar+c;
					}
					ev++;
				}
				
				/* set actions: pointers */
				ac= sn->actions;
				b= sn->totaction;

				while(b--) {
					if(ac->action<100) {
					}
					else if(ac->action<200) {	/* ipos */
						if(ac->flag & SN_IPO_SETVAR) {
							c= (int)ac->poin;
							if(c<0 && c>-5) ac->poin= lf->state-c-1;
							else if(c>=0) ac->poin= G.actvarar+c;
						}
						else {
							/* even niet: probelemen met DYNA_CHANGED */
							/* ac->poin= newlibadr(lf->id.lib, ac->poin); */
						}
					
					}
					else if(ac->action<300) {
						if(ac->poin) {
							if(ac->action==SN_GOTO) {
								ac->poin= newlibadr(lf->id.lib, ac->poin);
							}
							else if ELEM6(ac->action, SN_ROBBIE_M, SN_TRACKTO, SN_ADDLIFE, SN_REPLACEMESH, SN_REPLACELIFE, SN_SETCAMERA) {
								ac->poin= newlibadr(lf->id.lib, ac->poin);
								if(ac->poin==0)
								printf("lost poin: ac %d lf %s\n", ac->action, lf->id.name);
							}
						}
					}
					else if(ac->action<400) {
						c= (int)ac->poin;
						if(c<0 && c>-5) ac->poin= lf->state-c-1;
						else if(c>=0) ac->poin= G.actvarar+c;
					}
					ac++;
				}
				sn++;
			}
		}
		lf= lf->id.next;
	}
	
}

void direct_link_life(lf)
pLife *lf;
{
	pSensor *sn;
	pAction *ac;
	pEvent *ev;
	int a, b;
	
	link_list(&lf->ipo);

	// worden niet vrijgegeven!
	lf->links.ob= newadr(lf->links.ob);

	lf->sensors= newadr(lf->sensors);
	a= lf->totsens;
	sn= lf->sensors;
	while(a--) {
		sn->events= newadr(sn->events);
		sn->actions= newadr(sn->actions);
		
		/* directe event data */
		// ev= sn->events;
		// b= sn->totevent;
		// while(b--) {
		// 	
		// 	ev++;
		// }
		
		/* direct action data */
		ac= sn->actions;
		b= sn->totaction;
		while(b--) {

			if ELEM(ac->action, SN_LOADFILE, SN_PLAYMOVIE) {
				ac->poin= newadr(ac->poin);		// filenaam
			}
			ac++;
		}
		sn++;
	}
}


// in free_main() wordt dit vrijgegeven
pMesh *packet_copy_mesh(pMesh *me)
{
	pMesh *men;
	BHead *bhead;

	men= mallocN(sizeof(pMesh), "copy mesh");
	*men= *me;

	addtail(&G.main->mesh, men);
	men->id.flag |= LIB_MALLOC;
	men->id.us= 1;
	me->id.us--;
	
	bhead= (BHead *)me->packetdata;
	bhead--;

	men->packetdata= mallocN( bhead->len, "copy packet");
	memcpy(men->packetdata, me->packetdata, bhead->len);

	return men;
}

void single_user_texmesh()
{
	pSector *se;
	pObject *ob;

	ob= G.main->object.first;
	while(ob) {
		if(ob->type==OB_SECTOR) {
			se= ob->data;
			if(se->texmesh->id.us>1) {
				se->texmesh= packet_copy_mesh(se->texmesh);
			}
		}
		ob= ob->id.next;
	}
}

void lib_link_mesh(pMain *main)
{
	pMesh *me;
	pDFace *dface;
	short *mvert;
	int a;
	
	me= main->mesh.first;
	while(me) {

		if(me->id.flag & LIB_NEEDLINK) {			
			init_mesh(me);

			if(me->dface) {
				
				// materials en vertex pointers
				
				dface= me->dface;
				mvert= me->mvert;
				a= me->totface;

				while(a--) {

					dface->ma= newlibadr(me->id.lib, dface->ma);
					if(dface->ma==0) dface->ma= &G.defaultmaterial;

					dface->v1= mvert+ 4*(int)(dface->v1);
					dface->v2= mvert+ 4*(int)(dface->v2);
					if( (int)dface->v3 != -1) dface->v3= mvert+ 4*(int)(dface->v3);
					else dface->v3= 0;
					if( (int)dface->v4 != -1) dface->v4= mvert+ 4*(int)(dface->v4);
					else dface->v4= 0;
					
					// niet van de float versie afleiden?!
					dface->dist= (dface->no[0]*dface->v1[0] + dface->no[1]*dface->v1[1] + dface->no[2]*dface->v1[2] );
					dface++;

				}
			}
			
			me->id.flag -= LIB_NEEDLINK;
		}
		me= me->id.next;
	}
}



void direct_link_mesh(mesh)
pMesh *mesh;
{

	mesh->mvert= newadr(mesh->mvert);
	mesh->packetdata= newadr(mesh->packetdata);
	mesh->dface= newadr(mesh->dface);
	mesh->oc= newadr(mesh->oc);

}

void lib_link_image(pMain *main)
{
	pImage *ima;
	
	ima= main->image.first;
	while(ima) {
		if(ima->id.flag & LIB_NEEDLINK) {
			
			ima->id.flag -= LIB_NEEDLINK;
		}
		ima= ima->id.next;
	}
	
	init_twin_anims();	// peffect.c
}


void image_to_framebuffer(pImage *ima)
{
	RECT clutrect;
	int dx, sx, sy;
	
	if(ima->rect==0) return;
	
	ima->fba_rect = fb_alloc(&sx, &sy, ima->x, ima->y, "Image");
	if(ima->fba_rect<0) {
		printf("Error: framebuffer full: %s %d\n", ima->name, ima->fba_rect);
	}
	
	// LET OP: ima->x en y zijn framebuffer pixels!!!

	if(ima->type==0) dx= 4*ima->x;
	else if(ima->type==1) dx= 2*ima->x;
	else dx= ima->x;

	tpagenumbers[ima->tpagenr]= 
		LoadTPage(ima->rect, ima->type, 0, sx, sy, dx, ima->y);
	
	ima->sx= sx;
	ima->sy= sy;
	
	if(ima->clut) {

		ima->fba_clut = fb_alloc(&sx, &sy, ima->clutlen, 1, "clut");
		setRECT(&clutrect, sx, sy , ima->clutlen, 1);

  		LoadImage(&clutrect, ima->clut);
    	ima->clutnr= GetClut(sx, sy);

		clutnumbers[ima->tpagenr]= ima->clutnr;
		
		if(ima->tpageflag & IMA_COLCYCLE) {
			ima->xrep= sx;
			ima->yrep= sy;
		}
	}
	else clutnumbers[ima->tpagenr]= 0;

	ima->tpagenr= tpagenumbers[ima->tpagenr];

}

typedef struct TARGA 
{
	unsigned char numid;	
	unsigned char maptyp;
	unsigned char imgtyp;	
	short maporig;
	short mapsize;
	unsigned char mapbits;
	short xorig;
	short yorig;
	short xsize;
	short ysize;
	unsigned char pixsize;
	unsigned char imgdes;
} TARGA;


long checktarga(tga, mem)
TARGA *tga;
char *mem;
{
	tga->numid = mem[0];
	tga->maptyp = mem[1];
	tga->imgtyp = mem[2];

	tga->maporig = GSS(mem+3);
	tga->mapsize = GSS(mem+5);
	tga->mapbits = mem[7];
	tga->xorig = GSS(mem+8);
	tga->yorig = GSS(mem+10);
	tga->xsize = GSS(mem+12);
	tga->ysize = GSS(mem+14);
	tga->pixsize = mem[16];
	tga->imgdes = mem[17];

	if (tga->maptyp > 1) return(0);
	switch (tga->imgtyp){
	case 1:			/* raw cmap */
	case 2:			/* raw rgb */
	case 3:			/* raw b&w */
	case 9:			/* cmap */
	case 10:		/* rgb */
	case 11:		/* b&w */
		break;
	default:
		return(0);
	}
	if (tga->mapsize && tga->mapbits > 32) return(0);
	if (tga->xsize <= 0) return(0);
	if (tga->ysize <= 0) return(0);
	if (tga->pixsize > 32) return(0);
	return(1);
}


void decodetarga1(pImage *ima, uchar *mem)
{
	long count, col, size;
	uchar *rect;
	
	if (ima == 0) return;
	if (ima->rect == 0) return;

	size = ima->x * ima->y;
	rect = ima->rect;
	
	while (size > 0){
		count = *mem++;
		if (count >= 128) {
			count -= 127;
			size -= count;
			col = *mem++;
			for ( ;count > 0; count --) *rect++ = col;
		} else{
			count ++;
			size -= count;
			for ( ;count > 0; count --) *rect++ = *mem++;
		}
	}
	
	if (size) printf("decodetarga1: overwrote %d pixels\n", -size);
}

void decodetarga3(pImage *ima, uchar *mem)
{
	long count, col, size;
	short *rect;
	
	if (ima == 0) return;
	if (ima->rect == 0) return;

	size = ima->x * ima->y;
	rect = ima->rect;
	
	while (size > 0){
		count = *mem++;
		if (count >= 128) {
			count -= 127;
			size -= count;
			
			col = 0x8000 | ((mem[0] >> 3) << 10) | ((mem[1] >> 3) << 5) | (mem[2] >> 3);
			mem += 3;

			for ( ;count > 0; count --) *rect++ = col;
		} else{
			count ++;
			size -= count;
			
			for ( ;count > 0; count --) {
				col = 0x8000 | ((mem[0] >> 3) << 10) | ((mem[1] >> 3) << 5) | (mem[2] >> 3);
				mem += 3;
				
				*rect++ = col;
			}
		}
	}
	
	if (size) printf("decodetarga3: overwrote %d pixels\n", -size);
}


void decodetarga4(pImage *ima, uchar *mem)
{
	long count, col, size;
	short *rect, a;
	
	if (ima == 0) return;
	if (ima->rect == 0) return;

	size = ima->x * ima->y;
	rect = ima->rect;
	
	while (size > 0){
		count = *mem++;
		if (count >= 128) {
			count -= 127;
			size -= count;
			
			col = ((mem[0] >> 3) << 10) | ((mem[1] >> 3) << 5) | (mem[2] >> 3);
			a = mem[3];

			if (a < 64) col = 0;
			else if (a < 192) col |= 0x8000;
			else if (col == 0) col = (1 << 10);
			
			mem += 4;

			for ( ;count > 0; count --) *rect++ = col;
		} else{
			count ++;
			size -= count;
			
			for ( ;count > 0; count --) {
				col = ((mem[0] >> 3) << 10) | ((mem[1] >> 3) << 5) | (mem[2] >> 3);
				a = mem[3];

				if (a < 64) col = 0;
				else if (a < 192) col |= 0x8000;
				else if (col == 0) col = (1 << 10);
			
				mem += 4;
				
				*rect++ = col;
			}
		}
	}
	
	if (size) printf("decodetarga4: overwrote %d pixels\n", -size);
}


void flipy(pImage * ima)
{
	short x, y, backx;
	short *top, *bottom, temp;

	if (ima == 0) return;
	if (ima->rect == 0) return;

	x = ima->x;
	y = ima->y;
	backx = x << 1;

	top = ima->rect;
	bottom = top + ((y-1) * x);
	y >>= 1;

	for(;y > 0; y--){
		for(x = ima->x; x > 0; x--){
			temp = *top;
			*(top++) = *bottom;
			*(bottom++) = temp;
		}
		bottom -= backx;
	}
}


void next_image(int *filedata, int len)
{
	static pImage *ima;
	int file, id, flag, i, targa = FALSE;
	uchar *cp, a;
	short *sp;
	TARGA tga;
	
	if(filedata==0) {
		ima= 0;
		return;
	}
	if(len==0) return;
	
	if(ima==0) ima= G.main->image.first;
	
	ima->rect= 0;
	ima->clut= 0;
	
	/* doen alsof TIM lezen */
	
	if (*filedata == 16) {
		filedata++;
		
		flag= *(filedata++);
	
		ima->type= flag & 3;
		
		ima->fba_clut= 0;
		if(flag & 8)  {   // clut laden
	
			len= *(filedata++);
			id= *(filedata++);
			id= *(filedata++);
	
			ima->clutlen= (id>>16);
			
			ima->clut= filedata;
			filedata+= ima->clutlen/2;
		}
	
		id= *(filedata++);		// bnum
		id= *(filedata++);		// dx dy
		sp= (short *)(filedata++);
		ima->x= sp[0];
		ima->y= sp[1];
		
		ima->rect= filedata;
	} else {
		if (checktarga(&tga, filedata) == 0) {
			printf("not a TIM or Targa: %s\n", ima->name);
			return;
		}
		
		// it's a targa
		
		if (tga.imgtyp != 10) {
			if (tga.mapsize == 0 || tga.imgtyp != 9) {
				printf("Targa has no (valid) cmap: %s\n", ima->name);
				return;
			}
			
			if (tga.pixsize > 8) {
				printf("Targa cmap has more than 256 colors: %s\n", ima->name);
				return;
			}
		}
		
		// it's a VALID targa

		targa = TRUE;

		cp = (uchar *) filedata;
		cp += 18 + tga.numid;
				
		ima->x = tga.xsize;
		ima->y = tga.ysize;
		ima->fba_clut = 0;

		// colormap inlezen

		if (tga.mapsize) {
			ima->clutlen = tga.mapsize;
			ima->clut = mallocN(2 * ima->clutlen, "tga_clut");
			
			if (ima->clut) {
				sp = ima->clut;
				for (i = 0; i < ima->clutlen; i++) {
					// Intelligente omzetting cmap->psx ???
					
					if (tga.mapbits == 32) a = *cp++;
					else a = 0;
					
					sp[i] = (((255 - a) & 0x80) << 8) | ((cp[0] >> 3) << 10) | ((cp[1] >> 3) << 5) | (cp[2] >> 3);
	
					// zwart is NIET transparant
					if (sp[i] == 0) sp[i] == 1 << 10;
					
					cp += 3;
				}
				
				// kleur 0 transparant ??
				sp[0] = 0;
			}
		}
		
		// rect inlezen
		
		if (tga.imgtyp == 9) {
			ima->type = 1;

			ima->rect = mallocN(ima->x * ima->y, "tga_rect1");
			if (ima->rect) decodetarga1(ima, cp);

			if (ima->x & 1) {
				printf("Targa xsize is odd!: %s\n", ima->name);
			}
			ima->x /= 2;
		} else if (tga.imgtyp == 10) {
			ima->type = 2;
			ima->rect = mallocN(2 * ima->x * ima->y, "tga_rect2");
			if (ima->rect) {
				if (tga.pixsize <= 24) decodetarga3(ima, cp);
				else decodetarga4(ima, cp);
			}
			
			if (ima->clut) freeN(ima->clut);
			ima->clut = 0;
		}
		
		
		if ((tga.imgdes & 0x20) == 0) flipy(ima);
	}


	image_to_framebuffer(ima);
	//printf("Loading image %s to %d %d\n", ima->name, ima->sx, ima->sy);
	
	if (targa) {
		if (ima->rect) freeN(ima->rect);
		//if (ima->clut) freeN(ima->clut);
		if(ima->clut) ima->flag |= IMA_FREECLUT;
	}
	
	ima->rect= 0;
	//ima->clut= 0;
	
	ima= ima->id.next;
}


int read_libblock(pMain *main, BHead *bhead, int flag)
{
	/* deze routine leest libblock en direkte data. Met linkfunkties
	 * alles aan elkaar hangen.
	 */
	
	pID *id;
	ListBase *lb;
	int skipdata;
	char *fd;
	
	if(bhead->code==ID_ID) {
		/* id= (ID *)(bhead + 1); */
		/* lb= wich_libbase(main, GS(id->name)); */
	}
	else {
		lb= wich_libbase(main, bhead->code);
	}

	if(bhead->len==0 || lb==0) {
		return bhead->len+sizeof(BHead);
	}
	
	fd= (char *)bhead;
	
	/* hier stond libblock dna test */
	id= (pID *)(bhead+1);
	add_lib_adr(bhead->old, id);
	
	addtail(lb, id);
	
	/* eerste acht bits wissen */
	id->flag= (id->flag & 0xFF00) | flag | LIB_NEEDLINK;
	id->lib= main->curlib;
	if(id->flag & LIB_FAKEUSER) id->us= 1;
	else id->us= 0;
	
	/* deze mag niet door de direct_link molen: is alleen het ID deel */
	if(bhead->code==ID_ID) {
		skipdata= bhead->len+sizeof(BHead);
		return skipdata;	
	}

	skipdata= bhead->len+sizeof(BHead);
	fd+= bhead->len+sizeof(BHead);
	bhead= (BHead *)fd;

	
	/* alle data inlezen */
	while(bhead->code==DATA) {
		
		add_data_adr(bhead->old, (bhead+1));
		
		skipdata+= bhead->len+sizeof(BHead);
		fd+= bhead->len+sizeof(BHead);
		bhead= (BHead *)fd;		
	}

	/* pointers directe data goedzetten */

	switch( GS(id->name) ) {
		case ID_SCE:
			// printf("link scene\n");
			direct_link_scene(id);
			break;
		case ID_OB:
			// printf("link object\n");
			direct_link_object(id);
			break;
		case ID_ME:
			// printf("link mesh\n");
			direct_link_mesh(id);
			break;
		case ID_IM:
			// printf("read image\n");
			// direct_link_image(id);
			break;
		case ID_MA:
			// printf("read material %x\n", id);
			// direct_link_material(id);
			break;

		// case ID_KE:
			/* direct_link_key(id); */
			// break;
		// case ID_LT:
			/* direct_link_latt(id); */
			// break;
		// case ID_IK:
			/* direct_link_ika(id); */
			// break;
		case ID_SE:
			// printf("read sector\n");			
			direct_link_sector(id);
			break;
		case ID_LF:
			// printf("link life\n");
			direct_link_life(id);
			break;
		// case ID_WO:
			/* direct_link_world(id); */
			// break;
		// case ID_LI:
			/* direct_link_library(id); */
			// break;
		default:
			printf("unknown ID %d\n", GS(id->name));
	}
	
	/* vrijgeven, herinitialiseren */
	add_data_adr(0, (void *)1);	
	
	return skipdata;
}

short read_append_data(int file)
{
	BHead bhead;
	void *poin;
	short ret= 1;
	char str[5];
	
	while(ret>0) {
		
		ret= read(file, &bhead, sizeof(BHead));

		if(bhead.code==IMAG) {
		
			poin= mallocN(bhead.len, "imag");
			read(file, poin, bhead.len);

			next_image( poin , bhead.len);
			freeN(poin);
		}
		else if(bhead.code==ENDB) {
			return(1);
		}
		else {

			lseek(file, bhead.len, SEEK_CUR);
		}
	}
	return(0);
}

char *openblendpsxfile(char *name, int *filelen, int *filedesc)
{
	int len, file, version, temp, temp1;	
	char *filedata, str[12], psxname[32];
	
	len= strlen(name);
	if(len<3) return 0;
	if( name[len-1]=='/' ) return 0;

	if( name[len-1]=='x') file= open(name, 0);
	else {
		sprintf(psxname, "%s.psx", name);
		file= open(psxname, 0);
	}

	if(file == -1) {
		printf("Can't find file %s\n", name);
		return 0;
	}
	
	read(file, &temp, 4);
	read(file, &temp1, 4);
	if(temp!=BLEN || temp1!=DPSX) {
		close(file);
		guru("no BLENDPSX file!");
		return 0;
	}
	read(file, &version, 4);
	str[0]= ( (char *)&version )[1];
	str[1]= ( (char *)&version )[2];
	str[2]= ( (char *)&version )[3];
	str[3]= 0;
	G.versionfile= atoi(str);

	if(G.versionfile!=104) {
		close(file);
		guru("wrong version");
		return 0;
	}
	
	read(file, filelen, 4);
	
	/* eerste deel file inlezen */
	filedata= mallocN(*filelen, "filedata");
	read(file, filedata, *filelen);
	
	/* voor data */
	*filedesc= file;
	
	/* op zoek naar SDNA */
	/* read_file_dna(filedata); */

	return filedata;
}

void lib_link_all(pMain *main)
{
	
	/* do_versions(main); */
	lib_link_scene(main);
	lib_link_object(main);
	lib_link_image(main); 
	lib_link_mesh(main); // doet ook graphics init
	lib_link_sector(main);
	lib_link_life(main);
	// lib_link_material(main);

	/* lib_link_key(main); */
	/* lib_link_world(main); */
	/* lib_link_lamp(main); */
	/* lib_link_latt(main); */
	/* lib_link_ika(main); */

	/* alleen users goedzetten */
	/* lib_link_library(main);	 */

	G.scene= main->scene.first;
}


void read_file()
{
	extern short debuginfo;
	BHead *bhead;
	pMain *main;
	pObject *ob;
	int ok, len, file, filelen, skipdata, temp, temp1, version;
	char *filedata, *fd;
	
	G.f &= ~G_LOADFILE;
	
	fade_to_black();
	
	/* weinig geheugen, dus voor lezen: vrijgeven */
	free_main(G.main);

	/* deze doet alvast de append-data */
	filedata= openblendpsxfile(G.main->name, &filelen, &file);

	if(filedata) {
		
		next_image(0, 0);	// init
		
		if(debuginfo) printf("read finished\n");
		G.main->filedata= filedata;	
		
		/* alle data linken, images lezen: */
		fd= filedata;
		ok= 0;
		main= G.main;
			
		while(filelen>0) {
			
			bhead= (BHead *)fd;

			switch(bhead->code) {
			case GLOB:
				G.actvarar= (short *)(bhead+1);
				skipdata= bhead->len+sizeof(BHead);
				break;
			case DATA:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case DNA1:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case TEST:
				skipdata= bhead->len+sizeof(BHead);
				break;
			case ID_LI:
				/* skipdata= read_libblock(G.main, bhead, LIB_LOCAL); */
				/* main= G.mainbase.last; */
				break;
			case ID_ID:
				/* skipdata= read_libblock(main, bhead, LIB_READ+LIB_EXTERN); */
				break;
			default:
				skipdata= read_libblock(G.main, bhead, LIB_LOCAL);
			}
			
			fd+= skipdata;
			filelen-= skipdata;
		}

		/* voor de read_append!!! */
		if(G.f & G_NETWORK) G.fb_buf3= fb_add(  640, 0,  XSCR, YSCR, "Screen2");	// camera netwerk 3e buf

		/* images en sounds: niet malloccen */
		ok= read_append_data(file);
		close(file);

		if(ok==0) {
			guru("Warning: file not complete");
			exit_func();
		}

		/* read_libraries(); */
		
		/* LibData linken */
		
		lib_link_all(G.main);

		/* losslingerende blokken vrijgeven */
		add_data_adr(0, 0);
		add_lib_adr(0, 0);

		set_scene(G.scene);

		if(debuginfo) printf("linking finished\n");
	}
}



