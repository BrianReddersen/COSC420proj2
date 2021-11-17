main:
	mpicc -std=c99 -w adjmat.c -o adjmat
	
run:
	mpirun -n 1 ./adjmat
	
clean:
	rm adjmat
	
rebuild:
	make clean; make
