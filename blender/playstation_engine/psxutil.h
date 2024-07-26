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

#ifndef UTIL_H

#define UTIL_H

#include <sys/types.h>
#include <libcd.h>

#ifndef	NULL
#define NULL			0
#endif

#ifndef	FALSE
#define FALSE			0
#endif

#ifndef	TRUE
#define TRUE			1
#endif

#ifndef ulong
#define ulong unsigned long
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif


#define mallocstruct(x,y) (x*)malloc((y)* sizeof(x))
#define callocstruct(x,y) (x*)calloc((y), sizeof(x))
#define mallocstructN(x,y,name) (x*)mallocN((y)* sizeof(x),name)
#define callocstructN(x,y,name) (x*)callocN((y)* sizeof(x),name)

#define RMK(x)

#define ELEM(a, b, c)		( (a)==(b) || (a)==(c) )
#define ELEM3(a, b, c, d)	( ELEM(a, b, c) || (a)==(d) )
#define ELEM4(a, b, c, d, e)	( ELEM(a, b, c) || ELEM(a, d, e) )
#define ELEM5(a, b, c, d, e, f)	( ELEM(a, b, c) || ELEM3(a, d, e, f) )
#define ELEM6(a, b, c, d, e, f, g)	( ELEM(a, b, c) || ELEM4(a, d, e, f, g) )

#define MIN2(x,y)		( (x)<(y) ? (x) : (y) )
#define MIN3(x,y,z)		MIN2( MIN2((x),(y)) , (z) )
#define MIN4(x,y,z,a)		MIN2( MIN2((x),(y)) , MIN2((z),(a)) )

#define MAX2(x,y)		( (x)>(y) ? (x) : (y) )
#define MAX3(x,y,z)		MAX2( MAX2((x),(y)) , (z) )
#define MAX4(x,y,z,a)		MAX2( MAX2((x),(y)) , MAX2((z),(a)) )

#define SWAP(type, a, b)	{ type sw_ap; sw_ap=(a); (a)=(b); (b)=sw_ap; }

#ifndef ABS
#define ABS(x)	((x) < 0 ? -(x) : (x))
#endif


#define GS(x) (((uchar *)(x))[0] << 8 | ((uchar *)(x))[1])
#define GSS(x) (((uchar *)(x))[1] << 8 | ((uchar *)(x))[0])

#define MAKE_ID(a,b,c,d) ( (long)(d)<<24 | (long)(c)<<16 | (b)<<8 | (a) )

#define NEW(x) (x*)mallocN(sizeof(x),# x)
#define CLN(x) (x*)callocN(sizeof(x),# x)
#define PRINT(d, var1)	printf(# var1 ":%" # d "\n", var1)
#define PRINT2(d, e, var1, var2)	printf(# var1 ":%" # d " " # var2 ":%" # e "\n", var1, var2)
#define PRINT3(d, e, f, var1, var2, var3)	printf(# var1 ":%" # d " " # var2 ":%" # e " " # var3 ":%" # f "\n", var1, var2, var3)
#define PRINT4(d, e, f, g, var1, var2, var3, var4)	printf(# var1 ":%" # d " " # var2 ":%" # e " " # var3 ":%" # f " " # var4 ":%" # g "\n", var1, var2, var3, var4)

#define DCACHE_BASE	0x1F800000
#define DCACHE_SIZE	0x400
#define DCACHE_TOP	(DCACHE_BASE + DCACHE_SIZE - 4)

typedef struct byte8 {
	long rt1, rt2;
} byte8;

typedef struct byte12 {
	long rt1, rt2, rt3;
} byte12;

typedef struct byte16 {
	long rt1, rt2, rt3, rt4;
} byte16;

#define COPY_4(a,b)		*((long *)(a))= *((long *)(b))
#define COPY_8(a,b)		*((byte8 *)(a))= *((byte8 *)(b))
#define COPY_12(a,b)	*((byte12 *)(a))= *((byte12 *)(b))
#define COPY_16(a,b)	*((byte16 *)(a))= *((byte16 *)(b))

typedef struct Link
{
	struct Link *next,*prev;
} Link;


typedef struct ListBase
{
	void *first,*last;
} ListBase;


typedef struct MemHead {
	long tag1;
	struct MemHead *next,*prev;
	char * name;
	char * nextname;
	long len;
	long level;
	long tag2;
} MemHead;

typedef struct MemTail {
	long tag3;
} MemTail;

typedef struct MemBlock {
	struct MemBlock * next, * prev;
	int size;
	int tag;
}MemBlock;

typedef struct FreeBlock{
	struct FreeBlock * next, * prev;
	MemBlock block;
}FreeBlock;


#define MEMTAG1 MAKE_ID('M','E','M','O')
#define MEMTAG2 MAKE_ID('R','Y','B','L')
#define MEMTAG3 MAKE_ID('O','C','K','!')
#define MEMFREE MAKE_ID('F','R','E','E')
#define MEMBLCK MAKE_ID('B','L','C','K')

#define MEMNEXT(x) ((MemHead *)(((char *) x) - ((char *) & (((MemHead *)0)->next))))

#define open(x, y) my_open(x, y)
#define lseek(x, y, z) my_lseek(x, y, z)
#define read(x, y, z) my_read(x, y, z)
#define close(x) my_close(x)

#define DEBUG

#ifdef DEBUG
#define LINE _line = __LINE__; _file = __FILE__;
#else
#define LINE
#endif

typedef struct{
	CdlLOC	cdloc;
	uchar pad[8];
}cdheader;

typedef struct{
	uchar	data[2048];
} cdsector;

typedef struct{
	void	* next, * prev;
	ulong	start;
	ulong	maxsector;
	ulong	firstsector;
	ulong	lastsector;
	ulong	bytesize;
	long	byteseek;
	volatile long	sectorseek;
	volatile uchar	busy, done;
	volatile uchar	*index;
	char	name[16];
	int		fd;
	int		prevstart;
} Stream;

typedef struct{
	void * next, * prev;
	char name[128];
	ListBase filelist;
}CD_Dir;

typedef struct{
	void * next, * prev;
	char name[16];
	ulong start;
	ulong bytesize;
}CD_File;

#define C_File Stream

	extern Stream * active_stream;
	extern volatile Stream * urgent_stream;
	
	extern volatile char s_status[32], s_result[8];
	extern volatile int s_read_sector, s_next_sector, s_retry;
	
	extern volatile ushort s_used, s_freed;
	extern cdsector * cdcache;
	
	extern volatile uchar s_in_use[256];
	extern ListBase * s_list;

	extern void stream_add(Stream *);
	extern void stream_remove(Stream *);
	extern void stream_set_minmax(Stream *, int, int);
	extern Stream * stream_open(char *);
	extern void stream_close(Stream *);
	extern void stream_start(Stream *);
	
	extern void streaming_exit();
	extern void streaming_init();
	extern int streaming_is_on;
	
extern volatile int _line;
extern volatile char * _file;

extern short totblock;
extern long mem_in_use;
extern void (*memory_error)();
extern void *mallocN(long,char *);
extern void *callocN(long,char *);
extern short freeN();
extern long countlist(ListBase *);
extern void guru(char *);
extern short is_cd;
extern void psx_cls(uchar r, uchar g, uchar b);
extern void sleep(int sec);
extern char gurustr[128];

#endif /* UTIL_H */

