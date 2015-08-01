CC = g++
CFLAGS = -D_DEBUG -I../glm-0.9.6.3/glm 
CPPFLAGS = -g

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@  $<

LIBS = -L./lib -lSOIL -lGL -lGLU -lGLEW -lglut -lm

SRCS = src/main.cpp lib/LoadShaders.cpp src/readcontrol.cpp
OBJS = src/main.o lib/LoadShaders.o src/readcontrol.o

MACLIBS = -L./lib -lSOIL -framework OpenGL -framework GLUT -lglew -framework CoreFoundation

main: $(OBJS)
	$(CC) -g -o obscene $(OBJS) $(LIBS)

mac: $(OBJS)
	$(CC) -g -o obscene $(OBJS) $(MACLIBS)

clean:
	rm -f *.o ./lib/*.o
