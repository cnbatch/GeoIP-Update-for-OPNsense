CXXFLAGS = -Wall -O3 -std=c++17
INCLUDEDIR = -I./ -I/usr/include/ -I/usr/local/include/
LDFLAGS = -L/usr/lib -L/usr/local/lib 
LDLIBS = -lcurl -larchive -lc++
OUTPUT = updategeoip
SOURCES = main.cpp geoip_file.cpp
OBJECTS=${SOURCES:.cpp=.o}

all : ${OUTPUT}

install:
	@cp ${OUTPUT} /usr/local/bin

uninstall:
	@rm /usr/local/bin/${OUTPUT}

clean:
	@rm -f ${OUTPUT} *.o *~

${OUTPUT}: ${OBJECTS}
	@cc ${LDFLAGS} ${LDLIBS} ${OBJECTS} -o ${OUTPUT}

.cpp.o:
	@cc ${INCLUDEDIR} ${CXXFLAGS} -c ${.IMPSRC} -o ${.TARGET}

