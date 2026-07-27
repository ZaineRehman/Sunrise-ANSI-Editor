CONFIG_FLAGS := 
GCC_FLAGS := 
BUILD_FLAGS := --parallel 4


# force GCC on windows
ifeq ($(OS), Windows_NT)
	CONFIG_FLAGS += -G "MinGW Makefiles"
endif


all: debug


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


debug: CONFIG_FLAGS += -DCMAKE_BUILD_TYPE=Debug
debug: GCC_FLAGS += -g -Wall -Wextra -Wpedantic -march=native
debug: build

release: CONFIG_FLAGS += -DCMAKE_BUILD_TYPE=Release
release: BUILD_FLAGS += --config Release
release: GCC_FLAGS += -O3 -march=x86-64 -DNDEBUG -s -flto -ffast-math -static-libgcc -static-libstdc++  # TODO: check on these last 2
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


package: 
	cd build && cpack


clean_all: clean
clean_all: clean_sessions
clean_all: clean_exports

clean: 
ifeq ($(OS), Windows_NT)
	rmdir /s /q build
else
	rm -rf build
endif

clean_sessions: 
ifeq ($(OS), Windows_NT)
	rmdir /s /q Sessions
else
	rm -rf Sessions
endif

clean_exports: 
ifeq ($(OS), Windows_NT)
	rmdir /s /q Export
else
	rm -rf Export
endif


get_lines: 
ifeq ($(OS), Windows_NT)
	git ls-files | ForEach-Object { Get-Content $$_ } | Measure-Object -Line
else
	git ls-files | xargs wc -l
endif


	
.PHONY: all debug release build clean run_d run_r clean_sessions clean_exports clean_all get_lines