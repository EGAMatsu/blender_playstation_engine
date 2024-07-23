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

 *					 viewpsx.c
 *
 */


#include "psxblend.h"
#include "psxgraph.h"



MATRIX corr= {4096,0,0,  0,-4096,0,  0,0,-4096,  0,0,0};


void lookat_up(VECTOR *loc, SVECTOR *vec, MATRIX *mat)
{
	// in mat komt een viewmatrix
	
	VecUpMat3(vec, mat, 1);
	
	// inverse hoeft niet meer: vectomat doet dat //
	//TransposeMatrix(&viewt, mat);

	// inverse van loc
	ApplyMatrixLV(mat, loc, (VECTOR *)mat->t);
	mat->t[0]= -mat->t[0];
	mat->t[1]= -mat->t[1]; 
	mat->t[2]= -mat->t[2]; 

	// deze fie doet geen setmatrix: per object doen! 		

}	

void lookat_free(VECTOR *loc, SVECTOR *vec, MATRIX *mat)
{
	// in mat komt een viewmatrix

	VecUpMat3(vec, mat, 1);
	
	// inverse hoeft niet meer: vectomat doet dat //
	// TransposeMatrix(&viewt, mat);

	// inverse van loc
	ApplyMatrixLV(mat, loc, (VECTOR *)mat->t);
	mat->t[0]= -mat->t[0];
	mat->t[1]= -mat->t[1]; 
	mat->t[2]= -mat->t[2]; 

	// deze fie doet geen setmatrix: per object doen! 	
}	

void object_to_viewmat(pObject *ob, MATRIX *mat)
{
	// versie zonder imat: scheelt !!!
	MATRIX viewt;
	
	// dit tijdelijk:

	if(ob->parent==0) {
		eul_to_matrix(ob->rot, &ob->obmat);
		VECCOPY(ob->obmat.t, ob->loc);
	}

	// de correctiematrix roteert de cameraview naar GL formaat (0,0 linksonder, kijk naar neg z)
	// corrang.vx= 2048; //180 graden
	// corrang.vy= 0;
	// corrang.vz= 0;
	// RotMatrix(&corrang, &corr);

	// corrigeren
	MulMatrix0(&ob->obmat, &corr, mat);   
	
	MatrixNormal(mat, &viewt);

	if(HIRES) {
		viewt.m[0][0]= SMUL(viewt.m[0][0], (512<<12)/320);
		viewt.m[1][0]= SMUL(viewt.m[1][0], (512<<12)/320);
		viewt.m[2][0]= SMUL(viewt.m[2][0], (512<<12)/320);
	}

	
	// inverse //
	TransposeMatrix(&viewt, mat);

	// inverse translatievec 
	ApplyMatrixLV(mat, (VECTOR *)ob->obmat.t, (VECTOR *)mat->t);

	mat->t[0]= -mat->t[0];
	mat->t[1]= -mat->t[1]; 
	mat->t[2]= -mat->t[2]; 

}


void initmatrices()
{
	
	SetGeomScreen(G.scene->lens);		/* distance to viewing-screen */
	SetBackColor(0,0,0);      	/* set background(ambient) color*/

	G.viewmat.m[1][0]= 0;
	G.viewmat.m[1][1]= 0;
	G.viewmat.m[1][2]= -4096;
	G.viewmat.t[1]= -4000;
	G.viewmat.t[2]= 200;
	
	SetRotMatrix(&G.viewmat);
	SetTransMatrix(&G.viewmat);
}


int pad_reado()	/* minder oud: beetje dyna, werkt op aktieve camera */
{
	static pObject *lastcam= 0;
	static ulong opadd= 0;
	static short speed[3], omega[3];
	extern int sw_time;
	VECTOR *vec;
	SVECTOR *rot;
	ulong padd = PadRead(1);
	int	ret = 0, speedfac, rotfac;
	short frict;

	G.qual= padd;

	if(padd & PADh) {
		while( PadRead(1) & PADh);
		next_camera();
	}

	if(lastcam!=G.scene->camera) {
		lastcam= G.scene->camera;
		speed[0]=speed[1]=speed[2]= 0;
		omega[0]=omega[1]=omega[2]= 0;
	}

	if(lastcam->parent==0) {

		speedfac= sw_time/36;
		rotfac= sw_time/24;

		vec= (VECTOR *)G.scene->camera->obmat.t;
		rot= (SVECTOR *)G.scene->camera->rot;

		if (padd & PADLleft) omega[2]+= rotfac;
		if (padd & PADLright) omega[2]-= rotfac;

		if (padd & PADLup) omega[0]+= rotfac;
		if (padd & PADLdown) omega[0]-= rotfac;

		if (padd & PADl) speed[2]+= speedfac;
		if (padd & PADm) speed[2]-= speedfac;
		if (padd & PADn) speed[0]+= speedfac/2;
		if (padd & PADo) speed[0]-= speedfac/2;

		// friction: 4096==1.0
		frict= 3600;
		speed[0]= (speed[0]*frict)>>12;
		speed[1]= (speed[1]*frict)>>12;
		speed[2]= (speed[2]*frict)>>12;
		// afronden negatieve getallen!!! (bij frict==3600: hangt bij -8)
	 	if(speed[0]<0) speed[0]++;
		if(speed[1]<0) speed[1]++;
		if(speed[2]<0) speed[2]++;

		frict= 2700;
		omega[0]= (omega[0]*frict)>>12;
		omega[1]= (omega[1]*frict)>>12;
		omega[2]= (omega[2]*frict)>>12;
		// afronden negatieve getallen!!!
		if(omega[0]<0) omega[0]++;
		if(omega[1]<0) omega[1]++;
		if(omega[2]<0) omega[2]++;

		lastcam->rot[0]+= omega[0];
		lastcam->rot[1]+= omega[1];
		lastcam->rot[2]+= omega[2];

		lastcam->loc[0]+= (speed[0]*G.viewmat.m[0][0] + speed[2]*G.viewmat.m[2][0])>>12;
		lastcam->loc[1]+= (speed[0]*G.viewmat.m[0][1] + speed[2]*G.viewmat.m[2][1])>>12;
		lastcam->loc[2]+= (speed[0]*G.viewmat.m[0][2] + speed[2]*G.viewmat.m[2][2])>>12;
	}
	opadd= padd;
	return padd;
}


