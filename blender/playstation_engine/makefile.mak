# psymake /b is alles maken
# extra opties: -O4 -mgpopt -G256

# movie.c O2 andere O

CC = ccpsx -O2 -comments-c++

OBJ01 = screen.obj viewpsx.obj readbpsx.obj arithp.obj peffect.obj
OBJ02 = drawmesh.obj blendpsx.obj main01.obj psector.obj plife.obj pobject.obj
OBJ03 = sectormv.obj movie.obj psxutil.obj
OBJRIDE= ride.obj readride.obj drawride.obj screen.obj psxutil.obj

.c.obj:
	$(CC) -c $*.C

engine.cpe: $(OBJ01) $(OBJ02) $(OBJ03)
	psylink /m /c /g @link.lnk,engine.cpe,engine.sym,engine.map

ride.cpe: $(OBJRIDE) movie.obj file.obj stream.obj
	psylink /m /c /g @ride.lnk,ride.cpe,ride.sym,ride.map




