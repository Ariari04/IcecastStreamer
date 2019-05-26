CC=gcc
CPP=g++
DEFINES=-D_FILE_OFFSET_BITS=64 -DUSE_TAGGING

# -O2 - optimize for speed
CPPFLAGS=-Wall -O2 -I. -std=c++14 $(DEFINES) 
CFLAGS=$(CPPFLAGS)
LDFLAGS=-L./../lame-3.100/mpglib/.libs \
-L./../faad2-vs/libfaad/.libs \
-L./../id3lib-3.8.3/src/.libs \
-L./../libmp4v2-1.5.0.1/Compile/.libs \
-L./../lame-3.100/libmp3lame/.libs \
-L./3rd/libs -l:libmpgdecoder.a -l:libmp3lame.a -lz -l:libid3.a -l:libfaad.a -lstdc++  -lm -lboost_system -lboost_filesystem -lboost_thread -lpthread 

EXECUTABLE=conv

SOURCES_CPP=main.cpp \
	aac/AacDecoder.cpp \
	icecast/IcecastStreamer.cpp \
	mp3/Mp3Decoder.cpp \
	wave/WaveDecoder.cpp \
	wave/WaveFile.cpp
	
SOURCES_C=

OBJECTS_CPP=$(SOURCES_CPP:.cpp=.o)
OBJECTS_C=$(SOURCES_C:.c=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS_CPP) $(OBJECTS_C)
	$(CC) -s $(OBJECTS_CPP) $(OBJECTS_C) $(LDFLAGS) -o $@

clean:
	rm -f *.o $(EXECUTABLE)
	rm -f tags/*.o
	rm -f 3rd/blade/*.o
	rm -f 3rd/mp4ff/*.o
	rm -f 3rd/faad/*.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@
