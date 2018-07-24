CC = gcc --std=c++98

SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs)

CFLAGS = -w -fpermissive -I./Port $(SDL_CFLAGS)
LDFLAGS = -lm $(SDL_LDFLAGS)
EXE = clonk

DEPS = $(shell find -iname *.h)
OBJS = $(patsubst %.cpp, %.o, $(shell find -iname *.cpp))

$(EXE): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm $(OBJS)
