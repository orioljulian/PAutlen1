comp0: comp0.o
	gcc -o comp0 comp0.o
	rm -f comp0.o
input: input.o
	gcc -m32 -o input input.o alfalib.o
	rm -f input.o
comp0.o:
	gcc -c comp0.c
input.o:
	nasm -g -o input.o -f elf32 input.asm
cleanall:
	rm -f comp0
	rm -f comp0.o
	rm -f input.o
	rm -f input.asm
	rm -f input
clean:
	rm -f input.o
	rm -f input
todo: todo2 input
	./input
todo2:comp0
	./comp0 input.txt
