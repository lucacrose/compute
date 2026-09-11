RUN_ARGS = 4 32 8

all: gen

gen: main.cpp values.cpp baskets.cpp
	g++ main.cpp values.cpp baskets.cpp -o gen -std=c++23

run: gen
	./gen $(RUN_ARGS)

clean:
	rm -f gen
