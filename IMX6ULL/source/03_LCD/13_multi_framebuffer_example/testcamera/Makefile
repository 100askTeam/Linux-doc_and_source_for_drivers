
CC = mipsel-linux-gcc
CXX = mipsel-linux-g++
CFLAGS = -O3 -Wall
LFLAGS = -lpthread --static -s

OBJS = Framebuffer.o video.o main.o

TARGET = testcamera

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LFLAGS)

%.o: %.cxx
	$(CXX) $(CFLAGS) -c -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)

