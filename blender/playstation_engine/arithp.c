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

 *					 arithp.c
 *
 */

#include "psxdef.h"
#include "psxgraph.h"
#include "psxblend.h"


MATRIX matone= {4096,0,0,  0,4096,0,  0,0,4096,  0,0,0};

int Normalises(short *vec)
{
	int work[3], sq;
	
	VECCOPY(work, vec);
	sq= VectorNormalS((VECTOR *)work, (SVECTOR *)vec);

	return SquareRoot0(sq);
}

void printmatrix(MATRIX *m)
{
	printf("mat %d %d %d %d\n", m->m[0][0], m->m[0][1], m->m[0][2], m->t[0]);   
	printf("mat %d %d %d %d\n", m->m[1][0], m->m[1][1], m->m[1][2], m->t[1]);   
	printf("mat %d %d %d %d\n", m->m[2][0], m->m[2][1], m->m[2][2], m->t[2]);   
}	

void matpart_add_vec(MATRIX *mat, int part, int fac, VECTOR *vec)
{
	vec->vx +=  (fac*mat->m[part][0])>>12;
	vec->vy +=  (fac*mat->m[part][1])>>12;
	vec->vz +=  (fac*mat->m[part][2])>>12;
}

void matpart_add_vecs(MATRIX *mat, int part, int fac, SVECTOR *vec)
{
	
	vec->vx +=  (fac*mat->m[part][0])>>12;
	vec->vy +=  (fac*mat->m[part][1])>>12;
	vec->vz +=  (fac*mat->m[part][2])>>12;
	
	// patch
	if( abs(vec->vx)<100 && abs(vec->vy)<100)  {
		if(vec->vx<0) vec->vx= -100; else vec->vx= 100;
		if(vec->vy<0) vec->vy= -100; else vec->vy= 100;
	}
}	

void mat_ortho(MATRIX *mat)
{
	int work[3];
	int sqlen;

	// vervangen met: MatrixNormal(in, out);


	VECCOPY(work, mat->m[0]);
	VectorNormalS((VECTOR *)work, (SVECTOR *)mat->m[0]);
	VECCOPY(work, mat->m[1]);
	VectorNormalS((VECTOR *)work, (SVECTOR *)mat->m[1]);

	VECCOPY(work, mat->m[2]);
	sqlen= VectorNormalS((VECTOR *)work, (SVECTOR *)mat->m[2]);

	// if(sqlen) {
	// 	sqlen= SquareRoot0(sqlen);
	// 	mat->t[0]= (mat->t[0]<<12)/sqlen;
	// 	mat->t[1]= (mat->t[1]<<12)/sqlen;
	// 	mat->t[2]= (mat->t[2]<<12)/sqlen;
	// }
}

void mat_transp(MATRIX *mat)
{
	short t;

	t = mat->m[0][1] ; 
	mat->m[0][1] = mat->m[1][0] ; 
	mat->m[1][0] = t;
	t = mat->m[0][2] ; 
	mat->m[0][2] = mat->m[2][0] ; 
	mat->m[2][0] = t;
	t = mat->m[1][2] ; 
	mat->m[1][2] = mat->m[2][1] ; 
	mat->m[2][1] = t;
}

void mat_invert(MATRIX *imat, MATRIX *mat)
{
	// geen scaling !!!

	TransposeMatrix(mat, imat);

	// inverse translatievec 

	ApplyMatrixLV(imat, (VECTOR *)mat->t, (VECTOR *)imat->t);

	imat->t[0]= -imat->t[0];
	imat->t[1]= -imat->t[1]; 
	imat->t[2]= -imat->t[2]; 

	//imat->flag= MAT_CALC;
}

/* deze is langzamer!!! */
void eul_to_matrix1(short *eul, MATRIX *mat)
{
	short ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
	
	mat->m[0][0]=mat->m[1][1]=mat->m[2][2]= 4096;
	mat->m[0][1]=mat->m[1][2]=mat->m[2][1]= 0;
	mat->m[0][2]=mat->m[1][0]=mat->m[2][0]= 0;
	
	RotMatrixX(eul[0], mat);
	RotMatrixY(eul[1], mat);
	RotMatrixZ(eul[2], mat);
}

void eul_to_matrix(short *eul, MATRIX *mat)
{
	short ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
	
	ci = rcos(eul[0]); 
	cj = rcos(eul[1]); 
	ch = rcos(eul[2]);
	si = rsin(eul[0]); 
	sj = rsin(eul[1]); 
	sh = rsin(eul[2]);

	cc = (ci*ch)>>12; 
	cs = (ci*sh)>>12; 
	sc = (si*ch)>>12; 
	ss = (si*sh)>>12;

	// transpose!
	mat->m[0][0] = (cj*ch)>>12; 
	mat->m[0][1] = ((sj*sc)>>12) - cs ; 
	mat->m[0][2] = ((sj*cc)>>12) + ss;
	mat->m[1][0] = (cj*sh)>>12; 
	mat->m[1][1] = ((sj*ss)>>12) + cc; 
	mat->m[1][2] = ((sj*cs)>>12) - sc;
	mat->m[2][0] = -sj;	 
	mat->m[2][1] = (cj*si)>>12;
	mat->m[2][2] = (cj*ci)>>12;
	
}

void CrossS(c, a, b)
short *c, *a, *b;
{
	c[0] = (a[1] * b[2] - a[2] * b[1])>>12;
	c[1] = (a[2] * b[0] - a[0] * b[2])>>12;
	c[2] = (a[0] * b[1] - a[1] * b[0])>>12;
}

void Crossi(c, a, b)
int *c, *a, *b;
{
	c[0] = (a[1] * b[2] - a[2] * b[1])>>12;
	c[1] = (a[2] * b[0] - a[0] * b[2])>>12;
	c[2] = (a[0] * b[1] - a[1] * b[0])>>12;
}


void VecUpMat3(SVECTOR *vec, MATRIX *mat, short axis)
{
	VECTOR v;
	short inp;
	short cox, coy, coz;
	
	/* up varieeren heeft geen zin, is eigenlijk helemaal geen up!
	 */

	cox= 1; coy= 2; coz= 0;		/* Z up X tr */
	
	v.vx= vec->vx; v.vy= vec->vy; v.vz= vec->vz;
	VectorNormalS(&v, (SVECTOR *)mat->m[coz]);
	
	inp= mat->m[coz][2];
	v.vx= - ( (inp*mat->m[coz][0])>>12) ;
	v.vy= - ( (inp*mat->m[coz][1])>>12);
	v.vz= 4096 - ( (inp*mat->m[coz][2])>>12);
	VectorNormalS(&v, (SVECTOR *)mat->m[coy]);
	
	CrossS(mat->m[cox], mat->m[coy], mat->m[coz]);
	
}


