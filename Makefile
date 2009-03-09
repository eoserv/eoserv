
BIN = eoserv
OBJ = obj/config.o obj/database.o obj/eoclient.o obj/eodata.o obj/eoserv.o obj/handlers.o obj/main.o obj/packet.o obj/socket.o obj/util.o
CXXFLAGS = -O2 -DDATABASE_MYSQL -DDATABASE_SQLITE
LDFLAGS = -s
LIBS = -lmysql -lsqlite3

.PHONY: all all-before all-after clean clean-custom

all: all-before eoserv all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $(BIN) $(LIBS) $(LDFLAGS)

obj/%.o: src/%.cpp
	$(CXX) -I./src/ -c $< -o $@ $(CXXFLAGS)
