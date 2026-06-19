# https://wiki.libsdl.org/SDL3/README-linux#build-dependencies

CONFIG_FLAGS := 
GCC_FLAGS := 
BUILD_FLAGS := --parallel 4

# force GCC on windows
ifeq ($(OS), Windows_NT)
	CONFIG_FLAGS += -G "MinGW Makefiles"
endif

run: run_d
run_d: 
ifeq ($(OS), Windows_NT)
	./build/Debug/Sunrise_ANSI_Editor
else
	sudo ./build/Debug/Sunrise_ANSI_Editor
endif

run_r: 
ifeq ($(OS), Windows_NT)
	./build/Debug/Sunrise_ANSI_Editor
else
	sudo ./build/Debug/Sunrise_ANSI_Editor
endif

all: debug

debug: CONFIG_FLAGS += -DCMAKE_BUILD_TYPE=Debug
debug: GCC_FLAGS += -g -Wall -Wextra -Wpedantic -march=native
debug: build

release: CONFIG_FLAGS += -DCMAKE_BUILD_TYPE=Release
release: GCC_FLAGS += -O2 -march=native -DNDEBUG -s
release: build
ifeq ($(OS), Windows_NT)
	cd build  &&  cpack -G ZIP
else 
	cd build  &&  cpack -G TGZ
endif

build: 
ifeq ($(OS), Windows_NT)
	if not exist build mkdir build
	cd build  &&  cmake $(CONFIG_FLAGS) -DCMAKE_CXX_FLAGS="$(GCC_FLAGS)" ..  &&  cmake --build . $(BUILD_FLAGS)
else
	mkdir -p build  &&  cd build  &&  cmake $(CONFIG_FLAGS) -DCMAKE_CXX_FLAGS="$(GCC_FLAGS)" ..  &&  cmake --build . $(BUILD_FLAGS)
endif

clean: 
ifeq ($(OS), Windows_NT)
	rmdir /s /q build
else
	rm -rf build
endif
	
.PHONY: all debug release build clean run_d run_r