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

/*  testpsx.c   juli 96  

 * 
 * 
 */


#include "psxdef.h"


/*

	voor testen van readblendpsx, UNIX!

*/


void main(argc,argv)
long argc;
char **argv;
{
	
	if(argc!=2) {
		printf("usage: testpsx file\n");
		exit(0);
	}

	//init_psxutil();
	//init_blendpsx();
	
	/*read_file(argv[1]);
	
	if(1) {
		pMesh *me;
		
		me= G.main->mesh.first;
		while(me) {
			printf("me %c %c %c %c\n", me->id.name[0], me->id.name[1], me->id.name[2], me->id.name[3]);
			
			PRINT4(x, x, x, x, me->packetdata, me->mvert, me->totpacket, me->totvert);
			
			me= me->id.next;
		}
	}*/
	
	exit(0);
}


