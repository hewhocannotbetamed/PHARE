
cmake_minimum_required(VERSION 3.0)
project(phare-exe)



add_executable(phare-exe ${SOURCES_INC} phare.cpp)
target_compile_options(phare-exe PRIVATE ${PHARE_WERROR_FLAGS})
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)


target_link_libraries(phare-exe PUBLIC
    phare_simulator
    pybind11::embed
)

