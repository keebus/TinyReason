CFLAGS:= -Wall
LFLAGS:= -static
LIBS:=

ifdef DEBUG
	CFLAGS+= -O0 -g
else
	CFLAGS+= -O3
endif

ifdef WIN32
	TARGET:=salcr.exe
	LFLAGS+=
	LIBS+=
else
	TARGET:=salcr.elf
	LIBS+= 
endif

CC:=g++

OBJS:=$(shell find ./source -type f -name "*.cpp" | sed "s/cpp/o/g" | sed "s/source/build/g")
DIRS:=$(shell find ./source -type d | sed "s/source/build/g")

	
all:  $(DIRS) $(TARGET)
	
$(DIRS):
	mkdir -p $(DIRS)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) $(LIBS) -o $(TARGET)

$(OBJS): ./build/%.o : ./source/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -rf build $(TARGET)