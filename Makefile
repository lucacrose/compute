all: gen

gen: main.cpp values.cpp baskets.cpp
	g++ main.cpp values.cpp baskets.cpp -o gen -std=c++23

run: gen
	./gen

clean:
	rm -f gen
