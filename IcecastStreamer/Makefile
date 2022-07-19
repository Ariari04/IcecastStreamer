CC=gcc
CPP=g++
DEFINES=-D_FILE_OFFSET_BITS=64 -DUSE_TAGGING

# -O2 - optimize for speed
CPPFLAGS=-Wall -O2 -I. -I../../libfaad-output/include -I../../ogg-output/include -I../../vorbis-output/include -I../../minimp3 -std=c++14 $(DEFINES) 
CFLAGS=$(CPPFLAGS)
LDFLAGS=-L../../ogg-output/lib \
-L../../vorbis-output/lib \
-L../../libfaad-output/lib \
-logg -lvorbis -lvorbisenc -lz -lfaad -lstdc++  -lm -lboost_system -lboost_filesystem -lboost_thread -lpthread 

EXECUTABLE=icecastStreamer

SOURCES_CPP=main.cpp \
	aac/AacDecoder.cpp \
	icecast/IcecastStreamer.cpp \
	mp3/Mp3DecoderNew.cpp \
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
	#rm -f tags/*.o
	#rm -f 3rd/blade/*.o
	#rm -f 3rd/mp4ff/*.o
	#rm -f 3rd/faad/*.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.cpp.o:
	$(CPP) $(CPPFLAGS) -c $< -o $@
