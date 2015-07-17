ifeq ($(shell uname),Linux)
# for Linux
LIB += -L/usr/X11R6/lib -lglut -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm -lpthread
endif

ifeq ($(shell uname),Darwin)
# for Mac 
LIB += -framework Cocoa -framework OpenGL -framework GLUT -framework IOKit -framework Carbon -L/usr/local/lib/ -lglfw
endif

CC = gcc 
OPT += -O3 -Wall 
# OPT += -Wno-deprecated
OBJS1 = donothing.o 
OBJS2 = dodecagon.o 
OBJS3 = rectangle.o
OBJS4 = playground.o
OBJS6 = triangle_color.o
OBJS5 = playground.o

all: new
#
prog1:
	$(CC) $(OPT) $(PREDEF) -c nothing.c
	$(CC) $(OBJS1) $(LIB) -o nothing.out
prog2:
	$(CC) $(OPT) $(PREDEF) -c rectangle.c
	$(CC) $(OBJS3) $(LIB) -o rectangle.out
	$(CC) $(OPT) $(PREDEF) -c dodecagon.c
	$(CC) $(OBJS2) $(LIB) -o dodecagon.out
	$(CC) $(OPT) $(PREDEF) -c triangle_color.c
	$(CC) $(OBJS6) $(LIB) -o triangle_color.out
	$(CC) $(OPT) $(PREDEF) -c triangle.c
	$(CC) $(OBJS5) $(LIB) -o triangle.out
	$(CC) $(OPT) $(PREDEF) -c halfshadow.c
	$(CC) halfshadow.o $(LIB) -o halfshadow.out
	$(CC) $(OPT) $(PREDEF) -c mytexture1.c
	$(CC) mytexture1.o $(LIB) -o mytexture1.out
	$(CC) $(OPT) $(PREDEF) -c spotlight.c
	$(CC) spotlight.o $(LIB) -o spotlight.out
	$(CC) $(OPT) $(PREDEF) -c simplecircle.c
	$(CC) simplecircle.o $(LIB) -o simplecircle.out
new:
	$(CC) $(OPT) $(PREDEF) -c simplecube.c
	$(CC) simplecube.o $(LIB) -o simplecube.out
	$(CC) $(OPT) $(PREDEF) -c playground.c
	$(CC) $(OBJS4) $(LIB) -o playground.out


clean:
	rm -vf *.o

