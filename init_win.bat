@ECHO OFF
IF NOT EXIST build mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" -T "LLVM-vs2014" ..
cd ..