cmake -H. -Bbuild
cmake --build build
cd build
ctest -C debug -VV -R NativeTests
cd ..