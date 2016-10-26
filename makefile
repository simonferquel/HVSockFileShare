INCLUDES := ./Includes
CXX_FLAGS := -std=c++1y -g -ggdb
all: linuxclient

obj:
	mkdir obj || true
# commonlib

COMMONLIB_SOURCES := $(wildcard src/Common/*.cpp)
COMMONLIB_OBJECTS := $(patsubst src/Common/%.cpp, obj/commonlib/%.o, $(COMMONLIB_SOURCES))

commonlib: obj/commonlib $(COMMONLIB_OBJECTS)

obj/commonlib: obj
	mkdir obj/commonlib || true

obj/commonlib/%.o: src/Common/%.cpp
	$(CXX) $(CXX_FLAGS) $(CPP_FLAGS) -I $(INCLUDES) -c $< -o $@


#clientlib

CLIENTLIB_SOURCES := $(wildcard src/ClientLib/*.cpp)
CLIENTLIB_OBJECTS := $(patsubst src/ClientLib/%.cpp, obj/clientlib/%.o, $(CLIENTLIB_SOURCES))

clientlib: commonlib obj/clientlib $(CLIENTLIB_OBJECTS)

obj/clientlib: obj
	mkdir obj/clientlib || true
	
obj/clientlib/%.o: src/ClientLib/%.cpp
	$(CXX) $(CXX_FLAGS) $(CPP_FLAGS) -I $(INCLUDES) -c $< -o $@


#linuxclient
LINUXCLIENT_SOURCES := $(wildcard src/LinuxClient/*.cpp)
LINUXCLIENT_OBJECTS := $(patsubst src/LinuxClient/%.cpp, obj/linuxclient/%.o, $(LINUXCLIENT_SOURCES))

linuxclient: clientlib obj/linuxclient $(LINUXCLIENT_OBJECTS)
	$(CXX) $(CXX_FLAGS) $(LD_FLAGS)  $(LINUXCLIENT_OBJECTS) $(CLIENTLIB_OBJECTS) $(COMMONLIB_OBJECTS) -o $@
obj/linuxclient: obj
	mkdir obj/linuxclient || true
	
obj/linuxclient/%.o: src/LinuxClient/%.cpp
	$(CXX) $(CXX_FLAGS) $(CPP_FLAGS) -I $(INCLUDES) -c $< -o $@