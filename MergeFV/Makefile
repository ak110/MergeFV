SRCS     = $(wildcard *.cpp)
OBJS     = $(SRCS:.cpp=.o)
CXXFLAGS = -Wall -g -O3 -march=native -std=c++14 -fopenmp
CPPFLAGS = -DNDEBUG
LDFLAGS  = -pthread
LDFLAGS  += -lgomp -fopenmp

all: MergeFV

clean:
	rm -f MergeFV $(OBJS)

.PHONY: all clean

MergeFV: $(OBJS)
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) -o $@ $^

$(OBJS): $(SRCS) $(wildcard *.h)

