all: gen

gen: values.cpp baskets.cpp
	g++ values.cpp baskets.cpp -o gen -std=c++23

run: gen
	./gen

clean:
	rm -f gen
