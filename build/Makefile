CPP = g++
#CPPFLAGS = -I../wuduo -I../ -g -std=c++2a -Wall -Wextra -O0 -fsanitize=address -fno-omit-frame-pointer
CPPFLAGS = -DNDEBUG -I../wuduo -I../ -std=c++2a -Wall -Wextra -O3
# -lpthread should be positioned on last
LDFLAGS = -lgtest -lgtest_main -lpthread

WuduoFiles = $(shell find ../wuduo -maxdepth 1 -type f -regex '.*.cpp')
WuduoObjects = $(shell find ../wuduo -maxdepth 1 -type f -regex '.*.cpp' | sed -n \
					's@.*/\(.*\)\.cpp@\./\1\.o@p')

HttpFiles = $(shell find ../wuduo/http -maxdepth 1 -type f -regex '.*.cpp')
HttpObjects = $(shell find ../wuduo/http -maxdepth 1 -type f -regex '.*.cpp' | sed -n \
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

server_test: $(WuduoObjects) $(HttpObjects) ./server_main.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

./server_main.o: ../test/server_main.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

$(HttpObjects): %.o: ../wuduo/http/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

clean:
	rm ./*.o
	rm -rf ./*_test
