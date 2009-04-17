
BIN = eoserv
OBJ = obj/config.o obj/database.o obj/eoclient.o obj/eodata.o obj/eoserv.o obj/handlers.o obj/hash.o obj/main.o obj/nanohttp.o obj/packet.o obj/sha256.o obj/socket.o obj/timer.o obj/util.o
CFLAGS = -O3 -DDATABASE_MYSQL -DDATABASE_SQLITE -I"/usr/include/mysql/"
CXXFLAGS = -O3 -DDATABASE_MYSQL -DDATABASE_SQLITE -I"/usr/include/mysql/"
LDFLAGS = -s
LIBS = -lmysqlclient -lsqlite3

.PHONY: all clean

all: eoserv

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $(BIN) $(LIBS) $(LDFLAGS)

obj/%.o: src/%.c
	$(CC) -I./src/ -c $< -o $@ $(CFLAGS)

obj/%.o: src/%.cpp
	$(CXX) -I./src/ -c $< -o $@ $(CXXFLAGS)
