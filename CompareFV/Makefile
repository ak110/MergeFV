SRCS     = $(wildcard *.cpp)
OBJS     = $(SRCS:.cpp=.o)
CXXFLAGS = -Wall -g -O3 -march=native -std=c++11 -fopenmp
CPPFLAGS = -DNDEBUG
LDFLAGS  = -pthread
LDFLAGS  += -lgomp -fopenmp

all: CompareFV

clean:
	rm -f CompareFV $(OBJS)

.PHONY: all clean

CompareFV: $(OBJS)
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) -o $@ $^

$(OBJS): $(SRCS) $(wildcard *.h)

