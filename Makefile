TARGETS = mersenne-twister.o test-mt
CXXFLAGS = -W -Wall

all: $(TARGETS)

check: all
	./test-mt

test-mt: mersenne-twister.o

clean:
	rm -f $(TARGETS)
