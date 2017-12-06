TARGETS = mersenne-twister.o test-mt
CXXFLAGS = -W -Wall -Wextra -Wsign-compare \
					 --std=gnu++11 \
					 -O2 \
					 -march=native \
					 -ftree-vectorize \
					 -fomit-frame-pointer

all: $(TARGETS)

check: all
	./test-mt 20

benchmark: check

test-mt: mersenne-twister.o
test-bench: test-mt

clean:
	rm -f $(TARGETS)
