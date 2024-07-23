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

 *					 main01.c
 *					 MY second SONY!
 *
 */

#include "psxblend.h"
#include "psxgraph.h"

int old_sp;
short debuginfo= 0;
char str[160];	// van de stack af 

int psxprint_ot(int font, int x, int y, unsigned char *string, int unknown0, int unknown1) {
	return 0;
}
void exitprint() {
	return 0;
}
void free_font() {
	return 0;
}
int set_font() {
	return 0;
}
int set_font_prop() {
	return 0;
}
void load_menu_font() {
	return 0;
}
void free_menu_font() {
	return 0;
}
void fb_add() {
	return 0;
}

void draw_text_info()
{
	extern int sw_time, tots4, tots16;
	pBase *base;
	pObject *ob;
	pSector *se;
	int faces=0;
	char name[6];
	
	name[4]= 0;

	if(debuginfo) {
		sprintf(str, "swap: %d totsect: %d totface: %d mem: %dk", sw_time, G.totsect, G.totface, mem_in_use>>10);
		psxprint_ot(G.font, 10, 10, str, cdb->ot, &cdb->curpbuf);

	}
	if(debuginfo>=2) {

   		sprintf(str, "0 pbuf %d dbuf %d game %d calc %d draw %d", (int)(db[0].curpbuf-db[0].pbuf) , 
		(int)(db[0].curdbuf-db[0].dbuf), db[0].ga_time, db[0].ca_time, db[0].dr_time );
		psxprint_ot(G.font, 10, 20, str, cdb->ot, &cdb->curpbuf);

		sprintf(str, "1 pbuf %d dbuf %d game %d calc %d draw %d", (int)(db[1].curpbuf-db[1].pbuf) , 
		(int)(db[1].curdbuf-db[1].dbuf), db[1].ga_time, db[1].ca_time, db[1].dr_time );
		psxprint_ot(G.font, 10, 30, str, cdb->ot, &cdb->curpbuf);
	}
	
	tots4= tots16= 0;	

}

void draw_sector_info()
{
	/*
	pSector *se;
	int a, *vec, col;

	a= G.totsect;
	while(a--) {
		se= G.sectorbuf[a];
		if(se) {
			if(a==0) col= 0x4090; else col= 0x607060;
			vec= (int *)se->ob->obmat.t;
			psxboxi(160+((vec[0]-1024)>>7), 128-((vec[1]-1024)>>7), 
				160+((vec[0]+1024)>>7), 128-((vec[1]+1024)>>7), col);
		}
	}
	*/
}

void VecMat3MulVecfl(mat, vec, out)
float mat[][3];
float *vec, *out;
{
	float x,y;

	x=vec[0]; 
	y=vec[1];
	out[0]= x*mat[0][0] + y*mat[1][0] + mat[2][0]*vec[2];
	out[1]= x*mat[0][1] + y*mat[1][1] + mat[2][1]*vec[2];
	out[2]= x*mat[0][2] + y*mat[1][2] + mat[2][2]*vec[2];
}


void test_timer()
{
	MATRIX testmat;
	int a, fa, vec[3], out[3], t0, t1, t2, t3;
	short answ, vsho[3], vsho1[3];
	
	/* int mat */
	testmat.m[0][0]= 409;
	testmat.m[0][1]= 2024;
	testmat.m[0][2]= -800;
	testmat.m[1][0]= 2409;
	testmat.m[1][1]= 2024;
	testmat.m[1][2]= -2800;
	testmat.m[2][0]= 4409;
	testmat.m[2][1]= 4024;
	testmat.m[2][2]= -4800;
	vec[0]= 409;
	vec[1]= 2024;
	vec[2]= -800;

	vsho[0]= 409;
	vsho[1]= 2024;
	vsho[2]= -800;
	vsho1[0]= 409;
	vsho1[1]= 2024;
	vsho1[2]= -800;
	
	ResetRCnt(RCntCNT1);
	
	t0= GetRCnt(RCntCNT1);

	a= 2500;
	//testmat.flag= 0;
	while(a--) {
		ApplyMatrix12(&testmat, (VECTOR *)vec, (VECTOR *)out);
		out[0]*= 2;
	}
	
	t1= GetRCnt(RCntCNT1);
	
	a= 2500;
	//testmat.flag= MAT_ONE;
	while(a--) {
		ApplyMatrix12(&testmat, (VECTOR *)vec, (VECTOR *)out);
		out[0]*= 2;
	}
	
	t2= GetRCnt(RCntCNT1);

	printf("2500x int mat: %d fast mat: %d\n", t1-t0, t2-t1);


	ResetRCnt(RCntCNT1);
	
	t1= GetRCnt(RCntCNT1);
	ResetRCnt(RCntCNT1);

	a= 2500;
	while(a--) {
		fa= (vec[0]*out[0] + vec[1]*out[1] + vec[2]*out[2])>>12;
		fa*= 2;
	}
	
	t2= GetRCnt(RCntCNT1);
	ResetRCnt(RCntCNT1);
	
	a= 2500;
	while(a--) {
		answ= (vsho1[0]*vsho[0] + vsho1[1]*vsho[1] + vsho1[2]*vsho[2])>>12;
		answ*= 2;
	}
	
	t3= GetRCnt(RCntCNT1);

	printf("2500 int inpr: %d sho inpr: %d\n", t1, t2);



	{
		pObject *ob=G.scene->camera;

		ResetRCnt(RCntCNT1);

		a= 2500;
		while(a--) {
			eul_to_matrix(ob->rot, &ob->obmat);		
		}
		t1= GetRCnt(RCntCNT1);

		ResetRCnt(RCntCNT1);

		a= 2500;
		while(a--) {
			eul_to_matrix1(ob->rot, &ob->obmat);		
		}
		t2= GetRCnt(RCntCNT1);

		printf("2500 GTE eultoMat: %d  eigen: %d \n", t2, t1);


	}
}

void exit_func()
{
	cdb->draw.r0= cdb->draw.g0= 0;
	swapbuffers();
	frontbuffer(1);
	exitprint();
	if(G.font) free_font(G.font);
	free_menu_font();

	end_sectors();
	//movie_exit();
	end_blendpsx();
	end_display();
	end_psxutil();

	// end_sound();
	// set_dcache(0);	/* psxutil.c */
	
	printf("Exit OK\n");
	exit(1);
}


#define B_SETCLIP 	101
#define B_TEXMEM	102
#define B_SETSCENE	103

#define B_RESTART	200
#define B_LOADSTART	201
#define B_LOAD		210
#define B_LOAD1		211
#define B_LOAD2		212
#define B_LOAD3		213
#define B_LOAD4		214


void do_event(short event, short val)
{

	if(val!=64) return;	// (pad x) 
	
	switch (event) {
	case B_SETCLIP:
		SetGeomScreen(G.scene->lens);		/* distance to viewing-screen */
		break;
	case B_TEXMEM:
		viewmove();
		break;
	case B_SETSCENE:
		set_scene(G.scene);
		break;
	case B_RESTART:
		G.f |= G_RESTART;
		break;
	case B_LOADSTART:
		strcpy(G.main->name, "start.psx");
		G.f |= G_LOADFILE;
		break;
	case B_LOAD:
		strcpy(G.main->name, "rt.psx");
		G.f |= G_LOADFILE;
		break;
	case B_LOAD1:
		strcpy(G.main->name, "ton.psx");
		G.f |= G_LOADFILE;
		break;
	case B_LOAD2:
		strcpy(G.main->name, "joeri.psx");
		G.f |= G_LOADFILE;
		break;
	case B_LOAD3:
		G.f |= G_LOADFILE;
		break;
	case B_LOAD4:
		G.f |= G_LOADFILE;
		break;
	
	}
}

void make_menu()
{
	extern int subdiv4, subdiv16; // viewpsx.c
	extern int miststa, mistend;	//blendpsx.c
	short val, event;
	
	//while(PadRead(1));
	while(1);

	def_menu_block("Main");

/*
	def_menu_item (0, SUB, "LOAD");
	def_menu_item (0, SUB, "SCREEN");
	def_menu_item (0, SUB, "VIEW");
	def_menu_item (0, SUB, "MIST");
*/
	def_menu_block("LOAD");
    def_menu_item (B_RESTART, BUT, "Restart", 0, 0, 1);
    def_menu_item (B_LOADSTART, BUT, "start.psx", 0, 0, 1);
    def_menu_item (B_LOAD, BUT, "rt.psx", 0, 0, 1);
    def_menu_item (B_LOAD1, BUT, "ton.psx", 0, 0, 1);
    def_menu_item (B_LOAD2, BUT, "joeri.psx", 0, 0, 1);

	def_menu_block("SCREEN");

    def_menu_item (B_TEXMEM, BUT, "Scroll TexMem", 0, 0, 1);
    def_menu_item (0, NUM|SHO, "Debug Info", &debuginfo, 0, 2);

	def_menu_block("VIEW");

    def_menu_item (0, NUM|INT, "subdiv 4", &subdiv4, 0, 1600);
    def_menu_item (0, NUM|INT, "subdiv 16", &subdiv16, 0, 1600);
    if(G.scene) {
		def_menu_item (B_SETCLIP, NUM|SHO, "lens", &G.scene->lens, 150, 1000);
		def_menu_item (B_SETCLIP, NUM|SHO, "max sector", &G.scene->maxdrawsector, 1, 32);
		def_menu_item (B_SETCLIP, NUM|SHO, "rt", &G.scene->rt, -100, 100);
	}

	def_menu_block("MIST");
    def_menu_item (B_SETSCENE, NUM|INT, "mist sta", &miststa, 100, 6000);
    def_menu_item (B_SETSCENE, NUM|INT, "mist end", &mistend, 100, 6000);

    if(G.scene) {
    	def_menu_item (B_SETSCENE, TOG|CHA, "mist", &G.scene->mist,0,1);
		def_menu_item (B_SETSCENE, NUM|CHA, "mist r", &G.scene->mir, 0, 255);
    	def_menu_item (B_SETSCENE, NUM|CHA, "mist g", &G.scene->mig, 0, 255);
    	def_menu_item (B_SETSCENE, NUM|CHA, "mist b", &G.scene->mib, 0, 255);
	}

	frontbuffer(1);
	event= do_menu(1, &val);
	if(event) do_event(event, val);
	frontbuffer(0);

	if(val & PADh)  {	/* if start key is pressed: exit */
		exit_func();
	}
}


void test()
{
	MATRIX mat;
	SVECTOR olds;
	VECTOR old, new;
	
	mat= matone;
	mat.t[0]= 100;
	mat.t[1]= -400;

	olds.vx= 300;
	olds.vy= 0;
	olds.vz= 100;

	ApplyMatrix(&mat, &olds, &new);

	PRINT3(d,d,d, new.vx, new.vy, new.vz);
}
 
int main() {
	int a, padd, event, val;
	printf("Blender Game Engine is init...\n");

	printf("Starting PSXUTIL.\n");
		init_psxutil();	/* malloc */
	printf("Starting Display.\n");
		init_display();
	//set_divbuffer();
	//init_blackpoly();
	//init_blendpsx();
	//init_sound();
	printf("Loading font.\n");
	load_menu_font();
	

	// set_dcache(1);	/* psxutil.c */

	fb_add(960, 0,   64, 256, "MenuFont");
	G.font= set_font("fonts\\cour8.psf", 960, 0);
	
	if(G.font==0) {
		error("Can't find font");
		exit(0);
	}
	set_font_prop(G.font, 0);

	if(FALSE) {
		test_timer();
		old_sp= SetSp(DCACHE_TOP-12);
		printf("DCACHE ON\n");
		test_timer();
		SetSp(old_sp);
	}

	/* main loop */
	while(TRUE) {
		
		if(G.f & G_LOADFILE) read_file();
		if(G.f & G_RESTART) G.f &= ~G_RESTART;
		if(G.f & G_QUIT) exit_func();

		if(G.scene) sector_go();
		else make_menu(); 
	}
	return 0;
}

