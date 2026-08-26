all: gen

gen: gen.cpp
	g++ gen.cpp -o gen

run: gen
	./gen

clean:
	rm -f gen