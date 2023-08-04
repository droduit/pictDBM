LIBS = openssl vips
CFLAGS += -std=c99 -Wno-padded
CFLAGS += $$(pkg-config --cflags $(LIBS))
LDLIBS += $$(pkg-config --libs $(LIBS))

all: pictDBM

error.o: error.c
db_list.o: db_list.c pictDB.h error.h
db_utils.o: db_utils.c pictDB.h error.h
db_create.o: db_create.c pictDB.h error.h
pictDBM.o: pictDBM.c pictDB.h error.h

pictDBM: error.o db_utils.o db_list.o db_create.o pictDBM.o
