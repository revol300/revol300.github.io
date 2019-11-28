---
layout: post
title:  "googletest_2"
date:   2019-11-28
comments: true
categories: cpp
---
### Build googletest with CMake

#### Standalone CMake Project

googletest를 standalone project로 만든다면 보통 다음과 같다.

```bash
mkdir mybuild       # Create a directory to hold the build output.
cd mybuild
cmake ${GTEST_DIR}  # Generate native build scripts.
```

googletest 샘플도 같이 빌드 하고 싶으면 마지막 줄을 다음과 같이 변경하자

```bash
cmake -Dgtest_build_samples=ON ${GTEST_DIR}
```

 \*nix 시스템 사용자라면, Makefile이 잘 생성되었을 것이다. make 를 이용해 gtest를 빌드하자

윈도우를 사용한다거나 비주얼 스튜디오가 깔려 있으면 `gtest.sln` 파일 과 `.vcproj` 파일 몇개가 만들어졌을 것이다. 비주열 스튜디오를 이용해서 이를 빌드하자.
맥의 Xcode를 사용했다면 `.xcodeproj` 파일이 만들어졌을 것이다.

#### Incorporating Into An Existing CMake Project

CMake를 이미 사용하고 있는 프로젝트에서는 gtest를 해당 프로젝트의 일부로 넣는 것이 좋은 방법이다.
CMake 의 add_subdirectory() 명령을 통해 googletest를 link하고 googletest 소스 코드를 main build에서 사용 가능하도록 만들면된다. googletest의 소스코드를 main에서 사용가능하게 하는 방법은 다음과 같다.

1. googletest를 정해진 위치에 다운. 가장 덜 유연하고 continuous integration 시스템에서 사용하기 어려움. 

2. googletest 소스코드를 project 소스에 직접 넣는다. 간편한 방법이지만, 최신 상태로 유지하기가 어렵다. 몇몇 조직에서는 이 방법을 허용하지 않을 수 있다.

3. googletest를 git submodule로 추가한다. 2번과 비슷하지만 최신상태로 유지하기는 좀더 간편해보인다.

4. CMake를 사용하여 빌드 구성 단계의 일부로 GoogleTest를 다운받는다. 설정이 좀 복잡하지만 업데이트도 잘되고 유연하다.

별개의 파일(e.g. `CMakeLists.txt.in`)에 간단한 Cmake 코드 몇줄로 4번을 해볼 수 있다.
이 파일에 작성된 Cmake 코드가 cmake 단계에서 파일을 복사하고 sub-build를 수행한다.
그리고 이렇게 만들어진 directory를 main build에서 add_subdirectory()를 이용해서 사용할 수 있다.

코드를 통해 좀더 구체적인 예시를 들어 보면,

New file `CMakeLists.txt.in`:

```cmake
cmake_minimum_required(VERSION 2.8.2)

project(googletest-download NONE)

include(ExternalProject)
  ExternalProject_Add(googletest
      GIT_REPOSITORY    https://github.com/google/googletest.git
      GIT_TAG           master
      SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-src"
      BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
      TEST_COMMAND      ""
      )
```

Existing build's `CMakeLists.txt`:

```cmake
# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
      ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
      EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Now simply link against gtest or gtest_main as needed. Eg
  add_executable(example example.cpp)
  target_link_libraries(example gtest_main)
add_test(NAME example_test COMMAND example)
```

위 코드는 ExternalProject_Add()를 사용하므로 CMake 2.8.2 혹은 그 이상의 버전을 요구한다.
이 방법에 대해 좀더 자세히 알고 싶다면 다음을 참조하자. 일반적인 빌드 방법에 대한 정보가 있다.
[cmake-gtest](http://crascit.com/2015/07/25/cmake-gtest/)
