CPP = $(CXX)
CPPFLAGS = -g -std=c++17 -Wall -Wextra -O0
LDFLAGS = -lpthread

CppFiles = $(shell find . -type f -regex '.*.cpp')

Objects = $(CppFiles:.cpp=.o)

default: server_test

server_test: $(Objects)
	$(CPP) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

$(Objects): $(CppFiles)
	$(CPP) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

clean:
	rm ./*.o
	rm -rf ./*_test
