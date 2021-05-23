CPP = g++
CPPFLAGS = -g -std=c++17 -Wall -Wextra -O0
# -lpthread should be positioned on last
LDFLAGS = -lgtest -lgtest_main -lpthread

WuduoFiles = $(shell find ../wuduo -type f -regex '.*.cpp')
WuduoObjects = $(shell find ../wuduo -type f -regex '.*.cpp' | sed -n \
					's@.*/\(.*\)\.cpp@\./\1\.o@p')

default: event_loop_test

event_loop_test: $(WuduoObjects) event_loop_test.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

event_loop_test.o: %.o: ../test/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

timer_queue_test: $(WuduoObjects) timer_queue_test.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

timer_queue_test.o: %.o: ../test/timer_queue_test.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

$(WuduoObjects): %.o: ../wuduo/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

server_test: $(WuduoObjects) ./server_main.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm ./*.o
	rm -rf ./*_test
