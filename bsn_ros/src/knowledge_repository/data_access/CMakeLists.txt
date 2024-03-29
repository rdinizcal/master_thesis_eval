CMAKE_MINIMUM_REQUIRED (VERSION 2.8.3)
PROJECT(repository)

add_compile_options(-std=c++11)

###########################################################################
## Find catkin and any catkin packages
FIND_PACKAGE(catkin REQUIRED COMPONENTS roscpp std_msgs genmsg roslib messages archlib)

###########################################################################
# Export catkin package.
CATKIN_PACKAGE(
    INCLUDE_DIRS include
    LIBRARIES ${PROJECT_NAME}
    CATKIN_DEPENDS messages message_runtime archlib
)

###########################################################################
# Set catkin directory.
INCLUDE_DIRECTORIES(${catkin_INCLUDE_DIRS})

# Set header files from BSN.
SET (BSN_INCLUDE_DIRS /usr/local/include/bsn)
INCLUDE_DIRECTORIES (${BSN_INCLUDE_DIRS})

# Set include directory.
INCLUDE_DIRECTORIES(include)

###########################################################################
# Add and Set linking libraries to successfully link test suites and binaries.
FIND_LIBRARY(BSN_LIBRARY bsn)
SET (LIBRARIES ${BSN_LIBRARY})

###########################################################################
# Build this project.
FILE(GLOB ${PROJECT_NAME}-src "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

SET(data_access-src "${CMAKE_CURRENT_SOURCE_DIR}/src/DataAccess.cpp")
ADD_EXECUTABLE (data_access  "${CMAKE_CURRENT_SOURCE_DIR}/apps/data_access.cpp" ${${PROJECT_NAME}-src} ${data_access-src})
TARGET_LINK_LIBRARIES (data_access ${catkin_LIBRARIES} ${LIBRARIES})
ADD_DEPENDENCIES(data_access messages_generate_messages_cpp)