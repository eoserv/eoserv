
BIN = eoserv
OBJ = obj/config.o obj/eoclient.o obj/handlers.o obj/main.o obj/packet.o obj/socket.o obj/util.o obj/variant.o
LIBS = 
CXXFLAGS = -O2
LDFLAGS = -s

.PHONY: all all-before all-after clean clean-custom

all: all-before eoserv all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $(BIN) $(LIBS) $(LDFLAGS)

obj/%.o: src/%.cpp
	$(CXX) -I./src/ -c $< -o $@ $(CXXFLAGS)
