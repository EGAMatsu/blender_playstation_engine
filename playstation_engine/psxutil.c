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
 * psxutil.c
 * 
 * de lijst en malloc functies uit de util.c
 * 
 * 
 */

#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <libetc.h>

#include "psxutil.h"
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>
#include <sys/file.h>
#include <ctype.h>

#undef s_used
#undef s_freed

#ifndef s_used
	extern volatile ushort s_used;
#endif
#ifndef s_freed
	extern volatile ushort s_freed;
#endif

void __stack_chk_fail() {
	return 0;
}

void __stack_chk_guard() {
	return 0;
}

void PCinit() {
	return 0;
}

int PCopen() {
	return 0;
}

int PCread() {
	return 0;
}

int PClseek() {
	return 0;
}

void PCclose() {
	return 0;
}

int fb_free() {
	return 0;
}

void fb_alloc() {
	return 0;
}

int MallocSize = 0x200000;
void * MallocStart = (void *) 0x80200000;

#define MALLOCSIZE MallocSize
#define MALLOCSTA MallocStart

/*
#define MALLOCSIZE 0x200000
#define MALLOCSTA 0x80200000
*/


/* DEBUG */

extern ListBase * osview;

typedef struct checkpoint
{
	struct checkpoint	* next, * prev;
	char	* file;
	int		line;
	char	* string;
	int		time;
}checkpoint;

#ifdef CHECK_DEBUG
#define CHECKPOINT(x) {checkpoint * cp = NEW(checkpoint); cp->string = # x; cp->line = __LINE__; cp->file = __FILE__; cp->time = GetRCnt(RCntCNT1); addtail(osview, cp);}
#else
#define CHECKPOINT(x)
#endif


short psx_init_done = FALSE;

/*
	addhead(listbase,link)
	addtail(listbase,link)
	remlink(listbase,link)
	insertlink(listbase,prevlink,newlink)
	freelist(listbase)
	freelistN(listbase)
	long countlist(listbase)
	
	void 	*mallocN(len,str)
	void	*callocN(len,str)
	short	freeN(adr)
	
	short 	fileselect(tekst,dir)

*/

short totblock = 0, is_cd = 0;
long mem_in_use = 0;
long current_mem_level = 0;

ListBase _membase = {0, 0}, *membase = &_membase;
ListBase _dir_list = {0, 0}, * dir_list = & _dir_list;
ListBase _s_list = {0, 0}, * s_list = &_s_list;

volatile int _line = 0;
volatile char * _file = 0;

char tmpname[128];
char gurustr[128];
int noguru = 0;

cdsector * cdcache = 0;

// geven aan welke indexen gebruikt zijn
	volatile uchar s_in_use[256];



Stream * active_stream = 0;
volatile Stream * urgent_stream = 0;

volatile char s_status[32], s_result[8];
volatile int s_read_sector = 0, s_next_sector = 0, s_retry = 0;

#define MAX_RETRY 10
#define CDLREAD  CdlReadN

void cdreadycallback(uchar , uchar * );

void addhead(listbase,link)
struct ListBase *listbase;
struct Link *link;
{
	if (link == 0) return;
	if (listbase == 0) return;

	link->next = listbase->first;
	link->prev = 0;

	if (listbase->first) ((struct Link *)listbase->first)->prev = link;
	if (listbase->last == 0) listbase->last = link;
	listbase->first = link;
}


void addtail(listbase,link)
struct ListBase *listbase;
struct Link *link;
{
	if (link == 0) return;
	if (listbase == 0) return;

	link->next = 0;
	link->prev = listbase->last;

	if (listbase->last) ((struct Link *)listbase->last)->next = link;
	if (listbase->first == 0) listbase->first = link;
	listbase->last = link;
}


void remlink(listbase,link)
struct ListBase *listbase;
struct Link *link;
{
	if (link == 0) return;
	if (listbase == 0) return;

	if (link->next) link->next->prev = link->prev;
	if (link->prev) link->prev->next = link->next;

	if (listbase->last == link) listbase->last = link->prev;
	if (listbase->first == link) listbase->first = link->next;
}


void insertlink(listbase,prevlink,newlink)
struct ListBase *listbase;
struct Link *prevlink,*newlink;
{
	/* newlink komt na prevlink */

	if (newlink == 0) return;
	if (listbase == 0) return;

	if(listbase->first==0) { /* lege lijst */
		listbase->first= newlink;
		listbase->last= newlink;
		return;
	}
	if (prevlink== 0) {	/* inserten voor eerste element */
		newlink->next= listbase->first;
		newlink->prev= 0;
		newlink->next->prev= newlink;
		listbase->first= newlink;
		return;
	}

	if (listbase->last== prevlink) /* aan einde lijst */
		listbase->last = newlink;

	newlink->next= prevlink->next;
	prevlink->next= newlink;
	if(newlink->next) newlink->next->prev= newlink;
	newlink->prev= prevlink;
}

void insertlinkbefore(listbase, nextlink, newlink)
struct ListBase *listbase;
struct Link *nextlink,*newlink;
{
	/* newlink komt voor nextlink */

	if (newlink == 0) return;
	if (listbase == 0) return;

	if(listbase->first==0) { /* lege lijst */
		listbase->first= newlink;
		listbase->last= newlink;
		return;
	}
	if (nextlink== 0) {	/* inserten aan einde lijst */
		newlink->prev= listbase->last;
		newlink->next= 0;
		((struct Link *)listbase->last)->next= newlink;
		listbase->last= newlink;
		return;
	}

	if (listbase->first== nextlink) /* aan begin lijst */
		listbase->first = newlink;

	newlink->next= nextlink;
	newlink->prev= nextlink->prev;
	nextlink->prev= newlink;
	if(newlink->prev) newlink->prev->next= newlink;
}


void freelist(listbase)
struct ListBase *listbase;
{
	struct Link *link,*next;

	if (listbase == 0) return;
	link= listbase->first;
	while(link) {
		next= link->next;
		free(link);
		link= next;
	}
	listbase->first=0;
	listbase->last=0;
}


long countlist(listbase)
struct ListBase *listbase;
{
	Link * link;
	long count = 0;
	
	if (listbase){
		link = listbase->first;
		while(link) {
			count++;
			link= link->next;
		}
	}
	return(count);
}

#ifdef malloc
#undef malloc
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef free
#undef free
#endif


ListBase _freememlist = {0, 0}, * freememlist = & _freememlist;
ListBase _blocklist = {0, 0}, * blocklist = & _blocklist;

checkmemory()
{
	int totsize = 0;
	FreeBlock * fblock;
	MemBlock * mblock;
	
	mblock = blocklist->first;
	while (mblock)
	{
		totsize += mblock->size;
		mblock = mblock->next;
	}
		
	if (totsize != MallocSize) {
		sprintf(gurustr, "malloc: totsize != mallocsize\n %d != %d", totsize, MallocSize);
		guru(gurustr);
		
		printf("freeblocks:\n");
		
		fblock = freememlist->first;
		while (fblock)
		{
			printf(" %08x %d\n", fblock, fblock->block.size);
			fblock = fblock->next;
		}
		
		printf("memoryblocks:\n");

		mblock = blocklist->first;
		while (mblock)
		{
			printf("  %08x %d\n", mblock, mblock->size);
			mblock = mblock->next;
		}
	}
}

void InitHeap(unsigned long * start, unsigned long size)
{
	FreeBlock * fblock;
	
	if (1) {
		sprintf(gurustr, "in InitHeap (0x%x %d = %d kB)", start, size, (size + 512) >> 10);
		//guru(gurustr);
			
		MallocStart = start;
		MallocSize = size;
		
		fblock = (FreeBlock *) MallocStart;
		fblock->block.size = MallocSize;
		fblock->block.tag = MEMFREE;
		
		addtail(freememlist, fblock);
		addtail(blocklist, &fblock->block);
	} else {
		InitHeap2(start, size);
	}
}

int mallocNmalloc = FALSE;


int best_memblock(size_t len)
{
	FreeBlock * fblock;
	int size, bestsize = 0;
	
	size = len;
	if (size <= 0) return(0);
	
	size = (size + 3) & ~3;
	size += sizeof(FreeBlock);
	
	fblock = freememlist->first;

	while (fblock) {
		if (fblock->block.size >= size) {
			return(size);
		}
		if (fblock->block.size > bestsize) bestsize = fblock->block.size;
		fblock = fblock->next;
	}
	
	if (bestsize) bestsize -= sizeof(FreeBlock);
	return (bestsize);
}

void * malloc(size_t len)
{
	FreeBlock * fblock, * best = 0, * nblock;
	MemBlock * mblock, * mblock2;
	int bestsize = 0x7fffffff, newsize, size;
	
	size = len;
	
	if (mallocNmalloc == FALSE) printf("\n externalMalloc: size %d\n\n", len);

	if (size <= 0) return(0);
	
	size = (size + 3) & ~3;
	size += sizeof(FreeBlock);
	
	fblock = freememlist->first;

	while (fblock) {
		if (fblock->block.size >= size) {
			if (fblock->block.size < bestsize) {
				bestsize = fblock->block.size;
				best = fblock;
				if (bestsize == size) break;
			}
		}
		fblock = fblock->next;
	}
	
	if (best == 0) return(0);
	
	fblock = best;

	if (fblock->block.tag != MEMFREE) {
		guru("Malloc freetag corrupt");
		return(0);
	}
	
	// moeten we splitsen ?
	
		newsize = fblock->block.size - size;
		
		if (newsize >= sizeof(FreeBlock) + 32) {
			nblock = (FreeBlock *) (((uchar *) fblock) + size);
			nblock->block.size = newsize;
			nblock->block.tag = MEMFREE;
			addhead(freememlist, nblock);
			
			mblock = &fblock->block;
			mblock2 = &nblock->block;
			insertlink(blocklist, mblock, mblock2);
			
			fblock->block.size -= newsize;
		}
	
	fblock->block.tag = MEMBLCK;
	remlink(freememlist, fblock);

	fblock++;
	// printf("Malloc: %08x size %d\n", fblock, len);
	
	return(fblock);
	
}

void * calloc(size_t len, size_t count)
{
	uchar * pnt;
	
	guru("hi hi, in calloc\n");
	
	pnt = malloc(len * count);
	if (pnt) bzero(pnt, len * count);
	
	return(pnt);
}

void * realloc(void * pnt, size_t len)
{
	guru("realloc not implemented");
	return(0);
}

void free(void * vp)
{
	FreeBlock * fblock;
	MemBlock * pblock, * nblock, * mblock;
	short add_to_list = TRUE;
	
	if (vp == 0) {
		guru("freeing null pointer");
		return;
	}

	mblock = vp;
	mblock--;
	
	// printf("Free: %08x\n", vp);
	
	if (mblock->tag != MEMBLCK) {
		guru("free: block corrupt");
		return;
	}
	
	mblock->tag = MEMFREE;

	// samenvoegen met vorige ??

		pblock = mblock->prev;
	
		if (pblock) {
			if (pblock->tag == MEMFREE) {
				pblock->size += mblock->size;
				remlink(blocklist, mblock);
				
				mblock = pblock;
				add_to_list = FALSE;
			}
		}
	
	// samenvoegen met volgende ??
	
		nblock = mblock->next;
		if (nblock) {
			if (nblock->tag == MEMFREE) {
				remlink(blocklist, nblock);
				fblock = (FreeBlock *) ((uchar *) nblock - sizeof(FreeBlock) + sizeof(MemBlock));
				remlink(freememlist, fblock);
				mblock->size += nblock->size;
			}
		}
	
	if (add_to_list) {
		fblock = (FreeBlock *) ((uchar *) mblock - sizeof(FreeBlock) + sizeof(MemBlock));
		addhead(freememlist, fblock);
	}
}

void MemorY_ErroR(block,error)
char *block,*error;
{
	printf("Memoryblock %s: %s\n",block,error);
}

void (*memory_error)() = MemorY_ErroR;

void *mallocN(len,str)
long len;
char *str;
{
	MemHead *memh;
	MemTail *memt;
	static char buf[128];
	
	if(len<=0) {
		sprintf(gurustr, "Malloc error: len=%d in %s\n",len,str);
		guru(gurustr);
		return 0;
	}

	mallocNmalloc = TRUE;
	
	len = (len + 3 ) & ~3; 	/* eenheden van 4 */
	memh=(MemHead *)malloc(len+sizeof(MemHead)+sizeof(MemTail));

	mallocNmalloc = FALSE;
	
	if(memh!=0) {
		if( ((int)memh)+len > (int) MALLOCSTA + MALLOCSIZE ) {
			sprintf(gurustr, "Malloc beyond max: \nlen=%d in %s\n",len, str);
			sprintf(gurustr, "Malloc overflow in %s: \n%x > %x\n", str, (int)memh+len, (int) MALLOCSTA + MALLOCSIZE);
			guru(gurustr);
			
			// return 0;
			
		}

		memh->tag1 = MEMTAG1;
		memh->name = str;
		memh->nextname = 0;
		memh->len = len;
		memh->level = current_mem_level;
		memh->tag2 = MEMTAG2;

		memt = (MemTail *)(((char *) memh) + sizeof(MemHead) + len);
		memt->tag3 = MEMTAG3;

		addtail(membase,&memh->next);
		if (memh->next) memh->nextname = MEMNEXT(memh->next)->name;

		totblock++;
		mem_in_use += len;
		return (++memh);
	}
	
	sprintf(buf, "Malloc returns nill: len=%d in %s\n",len,str);
	guru(buf);
	
	return 0;
}

void *callocN(len,str)
long len;
char *str;
{
	void *poin;
	
	poin= mallocN(len, str);
	if(poin) bzero(poin, len);
	
	return poin;
}




void rem_memblock(memh)
MemHead *memh;
{
	remlink(membase,&memh->next);
	if (memh->prev){
		if (memh->next) MEMNEXT(memh->prev)->nextname = MEMNEXT(memh->next)->name;
		else MEMNEXT(memh->prev)->nextname = 0;
	}

	totblock--;
	mem_in_use -= memh->len;
	free(memh);
}


void printmemlist()
{
	MemHead *membl;

	membl = membase->first;
	if (membl) membl = MEMNEXT(membl);
	while(membl) {
		printf("%s len: %d\n",membl->name,membl->len);
		if(membl->next)
			membl= MEMNEXT(membl->next);
		else break;
	}
}

char *check_memlist(memh)
MemHead *memh;
{
	MemHead *forw,*back,*forwok,*backok;
	char *name;

	forw = membase->first;
	if (forw) forw = MEMNEXT(forw);
	forwok = 0;
	while(forw){
		if (forw->tag1 != MEMTAG1 || forw->tag2 != MEMTAG2) break;
		forwok = forw;
		if (forw->next) forw = MEMNEXT(forw->next);
		else forw = 0;
	}

	back = (MemHead *) membase->last;
	if (back) back = MEMNEXT(back);
	backok = 0;
	while(back){
		if (back->tag1 != MEMTAG1 || back->tag2 != MEMTAG2) break;
		backok = back;
		if (back->prev) back = MEMNEXT(back->prev);
		else back = 0;
	}

	if (forw != back) return ("MORE THAN 1 MEMORYBLOCK CORRUPT");

	if (forw == 0 && back == 0){
		/* geen foute headers gevonden dan maar op zoek naar memblock*/

		forw = membase->first;
		if (forw) forw = MEMNEXT(forw);
		forwok = 0;
		while(forw){
			if (forw == memh) break;
			if (forw->tag1 != MEMTAG1 || forw->tag2 != MEMTAG2) break;
			forwok = forw;
			if (forw->next) forw = MEMNEXT(forw->next);
			else forw = 0;
		}
		if (forw == 0) return (0);

		back = (MemHead *) membase->last;
		if (back) back = MEMNEXT(back);
		backok = 0;
		while(back){
			if (back == memh) break;
			if (back->tag1 != MEMTAG1 || back->tag2 != MEMTAG2) break;
			backok = back;
			if (back->prev) back = MEMNEXT(back->prev);
			else back = 0;
		}
	}

	if (forwok) name = forwok->nextname;
	else name = "No name found";

	if (forw == memh){
		/* voor alle zekerheid wordt dit block maar uit de lijst gehaald */
		if (forwok){
			if (backok){
				forwok->next = (MemHead *)&backok->next;
				backok->prev = (MemHead *)&forwok->next;
				forwok->nextname = backok->name;
			} else{
				forwok->next = 0;
				membase->last = (struct Link *) &forwok->next;
			}
		} else{
			if (backok){
				backok->prev = 0;
				membase->first = &backok->next;
			} else{
				membase->first = membase->last = 0;
			}
		}
	} else{
		memory_error(name,"Aditional error in header");
		return(0);
	}

	return(name);
}


short freeN(memh)
MemHead *memh;
{
	short error = 0;
	MemTail *memt;
	char *name;

	if (memh == 0){
		memory_error("free","attempt to free NULL pointer");
		return(-1);
	}

	if (((long) memh) & 0x3){
		memory_error("free","attempt to free illegal pointer");
		return(-1);
	}

	memh--;
	if(memh->tag1 == MEMFREE && memh->tag2 == MEMFREE) {
		memory_error(memh->name,"double free");
		return(-1);
	}

	if ((memh->tag1 == MEMTAG1) && (memh->tag2 == MEMTAG2) && ((memh->len & 0x3) == 0)) {
		memt = (MemTail *)(((char *) memh) + sizeof(MemHead) + memh->len);
		if (memt->tag3 == MEMTAG3){
			rem_memblock(memh,memt);
			memh->tag1 = MEMFREE;
			memh->tag2 = MEMFREE;
			memt->tag3 = MEMFREE;
			return(0);
		}
		error = 2;
		memory_error(memh->name,"end corrupt");
		name = check_memlist(memh);
		if (name != 0){
			if (name != memh->name) memory_error(name,"is also corrupt");
		}
	} else{
		error = -1;
		name = check_memlist(memh);
		if (name == 0) memory_error("free","pointer not in memlist");
		else memory_error(name,"error in header");
	}

	totblock--;
	/* hier moet een DUMP plaatsvinden */

	return(error);
}

void freelistN(listbase)
struct ListBase *listbase;
{
	struct Link *link,*next;

	if (listbase == 0) return;
	link= listbase->first;
	while(link) {
		next= link->next;
		freeN(link);
		link= next;
	}
	listbase->first=0;
	listbase->last=0;
}

void sleep(int sec)
{
	sec *= 25;
	
	for (; sec > 0; sec--) {
		VSync(0);
	}
}

void psx_init(short force)
{
	if (psx_init_done == 0 || force) {
		psx_init_done = 1;

		PadInit(0);
		SetVideoMode(MODE_PAL);
		ResetGraph(0);		/* reset graphic subsystem (0:cold,1:warm) */
		SetGraphDebug(0);	/* set debug mode (0:off, 1:monitor, 2:dump) */
		SetDispMask(1);		/* enable to display (0:inhibit, 1:enable) */
		
		// psx_cls(0, 0, 0);
	}
}


void psx_cls(uchar r, uchar g, uchar b)
{
	RECT	rect;
	short x;
	
	psx_init(FALSE);
	
	/* clear image */
	rect.w = 1024;
	rect.h = 512;
	rect.x = 0;
	rect.y = 0;
	ClearImage(&rect, r, g, b);
	
	r += 0x7f;
	g += 0x7f;
	b += 0x7f;
	
	rect.w = 1;
	
	for (x = 64; x < 1024; x += 64) {
		rect.x = x;
		ClearImage(&rect, r, g, b);
	}
}

void guru (char * string)
{
	DISPENV	Disp;
	DRAWENV drawenv;
	RECT	rect;
	int pad = (PADR1 | PADR2);

	if (noguru > 0) return;
	
	if (is_cd || 1) {
	
		psx_init(FALSE);
		
		if (PadRead(1) & PADL1) return;
		
		/* clear image */
		rect.w = 320;
		rect.h = 200;
		rect.x = 0;
		rect.y = 0;
		ClearImage(&rect, 128, 0, 0);

		SetDefDispEnv(&Disp, 0, 0, 320, 256);
		Disp.screen.h = Disp.disp.h;
		Disp.screen.w = 256;
		Disp.screen.y = (288 - 256) / 2;
		
		VSync(0);
		PutDispEnv(&Disp);
		
		FntLoad(960, 256);
		SetDumpFnt(FntOpen(8, 8, 296, 64, 0, 512));
		PutDrawEnv(SetDefDrawEnv(&drawenv, Disp.disp.x + 10, Disp.disp.y, 300, 200));
		FntPrint(string);
		FntFlush(-1);

		while (PadRead(1) & PADRdown) VSync(0);
		
		while ((PadRead(1) & pad) == pad) VSync(0);
		sleep (1);
		while ((PadRead(1) & pad) != pad) {
			if (PadRead(1) & PADRdown) break;
			VSync(0);
		}
		
		ClearImage(&rect, 224, 224, 32);
		
		VSync(0);
		VSync(0);
		VSync(0);
		VSync(0);		
	} else {
		printf("\n   G U R U :\n   %s\n\n", string);
	}
}

void init_psxutil()
{
	uchar result[8];

	ResetCallback();

	// first check if we're running on the pc or the ps
	
		CdInit();
		CdSetDebug(0);
	
		if (CdControl(CdlNop, 0, result)) is_cd = 1;
		else is_cd = 0;

	// initialiseer malloc
	
		if (is_cd) {
			// dit is 1.5 megabyte (vanaf 0.48Mb) gereserveerd
			// InitHeap2((void *) 0x80078000, 0x180000);
		} else {
			// dit is 2 megabyte (vanaf 2 Mb)gereserveerd
			// InitHeap2((void *) MALLOCSTA, MALLOCSIZE);
		}
	
	// onschuldige initialiseer functies
	
		psx_init(TRUE);
		
	// initialiseer filesysteem
	
		if (is_cd) {
			extern long cd_init();
			// _96_remove();
			// psx_cls(0, 128, 0);
			// _96_init();
			// psx_cls(0, 0, 128);
			cd_init();
		} else {
			PCinit();	// filesysteem reset
		}
}

void end_psxutil()
{
	if (is_cd) {
		extern long cd_exit();
		cd_exit();
	}

	if(totblock)  {
		printf("ERROR totblock %d\n", totblock);
		printmemlist();
	}
	
	checkmemory();
}

void set_dcache(int val)
{
	static unsigned long oldsp=0;

	if(val) {
		oldsp= SetSp(DCACHE_TOP);
	}
	else {
		SetSp(oldsp);
	}
}	

void error(str)
{
	printf("ERROR: %s\n", str);
}

int streaming_is_on = FALSE;

/*

  ####    #####  #####   ######    ##    #    #
 #          #    #    #  #        #  #   ##  ##
  ####      #    #    #  #####   #    #  # ## #
      #     #    #####   #       ######  #    #
 #    #     #    #   #   #       #    #  #    #
  ####      #    #    #  ######  #    #  #    #

*/

void streaming_pause()
{
	if (CdReadyCallback(0) == cdreadycallback) CdControl(CdlPause, 0, 0);
	
	if (active_stream) {
		active_stream->busy = FALSE;
		active_stream = 0;
	}
}

void streaming_stop()
{
	streaming_pause();
	streaming_is_on = FALSE;
}

void stream_start(Stream * stream)
{
	CdlLOC	cdloc;

	if (stream == 0) {
		guru("stream_start: stream = NULL");
		return;
	}
	
	if ((int) stream == -1) {
		guru("stream_start: stream = -1");
		return;
	}
	
	if (stream->index == 0) {
		guru("stream_start: stream->index = NULL");
		return;
	}
	
	if (active_stream) active_stream->busy = FALSE;
	
	stream->done = FALSE;
	stream->busy = TRUE;
	
	if (stream->lastsector > stream->maxsector) stream->lastsector = stream->maxsector;
	if ((stream->sectorseek < stream->firstsector) | (stream->sectorseek > stream->lastsector)) stream->sectorseek = stream->firstsector;
	
	active_stream = stream;
	s_next_sector = active_stream->start + active_stream->sectorseek;
	
	CdIntToPos (s_next_sector, &cdloc);
	if (CdControl(CDLREAD, (uchar *) &cdloc, 0) != 1) guru("CdControl");
}


void cdreadycallback(uchar status, uchar * result)
{
	ushort s_full;
	int index, offset;
	cdheader header;
	CdlLOC	cdloc;
	

	if ((active_stream != 0) && (active_stream->done == FALSE) && (active_stream->index != 0)) {
		// file is nog niet removed en zo

		switch (status) {
			case CdlDataReady:
				// er is een nieuw blok gelezen
				
				//s_full = s_used - s_freed;
				s_full = 0;
 				
				if (s_full < 256) {
					// er is een block vrij: zoeken (dit kan 4 keer zo snel met longs....)
						for (index = 1; index < 256; index++) {
							if (s_in_use[index] == FALSE) break;
						}
						
					// lees sector in cache
						if (index != 256) {
							CdGetSector(&header, 3);						
							if (CdGetSector(&cdcache[index].data, 512)) {
								// hier nog controleren of sector de juiste is...

								s_read_sector = CdPosToInt(&header.cdloc);
								if (s_read_sector == s_next_sector) {
									s_retry = 0;

									offset = s_read_sector - active_stream->start;
									if (offset < 0 || offset > active_stream->maxsector) {
										guru("Reading wrong sector !!");
									} else {
										// is block al gecached ??

										if (active_stream->index[offset] == 0) {
											//s_used++;
											s_in_use[index] = TRUE;
											active_stream->index[offset] = index;
										} else {
											// we gaan springen want dit gedeelte is nog gecached...
											
											for (index = offset; index <= active_stream->lastsector; index++) {
												if (active_stream->index[index] == 0) break;
											}
											active_stream->sectorseek = index;
											
											if (active_stream->sectorseek > active_stream->lastsector) active_stream->done = TRUE;
										}
										
										if (offset == active_stream->sectorseek) {
											// deze sector werd ook verwacht, niemand heeft sectorseek veranderd
	
											active_stream->sectorseek++;
											
											if (active_stream->sectorseek > active_stream->lastsector) {
												// dit was de laatste sector. We zijn nu dus klaar.
												active_stream->done = TRUE;
											} else {
												// is er een belangrijke file waar op gewacht wordt ?

												if (urgent_stream == 0) {
													s_next_sector = active_stream->start + active_stream->sectorseek;;
													// ALLES EINDELIJK HELEMAAL GOED !!
													return;
												}
											}
										}
									}
								} else s_retry++;
							} else guru("CdGetSector");
						} else guru("s_used / s_freed incorrect");
				} else {
					// Hele cache is vol, we proberen het nog een keer
					// Dit is echter geen disk error
					s_retry = 0;
				}
				break;
			case CdlDiskError:
				strcpy((void *) s_status, "CdlDiskError");
				break;
			default:
				sprintf((void *) s_status, "Unknown: %d", status);
				break;
			
		}
	}
	
	if (active_stream) {
		if (active_stream->done == TRUE) {
			active_stream->busy = FALSE;
			active_stream = 0;
		}
	}
	
	if ((active_stream == 0) || (urgent_stream != 0)) {
		// look for new file and start again
		
		if (active_stream) active_stream->busy = FALSE;

		if (urgent_stream) {
			active_stream = (void *) urgent_stream;
			urgent_stream = 0;
		} else {
			active_stream = s_list->first;
			while(active_stream) {
				if (active_stream->done == FALSE) break;
				active_stream = active_stream->next;
			}
		}
	}
	
	if (active_stream == 0) streaming_pause();
	else stream_start(active_stream);
}


void streaming_exit()
{
	// controleren of er nog een callback is, en die dan netjes stop zetten !!
	// HOE ?
	
	streaming_stop();
	
	// cache sluiten
		if (cdcache) freeN(cdcache);
		cdcache = 0;
}

void streaming_init()
{
	unsigned char mode;
	int i;
	
	static int first = TRUE;

	if (first) {
		first = FALSE;
		// guru("stream init");
	}

	strcpy((void *) s_status, "no error");
	
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
		s_in_use[0] = TRUE;
		for (i = 1; i <= 255; i++) s_in_use[i] = FALSE;
	
	// variabelen goed zetten
		//s_used = 1;
		//s_freed = 0;
}

void streaming_start()
{
	Stream * stream = 0;
	static int first = TRUE;

	if (first) {
		first = FALSE;
		// guru("streaming start");
	}
	
	// is er iets te streamen ?
	
		if (s_list->first == 0) return;
	
	// is init al gedaan ?
	
		if (cdcache == 0) streaming_init();
	
	if (is_cd == 0)	return;

	if (CdReadyCallback(cdreadycallback) != cdreadycallback) {
		// streaming was niet (meer) bezig
		
		streaming_is_on = TRUE;
		
		if (urgent_stream) {
			stream = (Stream *) urgent_stream;
			urgent_stream = 0;
			stream_start(stream);
			return;
		} else { 
			for (stream = s_list->first; stream; stream = stream->next) {
				if (stream->done == FALSE) {
					stream_start(stream);
					return;
				}
			}
		}
		
		// error 
		CdReadyCallback(0);
		streaming_is_on = FALSE;
	}
}

void stream_add(Stream * stream)
{
	static int first = TRUE;
	if (first) {
		first = FALSE;
		// guru("add_stream");
	}
	
	if (stream == 0) {
		guru("stream_add: stream = NULL");
		return;
	}
	
	if ((int) stream == -1) {
		guru("stream_add: stream = -1");
		return;
	}
	
	stream->done = stream->busy = 0;

	// some bounds checks
		if (stream->lastsector > stream->maxsector) stream->lastsector = stream->maxsector;
		if (stream->firstsector > stream->lastsector) stream->firstsector = 0;
				
		if (stream->sectorseek > stream->lastsector) stream->sectorseek = stream->firstsector;
		if (stream->sectorseek < stream->firstsector) stream->sectorseek = stream->firstsector;

	if (stream->index == NULL) {
		// nieuwe stroom altijd rewinden
			stream->sectorseek = stream->firstsector;

		stream->index = callocN(stream->maxsector + 1, "stream->index");
	
		EnterCriticalSection();
			addtail(s_list, stream);
		ExitCriticalSection();
	}
	
	streaming_start();
}

void stream_remove(Stream * stream)
{
	int i, index;
	
	if (stream == 0) {
		guru("stream_remove: stream = NULL");
		return;
	}
	
	if ((int) stream == -1) {
		guru("stream_remove: stream = -1");
		return;
	}
	
	if (stream->index) {			
		EnterCriticalSection();
			stream->done = TRUE;
			if (stream == active_stream) active_stream = 0;
			remlink(s_list, stream);	
		ExitCriticalSection();

		for (i = 0; i <= stream->maxsector; i++) {
			if (index = stream->index[i]) {
				s_in_use[index] = 0;
				//s_freed++;
			}
		}
		
		freeN(stream->index);
		stream->index = 0;
	}
}


void stream_close(Stream * stream)
{
	int i, index;
	extern long my_close(long);
	
	if (stream == 0) {
		guru("stream_close: stream = NULL");
		return;
	}

	if ((int) stream == -1) {
		guru("stream_close: stream = -1");
		return;
	}

	close((int) stream);
}

Stream * stream_open(char * name)
{
	int fd;
	extern long my_open(char *, ulong);
	
	fd = open(name, O_RDONLY);
	
	if (fd == -1) {
		sprintf(gurustr, "stream_open: can not open %s\n", name);
		guru(gurustr);
		fd = 0;
	}
	return((Stream *) fd);
}

void stream_set_minmax(Stream * stream, int min, int max)
{	
	if (stream == 0) {
		guru("stream_set_minmax: stream = NULL");
		return;
	}

	if ((int) stream == -1) {
		guru("stream_set_minmax: stream = -1");
		return;
	}

	if (max < min)
	{
		int t;
		t = max; max = min; min = t;
	}
	
	if (min < 0) min = 0;
	if (min > stream->maxsector) min = stream->maxsector;
	
	if (max == -1) max = stream->maxsector;
	if (max < 0) max = 0;
	if (max > stream->maxsector) max = stream->maxsector;
	
	stream->firstsector = min;
	stream->lastsector = max;
	
	stream->done = FALSE;
		
	streaming_start();
}


/* 
 
 ######     #    #       ######
 #          #    #       #
 #####      #    #       #####
 #          #    #       #
 #          #    #       #
 #          #    ######  ######

*/


char * cd_pc_name(char * name)
{
	int i, slen;
	
	tmpname[0] = 0;
	tmpname[1] = 0;

		
	if (name) {	
		if (name[0] == '.') {
			if (name[1] == '/') name += 2;
			else if (name[1] == '\\') name += 2;
		}
		
		if (is_cd == 0) {
			strcpy(tmpname, name);
		} else {
			if ((name[0] != '/') && (name[0] != '\\')) strcat(tmpname, "\\");
			strcat(tmpname, name);

			slen = strlen(tmpname);
						
			// if its not a directory cat ;1 at the end
			
				if (tmpname[slen - 1] != '/' && tmpname[slen - 1] != '\\') {
					// het is een file
					if (strrchr(tmpname, '.') == 0) strcat(tmpname, ".");
					strcat(tmpname, ";1");
				}
		}
	}
	
	// forward slash moet backward slash worden
	// lowercase moet uppercase worden

		slen = strlen(tmpname);
		
		for (i = slen - 1; i >= 0; i--) {
			char c = tmpname[i];
			
			if (c == '/') tmpname[i] = '\\';
			else if (c >= 'a' && c <= 'z') tmpname[i] = c + 'A' - 'a';
		}
	
	return(tmpname);
}


long my_open(char * name, ulong flag)
{
	long fd, i, error;
	CdlFILE _cdfile, *cdfile = &_cdfile;
	Stream * stream = 0;
	CD_File * cd_file = 0;
	CD_Dir * cd_dir;
	char * newname, * filename;
	
	newname = cd_pc_name(name);
	filename = strrchr(newname, '\\');
	
	if (is_cd == 0) {
		fd = PCopen(newname, 0, 0);
		if (fd != -1) {
			stream = CLN(Stream);
			if (stream) {
				stream->fd = fd;
				stream->bytesize = PClseek(fd, 0, SEEK_END);
			} else PCclose(fd);
		}
	} else {
		if (dir_list->first) {
			if (filename) {
				filename[1] = 0;
				
				cd_dir = dir_list->first;
				while (cd_dir) {
					if (strcmp(cd_dir->name, newname) == 0) break;
					cd_dir = cd_dir->next;
				}
				
				if (cd_dir) {
					newname = cd_pc_name(name);
					filename = strrchr(newname, '\\');
					filename++;
					
					cd_file = cd_dir->filelist.first;
					while(cd_file) {
						if (strcmp(cd_file->name, filename) == 0) break;
						cd_file = cd_file->next;
					}
					
				}
			}
			
			if (cd_file == 0) {
				sprintf(gurustr, "Can't find file for %s\n", name);
				guru(gurustr);
			}
		}
	
		newname = cd_pc_name(name);
		filename = strrchr(newname, '\\');

		if (cd_file) {
			stream = CLN(Stream);
			stream->start = cd_file->start;
			stream->bytesize = cd_file->bytesize;
			strcpy(stream->name, cd_file->name);
		} else {
			// zet eventueel streaming stop
				stream = active_stream;
				if (stream)	streaming_pause();
			
			// zoeken		
				error = (CdSearchFile(cdfile, newname) == 0);
			
			// streamen weer starten
				if (stream) {
					urgent_stream = stream;
					streaming_start();
				}
			
			// verdere afhandeling
				if (error) {
					guru(newname);
					return(-1);
				}
				
				stream = CLN(Stream);
				stream->start = CdPosToInt(&cdfile->pos);
				stream->bytesize = cdfile->size;
		}
	}
		
	if (stream == 0) {
		guru(newname);
		return(-1);
	} else {
		if (filename) name = filename + 1;
		else name = newname;
		if (strlen(newname) < sizeof(stream->name)) name = newname;
		if (name == 0) name = "UNKNOWN";
		
		if (strlen(name) >= sizeof(stream->name)) {
			name += strlen(name) - (sizeof(stream->name) + 1);
		}
		
		strcpy(stream->name, name);
		stream->maxsector = stream->lastsector = (stream->bytesize - 1) >> 11;
	}
	
	return((long) stream);
}



long my_lseek(long fd, long pos, long flag)
{
	Stream * stream;

	if (fd == -1) return (-1);
	if (fd == 0) return (-1);
	
	stream = (Stream *) fd;
	
	switch (flag){
		case SEEK_SET:
			stream->byteseek = pos;
			break;
		case SEEK_CUR:
			stream->byteseek += pos;
			break;
		case SEEK_END:
			stream->byteseek = stream->bytesize - pos;
			break;
	}
	
	if (stream->byteseek < 0) stream->byteseek = 0;
	if (stream->byteseek > stream->bytesize) stream->byteseek = stream->bytesize;
	
	return(stream->byteseek);
}

long my_read(int fd, void * buf, long size)
{
	int start, end, sect, i, index;
	Stream * stream;
	char * name;
	

CHECKPOINT(read1)

	if (fd == -1) {
		guru("read fd = -1");
		return (-1);
	}

	if (fd == 0) {
		guru("read fd = 0");
		return (-1);
	}

	stream = (Stream *) fd;
	name = stream->name;
	
	if (size <= 0) {
		if (size == 0) return(0);
		sprintf(gurustr, "%s: read size %d", name, size);
		guru(gurustr);
		return(-1);
	}

	if (buf == 0) {
		sprintf(gurustr, "%s: buf = 0", name);
		guru(gurustr);
		return(-1);
	}

	if (stream->byteseek + size > stream->bytesize) {
		// sprintf(gurustr, "myread: size to big \n%d + %d > %d", stream->byteseek, size, stream->bytesize);
		// guru(gurustr);
		size = stream->bytesize - stream->byteseek;
	}

	if (is_cd == 0) {
		PClseek(stream->fd, stream->byteseek, SEEK_SET);
		size = PCread(stream->fd, buf, size);
		if (size > 0) stream->byteseek += size;
	} else {
		// calc
			start = stream->byteseek >> 11;
			end = (stream->byteseek + size - 1) >> 11;
	
			if (end > stream->maxsector) end = stream->maxsector;
	
			if (end > stream->lastsector) {
				guru("end > stream->lastsector: adjusting");
				stream->lastsector = end;
			}
			
			if (start < stream->firstsector) {
				guru("start < stream->firstsector: adjusting");
				stream->firstsector = start;
			}
			
			sect = end - start + 1;
	
		if (streaming_is_on == FALSE) {	
			CdlLOC	cdloc;
			char * tbuf;
			unsigned char mode = CdlModeSpeed;
			int largest;
			
			largest = best_memblock(sect << 11);
			
			if (largest < (sect << 11)) {
				int totdone = 0, newsize, partdone;
				
				// korte blokken gaan lezen ivm met geheugen
				
				largest = (largest - 2048) & (~2047);
				printf("my_read: read of %d to large, reading chunks of %d\n", size, largest);
				
				while (size) {
					if (size > largest) newsize = largest;
					else newsize = size;
					
					partdone = read(fd, buf, newsize);

					if (partdone != -1) totdone += partdone;
					if (partdone != newsize) break;
					
					buf += partdone;
					size -= partdone;
				}
				
				size = totdone;
				if (size == 0) size = -1;
			} else {
				// seek
					CdIntToPos (stream->start + (stream->byteseek >> 11), &cdloc);
					CdControl(CdlSetloc, (u_char *) &cdloc, 0);
		
				// malloc
					tbuf = mallocN(sect << 11, "my_read");
					if (tbuf== 0) return (-1);
		
				// read
					if (CdRead(sect, (void *) tbuf, CdlModeSpeed) == 0) {
						guru("cdread"); size = -1;
					} else {
						// guru("read started");
						while ((sect = CdReadSync(1, 0)) > 0) {
							// sprintf(gurustr, "CdReadSync %d\n", sect);
							// guru(gurustr);
						}
						if (sect < 0) {
							guru("cdreadSync");
							size = -1;
						}
					}
		
				// memcpy
					if (size > 0) {
						memcpy(buf, tbuf + (stream->byteseek & 2047), size);
						stream->byteseek += size;
					}
					freeN(tbuf);
			}
			
		} else {
			u_short s_full, s_free;
			int first, oldseek, len, index, remaining, prevstart;
			uchar * from, * to;
	
	CHECKPOINT(read2)
	
			if (stream->index == 0) {
				sprintf(gurustr, "read() %s: Can't mix stream / non-stream files !");
				guru(gurustr);
				return(-1);
			}
	
			// free cache blocks before starting point
				
				prevstart = stream->prevstart;
				
				if (start < prevstart) prevstart = 0;
				if (stream->sectorseek < prevstart) prevstart = 0;
				
				for (i = prevstart; i < start; i++) {
					if (index = stream->index[i]) {
						stream->index[i] = 0;
						s_in_use[index] = 0; //s_freed++;
					}
				}
	
				stream->prevstart = start;
				
			// look if data is cached
	
	CHECKPOINT(read3)
	
	/*
		Als een file aan het eind al voor een groot gedeelte gecached
		is, en er wordt een plaatje aan het begin gelezen,  wordt er
		maar net genoeg ruimte voor dat ene plaatje vrijgemaakt. Daarna
		stopt de caching weer direct. Oplossing : grotere ruimte
		vrijmaken
	*/
	
				do {
					sect = 0;
	
					for (i = end; i >= start; i--) {
						if (stream->index[i] == 0) {
							sect++;
							first = i;
						}
					}
	
					if (sect) {
						// not all data is present...
						// is the cache full ?
						
						//s_full = s_used - s_freed;
						//s_free = 256 - s_full;
						s_full = 0;
						s_free = 256;
						
						if (s_free < sect) {
							// not eneough space to hold the data
							// solution: free from the end of this file
							for (i = stream->maxsector; i > end ; i--) {
								if (index = stream->index[i]) {
									stream->index[i] = 0;
									s_in_use[index] = 0;
									//s_freed++;
									s_free++;
									if (s_free >= sect) break;
								}
							}
							
							if (s_free < sect) {
								guru("Cache is full, what now ?");
								return(-1);
							}
						}
						
						if ((stream->busy == FALSE) || (stream->sectorseek < (first - 10)) || (stream->sectorseek > (first + 1))) {
							stream->sectorseek = first;
						}
						
						oldseek = stream->sectorseek;
						
						if (stream->busy == FALSE) urgent_stream = stream;
						
						// restart streaming if it was stopped
							streaming_start();
							
						// wait for things to happen
							while (stream->busy == FALSE) {
							}
							while (stream->sectorseek == oldseek && stream->busy == TRUE) {
							}
					}
				} while (sect);
				
	
	CHECKPOINT(read4)
	
				// alle data is nu voorhanden
				
				to = buf;
				remaining = size;
	
				for (i = start; i <= end ; i++)
				{
					index = stream->index[i];
					from = (uchar *) & cdcache[index].data;
					len = 2048;
					len -= stream->byteseek & 0x7ff;
					from += stream->byteseek & 0x7ff;
					if (len > remaining) len = remaining;
					memcpy(to, from, len);
					to += len;
					stream->byteseek += len;
					remaining -= len;
				}
				
	CHECKPOINT(read5)
		}
	}
	
	return (size);
}


long my_close(long fd)
{
	Stream * stream;

	if (fd == -1) return (-1);
	if (fd == 0) return (-1);
	
	stream = (Stream *) fd;

	if (stream->index) stream_remove(stream);

	if (is_cd == 0) PCclose(stream->fd);
	
	freeN(stream);
	
	return(fd);
}

int cd_tonext(char *str)
{
	int len= 0;
	
	while(*str!=' ' && isgraph( *str ) ) {
		str++;
		len++;
		if(len>256) return(len);
	}
	
	while(*str==' ' || isgraph( *str )==0 ) {
		str[0]= 0;
		str++;
		len++;
		if(len>256) return(len);
	}
	
	return(len);
}

long cd_init()
{
	Stream * stream;
	CdlFILE _cdfile, *cdfile = &_cdfile;
	int fd, size, len, slen, org_cd = is_cd;
	uchar * buf, * pnt;
	char * name;
	CD_Dir * cd_dir;
	CD_File * cd_file;
	
	if (is_cd == 0) return(-1);
	
	if (dir_list->first) return (1);
	
	// open file.lst en anders auto.lst

		noguru++;
			fd = open("file.lst", O_RDONLY);
			if (fd == -1) fd = open("auto.lst", O_RDONLY);
		noguru--;
		
		if (fd == -1) {
			guru("no file.lst or auto.lst found");
			return(-1);
		}
	
		if (is_cd) {
			stream = (Stream *) fd;
		} else {
			stream = CLN(Stream);
			stream->bytesize = lseek(fd, 0, SEEK_END);
			lseek(fd, 0, 0);
		}

	// lees file in en maak een index van alle files
					
		buf = mallocN(stream->bytesize, "cd_init");
		if (buf) {
			size = stream->bytesize;
			if (read(fd, buf, size) == size) {
				pnt = buf;
				
				is_cd = TRUE;

				cd_dir = CLN(CD_Dir);
				strcpy(cd_dir->name, cd_pc_name("/"));
				addtail(dir_list, cd_dir);
				
				while (size > 1) {
					len = cd_tonext(pnt);
					slen = strlen(pnt);
					if (slen) {
						if (pnt[slen - 1] == ':') {
							// newdir
								pnt[slen - 1] = '/';
								cd_dir = CLN(CD_Dir);
								name = cd_pc_name(pnt);
								strcpy(cd_dir->name, name);
								addtail(dir_list, cd_dir);
								// sprintf(gurustr, "newdir: %s\n", cd_dir->name);
								// guru(gurustr);
						} else if (pnt[slen - 1] == '/') {
							// skipping directories
								// sprintf(gurustr, "skipping %s\n", pnt);
								// guru(gurustr);
						} else {
							// regular file
								cd_file = CLN(CD_File);
								name = cd_pc_name(pnt);
								strcpy(cd_file->name, name + 1);
								addtail(&cd_dir->filelist, cd_file);
								// sprintf(gurustr, "  adding %s to %s\n", cd_file->name, cd_dir->name);
								// guru(gurustr);
							// get data
								strcpy(tmpname, cd_dir->name);
								strcat(tmpname, cd_file->name);
								
								if (is_cd == org_cd) {
									if (CdSearchFile(cdfile, tmpname) == 0) {
										guru(tmpname);
										remlink(&cd_dir->filelist, cd_file);
										freeN(cd_file);
									} else {
										cd_file->start = CdPosToInt(&cdfile->pos);
										cd_file->bytesize = cdfile->size;
									}
								}

						}
					}
					pnt += len;
					size -= len;
				}
				
				is_cd = org_cd;
				
			} else guru("Error reading auto.list / file.list");
			
			freeN(buf);
		}
		
		close(fd);
		if (is_cd == 0) freeN(stream);

	return(0);
}


long cd_exit()
{
	ListBase * file_list;
	CD_Dir * cd_dir;
	CD_File * cd_file;
	
	
	if (is_cd == 0) return(-1);
	
	while (cd_dir = dir_list->first) {
		remlink(dir_list, cd_dir);

		file_list = & cd_dir->filelist;
		while (cd_file = file_list->first) {
			remlink(file_list, cd_file);
			freeN(cd_file);
		}
		
		freeN(cd_dir);
	}
	
	CdControl(CdlStop, 0, 0);
	
	return(0);
}


