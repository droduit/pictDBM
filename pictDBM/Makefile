LIBS = openssl vips json-c
CFLAGS += -std=c99 -Wno-padded
CFLAGS += $$(pkg-config --cflags $(LIBS))
LDLIBS += $$(pkg-config --libs $(LIBS))

all: pictDBM pictDB_server

error.o: error.c error.h
image_content.o: image_content.c image_content.h
pictDBM_tools.o: pictDBM_tools.c pictDBM_tools.h
db_list.o: db_list.c pictDB.h error.h
db_utils.o: db_utils.c pictDB.h error.h
db_create.o: db_create.c pictDB.h error.h
db_delete.o: db_delete.c pictDB.h error.h
db_insert.o: db_insert.c pictDB.h error.h
db_read.o: db_read.c pictDB.h error.h
db_gbcollect.o: db_gbcollect.c pictDB.h error.h
dedup.o: dedup.c dedup.h
pictDBM.o: pictDBM.c pictDB.h error.h
pictDB_server.o : pictDB_server.c

pictDBM: error.o db_utils.o db_list.o db_create.o db_delete.o db_insert.o db_read.o db_gbcollect.o dedup.o pictDBM_tools.o image_content.o pictDBM.o

pictDB_server: CFLAGS += -isystem libmongoose
pictDB_server: LDFLAGS += -Llibmongoose
pictDB_server: LDLIBS += -lmongoose
pictDB_server: error.o db_utils.o db_list.o db_delete.o db_insert.o dedup.o db_read.o image_content.o pictDB_server.o

clean:
	rm -f *.o *.orig

clean-all: clean
	rm -f pictDBM
	rm -f pictDB_server

style:
	astyle -A8 *.c *.h

server:
	LD_LIBRARY_PATH=libmongoose ./pictDB_server testDB02.pictdb_static

testRendu1:
	./pictDBM create test
	./pictDBM insert test pic1 papillon.jpg
	./pictDBM list test

	./pictDBM read test pic1
	./pictDBM read test pic1 thumb
	./pictDBM read test pic1 small
	./pictDBM list test

	./pictDBM delete test pic1
	./pictDBM insert test pic1 foret.jpg
	./pictDBM list test

	./pictDBM read test pic1
	./pictDBM read test pic1 thumb
	./pictDBM read test pic1 small
	./pictDBM list test

testGC:
	./pictDBM create test
	./pictDBM insert test pic1 papillon.jpg
	./pictDBM insert test pic2 foret.jpg
	./pictDBM insert test pic3 papillon.jpg
	./pictDBM insert test pic4 papillon.jpg
	./pictDBM insert test pic5 papillon.jpg
	./pictDBM read test pic1
	./pictDBM read test pic2
	./pictDBM read test pic3
	./pictDBM read test pic4
	./pictDBM read test pic5
	./pictDBM list test
	ls -la
	./pictDBM delete test pic5
	./pictDBM delete test pic4
	./pictDBM delete test pic3
	./pictDBM delete test pic2
	./pictDBM delete test pic1
	./pictDBM list test
	ls -la
	./pictDBM gc test tmpTest
	./pictDBM list test
	ls -la
