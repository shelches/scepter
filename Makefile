#CFLAGS	= +Wc,t=5000 +Wc,+u +Wi,t=10000 S=3000
#TARGET = -DERUG
CFLAGS = -c -w3 -DPROTO $(TARGET)

OBJS = scepter.o cmd1.o cmd2.o cmd3.o cmd4.o db.o execute.o \
edit1.o edit2.o io.o ipc.o log.o main.o matt.o \
mon.o obj.o room.o spells.o user.o rand.o

all: mud scepterd scepter

## scepterd
scepterd: $(OBJS)
	cc -o scepterd $(OBJS) -lwm -lunix
#	cc x=link c=scepterd
#	cp scepterd 3:/cmds

scepter:	inter.o
	cc -o scepter inter.o -lwm

inter.o: inter.c

mud:	mud.o
	cc -o mud mud.o -lwm

mud.o:	mud.c common.h

arc:
	rm scepter.arc
	arc a scepter Makefile link *.[ch]

$(OBJS):	scepter.h

install: scepterd scepter
	cp scepterd ~wm/bin
	cp scepter ~wm/wmbin

cmd1.o:		scepter.h
cmd2.o:		scepter.h
cmd3.o:		scepter.h
cmd4.o:		scepter.h
db.o:			scepter.h
edit1.o:		scepter.h
edit2.o:		scepter.h
execute.o:	scepter.h
inter.o:		Makefile common.h
io.o:			scepter.h
ipc.o:		Makefile common.h scepter.h
log.o:		scepter.h
main.o:		Makefile common.h
matt.o:		scepter.h
mon.o:		scepter.h
obj.o:		scepter.h
rand.o:		scepter.h
room.o:		scepter.h
scepter.o:	Makefile common.h proto.h scepter.h
spells.o:	scepter.h
user.o:		Makefile common.h scepter.h

.c.o:
	cc $(CFLAGS) $<
