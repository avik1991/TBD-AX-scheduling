all: sim

sim: sim.c nda.h rb-hun.c
	clang -o $@ $< -O3 -g -lgsl -lgslcblas -lm -static


debug: sim.c nda.h rb-hun.c
	clang -o sim $< -O3 -g -lgsl -lgslcblas -lm -static -DDEBUG
