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

#include <sys/types.h>

#include <sys/file.h>
#include <libetc.h>

#include "psxutil.h"
#include "file.h"

cdsector * cdcache = 0;

// geven aan welke indexen gebruikt zijn
	volatile uchar c_in_use[256];

// hoeveel blokken zijn er in gebruik (c_used - c_freed)
	ushort c_used = 0, c_freed = 0;

// listbase
	ListBase _c_list = {0, 0}, * c_list = &_c_list;

C_File * active_c_file = 0;
volatile C_File * urgent_c_file = 0;

volatile char c_status[32], c_result[8];
volatile int c_read_sector = 0, c_next_sector = 0, c_retry = 0;

#define MAX_RETRY 10
#define CDLREAD  CdlReadN


void cdreadycallback(uchar , uchar * );


void streaming_stop()
{
	if (CdReadyCallback(0) != cdreadycallback) return;
	
	CdControlB( CdlPause, 0, 0 );
	if (active_c_file) active_c_file->busy = FALSE;
	active_c_file = 0;
}

void c_file_start(C_File * c_file)
{
	CdlLOC	cdloc;

	if (c_file == 0) {
		guru("c_file_start: c_file = NULL");
		return;
	}
	
	if (c_file->index == 0) {
		guru("c_file_start: c_file->index = NULL");
		return;
	}
	
	// guru("c_file_start");

	c_file->done = FALSE;
	c_file->busy = TRUE;
	
	if ((c_file->sectorseek < 0) | (c_file->sectorseek > c_file->maxsector)) c_file->sectorseek = 0;
	
	active_c_file = c_file;
	c_next_sector = active_c_file->start + active_c_file->sectorseek;
	
	CdIntToPos (c_next_sector, &cdloc);
	if (CdControl(CDLREAD, (uchar *) &cdloc, 0) != 1) guru("CdControl");
}


void cdreadycallback(uchar status, uchar * result)
{
	ushort c_full;
	int index, offset;
	cdheader header;
	CdlLOC	cdloc;
	

	if ((active_c_file != 0) && (active_c_file->done == FALSE) && (active_c_file->index != 0)) {
		// file is nog niet removed en zo

		switch (status) {
			case CdlDataReady:
				// er is een nieuw blok gelezen
				
				c_full = c_used - c_freed;
 				
				if (c_full < 256) {
					// er is een block vrij: zoeken (dit kan 4 keer zo snel met longs....)
						for (index = 1; index < 256; index++) {
							if (c_in_use[index] == FALSE) break;
						}
						
					// lees sector in cache
						if (index != 256) {
							CdGetSector(&header, 3);						
							if (CdGetSector(&cdcache[index].data, 512)) {

							// if (CdGetSector(&cdcache[index], sizeof(cdcache[index]) / 4)) {
								// hier nog controleren of sector de juiste is...

								c_read_sector = CdPosToInt(&header.cdloc);
								if (c_read_sector == c_next_sector) {
									c_retry = 0;

									offset = c_read_sector - active_c_file->start;
									if (offset < 0 || offset > active_c_file->maxsector) {
										guru("Reading wrong sector !!");
									} else {
										// is block al gecached ??

										if (active_c_file->index[offset] == 0) {
											c_used++;
											c_in_use[index] = TRUE;
											active_c_file->index[offset] = index;
										} else {
											// we gaan springen want dit gedeelte is nog gecached...
											
											for (index = offset; index <= active_c_file->maxsector; index++) {
												if (active_c_file->index[index] == 0) break;
											}
											active_c_file->sectorseek = index;
											
											if (active_c_file->sectorseek > active_c_file->maxsector) active_c_file->done = TRUE;
										}
										
										if (offset == active_c_file->sectorseek) {
											// deze sector werd ook verwacht, niemand heeft sectorseek veranderd
	
											active_c_file->sectorseek++;
											
											if (active_c_file->sectorseek > active_c_file->maxsector) {
												// dit was de laatste sector. We zijn nu dus klaar.
												active_c_file->done = TRUE;
											} else {
												// is er een belangrijke file waar op gewacht wordt ?

												if (urgent_c_file == 0) {
													c_next_sector = active_c_file->start + active_c_file->sectorseek;;
													// ALLES EINDELIJK HELEMAAL GOED !!
													return;
												}
											}
										}
									}
								} else c_retry++;
							} else guru("CdGetSector");
						} else guru("c_used / c_freed incorrect");
				} else {
					// Hele cache is vol, we proberen het nog een keer
					// Dit is echter geen disk error
					c_retry = 0;
				}
				break;
			case CdlDiskError:
				strcpy((void *) c_status, "CdlDiskError");
				break;
			default:
				sprintf((void *) c_status, "Unknown: %d", status);
				break;
			
		}
	}
	
	if ((active_c_file == 0) || (active_c_file->done == TRUE) || (urgent_c_file != 0)) {
		// look for new file and start again
		
		if (active_c_file) active_c_file->busy = FALSE;

		if (urgent_c_file) {
			active_c_file = (void *) urgent_c_file;
			urgent_c_file = 0;
		} else {
			active_c_file = c_list->first;
			while(active_c_file) {
				if (active_c_file->done == FALSE) break;
				active_c_file = active_c_file->next;
			}
		}
	}
	
	if (active_c_file == 0) streaming_stop();
	else c_file_start(active_c_file);
}


void stream_exit()
{
	// controleren of er nog een callback is, en die dan netjes stop zetten !!
	// HOE ?
	
	streaming_stop();
	
	// cache sluiten
		if (cdcache) freeN(cdcache);
		cdcache = 0;
}

void stream_init()
{
	unsigned char mode;
	int i;
	
	static int first = TRUE;

	if (first) {
		first = FALSE;
		// guru("stream init");
	}

	strcpy((void *) c_status, "no error");
	
	// eerst alles netjes afsluiten
		stream_exit();

	// double speed: CdlModeSpeed, we lezen sector headers: CdlModeSize1
		mode = CdlModeSpeed | CdlModeSize1;
		CdControlB(CdlSetmode, &mode, 0);

	// VSync(3) != 3 * VSync(0)
		VSync(0);
		VSync(0);
		VSync(0);
		VSync(0);
	
	// cache openen
		if (cdcache == 0) cdcache = mallocstructN(cdsector, 256, "cdcache");
		if (cdcache == 0) guru("Not enough memory for cdcache !");
		
	// indexen goedzetten
		c_in_use[0] = TRUE;
		for (i = 1; i <= 255; i++) c_in_use[i] = FALSE;
	
	// variabelen goed zetten
		c_used = 1;
		c_freed = 0;
}

void streaming_start()
{
	C_File * c_file = 0;
	static int first = TRUE;

	if (first) {
		first = FALSE;
		// guru("streaming start");
	}

	if (is_cd == 0)	return;
	
	
	// is er iets te streamen ?
	
		if (c_list->first == 0) return;
	
	// is init al gedaan ?
	
		if (cdcache == 0) stream_init();
	
	if (CdReadyCallback(cdreadycallback) != cdreadycallback) {
		// streaming was niet (meer) bezig
		
		if (urgent_c_file) {
			c_file = urgent_c_file;
			urgent_c_file = 0;
			c_file_start(c_file);
			return;
		} else { 
			for (c_file = c_list->first; c_file; c_file = c_file->next) {
				if (c_file->done == FALSE) {
					c_file_start(c_file);
					return;
				}
			}
		}
		
		// error 
		CdReadyCallback(0);
	}
}

void stream_add(C_File * c_file)
{
	static int first = TRUE;
	if (first) {
		first = FALSE;
		// guru("add_stream");
	}
	
	if (is_cd == 0)	return;

	if (c_file == 0) {
		guru("stream_add: c_file = NULL");
		return;
	}
	
	c_file->done = c_file->busy = 0;
	c_file->sectorseek = 0;

	if (c_file->index == NULL) {
		c_file->index = callocN(c_file->maxsector + 1, "c_file->index");
	
		EnterCriticalSection();
			addtail(c_list, c_file);
		ExitCriticalSection();
	}
	
	streaming_start();
}

void stream_remove(C_File * c_file)
{
	int i, index;
	
	if (is_cd == 0)	return;

	if (c_file == 0) {
		guru("stream_remove: c_file = NULL");
		return;
	}
	
	if (c_file->index) {
		c_file->done = TRUE;
		while(c_file->busy);
		
		EnterCriticalSection();
			remlink(c_list, c_file);	
		ExitCriticalSection();
	
		for (i = 0; i <= c_file->maxsector; i++) {
			if (index = c_file->index[i]) {
				c_in_use[index] = 0;
				c_freed++;
			}
		}
		
		freeN(c_file->index);
		c_file->index = 0;
	}
}


