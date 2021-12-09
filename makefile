main:
	mpicc -std=c99 -w adjmat.c -o adjmat
	
run:
	mpirun -n 1 ./adjmat
	
clean:
	rm adjmat; rm bst
	
rebuild:
	make clean; make

bst:
	mpicc -std=c99 -w testbst.c -o bst

runbst:
	mpirun -n 1 ./bst

rebuild2:
	make clean; make bst

test:
	make rebuild2; make runbst
	
graph:
	mpicc -std=c99 -w buildGraph.c -o buildgraph
	
db:
	mpicc -o dbsearch dbsearch.c $$(pkg-config --libs --cflags libmongoc-1.0)

parse:
	gcc -o parse parse.c $$(pkg-config --libs --cflags libmongoc-1.0)
