TARGETS=mmap_basic mmap_basic2 pipe_basic

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

mmap_basic: mmap_basic.c
	gcc -O3 -o $@ $< -lrt -pthread

mmap_basic2: mmap_basic2.c
	gcc -O3 -o $@ $< -lrt -pthread

pipe_basic: pipe_basic.c
	gcc -O3 -o $@ $<

