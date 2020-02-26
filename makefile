CC=gcc
CFLAGS=-m32 -g
simple_thread: simple_thread.o my_setjmp.o
	$(CC) $(CFLAGS) -o $@ $^

simple_thread.o: simple_thread.c my_setjmp.h
	$(CC) $(CFLAGS) -c $<

my_setjmp.o: my_setjmp.S my_setjmp.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o simple_thread
