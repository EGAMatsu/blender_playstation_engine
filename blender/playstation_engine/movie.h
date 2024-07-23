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




/* ***************** 

	movie.h

	0: init funktie
	
		DecDCTReset(0);

	1. open movie: 
		
		mv= movie_open(char * name, int FM);
	
		FM: 'm' laad in memory
			'f' als file (default)
			's'
			'fs' als stream file
			'ms' als stream to memory
			
	2. frame naar framebuffer:
	
		mf= movie_getframe(mv, frame, screenx, screeny, char * done);
		
		mv: movie pointer (wordt al vrijgegeven)
		frame: start bij 1
		screenx, y: linksboven eerste pixel
		done: testen of ie klaar is

	3. movie sluiten

	4. eind
	
		StopCallback();
	

**************** */

#include "psxutil.h"
#include <libpress.h>


#define MDEC MAKE_ID('M', 'D', 'E', 'C')
#define INFO MAKE_ID('I', 'N', 'F', 'O')
#define VIEW MAKE_ID('V', 'I', 'E', 'W')
#define CAMI MAKE_ID('C', 'A', 'M', 'I')
#define SSND MAKE_ID('S', 'S', 'N', 'D')
#define SLIC MAKE_ID('S', 'L', 'I', 'C')


#define MOVIE_MAGIC 0x4d56

	enum{
		_MV_24bits = 0,
		_MV_nodither,
		_MV_loop,
		_MV_pingpong,
		_MV_close_fd,
		_MV_play_audio,
	};
	
	#define MV_24bits (1 << _MV_24bits)
	#define MV_nodither (1 << _MV_nodither)
	#define MV_close_fd (1 << _MV_close_fd)
	#define MV_play_audio (1 << _MV_play_audio)

/* een paar structen */

	typedef struct Movie{
		ushort	magic, version;
		ushort	sizex;
		ushort	sizey;
		ushort	frames;
		ushort	flags;
	/*
				MV_video
				MV_horizontal
				MV_pingpong
				MV_loop
	
				MV_audio
				MV_stereo
				MV_38Khz
	*/
		ushort	current;
		ushort	audio_size;
		uchar	ytab[64];
		uchar	ctab[64];	
		int		fd;
		int		padint[16];
		uint 	offset[1];
	}Movie;
	
	
	typedef struct RunL{
		struct	RunL * next, * prev;
		RECT	rect;
		long	size;
		ulong	data[1];
	}RunL;
	
	typedef struct MFrame{
		struct	MFrame * next, * prev;
		
		ulong	* runl;			// wordt dynamisch gealloceerd
		ulong	* image;		// single ?? buffer
		RECT	slicerect;		// waar moet het plaatje komen te staan
		short 	minx, maxx;		// als !=0 clip
		int		slicesize;		// hoeveel pixels in kolom
		char	use24;			// 24 bits output
		char	count;			// aantal kolommen
		char	draw_started;	// er mag getekend gaan worden
		char	in_started;		// loopt er al een decdctin voor dit frame ?
		char	out_started;	// loopt er al een decdctout voor dit frame ?
		char	video_finished;	// video klaar
		char	audio_finished;	// audio klaar
		char	finished;		// old mdec klaar
		short	addr, addb, addy;
		ushort	sizex, sizey, flags;	
		Movie	* rt_movie;
		volatile char	* done;
		void	* data;			// voor file_movies
		ListBase chunks;
		DECDCTENV env;
		ulong	voice[8];
		RunL	* nextrun;
		ListBase runs;
	}MFrame;

	typedef struct Chunk{
		struct	Chunk * next, *prev;
		ulong	id;
		ulong	size;
		void	* data;
	}Chunk;
	
extern		ListBase * mdec_list;


/* een paar functie definities */

extern void start_dct_in();
extern void start_dct_out();
extern Movie * movie_open(void * pnt, int FM);
extern MFrame * movie_getframe(Movie * movie, int frame, int screenx, int screeny, volatile char * done);
extern MFrame * movie_readframe(Movie * movie, int frame);
extern int movie_calcframe(MFrame *);
extern int movie_drawframe(MFrame *, int screenx, int screeny, volatile char * done);
extern void free_mframe(MFrame * mframe);
extern Chunk * frame_find_chunk (MFrame * mframe, ulong id);
extern void movie_init();
extern void movie_exit();








