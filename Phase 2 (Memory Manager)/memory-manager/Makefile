build:
	gcc process_generator.c -o process_generator.out -w
	gcc clk.c -o clk.out -w
	gcc scheduler.c -o scheduler.out -lm -w
	gcc process.c -o process.out -w
	gcc test_generator.c -o test_generator.out -w

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out
