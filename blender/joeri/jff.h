#ifndef JFF
#define JFF


typedef struct phead{     /* *** 16 bytes *** */ 
  char  name[4];
  ulong size;
  ulong old_ptr;
  ulong extra;
}phead;

typedef struct pFrame{    /* ***   32 bytes  *** */
  struct pFrame *next;
  POLY_FT4 po;
  short xx[2];            /* extra's */
}pFrame;              

typedef struct pCmap{     /* *** 16 bytes *** */
  struct pCmap  *next, *prev; 
  short nr;               /* internal nr for finding this cmap */
  short bits;             /* 4 or 8 bits cmap */
  short uv[2];            /* framebuffer location */
  short *data;
}pCmap;
/* in file data is written after this struct and can be 32 or 512 bytes */

typedef struct pAnim{     /* *** 16 bytes *** */
  struct pAnim *next, *prev;
  
  char name[16];
  short frames;           /* number of frames */  
  char flag, trans;       /* flag : 4/8/16 bits, trans  0:mix 1:add 2:sub 3:1/4 4:off */
    
  struct pFrame *first_frame;
  struct pCmap  *cmap;
  int fb_cmap, fbx, fby;

}pAnim;

typedef struct pRectInfo{ /* *** 8 bytes *** */
  struct pRectInfo *next;
  short uv[2];            /* prefered location in framebuffer */
  short wh[2];            /* size */
  short info[2];          /* info 1; /playstation/nintendo/pc/ , info 2; /top/bindL/bindR/ */ 
}pRectInfo;
/* in file data is written after this struct and can be any size */

typedef struct pGroup{    /* *** 48 bytes *** */
  struct pGroup *next, *prev;
  char name[32];
  short anims, cmaps;
  struct pAnim *first_anim;
  struct pCmap *fist_cmap;
  int fb_rect, fbx, fby;
}pGroup;


#endif /*  JFF  */
