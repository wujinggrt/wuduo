CPP = g++
CPPFLAGS = -I../wuduo -I../ -g -std=c++2a -Wall -Wextra -O0 -fsanitize=address -fno-omit-frame-pointer
 LDFLAGS = -lpthread
#CPPFLAGS = -DNDEBUG -I../wuduo -I../ -std=c++2a -Wall -Wextra -O3
# -lpthread should be positioned on last
# LDFLAGS = -lgtest -lgtest_main -lpthread
BuildDir = build
EnsureBuildDirExisted := $(shell if [ ! -d $(BuildDir) ]; then mkdir -p $(BuildDir); fi)

WuduoFiles = $(shell find ./wuduo -maxdepth 1 -type f -regex '.*.cpp')
WuduoObjects = $(shell find ./wuduo -maxdepth 1 -type f -regex '.*.cpp' | sed -n \
					's@.*/\(.*\)\.cpp@$(BuildDir)/\1\.o@p')

HttpFiles = $(shell find ./wuduo/http -maxdepth 1 -type f -regex '.*.cpp')
HttpObjects = $(shell find ./wuduo/http -maxdepth 1 -type f -regex '.*.cpp' | sed -n \
					's@.*/\(.*\)\.cpp@$(BuildDir)/\1\.o@p')

default: $(BuildDir)/server_test

$(BuildDir)/server_test: $(WuduoObjects) $(HttpObjects) $(BuildDir)/server_main.o
	$(CPP) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(WuduoObjects): $(BuildDir)/%.o: wuduo/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

$(BuildDir)/server_main.o: ./test/server_main.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

$(HttpObjects): $(BuildDir)/%.o: wuduo/http/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<

clean:
	@rm ./$(BuildDir)/*.o
	@rm -rf ./$(BuildDir)/*_test
