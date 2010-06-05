all: socket.o download.o filesearch.o md5.o request_handler.o model_gslist.o gui.o file_db.o
	gcc -o pingu download.o model_gslist.o file_db.o socket.o md5.o filesearch.o gui.o request_handler.o `pkg-config --libs gtk+-2.0 gthread-2.0` -lm

GCCOPTS=-g

download.o: download.c servlet.h protocol.h request_handler.h
	gcc $(GCCOPTS) -c `pkg-config --cflags glib-2.0` $<

request_handler.o: request_handler.c request_handler.h
	gcc $(GCCOPTS) -c `pkg-config --cflags glib-2.0 gthread-2.0` $<

md5.o:md5.c
	# This is from GNU core utils
	gcc -c $<

model_gslist.o: model_gslist.c model_gslist.h
	gcc $(GCCOPTS) -c `pkg-config --cflags gtk+-2.0` $<


gui.o: gui.c model_gslist.h
	gcc $(GCCOPTS) -c `pkg-config --cflags gtk+-2.0` $<

socket.o: socket.c servlet.h protocol.h request_handler.h
	gcc $(GCCOPTS) -c `pkg-config --cflags glib-2.0 gthread-2.0` $<		

filesearch.o: filesearch.c servlet.h protocol.h request_handler.h
	gcc $(GCCOPTS) -c `pkg-config --cflags gtk+-2.0` $<

file_db.o: file_db.c file_db.h md5.c md5.h
	gcc $(GCCOPTS) -c `pkg-config --cflags glib-2.0` $<
