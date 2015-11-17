TARGET:= example
OBJLIBS	= libxcommunication libxstypes
CPP_FILES := $(wildcard *.cpp)
OBJECTS := $(CPP_FILES:.cpp=.o)
OBJECTS+=conio.o
HEADERS = $(wildcard *.h)
INCLUDE=-I. -Iinclude
CFLAGS=-g $(INCLUDE) -include config.h
CXXFLAGS=$(CFLAGS)
LFLAGS=-Lxcommunication -Lxstypes -lxcommunication -lxstypes -lpthread -lrt -ldl

all : $(OBJLIBS) $(TARGET)

libxcommunication : libxstypes
	$(MAKE) -C xcommunication $(MFLAGS)

libxstypes :
	$(MAKE) -C xstypes $(MFLAGS) libxstypes.a

$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LFLAGS)

clean :
	-$(RM) $(OBJECTS) $(TARGET)
	-$(MAKE) -C xcommunication $(MFLAGS) clean
	-$(MAKE) -C xstypes $(MFLAGS) clean
