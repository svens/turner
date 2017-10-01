cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
cmake --build .
cmake --build . --target test -- ARGS=--output-on-failure
