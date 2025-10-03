# CMake generated Testfile for 
# Source directory: D:/Repos/dawproject_c/05-implementation/tests/unit
# Build directory: D:/Repos/dawproject_c/05-implementation/build_tdd
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[PlatformTests]=] "D:/Repos/dawproject_c/05-implementation/build_tdd/Debug/test_platform_red.exe")
  set_tests_properties([=[PlatformTests]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/Repos/dawproject_c/05-implementation/tests/unit/CMakeLists.txt;42;add_test;D:/Repos/dawproject_c/05-implementation/tests/unit/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[PlatformTests]=] "D:/Repos/dawproject_c/05-implementation/build_tdd/Release/test_platform_red.exe")
  set_tests_properties([=[PlatformTests]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/Repos/dawproject_c/05-implementation/tests/unit/CMakeLists.txt;42;add_test;D:/Repos/dawproject_c/05-implementation/tests/unit/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[PlatformTests]=] "D:/Repos/dawproject_c/05-implementation/build_tdd/MinSizeRel/test_platform_red.exe")
  set_tests_properties([=[PlatformTests]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/Repos/dawproject_c/05-implementation/tests/unit/CMakeLists.txt;42;add_test;D:/Repos/dawproject_c/05-implementation/tests/unit/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[PlatformTests]=] "D:/Repos/dawproject_c/05-implementation/build_tdd/RelWithDebInfo/test_platform_red.exe")
  set_tests_properties([=[PlatformTests]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/Repos/dawproject_c/05-implementation/tests/unit/CMakeLists.txt;42;add_test;D:/Repos/dawproject_c/05-implementation/tests/unit/CMakeLists.txt;0;")
else()
  add_test([=[PlatformTests]=] NOT_AVAILABLE)
endif()
subdirs("_deps/catch2-build")
