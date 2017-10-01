case "${TRAVIS_OS_NAME}-${CC}" in
  linux-clang)
    export CC=clang-5.0
    export CXX=clang++-5.0
    ;;

  linux-gcc)
    export CC=gcc-6
    export CXX=g++-6
    export GCOV=gcov-6
    ;;

  osx-clang)
    export CC=clang
    export CXX=clang++
    brew update
    brew install cmake || true
    ;;

  osx-gcc)
    export CC=gcc-7
    export CXX=g++-7
    brew cask uninstall oclint || true
    brew update
    brew install cmake gcc || true
    ;;
esac
