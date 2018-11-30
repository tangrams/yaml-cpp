cmake_minimum_required(VERSION 3.5)

#add_sources(parse.cpp)
add_executable(parse parse.cpp)
target_link_libraries(parse yaml-cpp)

#add_sources(sandbox.cpp)
add_executable(sandbox sandbox.cpp)
target_link_libraries(sandbox yaml-cpp)

#add_sources(read.cpp)
add_executable(read read.cpp)
target_link_libraries(read yaml-cpp)

if(YAML_CPP_BUILD_PROTOBUF)
  if(YAML_CPP_USE_GOOGLE_PROTOBUF)
    add_executable(gprotodump gprotodump.cpp)
    target_link_libraries(gprotodump yaml-cpp)

    add_executable(gprotoload gprotoload.cpp)
    target_link_libraries(gprotoload yaml-cpp)
  endif()

  add_executable(protodump protodump.cpp)
  target_link_libraries(protodump yaml-cpp)

  add_executable(protoload protoload.cpp)
  target_link_libraries(protoload yaml-cpp)
endif()
