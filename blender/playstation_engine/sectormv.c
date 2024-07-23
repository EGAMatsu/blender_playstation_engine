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
 *					 sectormv.c
 *
 */

#include "psxblend.h"
#include "psxgraph.h"
//#include "../frank/movie.h"
#include <libspu.h>

void sector_back(short doflush);
void evaluate_camera_network(pObject *set, short mode);

// globals
short visifac[2]= {2800, 1200};
short distfac[2]= {5000, 2000};
extern pObject *main_actor;
pAction *playmovie= 0;
BMovie *mainmovie= 0;
Stream *only_one= 0;
volatile char moviedone=2;

typedef struct pCamInfo {
	int loc[3];
	short rot[3], lens;
} pCamInfo;

#define CAMI MAKE_ID('C', 'A', 'M', 'I')


	/* capo->visi */
#define	CA_INVALID		0
#define	CA_ALWAYS_TEST	1
#define CA_TEST_OUTWARD	2
#define	CA_OK			3
	
	/* capo->direction */
#define CA_OUTWARD		0
#define CA_MIDDLE		1
#define CA_INWARD		2

	/* capo->dist */
#define CA_TOTAL		0
#define CA_MEDIUM		1
#define CA_CLOSE		2

	/* capo->d_axis */
/* #define CA_INVALID		0 */
#define CA_SMALL		1
/* #define CA_OK			3 */

	/* capo->view */
#define CA_SIDE		0
#define CA_FRONT	1
#define CA_BACK		2

	/* capo->flag */
#define CA_SETPOS		1


/*********************************
 * 
 * 
 * AUDIO STREAMING
 * 
 */


#define MAX_VOICE 2
#define ALL_VOICES ((1 << MAX_VOICE) - 1)

/* 44100 samples / seconde
 * = 1764 samples / frame
 * = 63 blokjes van 28 samples / frame
 * = 63 blokjes van 16 bytes / frame
 * = 1008 bytes / frame
 */

#define SPU_BUFSIZE		2016
#define SPU_BUFSIZEHALF (SPU_BUFSIZE >> 1)

SpuStEnv *st;

unsigned long st_stop;
#define ST_STOP_INIT_VAL 0

unsigned long st_stat = 0;
long st_load_ave = 0;

char spu_malloc_rec [SPU_MALLOC_RECSIZ * (MAX_VOICE + 1)];
char silence[SPU_BUFSIZEHALF];

char voicebuf[MAX_VOICE][SPU_BUFSIZEHALF];

int spu_started = FALSE;

void
spust_prepare (void)
{
	register long i;

	for (i = 0; i < MAX_VOICE; i ++) {
		st->voice [i].data_addr = (ulong) silence;
		st->voice [i].status    = SPU_ST_PLAY;
	}

	st_stop = ST_STOP_INIT_VAL;
	st_stat = 0;
	st_load_ave = 0;
}

long
spust_start (unsigned long voice_bit)
{
	if (st_stat == 0L) {
		spust_prepare ();
	}
	st_stat |= voice_bit;

	return SpuStTransfer (SPU_ST_PREPARE, voice_bit);
}

void
spustCB_next (unsigned long voice_bit)
{

}

SpuStCallbackProc
spustCB_preparation_finished (unsigned long voice_bit, long p_status)
{

	if (p_status == SPU_ST_PREPARE) {
		spustCB_next (voice_bit);
	}

	if (st_stat != 0L) {
		SpuStTransfer (SPU_ST_START, voice_bit);
	}
}

SpuStCallbackProc
spustCB_transfer_finished (unsigned long voice_bit, long t_status)
{
	spustCB_next (voice_bit);
}

SpuStCallbackProc
spustCB_stream_finished (unsigned long voice_bit, long s_status)
{
	SpuSetKey (SPU_OFF, voice_bit);
}

int init_audio_stream()
{
	
}


exit_audio_stream()
{
	
}


/********************************** */

void start_mainmovie(char *name)
{
/*
	if(name==0) return;
	if((G.f & G_NETWORK)==0) return;
	
	if(only_one) stream_close(only_one);
	if(mainmovie) movie_close(mainmovie->mv);
	else mainmovie= mallocN(sizeof(BMovie), "bmovie");
	
	bzero((char *)mainmovie, sizeof(BMovie));
	
	strcpy(mainmovie->file, name);
	strcat(mainmovie->file, ".mdc");
PRINT(s, mainmovie->file);
	only_one = stream_open(mainmovie->file);
	stream_add(only_one);

	mainmovie->mv= movie_open(only_one, 's');
	mainmovie->lastfra = -1;
	
	if(mainmovie->mv == 0) printf("can't find %s\n", mainmovie->file);
	else if (mainmovie->mv->audio_size) {
		mainmovie->mv->flags |= MV_play_audio;
		mainmovie->mv->frames--;
		init_audio_stream();
	}*/
}

void end_mainmovie()
{
	/*
	if(only_one) stream_close(only_one);
	only_one= 0;

	if(mainmovie) {
		movie_close(mainmovie->mv);
		freeN(mainmovie);
		mainmovie= 0;
	}
	
	exit_audio_stream();

printf("exit movie\n");*/
}

/* ************ NETWORK ************************* */


void sector_back(short doflush)
{		
	/*
	static MFrame *mframe= 0;
	Chunk *chunk;
	pCamInfo *ci=0;
	Movie *mv;
	pObject *ob;
	short sx, sy, ret;

	if((G.f & G_NETWORK)==0) return;

	if(mframe) {
		moviedone= 0;
		movie_drawframe(mframe, cdb->draw.clip.x, cdb->draw.clip.y, &moviedone);
		mframe= 0;
	}
	
	if(doflush) {
		VSync(0);	// loadimage afmaken
		VSync(0);
		VSync(0);

		playmovie= 0;		// mogelijk
		end_mainmovie();
		moviedone= 2;	// signaal voor 'al gedaan?'
		streaming_exit();
		return;
	}

	if(mainmovie==0 || mainmovie->mv==0) {	
		moviedone= 1;
		return;
	}
	

	// frame nr?
	mv= mainmovie->mv;
	CLAMP(mainmovie->cfra, 0, mv->frames-1);

	// al gedaan? 
	if(mainmovie->lastfra==mainmovie->cfra || moviedone==1) {
		if((G.f & G_PLAYMOVIE)==0) {
			RECT rt;
			
			if(moviedone==0) return;	// is bezig
			
			rt.x= 640; rt.y= 0;
			rt.w= 320; rt.h= 256;
			MoveImage(&rt, cdb->draw.clip.x, cdb->draw.clip.y);
			DrawSync(0);
			if(mainmovie->lastfra==mainmovie->cfra) return;
		}
	}

	// load next	
	mframe = movie_readframe(mv, mainmovie->cfra+1);
	if(mframe==0) { 
		printf("movie_readframe returned 0 fra:%d\n", mainmovie->cfra);
	
	}
	else if (movie_calcframe(mframe)) {
		printf("movie_calcframe error fra:%d\n", mainmovie->cfra);
		mframe= 0;
	}

	if(mframe==0) moviedone= 1;
	else {

		mainmovie->lastfra= mainmovie->cfra;

		// info struct lezen
		chunk = mframe->chunks.first;
		while(chunk) {
			if(chunk->id==CAMI) {
				ci= chunk->data;
				
				ob= G.scene->camera;
				VECCOPY(ob->loc, ci->loc);
				VECCOPY(ob->rot, ci->rot);

				object_to_viewmat(ob, &G.viewmat);

				G.scene->lens= ci->lens;
				SetGeomScreen(G.scene->lens);

				break;
			}
			chunk= chunk->next;
		}
		
		if(chunk==0) G.f |= G_PLAYMOVIE;
	}
	*/
}

void evaluate_camera(pCamPos *capo)
{
	VECTOR hoco, new;
	int sx, sy, inp;
	int min, max;
	short fl;
	
	ApplyMatrixLV(&capo->piramat, (VECTOR *)main_actor->obmat.t, &hoco);
	hoco.vx+= capo->piramat.t[0]; 
	hoco.vy+= capo->piramat.t[1]; 
	hoco.vz+= capo->piramat.t[2];

	capo->rt=max= hoco.vz;
	min= -max;
	
	fl= 0;

	if(hoco.vx < min) fl+= 1; else if(hoco.vx > max) fl+= 2;
	if(hoco.vy < min) fl+= 4; else if(hoco.vy > max) fl+= 8;
	if(hoco.vz < capo->sta) fl+= 16; else if(hoco.vz >capo->end) fl+= 32;
	
	if(fl) {
		capo->visi= CA_INVALID;
		return;
	}
	
	/* visibility */
	
	hoco.vx= ((hoco.vx<<12)/min);
	hoco.vy= ((hoco.vy<<12)/min);

	max= MAX2( abs(hoco.vx), abs(hoco.vy));
	if(max < visifac[1]) capo->visi= CA_OK;
	else if(max < visifac[0]) capo->visi= CA_TEST_OUTWARD;
	else capo->visi= CA_ALWAYS_TEST;
	
	/* dist: 'echte' coords */

	min= capo->sta + SMUL(800, capo->end - capo->sta);
	max= capo->sta + SMUL(3200, capo->end - capo->sta);

	if(hoco.vz > max) capo->dist= CA_TOTAL;
	else if(hoco.vz > min) capo->dist= CA_MEDIUM;
	else capo->dist= CA_CLOSE;
	
	/* internal direction (Y-as) */
	sx= hoco.vx;
	sy= hoco.vy;

	new.vx= main_actor->obmat.m[0][1];
	new.vy= main_actor->obmat.m[1][1];
	new.vz= main_actor->obmat.m[2][1];
	
	ApplyMatrixLV(&capo->piramat, &new, &hoco);
	
		/* eenheden van 256: ivm inprodukt test */
	capo->axis[0]= (hoco.vx>>4);
	capo->axis[1]= (hoco.vy>>4);
	
	if(capo->visi==CA_OK) capo->direction= CA_MIDDLE;
	else {
		inp= sx*hoco.vx + sy*hoco.vy;
		if(inp<-200) capo->direction= CA_OUTWARD;
		else if(inp>200) capo->direction= CA_INWARD;
		else capo->direction= CA_MIDDLE;
	}
	
	if(hoco.vz < -800) capo->view= CA_FRONT;
	else if(hoco.vz > 800) capo->view= CA_BACK;
	else capo->view= CA_SIDE;
}

pCamPos *find_best_campos2(pCamPos *curcapo)		/* aktie */
{
	pCamPos *capo, *bestcapo= 0;
	int a, round= 0;

	while(bestcapo==0) {

		if(curcapo->visi==CA_OK) bestcapo= curcapo;		/* stabiliseert */
		if(round==1 && curcapo->visi)  bestcapo= curcapo;
		
		capo= G.cursector->campos;
		for(a=0; a<G.cursector->totcam; a++, capo++) {
			
			if(capo->visi==CA_INVALID) continue;
			if(capo->d_axis==CA_INVALID) continue;		/* alleen 1e ronde */
			
			if(curcapo->visi==0 || curcapo->direction==CA_OUTWARD) {	/* altijd betere vinden */
				if(capo->direction!=CA_OUTWARD) {

					/* beste visi */
					if(bestcapo==0 || bestcapo->visi<capo->visi) bestcapo= capo;
					/* kleinste dist */
					if(bestcapo==0 || bestcapo->dist>capo->dist) bestcapo= capo;
				}
			}
			
		}
		
		if(bestcapo==0) {
			if(round==0) {
				capo= G.cursector->campos;
				for(a=0; a<G.cursector->totcam; a++, capo++) capo->d_axis= CA_SMALL;
			}
			else if(round==1) {
			
				/* noodgreep! */
				
				capo= G.cursector->campos;
				for(a=0; a<G.cursector->totcam; a++, capo++) {
					if(capo->visi) return capo;
				}
			}
			else break;
		}

		round++;
	}

	if(bestcapo==0) bestcapo= curcapo;
	
	return bestcapo;
}

pCamPos *find_best_campos3(pCamPos *curcapo)		/* aktie + back */
{
	pCamPos *capo, *bestcapo= 0;
	short a, round= 0;

	if(curcapo->visi && curcapo->view==CA_BACK) bestcapo= curcapo;

	capo= G.cursector->campos;
	for(a=0; a<G.cursector->totcam; a++, capo++) {
		
		if(capo->visi==CA_INVALID) continue;
		if(capo->view!=CA_BACK) continue;
		
		/* als we hier zijn: capo is back+visi */
		if(bestcapo==0) bestcapo= capo;
		
		/* beste axis */
		if(bestcapo->direction==0 && bestcapo->direction < capo->direction) bestcapo= capo;
		
		/* beste dist */
		if(capo->direction==CA_INWARD) {
			if(bestcapo->dist < capo->dist) bestcapo= capo;
			
			/* beste visi */
			if(capo->dist==CA_CLOSE) {
				if(bestcapo->visi < capo->visi) bestcapo= capo;
			}
		}
	}
	return bestcapo;
}


void evaluate_camera_network(pObject *set, short mode)
{
	static pCamPos *curcapo, *bestcapo;
	pCamPos *capo;
	pLife *lf;
	int inp;
	short a, newsec=0;
	
	if((G.f & G_NETWORK)==0) return;

	if(mode== -1) { // init
		curcapo= bestcapo= 0;
		playmovie= 0;
		return;
	}

	lf= main_actor->data;
	if(lf->sector != G.cursector) {
		curcapo= bestcapo= 0;
		G.cursector= lf->sector;
		if(playmovie==0 && G.cursector) start_mainmovie(G.cursector->name);
	}


	if(G.cursector==0 || G.cursector->totcam==0) return;

	if(set) {
		capo= G.cursector->campos;
		for(a=0; a<G.cursector->totcam; a++, capo++) {
			if(capo->ob==set) {
				bestcapo= capo;
				bestcapo->flag |= CA_SETPOS;
				if(curcapo) curcapo->dura= 0;
				bestcapo->dura= bestcapo->hold;
				break;
			}
		}
	}

	if(G.fields==0) return;
	if(mainmovie==0) return;

	if(playmovie) {		// event
		
		if(playmovie->cur>= playmovie->end) {
			playmovie= 0;
			if(G.cursector) start_mainmovie(G.cursector->name);
			G.f &= ~G_PLAYMOVIE;
		}
		else {
			//  was: playmovie->cur+= G.fields;

			playmovie->cur+= 2;
			mainmovie->cfra= playmovie->cur/2;
			
			return;
		}
	}

	if(curcapo && curcapo->nl) {	/* link aan't afspelen */
		
		curcapo->cfie+= G.fields;   // rekenen in fields

		if( curcapo->cfie >= curcapo->nl->len) {

			curcapo->nl= 0;
			curcapo= bestcapo;
			mainmovie->cfra= curcapo->sfra;
		}
		else {
			if(curcapo->nl->flag & NL_BACKWARD) 
				mainmovie->cfra= curcapo->nl->sfra - (curcapo->cfie/2);
			else mainmovie->cfra= curcapo->nl->sfra + (curcapo->cfie/2);

			return;
		}
	}

	if(curcapo==0) {		/* Of de eerste Of de huidige camera, NIET bestcapo */
		capo=curcapo= G.cursector->campos;
		if(curcapo==0) return;
		
		for(a=0; a<G.cursector->totcam; a++, capo++) {
			if(bestcapo==capo);
			else if(capo->ob==G.scene->camera) curcapo= capo;
		}
		curcapo->dura= 0;
		newsec= 1;
	}
	
	/* alle camera's in huidige sector aflopen PLUS huidige camera */
	capo= G.cursector->campos;
	for(a=0; a<G.cursector->totcam; a++, capo++) {
		if(capo!=curcapo) evaluate_camera(capo);
	}
	evaluate_camera(curcapo);
	
	
	/* de as regel */
	capo= G.cursector->campos;
	for(a=0; a<G.cursector->totcam; a++, capo++) {
		
		inp= curcapo->axis[0]*capo->axis[0] + curcapo->axis[1]*capo->axis[1];
		if(inp<-100) capo->d_axis= CA_INVALID;
		else if(inp<10000) capo->d_axis= CA_SMALL;
		else capo->d_axis= CA_OK;
		
		// if(G.qual & 1) PRINT4(x, d, d, d, capo, capo->visi, capo->dist, capo->view);

	}
	
	/* de conservatieve regel */
	if(curcapo->dura>0) curcapo->dura-= G.fields;

	if(curcapo->dura<=0) {

		/* de beste */	
		if(bestcapo && (bestcapo->flag & CA_SETPOS)) bestcapo->flag &= ~CA_SETPOS;
		else {
			bestcapo= find_best_campos3(curcapo);
			if(bestcapo==0) bestcapo= find_best_campos2(curcapo);	/* tzt vervangen */
		}
			
		if(bestcapo) {
			if(bestcapo!=curcapo) {
				pNetLink *nl= curcapo->ob->network.first;
				
				if(newsec) nl= 0;

				/* is er een netlink? */
				while(nl) {
					if(nl->ob==bestcapo->ob) {
						// tijdelijk (?) geen achterwaartse 
						if(nl->flag & NL_BACKWARD);
						else break;
					}
					nl= nl->next;
				}
			
				if(nl) {
					curcapo->nl= nl;
					curcapo->cfie= 0;	// fields
					mainmovie->cfra= nl->sfra;	// frames
				}
				else {
					mainmovie->cfra= bestcapo->sfra;	// frames
					curcapo= bestcapo;
				}
				bestcapo->dura= bestcapo->hold;


			}
			else mainmovie->cfra= bestcapo->sfra;	// frames
			
		}
	}
	else mainmovie->cfra= curcapo->sfra;
}





