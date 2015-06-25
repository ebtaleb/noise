# Noiz2sa makefile(MinGW 2.0.0)
# $Id: Makefile,v 1.4 2003/08/10 09:00:05 kenta Exp $

NAME   = noiz2sa
O      = o
RM     = rm -f
CC     = gcc
CXX    = g++

PROG        = $(NAME)

DEFAULT_CFLAGS = `sdl-config --cflags`
LDFLAGS        = `sdl-config --libs` -L. -L./bulletml -lSDL_mixer -lbulletml -lstdc++ -lm

MORE_CFLAGS = -O3

CFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -ggdb3
CPPFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -ggdb3 -I./bulletml/

OBJS =	$(NAME).$(O) barragemanager.$(O) foecommand.$(O) foe.$(O) \
	screen.$(O) clrtbl.$(O) vector.$(O) degutil.$(O) rand.$(O)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(PROG) *.$(O)
