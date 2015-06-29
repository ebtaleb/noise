NAME   = noise
O      = o
RM     = rm -f
CC     = gcc
CXX    = g++

PROG        = $(NAME)

DEFAULT_CFLAGS = `sdl-config --cflags`
LDFLAGS        = `sdl-config --libs` -L./bulletml -lbulletml -lstdc++ -lm

MORE_CFLAGS = -O3

CFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -ggdb3
CPPFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -ggdb3 -I./bulletml/

OBJS =	$(NAME).$(O) barragemanager.$(O) foecommand.$(O) foe.$(O) \
	screen.$(O) clrtbl.$(O) vector.$(O) degutil.$(O) rand.$(O)

$(PROG): $(OBJS)
	make -C bulletml
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

clean:
	make -C bulletml clean
	$(RM) $(PROG) *.$(O)
