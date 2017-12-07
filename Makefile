TARGETS = mersenne-twister.o reference/mt19937ar.o test-mt
CXXFLAGS = -W -Wall -Wextra -Wsign-compare \
					 --std=gnu++11 \
					 -m64 \
					 -msse \
					 -O2 \
					 -march=native \
					 -funroll-loops \
					 -ftree-vectorize \
					 -fomit-frame-pointer

all: $(TARGETS)

check: all
	./test-mt 20

benchmark: check

test-mt: mersenne-twister.o reference/mt19937ar.o
test-bench: test-mt

clean:
	rm -f $(TARGETS)
