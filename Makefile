NAME   = noise
O      = o
RM     = rm -f
CC     = ccache gcc
CXX    = ccache g++

PROG        = $(NAME)

DEFAULT_CFLAGS = `sdl-config --cflags` -Wall
LDFLAGS        = `sdl-config --libs` -L./bulletml -lbulletml -lstdc++ -lm

CFLAGS  = $(DEFAULT_CFLAGS)
CPPFLAGS  = $(DEFAULT_CFLAGS) -I./bulletml/

OBJS =	$(NAME).$(O) barragemanager.$(O) foecommand.$(O) foe.$(O) \
	screen.$(O) clrtbl.$(O) vector.$(O) degutil.$(O) rand.$(O)

debug: CFLAGS += -DDEBUG -ggdb3
debug: CPPFLAGS += -DDEBUG -ggdb3
debug: $(PROG)

release: CFLAGS += -O3
release: CPPFLAGS += -O3
release: $(PROG)

$(PROG): $(OBJS)
	make -C bulletml
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

clean:
	make -C bulletml clean
	$(RM) $(PROG) *.$(O) core
