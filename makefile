all: sim

sim: onefully.c nda.h rb-hun.c
	clang -o $@ $< -O3 -g -lgsl -lgslcblas -lm -static


debug: onefully.c nda.h rb-hun.c
	clang -o sim $< -O3 -g -lgsl -lgslcblas -lm -static -DDEBUG
