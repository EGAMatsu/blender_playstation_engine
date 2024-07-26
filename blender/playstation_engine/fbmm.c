
#ifdef SGI
#	include <sys/types.h>	/* u_char in ../../blend/psx.h */
#	include "../../blend/psx.h"
#	include "fbmm.h"

#else
#	include "psxutil.h"
#	include "psxgraph.h"
#	include "../joeri/fbmm.h"
#endif


struct FrameBufferAlloc fbA[MAXFBA];
int	   fbAcnt = -1;

int fb_free(int bf_ptr)
{
	if (fbA[bf_ptr].iu == FB_CLEAR){
		printf("Double free fp_ptr (%i %s)\n", bf_ptr, fbA[bf_ptr].name);
		return(-1);
	}
	fbA[bf_ptr].iu = FB_CLEAR;
					
	return(0);
}
int fb_free_all(short permission)
{
	int i;
	for (i=0; i<MAXFBA; i++){
		if ( (fbA[i].iu & permission) == permission ) fb_free(i);
	}
}

int fb_add(int orgx, int orgy, int sizx, int sizy, char *name)
{
	int i, c;
	
	if (fbAcnt == -1){
		/* FirstTime so Init */
		for (i=0; i<MAXFBA; i++){
			fbA[i].iu = FB_CLEAR;
		}
		fbAcnt = 0; 
	}
	c = i = fbAcnt;
	while (fbA[i].iu != FB_CLEAR) { 
		i++; 
		if (i == MAXFBA) i = 0;
		if (i == c) return(-1);
	}
	
	if (strlen(name) > 15) { name[14] = 0; }
	fbA[i].sx = orgx;
	fbA[i].sy = orgy;
	fbA[i].ex = orgx + sizx-1;
	fbA[i].ey = orgy + sizy-1;
	fbA[i].iu = FB_ADD;
	strcpy(fbA[i].name, name);
	fbAcnt = i;
	return(i);
}

int	does_it_fit(int sx, int sy,  int ex, int ey)
{
	int i, fsx, fsy, fex, fey, bits =0; 
	
	if (sy <    0) return(1);
	if (ex > 1023) return(2);
	if (ey >  511) return(4);
	if (sx <    0) return(8);
	
	for (i=0; i<MAXFBA; i++){
		if (fbA[i].iu != FB_CLEAR){
			fsx = fbA[i].sx;
			fsy = fbA[i].sy;
			fex = fbA[i].ex;
			fey = fbA[i].ey;
		
			if ( (sx >=fsx) && (ex <=fex) && 
			     (sy >=fsy) && (ey <=fey) ) return(15);
			
			if ( (sx >=fsx) && (sx < fex) ){
				if ( (sy >=fsy) && (sy <=fey) ) return( 9);
				if ( (ey >=fsy) && (ey <=fey) ) return(12);
				if ( (sy < fsy) && (ey >=fey) ) return( 8);
			} 
			if ( (ex >=fsx) && (ex <=fex) ){
				if ( (sy >=fsy) && (sy <=fey) ) return( 3);
				if ( (ey >=fsy) && (ey <=fey) ) return( 6);
				if ( (sy < fsy) && (ey > fey) ) return( 2);
			}
			
			if ( (sx <fsx) && (ex >fex) ) {
				if ( (sy >=fsy) && (sy <=fey) ) return( 1);
				if ( (ey >=fsy) && (ey <=fey) ) return( 4);
			}
		}
	}
	return(0);
}

int fit_it(int *orgx, int *orgy, 
		   int dx,int dy, int sdy, int sizx,int sizy, int fbn)
{
	int x, y, h,  sx, sy, ex, ey, sh, eh, dh, bits;
	
	sx = 0 ; if (dx < 0)  sx = 1024 - sizx;  
	sy = 0 ; if (dy < 0)  sy =  512;  

	ex = 0; if (dx > 0)  ex = 1024;  
	ey = 0; if (dy > 0)  ey =  512;
	
	dx *=  64;
	dy *= 256;

	for(y = sy; y != ey; y += dy){
	
		sh = y; eh = y + 256 - sizy + 1;
		if (sdy < 0) { 
			sh = 512 - sizy; eh = 256;
			if (y < 512){
				sh = 256 - sizy; eh = 0;
			}
		}
		
		if ((dy < 0) && (sdy > 0)) { sh -=256; eh-=256; }
		
		/* printf("x:%d,%d y(%i):%d,%d h:%d,%d   %i\n", sx, ex, y,  sy, ey, sh, eh-1, sdy); */
		
		for (x = sx; x != ex; x += dx){
			for (h = sh; h != eh; h+=sdy){
			
				bits = does_it_fit(x, h, x+sizx-1, h+sizy-1);
	
				if (bits == 0) { 
					*orgx = x;
					*orgy = h;
					fbA[fbn].sx = x;
					fbA[fbn].sy = h;
					fbA[fbn].ex = x + sizx-1;
					fbA[fbn].ey = h + sizy-1;
					fbA[fbn].iu = FB_ALLOC;
					return(1);	/* OK */
				}
			}
		}
	}
	return(0);	/* ERROR */
}


int fb_alloc(int *putx, int *puty, int sizx, int sizy, char *name)
{
	int i, c, bits;
	int xb, yb;
	int sx, sy, ex, ey;
	int gputx, gputy;
	int searchtype;
	
	if (fbAcnt == -1){
		/* FirstTime so Init */
		for (i=0; i<MAXFBA; i++){
			fbA[i].iu = FB_CLEAR;
		}
		fbAcnt = 0;
	}
	c = i = fbAcnt; 
 
	
	if (sizy > 256) return(-3); 
	
	searchtype = 0;						/*  Garbage			*/
	if (sizy ==   1) searchtype = 1;	/*  Palette			*/
	if (sizy == 256) searchtype = 2;	/*  PictureBlock	*/

    
	while (fbA[i].iu != FB_CLEAR) { 
		i++; 
		if (i == MAXFBA) i = 0;
		if (i == c) return(-2);
	}
	if (strlen(name) > 15) name[14] = 0;
	fbAcnt = i; 
	
	sizx = ((sizx + 63) / 64);
	sizx = sizx * 64;
	
	switch(searchtype){
		case 0: 
			if( fit_it(&gputx, &gputy, -1,-1, -1, sizx, sizy, i)) {
				*putx = gputx;
				*puty = gputy;
				strcpy(fbA[i].name, name);
				return(i);
			}
			break;
			
		case 1: 
			if( fit_it(&gputx, &gputy, -1,-1,  1, sizx, 1, i)) {
				*putx = gputx;
				*puty = gputy;
				strcpy(fbA[i].name, name);
				return(i);
			}
			break;
			
		case 2:
			if( fit_it(&gputx, &gputy,  1, 1,  1, sizx, sizy, i)) {
				*putx = gputx;
				*puty = gputy;
				strcpy(fbA[i].name, name);
				return(i);
			}
			break;
	}
	return(-1);
}

void dev_print_fba_block()
{
	int i;
	
	for (i=0; i<MAXFBA; i++){
		if (fbA[i].iu != FB_CLEAR){
			printf("block nr%3i %16s type:%i s:%4i, %4i  e:%4i, %4i \n", 
			i, fbA[i].name, fbA[i].iu , fbA[i].sx , fbA[i].sy , fbA[i].ex , fbA[i].ey);
		}
	}
}
