CPP = $(CXX)
CPPFLAGS = -g -std=c++2a -Wall -Wextra -O2
LDFLAGS = -pthread

ServerFiles = $(shell find . -type f -regex '.*.cpp')

ServerObjects = $(ServerFiles:.cpp=.o)

default: server_test

server_test: $(ServerObjects)
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm ./*.o
	rm -rf ./*_test
