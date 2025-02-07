all: mlk init.so foo.so bar.so

mlk : my_litle_kernel.c scc.so syscall.h
	gcc -Wno-deprecated my_litle_kernel.c scc.so -ldl -o mlk
scc.so: scc.o
	gcc -shared -o scc.so scc.o
%.so: %.o
	gcc -shared -o $@ $< scc.so
%.o: %.c
	gcc -fPIC -c $<

