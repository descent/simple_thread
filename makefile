CC=gcc
CFLAGS=-g -Wall 
#-no-pie -fno-pic -fno-stack-protector -static

test_setjmp: test_setjmp.o my_setjmp.o
	$(CC) -m32 $(CFLAGS) -o $@ $^

test_setjmp.o: test_setjmp.c my_setjmp.h
	$(CC) -DX86_32 -m32 $(CFLAGS) -c -o $@ $<

test_signal: test_signal.c
	$(CC) $(CFLAGS) -o $@ $^

simple_thread: simple_thread.o my_setjmp.o
	$(CC) -m32 $(CFLAGS) -o $@ $^

simple_thread.o: simple_thread.c my_setjmp.h
	$(CC) -m32 -DX86_32 -m32 $(CFLAGS) -c $<

my_setjmp.o: my_setjmp.S my_setjmp.h
	$(CC) -m32 -DX86_32 $(CFLAGS) -c $<

simple_thread_64: simple_thread_64.o my_setjmp_64.o
	$(CC) -m64 $(CFLAGS) -o $@ $^

simple_thread_64.o: simple_thread.c my_setjmp.h
	$(CC) -DX86_64 -m64 $(CFLAGS) -c $< -o $@

my_setjmp_64.o: my_setjmp.S my_setjmp.h
	$(CC) -m64 -DX86_64 $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o simple_thread
