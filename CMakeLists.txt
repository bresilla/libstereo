CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
PROJECT(libstereo)

FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(PCL REQUIRED)
INCLUDE_DIRECTORIES(${PCL_INCLUDE_DIRS})
LINK_DIRECTORIES(${PCL_LIBRARY_DIRS})
ADD_DEFINITIONS(${PCL_DEFINITIONS})

include_directories(/usr/local/include)
include_directories(./include)

SET(SOURCE_CALIB calib.cpp ./src/stereocalib.cpp ./src/stereorect.cpp)
SET(SOURCE_TEST test.cpp)
SET(SOURCE_MATCH match.cpp ./src/stereomatch.cpp)

ADD_EXECUTABLE(stereocalib ${SOURCE_CALIB})
TARGET_LINK_LIBRARIES(stereocalib ${OpenCV_LIBS})
ADD_EXECUTABLE(stereotest ${SOURCE_TEST})
TARGET_LINK_LIBRARIES(stereotest ${OpenCV_LIBS})
ADD_EXECUTABLE(stereomatch ${SOURCE_MATCH})
TARGET_LINK_LIBRARIES(stereomatch ${OpenCV_LIBS} ${PCL_LIBRARIES} -lpthread)

