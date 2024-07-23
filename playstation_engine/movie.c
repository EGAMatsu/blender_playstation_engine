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



/* *****************************
 * 
 * 
 *  movie.c 
 * 
 * 
 * 
 * 
 */
 
#include <sys/types.h>
#include <sys/file.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <stdio.h>
#include <libcd.h>

#include "movie.h"

/* een paar globale variabelen */

ListBase _mdec_list = {0, 0}, * mdec_list = &_mdec_list;
volatile MFrame * in_mframe = 0, * out_mframe = 0;
int ditint = 120;
char str[128];

extern void draw_it(int, char *);


Chunk * frame_find_chunk (MFrame * mframe, ulong id)
{
	Chunk * chunk;
	char tmp[5];
	
	chunk = mframe->chunks.first;
	
	if (chunk == 0)	guru("No chunks\n");
	
	while (chunk) {
		// memcpy(tmp, &chunk->id,4);
		// tmp[4] = 0;
		// printf("checking chunk '%s'\n",tmp);
		
		if (chunk->id == id) return (chunk);
		chunk = chunk->next;
	}
	
	return (0);
}

void movie_close(Movie *movie)
{
	/* file close en freeN */
	
	if (movie == 0) return;
	
	if (movie->flags & MV_close_fd) {
		if(movie->fd != -1) close(movie->fd);
	}
	
	freeN(movie);	
}

Movie * movie_open(void * pnt, int FMS)
{
	int fd = -1, size, error = FALSE, flags = 0;
	Movie * movie, _movie;
	char	* name;
	C_File * c_file;
	
	// wordt er een file of een naam doorgegeven
		
		if (((FMS | 0x20) & 0xff) == 's') {
			fd = (int) pnt;
			c_file = pnt;
			if (is_cd == 0) name = "pc_filedescriptor";
			else name = c_file->name;
			FMS >>= 8;
		} else {
			name = pnt;
			fd = open(name, O_RDONLY);
			flags |= MV_close_fd;
		}
			
	if (fd == -1) {
		sprintf(str, "Couldn't open movie '%s'\n", name);
		guru(str);
		return(0);
	}
	
	// sprintf(str, "open OK [%d]", fd);
	// guru(str);
	
	// moet de hele movie in het geheugen gelezen worden ?
	
		if ((FMS | 0x20) == 'm') {
			size = lseek(fd, 0, 2);
			lseek(fd, 0, 0);
			movie = (Movie *) mallocN(size, "MovieMem");
			if (movie) {
				if (read(fd, movie, size) != size) {
					sprintf(str, "%s: read error\n", name);
					guru(str);
					error = TRUE;
				}
			} else {
				guru("Out of mem : Using file version instead\n");
				FMS = 'f';
			}
			if (flags & MV_close_fd) {
				close(fd);
				fd = -1;
			}
		}
	

	// of komt de movie uit een file (FMS == 'f')

		if ((FMS | 0x20) != 'm') {
			movie = &_movie;
			
			if (read(fd, movie, sizeof(Movie)) != sizeof(Movie)) {
				sprintf(str, "%s: read1 error\n", name);
				guru(str);
				error = TRUE;
			} else {
				size = movie->frames * sizeof(movie->offset[0]);
				movie = (Movie *) mallocN(sizeof(Movie) + size, "MovieFile");
				if (movie) {
					memcpy(movie, &_movie, sizeof(Movie));
					if (read(fd, &movie->offset[1], size) != size) {
						sprintf(str, "%s: read2 error\n", name);
						guru(str);
						error = TRUE;
					}
				}
			}
		}
	
	// guru("read OK");
	
	/* controleer of alles wel klopt */
	
		if (movie) {
			movie->fd = fd;
			movie->flags |= flags;
			if (movie->magic != MOVIE_MAGIC || movie->version > 1) {
				guru("Unsupported file format\n");
				error = TRUE;
			}
		}
	
	
	if (error) {
		if (movie) {
			if (movie != &_movie) freeN(movie);
			movie = 0;
		}
		if (fd != -1) {
			if (flags & MV_close_fd) close(fd);
		}
	}

	return(movie);
}


void free_mframe(MFrame * mframe)
{
	if (mframe == 0) return;
	
	if (mframe->runl) freeN(mframe->runl);
	if (mframe->image) freeN(mframe->image);
	if (mframe->data) freeN(mframe->data);
	
	freelistN(&mframe->chunks);
	
	freeN(mframe);
}


void movie_exit()
{
	MFrame * mframe;
	
	DecDCTinCallback(0);
	DecDCToutCallback(0);
	
	// VSync(0);
	
	DecDCTinSync(0);
	DecDCToutSync(0);
	
	mframe = mdec_list->first;
	
	while (mframe) {
		free_mframe(mframe);
		mframe = mframe->next;
	}
	
	mdec_list->first = mdec_list->last = 0;
	
	in_mframe = out_mframe = 0;
	DecDCTReset(0);	
}


void movie_reset()
{
	DecDCTinCallback(0);
	DecDCToutCallback(0);
		
	if (in_mframe) {
		guru("In not ready\n");
		in_mframe->video_finished = TRUE;
		in_mframe->audio_finished = TRUE;
		if (in_mframe->done) *in_mframe->done = TRUE;
		in_mframe->done = 0;
		in_mframe = 0;
	}

	if (out_mframe) {
		guru("Out not ready\n");
		out_mframe->video_finished = TRUE;
		out_mframe->audio_finished = TRUE;
		if (out_mframe->done) *out_mframe->done = TRUE;
		out_mframe->done = 0;
		out_mframe = 0;
	}
	
	DecDCTReset(0);		
}

void movie_init()
{
	if (in_mframe)  guru("In not ready\n");
	if (out_mframe) guru("Out not ready\n");
	
	
	DecDCTReset(0);	
}


void decdctincallback()
{
	if (in_mframe == 0) {
		guru("decdctincallback: SHOULDN'T GET HERE\n");
		return;
	}

	in_mframe = 0;
	start_dct_in();
}


void decdctoutcallback()
{
	if (out_mframe == 0) {
		guru("decdctoutcallback: SHOULDN'T GET HERE\n");
		return;
	}
	
	// test op clipborders
	if(out_mframe->minx || out_mframe->maxx) {
		if (out_mframe->slicerect.x < out_mframe->minx);
		else if( out_mframe->slicerect.x >= out_mframe->maxx);
		else LoadImage((void *) &out_mframe->slicerect, out_mframe->image);
	}
	else LoadImage((void *) &out_mframe->slicerect, out_mframe->image);

	out_mframe->count--;
		
	if (out_mframe->count) {
		out_mframe->slicerect.x += out_mframe->slicerect.w;
		DecDCTout(out_mframe->image, out_mframe->slicesize);
		
	} else {
		/* variabele zetten ? */

		out_mframe->video_finished = TRUE;
		if (out_mframe->audio_finished) {
			if (out_mframe->done) *out_mframe->done = TRUE;
			out_mframe->done = 0;	
		}
		
		out_mframe = 0;
		start_dct_out();
	}
}


void start_dct_in()
{
	MFrame * mframe;
	
	if (in_mframe) return;
	DecDCTinCallback(0);
	
	mframe = mdec_list->first;
	while (mframe) {
		if (mframe->draw_started == TRUE && mframe->in_started == FALSE) {
			in_mframe = mframe;
			mframe->in_started = TRUE;
			
			// matrices goed zetten 
			DecDCTPutEnv(&mframe->env);
						
			DecDCTinCallback(decdctincallback);
			DecDCTin(mframe->runl, mframe->use24);
			return;
		}
		mframe = mframe->next;
	}
}

void start_dct_out()
{
	MFrame * mframe;
	
	if (out_mframe)	return;
	DecDCToutCallback(0);
	
	mframe = mdec_list->first;
	while (mframe) {
		if (mframe->in_started == TRUE && mframe->out_started == FALSE) {
			out_mframe = mframe;
			mframe->out_started = TRUE;
			DecDCToutCallback(decdctoutcallback);
			DecDCTout(mframe->image, mframe->slicesize);
			return;
		}
		mframe = mframe->next;
	}
}

void check_mdec_list()
{
	MFrame * mframe, * next;
	
	/* DEZE FUNCTIE MAG ALLEEN IN HET HOOFDPROGRAMMA AANGEROEPEN WORDEN */
	
	/* data vrijgeven */
	
		mframe = mdec_list->first;
		while (mframe) {
			next = mframe->next;
			if (mframe->video_finished && mframe->audio_finished) {
				remlink(mdec_list, mframe);
				free_mframe(mframe);
			}
			mframe = next;
		}
	
	
	/* IN en OUT starten */
	
		if (in_mframe == 0) start_dct_in();
		if (out_mframe == 0) start_dct_out();
}


MFrame * movie_readframe(Movie * movie, int frame)
{
	MFrame	* mframe = 0;
	Chunk	* chunk;
	ulong * data, * runl, size, framesize;
	int bpp, lines;
	
	// even alles controleren
		
		check_mdec_list();
		if (movie == 0) return (0);
	
		if (movie->magic != MOVIE_MAGIC || movie->version > 1) {
			guru("Not a (supported) movie !\n");
			return(0);
		}

	// frame # controleren. Slimmer oplossen aan de hand van loop / once / pingpong ??
	
		if (frame > movie->frames) return (0);
		if (frame < 1) return (0);
		
		frame--;
	
	// maak een mframe struct aan
	
		mframe = CLN(MFrame);
		if (mframe == 0) return(0);
		mframe->audio_finished = TRUE;
		mframe->video_finished = TRUE;
		
	// lees data in vanuit file, of uit geheugen
	
		framesize = movie->offset[frame + 1] - movie->offset[frame];
		if (movie->fd != -1) {
			mframe->data = data = mallocN(framesize, "frame_data");

			if (data == 0) {
				free_mframe(mframe);
				return(0);
			}
	
			lseek(movie->fd, movie->offset[frame], 0);
			
			if (framesize != read(movie->fd, data, framesize)) {
				sprintf(str, "Frame %d: read error\n", frame);
				guru(str);
				free_mframe(mframe);
				return(0);
			}

			// lines = VSync(1);
			// while ((lines + 3) > VSync(1)) ;

		} else data = (ulong *) ((uchar *) movie + movie->offset[frame]);

			
	// en nu de chunks uit elkaar plukken
	
		if (movie->version == 0) {
			chunk = CLN(Chunk);
			if (chunk == 0) {
				free_mframe(mframe);
				return(0);
			}
			
			chunk->id = MDEC;
			chunk->size = framesize;
			chunk->data = data;
			mframe->video_finished = FALSE;
			
			addtail(&mframe->chunks, chunk);
			

		} else if (movie->version == 1) {
			while (framesize > 0) {
				chunk = CLN(Chunk);
				if (chunk == 0) {
					free_mframe(mframe);
					return(0);
				}
				
				chunk->id = data[0];
				chunk->size = data[1];
				chunk->data = data + 2;
				addtail(&mframe->chunks, chunk);

				switch (data[0]) {
					case MDEC:
						{
							mframe->video_finished = FALSE;
							// static int first = TRUE;
							// if (first) guru("Found MDEC\n");
							// first = FALSE;
						}
						break;
					case INFO:
					case CAMI:
					case VIEW:
						break;
					case SSND:
						{
							if (movie->flags & MV_play_audio) {
								mframe->audio_finished = FALSE;
								mframe->voice[0] = (ulong) & data[2];
								mframe->voice[1] = mframe->voice[0] + movie->audio_size;
							}
							
							// static int first = TRUE;
							// if (first) guru("Found AUDI\n");
							// first = FALSE;
						}
						break;
					default:
						{
							sprintf(str, "0x%08x 0x%08x 0x%08x \n0x%08x", data[0], data[1], data[2], data[3]);
							//guru(str);
							//framesize = 0;
							//size = 0;
						}
						break;
				}

				// size moet deelbaar zijn door 4
				size = (chunk->size + 3) & ~3;
				framesize -= size + 8;
				data += (size + 8) >> 2;
			}
		}
	
	// afronden 
	
		mframe->sizex = movie->sizex;
		mframe->sizey = movie->sizey;
		mframe->flags = movie->flags;

	// DCTenv lezen: moet VOOR ditheren gebeuren
	
 		DecDCTGetEnv(&mframe->env);
		memcpy(&mframe->env.iq_y[0], &movie->ytab[0], 128);	

		addtail(mdec_list, mframe);
		check_mdec_list();
		
	return(mframe);
}

int movie_dither(MFrame * mframe, ulong size)
{
	ulong	count, extra, offset;
	ushort	*_out;
	register ushort * in, * out, val;
	short	addr, addb, addy, top, bottom;
	short	qscale, mat, quant;	

	// ditheren
	
	// zowel kleur als helderheid ditheren
	// patroontjes: 45 = run 51, 54 = run 59
	// oude was 53 (of misschien 52)

#define INDEX   59
#define DITINT 120
// #define DITINT ditint
#define MAXOFFSET INDEX
		
	addr = mframe->addr;
	addb = mframe->addb;
	addy = mframe->addy;
	// addy = 0;
	
	in = (ushort *) mframe->runl;
	extra = mframe->sizex * mframe->sizey;
	extra /= 256;
	extra *= 6;
			
	_out = out = mallocN(size + 2 * extra, "dither out");

	if (out) {
		*out++ = *in++;
		*out++ = *in++;
						
		// matrix aanpassen aan gewenste dither intensiteit
		
		qscale = (*in) >> 10;
		mat = mframe->env.iq_y[INDEX];
		quant = (DITINT / (mat * qscale)) + 0.5;
		
		if (quant == 0) {
			quant = 1;
			mat = (DITINT / qscale) + 0.5;
			mframe->env.iq_y[INDEX] = mat;
			mframe->env.iq_c[INDEX] = mat;
		}
		
		quant = ((INDEX - 1) << 10) + quant;
		
		// printf("qscale: %d\n", (*in) >> 10);
						
		for (count = extra / 6; count > 0; count --) {
			// DCT component
			// *out++ = *in++;
			
			top = bottom = *in++;
			top &= ~0x3ff;
			bottom <<= 6;
			bottom >>= 6;
			
			bottom += addr;
			if (bottom < -512) bottom = -512;
			else if (bottom > 511) bottom = 511;
			
			*out++ = (bottom & 0x3ff) + top;

			offset = 0;
			while (1)
			{
				val = *out++ = *in++;
				
				if (val == 0xfe00) {
					// ditherbaar ?
					if (offset < MAXOFFSET) {
						out[-1] = quant - (offset << 10);
						*out++ = 0xfe00;
					}
					break;
				} else {
					offset += (val >> 10) + 1;
				}
			}
			
			// DCT component
			// *out++ = *in++;
			
			top = bottom = *in++;
			top &= ~0x3ff;
			bottom <<= 6;
			bottom >>= 6;
			
			bottom += addb;
			if (bottom < -512) bottom = -512;
			else if (bottom > 511) bottom = 511;
			
			*out++ = (bottom & 0x3ff) + top;

			offset = 0;
			while (1)
			{
				val = *out++ = *in++;
				
				if (val == 0xfe00) {
					// ditherbaar ?
					if (offset < MAXOFFSET) {
						out[-1] = quant - (offset << 10);
						*out++ = 0xfe00;
					}
					break;
				} else {
					offset += (val >> 10) + 1;
				}
			}
			
			// DCT component
			// *out++ = *in++;
			
			top = bottom = *in++;
			top &= ~0x3ff;
			bottom <<= 6;
			bottom >>= 6;
			
			bottom += addy;
			if (bottom < -512) bottom = -512;
			else if (bottom > 511) bottom = 511;
			
			*out++ = (bottom & 0x3ff) + top;
				
			offset = 0;
			while (1)
			{
				val = *out++ = *in++;
				
				if (val == 0xfe00) {
					// ditherbaar ?
					if (offset < MAXOFFSET) {
						out[-1] = quant - (offset << 10);
						*out++ = 0xfe00;
					}
					break;
				} else {
					offset += (val >> 10) + 1;
				}
			}
			
			// DCT component
			// *out++ = *in++;
			
			top = bottom = *in++;
			top &= ~0x3ff;
			bottom <<= 6;
			bottom >>= 6;
			
			bottom += addy;
			if (bottom < -512) bottom = -512;
			else if (bottom > 511) bottom = 511;
			
			*out++ = (bottom & 0x3ff) + top;

			offset = 0;
			while (1)
			{
				val = *out++ = *in++;
				
				if (val == 0xfe00) {
					// ditherbaar ?
					if (offset < MAXOFFSET) {
						out[-1] = quant - (offset << 10);
						*out++ = 0xfe00;
					}
					break;
				} else {
					offset += (val >> 10) + 1;
				}
			}
			
			// DCT component
			// *out++ = *in++;
			
			top = bottom = *in++;
			top &= ~0x3ff;
			bottom <<= 6;
			bottom >>= 6;
			
			bottom += addy;
			if (bottom < -512) bottom = -512;
			else if (bottom > 511) bottom = 511;
			
			*out++ = (bottom & 0x3ff) + top;

			offset = 0;
			while (1)
			{
				val = *out++ = *in++;
				
				if (val == 0xfe00) {
					// ditherbaar ?
					if (offset < MAXOFFSET) {
						out[-1] = quant - (offset << 10);
						*out++ = 0xfe00;
					}
					break;
				} else {
					offset += (val >> 10) + 1;
				}
			}
			
			// DCT component
			// *out++ = *in++;
			
			top = bottom = *in++;
			top &= ~0x3ff;
			bottom <<= 6;
			bottom >>= 6;
			
			bottom += addy;
			if (bottom < -512) bottom = -512;
			else if (bottom > 511) bottom = 511;
			
			*out++ = (bottom & 0x3ff) + top;
			
			offset = 0;
			while (1)
			{
				val = *out++ = *in++;
				
				if (val == 0xfe00) {
					// ditherbaar ?
					if (offset < MAXOFFSET) {
						out[-1] = quant - (offset << 10);
						*out++ = 0xfe00;
					}
					break;
				} else {
					offset += (val >> 10) + 1;
				}
			}					
		}

		count = out - _out;
		count += 0x3f;
		count &= ~0x3f;
		count /= 2;
		
		_out[0] = count;
				
		// rest vullen met NOP's
		
		for (count = (_out - out) + (size >> 1) + extra; count > 0; count--) *out++ = 0xfe00;
		
		freeN(mframe->runl);
		mframe->runl = (void *) _out;
	}
}

int movie_calcframe (MFrame * mframe)
{
	Chunk	* chunk;
	ulong	size;
	// controles
	
		check_mdec_list();
		if (mframe == 0) return (1);
		
	// MDEC chunk opzoeken
		
		chunk = frame_find_chunk(mframe, MDEC);

		if (chunk) {
			// start conversie bs -> runl
		
			size = 4 * DecDCTBufSize(chunk->data) + 8 + 128;
			
			if (mframe->runl) freeN(mframe->runl);
			mframe->runl = mallocN(size, "mframe->runl");
			
			if (mframe->runl == 0) return (3);
			
			DecDCTvlc(chunk->data, mframe->runl);
			
			if (((mframe->flags & (MV_24bits | MV_nodither)) == 0) && ((PadRead(1) & 0x10000) == 0))
			{
				movie_dither(mframe, size);
			}
		}
		
	// afronden
		
		check_mdec_list();
		
	return(0);
}


int movie_drawframe (MFrame * mframe, int screenx, int screeny, volatile char * done)
{
	int ret, bpp;
	
	// controles
	
		check_mdec_list();
		if (mframe == 0) return (1);
	
	// is er al een runl ??
	
		if (mframe->runl == 0) {
			ret = movie_calcframe(mframe);
			if (ret) return (ret);
		}
	
	// overige waardes invullen
	
		if (mframe->flags & MV_24bits) mframe->use24 = TRUE;
		else mframe->use24 = FALSE;

		if (mframe->use24) bpp = 3;
		else bpp = 2;

		if (mframe->image) freeN(mframe->image);
		
		mframe->image = mallocN(16 * bpp * mframe->sizey, "mframe->image");
		if (mframe->image == 0) return(4);

		mframe->slicerect.x = screenx;
		mframe->slicerect.y = screeny;
		mframe->slicerect.w = 8 * bpp;
		mframe->slicerect.h = mframe->sizey;
		
		mframe->slicesize = (mframe->slicerect.w * mframe->slicerect.h) / 2;
		mframe->count = mframe->sizex / 16;		
 		mframe->done = done;

		mframe->draw_started = TRUE;
		
	// Nu kan het echte werk beginnen
		
		check_mdec_list();
	
	return(0);
}


