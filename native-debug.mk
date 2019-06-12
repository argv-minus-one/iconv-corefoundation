CXXFLAGS := -mmacosx-version-min=10.10 -arch x86_64 -Inode_modules/node-addon-api -I/usr/local/include/node -fno-rtti -fvisibility=hidden -Wall -std=c++17 -DBUILDING_NODE_EXTENSION -g $(CXXFLAGS)
LDFLAGS := -macosx_version_min 10.10 -arch x86_64 -bundle -undefined dynamic_lookup -framework CoreFoundation $(LDFLAGS)

lib/native.node: build/iccf.o build/string-utils.o build/StringEncoding.o build/transcode.o
	@mkdir -p lib
	$(LD) $(LDFLAGS) -o $@ $^

build/%.o: src/%.cc
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c -o $@ $^
