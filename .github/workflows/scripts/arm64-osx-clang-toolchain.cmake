set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm@18/bin/clang++)
set(CMAKE_CXX_FLAGS "-nostdinc++ -nostdlib++ -isystem /opt/homebrew/opt/llvm@18/include/c++/v1")
set(CMAKE_EXE_LINKER_FLAGS "-L /opt/homebrew/opt/llvm@18/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm@18/lib/c++ -lc++")