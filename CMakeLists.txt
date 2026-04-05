cmake_minimum_required(VERSION 4.2)
project(8inf259_tp3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc-13)
set(CMAKE_CXX_COMPILER g++-13)

add_executable(8inf259_tp3 main.cpp
        include/library/json.hpp
        Device.h
        Machine.h
        Scheduler.h
        Team.h
        include/factories/DeviceFactory.h
        source/factories/DeviceFactory.cpp
        source/Device.cpp
        source/factories/MachineFactory.cpp
        include/factories/MachineFactory.h
        source/Machine.cpp
        source/factories/TeamFactory.cpp
        include/factories/TeamFactory.h
        source/Team.cpp
        source/Scheduler.cpp
        include/library/string.h
        source/ds/DependencyGraph.cpp
        include/ds/DependencyGraph.h)
