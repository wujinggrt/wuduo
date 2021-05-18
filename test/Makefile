CPPFLAGS = -g -std=c++2a -Wall -Wextra -O2
LDFLAGS = -lpthread -lgtest

EventLoopFile = $(shell find ../wuduo -type f -regex '.*.cpp')
EventLoopObject = $(EventLoopFile:.cpp=.o)

EventLoopTestFile = $(shell find ../wuduo -type f -regex '.*.cpp') "./main.cpp"
EventLoopTestObject = $(EventLoopFile:.cpp=.o)

defualt: event_loop_tets

event_loop_tets: $(EventLoopObject) $(EventLoopTestObject)
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS);
	mv $^ .

clean:
	rm ./*.o
	rm $(EventLoopTestObject)
