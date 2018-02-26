CC=gcc
CPP=g++
DEFINES=-D_FILE_OFFSET_BITS=64 -DUSE_TAGGING

# -O2 - optimize for speed
CPPFLAGS=-Wall -O2 $(DEFINES)
CFLAGS=$(CPPFLAGS)
LDFLAGS=-L./3rd/libs -lmysqlpp -lmp3lame -lid3 -lfaad -lmp4v2 -lmpgdecoder

EXECUTABLE=conv

SOURCES_CPP=main.cpp \
	AacDecoder.cpp \
	ConsoleHelpers.cpp \
	Conversion.cpp \
	Converter528.cpp \
	ConverterApp.cpp \
	ConvException.cpp \
	DecoderAdapter.cpp \
	EncoderAdapter.cpp \
	FlatConverter.cpp \
	KString.cpp \
	Mp3Decoder.cpp \
	Mp3Encoder.cpp \
	Settings.cpp \
	WaveFileAdapter.cpp \
	WaveFile.cpp \
	tags/ID3Tags.cpp \
	tags/Tags.cpp \
	tags/TagsUsingMP4v2.cpp \
	
SOURCES_C=ConvertUTF.c \
	  3rd/blade/BladeMP3Enc.c \
	  3rd/mp4ff/drms.c \
	  3rd/mp4ff/mp4atom.c \
	  3rd/mp4ff/mp4ff.c \
	  3rd/mp4ff/mp4meta.c \
	  3rd/mp4ff/mp4sample.c \
	  3rd/mp4ff/mp4tagupdate.c \
	  3rd/mp4ff/mp4util.c \
	  3rd/faad/main.c

OBJECTS_CPP=$(SOURCES_CPP:.cpp=.o)
OBJECTS_C=$(SOURCES_C:.c=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS_CPP) $(OBJECTS_C)
	$(CC) -s $(LDFLAGS) $(OBJECTS_CPP) $(OBJECTS_C) -o $@

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
