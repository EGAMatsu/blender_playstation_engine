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

/*  blendpsx.c   juli 96  

 * 
 * 
 */


#include "psxdef.h"
#include "psxblend.h"
#include "psxgraph.h"

struct pGlobal G;

/*
-	denk aan: (mmgm.obj)
	InitHeap( head, size );
*/


void next_camera()
{
	pBase *old, *base;
	int vec[3], local[3];
	char str[8];

	str[4]= 0;

	base= FIRSTBASE;
	while(base) {
		if(base->object==G.scene->camera) break;
		base= base->next;
	}

	old= base;
	if(base) base= base->next;
	if(base==0) base= FIRSTBASE;

	while(base) {
		
		if(base->object->type==OB_CAMERA) {
			if(old!=base && (G.scene->lay & base->lay)) {
			
				G.scene->camera= base->object;

				object_to_viewmat(G.scene->camera, &G.viewmat);
				COPY_4(str, G.scene->camera->id.name);
				printf("camera %x %s\n", base->object, str);

				vec[0]= G.scene->camera->obmat.t[0]<<2;
				vec[1]= G.scene->camera->obmat.t[1]<<2;
				vec[2]= G.scene->camera->obmat.t[2]<<2;
				G.cursector= (pSector *)find_sector(vec, local);	

				return;
			}
		}

		base= base->next;
		if(base==0 && old) {
			old= 0;
			base= FIRSTBASE;
		}
	}

}

int miststa= 2800, mistend=4096;


void set_scene(pScene *sce)
{

	G.scene= sce;

	db[0].draw.r0= db[1].draw.r0= G.scene->mir;
	db[0].draw.g0= db[1].draw.g0= G.scene->mig;
	db[0].draw.b0= db[1].draw.b0= G.scene->mib;

	/* lens, mist etc */
	SetFarColor(G.scene->mir, G.scene->mig, G.scene->mib);

	SetFogNearFar(4*miststa, 4*mistend, G.scene->lens);
	initmatrices();

}


void free_images(ListBase *lb)
{
	pImage *ima;
	
	ima= lb->first;
	while(ima) {
		
		if(ima->fba_rect) fb_free(ima->fba_rect);
		if(ima->fba_clut) fb_free(ima->fba_clut);
		ima->fba_rect= 0;
		ima->fba_clut= 0;
		if(ima->flag & IMA_FREECLUT) freeN(ima->clut);
		ima->clut= 0;
		
		ima= ima->id.next;
	}
}	

void free_main()
{
	ListBase zero;
	pID *id;
	pMesh *me, *men;

	if(G.main==0) return;
	
	/* zolang alles uit de file komt: niet libdata vrijgeven! */

	if(G.main->filedata) freeN(G.main->filedata);
	G.main->filedata= 0;
	G.scene= 0;
	G.camera= 0;
	G.cursector= 0;
	G.totlife= 0;
	G.totsect= 0;
	
	free_images(&G.main->image);	/* ook de fb_free */
	
	zero.first= zero.last= 0;
	G.main->scene= zero;
	G.main->library= zero;
	G.main->object= zero;
	G.main->mesh= zero;
	G.main->mat= zero;
	G.main->image= zero;
	G.main->ika= zero;
	G.main->sector= zero;
	G.main->life= zero;
	G.main->lamp= zero;
	G.main->camera= zero;
	G.main->ipo= zero;
	G.main->key= zero;
	G.main->world= zero;

	G.f &= ~(G_NETWORK|G_PLAYMOVIE);
	// cameranet 3e buffer?
	if(G.fb_buf3) fb_free(G.fb_buf3);
	G.fb_buf3= 0;
	
}

void init_blendpsx()
{
	
	bzero((char *)&G, sizeof(pGlobal));
	
	G.main= callocN(sizeof(pMain), "initglobals");
	addtail(&G.mainbase, G.main);
	
	strcpy(G.main->name, "start.psx");
	G.f |= G_LOADFILE;
	
	G.maxsect= 9;
	G.defaultmaterial.ref= 409;

}

void end_blendpsx()
{

	free_main();
	freeN(G.main);
	G.main= 0;
	
	
}	


void templfunc(pObject *ob)
{
	MATRIX tmat, tempmat;
	pLife *lf= ob->data;
	
	if(lf->flag & LF_RECALCLIGHT) {
		
		// in deze volgorde! en MET transpose. Alle combinaties gestest...
		MulMatrix0( &ob->imat, (MATRIX *)ob->loclight[0], &tmat);		// destroys!

		TransposeMatrix(&tmat, &tempmat);

		// MulMatrix0( &G.viewmat, &tempmat, &tmat);		// destroys!
		
		SetLightMatrix(&tempmat);
	}
	else SetLightMatrix( (MATRIX *)ob->loclight[0]);

}

