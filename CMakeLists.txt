cmake_minimum_required(VERSION 4.2)
project(8inf259_tp3)

set(CMAKE_CXX_STANDARD 20)

add_executable(8inf259_tp3 main.cpp
        include/library/json.hpp
        Device.h
        Machine.h
        Scheduler.h
        Team.h
        include/factories/DeviceFactory.h
        source/factories/DeviceFactory.cpp
        source/Device.cpp)
