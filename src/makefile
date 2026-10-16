CC       = gcc
CFLAGS   = -Wall -Wextra -g -MMD -MP \
					 -lcurl   \
					 -ljson-c \
					 $(INCLUDES)

INCLUDES = -I. \
           -I$(DIR_STEAMCURL) \
           -I$(DIR_ICKLIB) \
           -I$(DIR_ICKSESSION)

# Separate CFLAGS (for compiling) and LIBS (for linking)
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)

DIR_STEAMCURL  = ./libs/steamcurl
DIR_ICKLIB     = ./libs/icklib
DIR_ICKSESSION = ./libs/icksession
DIR_OUTPUTLOC  = ./build

SRC = $(DIR_STEAMCURL)/steamcurl.c  	\
      $(DIR_ICKLIB)/ickclock.c      	\
      $(DIR_ICKLIB)/ickutils.c      	\
			$(DIR_ICKLIB)/ickstorage.c			\
			$(DIR_ICKSESSION)/icksession.c 	\
			$(DIR_ICKSESSION)/dirconf/ickconf.c					\
			$(DIR_ICKSESSION)/dirmain/icksession_menu.c \
			$(DIR_ICKSESSION)/dirmain/icksession_home.c \
     	sscvisual.c

OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)
TARGET = executable

.PHONY: all clean

all: $(TARGET)

# Linking stage (uses GTK_LIBS at the END of the command)
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(DIR_OUTPUTLOC)/$@ $(GTK_LIBS)
# Compilation stage (uses GTK_CFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@ 

# Auto-dependency inclusion
-include $(DEP)

clean:
	rm -f $(OBJ) $(DEP)
	rm -f $(DIR_OUTPUTLOC)/$(TARGET)

