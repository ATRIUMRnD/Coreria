CC=gcc
CFLAGS=-Wall
OBJS=main.o player_controller.o combat_system.o physics_manager.o game_state.o

all: chaosforge

chaosforge: $(OBJS)
	$(CC) -o chaosforge $(OBJS) -lode -lGL

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o chaosforge
