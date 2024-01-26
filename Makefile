JSON = "../game_data/map2.json"
IP = "127.0.0.1"
PORT = "9995"

CC=../../../AFLplusplus/afl-clang-fast
LIBS = `pkg-config --libs --cflags gtk+-2.0`

# ifdef gcov
# CFLAGS=-O0 --coverage 
# else
# CFLAGS=-g -Wall 
# endif
CFLAGS=-O0 --coverage 


# build original server, client
client: client.c 
	$(CC) $(CFLAGS) client.c ../../cJSON.c -o client $(LIBS)

server: server.c 
	$(CC) $(CFLAGS) server.c ../../cJSON.c -o server


# build original server, client
test_client: client.c 
	$(CC) $(CFLAGS) -c client.c ../../cJSON.c $(LIBS) -DTEST -pthread

test_server: server.c 
	$(CC) $(CFLAGS) server.c ../../cJSON.c  -DTEST


# build test main file
parseJson: main/parseJson.c 
	$(CC) $(CFLAGS) main/parseJson.c client.o ../../cJSON.c -I. $(LIBS) -o parseJson 

convey_bytes: main/convey_bytes.c 
	$(CC) $(CFLAGS) main/convey_bytes.c client.o ../../cJSON.c -I. $(LIBS) -o convey_bytes

check_validation: main/check_validation.c 
	$(CC) $(CFLAGS) main/check_validation.c client.o ../../cJSON.c -I. $(LIBS) -o check_validation 

update_cell: main/update_cell.c 
	$(CC) $(CFLAGS) main/update_cell.c client.o ../../cJSON.c -I. $(LIBS) -o update_cell

move: main/move.c 
	$(CC) $(CFLAGS) main/move.c client.o ../../cJSON.c -I. $(LIBS) -o move

update_model: main/update_model.c 
	$(CC) $(CFLAGS) main/update_model.c client.o ../../cJSON.c -I. $(LIBS) -o update_model

handle: main/handle_cmd.c 
	$(CC) $(CFLAGS) main/handle_cmd.c client.o ../../cJSON.c -I. $(LIBS) -o handle_cmd -pthread



# run afl-fuzz
pj:
	../../../AFLplusplus/afl-fuzz -i inputs/parseJson -o outputs/pj ./parseJson @@

cb:
	../../../AFLplusplus/afl-fuzz -i inputs/convey_bytes -o outputs/cb ./convey_bytes

cv:
	../../../AFLplusplus/afl-fuzz -i inputs/check -o outputs/cv ./check_validation

uc:
	../../../AFLplusplus/afl-fuzz -i inputs/update_cell -o outputs/uc ./update_cell @@


# make gcov arg1=convey_bytes.c
gcov:
	gcov -bcif $(arg1)

clean:
	rm -rf *.gcno *.gcda *.gcov client server convey_bytes parseJson check_validation update_cell move handle_cmd update_model

clean_o:
	rm -rf *.o

clean_g:
	rm -rf *.gcda *.gcov


# ../../../AFLplusplus/afl-clang-fast -O0 --coverage -o client client.c ../../cJSON.c  `pkg-config --libs --cflags gtk+-2.0` -DDEBUG