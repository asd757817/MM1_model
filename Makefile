all: run

run: mm1.c
	gcc -o mm1 mm1.c -lm && ./mm1
	gnuplot p.gp
	eog pn.png


clean:
	rm -f *.o

