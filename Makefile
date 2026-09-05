all: gen

gen: gen.cpp
	g++ gen.cpp -o gen -std=c++23

run: gen
	./gen

clean:
	rm -f gen
	rm -f out.csv
