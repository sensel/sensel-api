NAME = libsensel

SRC = sensel.c \
			sensel_register.c \
			sensel_serial_linux.c

SRCPRFX = $(addprefix src/, $(SRC))

OBJ = $(SRCPRFX:.c=.o)

OBJPRFX = build/release/nopressure/

CC = gcc

CFLAGS = -c -std=c99 -Wall -Werror -fPIC -fvisibility=hidden -Isrc/ -DSENSEL_EXPORTS 

LDFLAGS = 

CFLAGSOPT = -O2

$(NAME): CFLAGS += $(CFLAGSOPT)
$(NAME): cleanobj $(OBJ)
	mkdir -p $(OBJPRFX)/obj
	mv $(OBJ) $(OBJPRFX)/obj
	$(CC) -shared -Wl,-soname,$(NAME).so -o $(addprefix $(OBJPRFX), $(NAME).so) $(addprefix $(OBJPRFX)/obj/, $(notdir $(OBJ))) $(LDFLAGS)
	strip $(addprefix $(OBJPRFX), $(NAME).so)

debug: OBJPRFX := build/debug/nopressure/
debug: CFLAGS += -g -DDEBUG
debug: LDFLAGS := 
debug: CFLAGSOPT :=
debug: $(NAME)

pressure: OBJPRFX := build/release/pressure/
pressure: CFLAGS  += -DSENSEL_PRESSURE
pressure: LDFLAGS += -lsenseldecompress
pressure: $(NAME)

debug_pressure: OBJPRFX := build/debug/pressure/
debug_pressure: CFLAGS  += -DSENSEL_PRESSURE -g -DDEBUG
debug_pressure: LDFLAGS += -lsenseldecompress
debug_pressure: CFLAGSOPT :=
debug_pressure: $(NAME)

clean: cleanobj
	rm -rf build/

cleanobj:
	rm -f src/*.o

re: clean $(NAME)
