if test "${BUILD_TYPE}" = "Coverity"; then
  cmake . -Dturner_unittests=no
  bash <(curl -s https://scan.coverity.com/scripts/travisci_build_coverity_scan.sh)
elif test "${BUILD_TYPE}" = "Coverage"; then
  cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -Dturner_unittests=yes
  cmake --build .
  travis_wait cmake --build . --target turner-cov -- ARGS=--output-on-failure
else
  cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -Dturner_unittests=yes
  cmake --build .
  cmake --build . --target test -- ARGS=--output-on-failure
fi
