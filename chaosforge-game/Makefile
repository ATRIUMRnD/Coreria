


CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c99
OBJS=main.o player_controller.o combat_system.o physics_manager.o game_state.o

# Windows build (MinGW/MSYS2)
ifeq ($(OS),Windows_NT)
    LIBS=-lopengl32 -lglu32 -lgdi32 -luser32 -lkernel32 -lm
    TARGET=chaosforge.exe
    RM=del
else
    # Linux/Unix build (would need OpenGL libraries)
    LIBS=-lGL -lGLU -lglfw -lglut -lm
    TARGET=chaosforge
    RM=rm -f
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o $(TARGET)

.PHONY: all clean
