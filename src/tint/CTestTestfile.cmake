# CMake generated Testfile for 
# Source directory: /Users/artaleee/nir/dawn/src/tint
# Build directory: /Users/artaleee/nir/dawn/src/tint
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tint_unittests "/Users/artaleee/nir/dawn/tint_unittests")
set_tests_properties(tint_unittests PROPERTIES  _BACKTRACE_TRIPLES "/Users/artaleee/nir/dawn/src/tint/CMakeLists.txt;701;add_test;/Users/artaleee/nir/dawn/src/tint/CMakeLists.txt;0;")
subdirs("fuzzers")
