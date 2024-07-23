# psymake /b is alles maken
# extra opties: -O4 -mgpopt -G256

# movie.c O2 andere O

INC_FOLDER = -I../PSYQ/PSYQ_LIB/third_party/psyq/include
LIB_FOLDER = -L../PSYQ/PSYQ_LIB/third_party/psyq/lib
LDFLAGS = -Wl,--start-group
LDFLAGS += -lapi
LDFLAGS += -lc
LDFLAGS += -lc2
LDFLAGS += -lcard
LDFLAGS += -lcd
LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
LDFLAGS += -lgs
LDFLAGS += -lgte
LDFLAGS += -lgun
LDFLAGS += -lhmd
LDFLAGS += -lmath
LDFLAGS += -lmcrd
LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
LDFLAGS += -ltap
LDFLAGS += -Wl,--end-group

CC = mipsel-linux-gnu-gcc -mabicalls -O4 -mgpopt -fstack-protector

#OBJ01 = psxutil.o blendpsx.o psector.o screen.o pobject.o viewpsx.o readbpsx.o arithp.o peffect.o
#OBJ02 = drawmesh.o main01.o plife.o 
#OBJ03 = sectormv.o movie.o
BTM		= psxutil.o blendpsx.o psector.o screen.o pobject.o viewpsx.o readbpsx.o arithp.o peffect.o
MID		= drawmesh.o plife.o sectormv.o movie.o
TOP		= main01.o
#OBJRIDE= ride.o readride.o drawride.o screen.o psxutil.o

%.o: %.c
	$(CC) $(INC_FOLDER) -c $< $(LDFLAGS) -o $@

engine.cpe: $(BTM) $(MID) $(TOP)
	$(CC) $(BTM) $(MID) $(TOP) $(LIB_FOLDER) $(LDFLAGS) -o ./engine.cpe

clean:
	rm *.o
	ls

#ride.cpe: $(OBJRIDE) movie.o file.o stream.o
#	psylink /m /c /g @ride.lnk,ride.cpe,ride.sym,ride.map
