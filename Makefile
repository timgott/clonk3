CC = g++

SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs)

CFLAGS = -O3 -w -fpermissive -I./Port $(SDL_CFLAGS)
LDFLAGS = $(SDL_LDFLAGS)

OUTDIR = build
EXE = $(OUTDIR)/clonk
CONTENTDIR = Content

CONTENT = C3HTITLE.PCX C3RSOUND.GRP CLONK-2.DAT CLONK3.txt MISSIONS.SCR C3RGRAFX.GRP C3RTITLE.GRP CLONK3.HLP CLONK.DAT NAMES.TXT
CONTENTTARGETS = $(patsubst %, $(OUTDIR)/%, $(CONTENT))
DEPS = $(shell find -iname *.h)
OBJS = $(patsubst %.cpp, %.o, $(shell find -iname *.cpp))

$(EXE): $(OUTDIR) $(OBJS) $(CONTENTTARGETS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

$(CONTENTTARGETS):
	cp $(patsubst $(OUTDIR)%, $(CONTENTDIR)%, $@) $(OUTDIR)

$(OUTDIR):
	mkdir $(OUTDIR)

run:
	$(EXE)

clean:
	rm $(OBJS)
	rm -r $(OUTDIR)
