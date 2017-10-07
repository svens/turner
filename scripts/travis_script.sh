if test "${BUILD_TYPE}" = "Coverity"; then
  cmake . -DTURNER_UNITTESTS=no -DTURNER_DOCS=no
  bash <(curl -s https://scan.coverity.com/scripts/travisci_build_coverity_scan.sh)
elif test "${BUILD_TYPE}" = "Coverage"; then
  cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DTURNER_DOCS=no
  cmake --build .
  travis_wait cmake --build . --target gen-cov -- ARGS=--output-on-failure
else
  cmake . -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DTURNER_DOCS=no
  cmake --build .
  cmake --build . --target test -- ARGS=--output-on-failure
fi
