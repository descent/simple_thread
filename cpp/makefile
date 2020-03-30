CXX=g++
CFLAGS=-save-temps -std=c++17 -g -Wall -I.. -no-pie -fno-pic -fno-stack-protector -static

LINKS=

ifeq ($(PTHREAD), 1)
LINKS+=-pthread
CFLAGS+=-DUSE_PTHREAD
endif

simple_thread: simple_thread.o my_setjmp.o
	$(CXX) -m32 $(CFLAGS) -o $@ $^ $(LINKS)

simple_thread.o: simple_thread.cpp ../my_setjmp.h
	$(CXX) -m32 -DX86_32 $(CFLAGS) -c $<

my_setjmp.o: ../my_setjmp.S ../my_setjmp.h
	$(CXX) -m32 -DX86_32 $(CFLAGS) -c $<

simple_thread_64: simple_thread_64.o my_setjmp_64.o
	$(CXX) -m64 $(CFLAGS) -o $@ $^

simple_thread_64.o: simple_thread.cpp my_setjmp.h
	$(CXX) -DX86_64 -m64 $(CFLAGS) -c $< -o $@

my_setjmp_64.o: ../my_setjmp.S ../my_setjmp.h
	$(CXX) -m64 -DX86_64 $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o simple_thread
distclean:
	make clean
	rm -rf *.i *.ii *.s
