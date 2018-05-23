CC = gcc

SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs)

CFLAGS = -fpermissive -I./Port $(SDL_CFLAGS)
LDFLAGS = $(SDL_LDFLAGS)
OBJDIR = ./obj
EXE = clonk

OBJS = $(patsubst %.cpp, %.o, $(shell find -iname *.cpp))

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/$@ $< $(LDFLAGS)

clean:
	rm *.o
