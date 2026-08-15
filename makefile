CXXFLAGS = -Wall -O3 -std=c++17
INCLUDEDIR = -I./ -I/usr/include/ -I/usr/local/include/
LDFLAGS = -L/usr/lib -L/usr/local/lib 
LDLIBS = -lcurl -larchive -lmd -lc++

OBJDIR = bin
OUTPUT = updategeoip
TARGET = ${OBJDIR}/${OUTPUT}

SOURCES = main.cpp geoip_file.cpp
OBJECTS = ${SOURCES:S/^/${OBJDIR}\//:S/.cpp/.o/}

all : ${TARGET}

install:
	-@mv /usr/local/bin/${OUTPUT} /usr/local/sbin/${OUTPUT} 2>/dev/null
	@cp ${TARGET} /usr/local/sbin/${OUTPUT}

uninstall:
	-@rm /usr/local/bin/${OUTPUT} 2>/dev/null
	-@rm /usr/local/sbin/${OUTPUT} 2>/dev/null

clean:
	@rm -rf ${OBJDIR} *~

${TARGET}: ${OBJECTS}
	@cc ${LDFLAGS} ${LDLIBS} ${OBJECTS} -o ${TARGET}

.for src in ${SOURCES}
${OBJDIR}/${src:S/.cpp/.o/}: ${src}
	@mkdir -p ${OBJDIR}
	@cc ${INCLUDEDIR} ${CXXFLAGS} -c ${src} -o ${.TARGET}
.endfor
