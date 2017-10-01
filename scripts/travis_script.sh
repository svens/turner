if test "${BUILD_TYPE}" = "Coverage"; then
  cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  cmake --build .
  travis_wait cmake --build . --target gen-cov -- ARGS=--output-on-failure
else
  cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  cmake --build .
  cmake --build . --target test -- ARGS=--output-on-failure
fi
