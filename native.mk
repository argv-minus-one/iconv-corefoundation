CXXFLAGS := -mmacosx-version-min=10.10 -arch x86_64 -arch arm64 -Inode_modules/node-addon-api -I/usr/local/include/node -flto -fno-rtti -Os -fvisibility=hidden -Wall -std=c++17 -DBUILDING_NODE_EXTENSION -flto $(CXXFLAGS)
LDFLAGS := $(CXXFLAGS) -bundle -undefined dynamic_lookup -Wl,-x -framework CoreFoundation -Wl,-dead_strip -g0 $(LDFLAGS)

lib/native.node: build/iccf.o build/string-utils.o build/StringEncoding.o build/transcode.o
	@mkdir -p lib
	$(CXX) $(LDFLAGS) -o $@ $^

build/%.o: src/%.cc
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c -o $@ $^
