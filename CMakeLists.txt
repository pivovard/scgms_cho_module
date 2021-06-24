CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT("SmartCGMS")

SET(PROJ "detection")
SET(CMAKE_CXX_STANDARD 17)
#set common dir
SET(SMARTCGMS_COMMON_DIR "../../common/")

FILE(GLOB SRC_FILES "src/*.cpp" "src/*.hpp" "src/*.c" "src/*.h")
FILE(GLOB SRC_ML_FILES "src/ML/*.cpp" "src/ML/*.hpp" "src/ML/*.h" "src/ML/*.def")
FILE(GLOB SRC_SKLEARN_FILES "src/ML/sklearn/*.cpp" "src/ML/sklearn/*.hpp" "src/ML/sklearn/*.h" "src/ML/sklearn/*.def")

IF(WIN32)
	FILE(GLOB SRC_WIN_FILES "src/win/*.cpp" "src/win/*.hpp" "src/win/*.h" "src/win/*.def")
	SET(SRC_FILES "${SRC_FILES};${SRC_WIN_FILES}")
ENDIF()

INCLUDE_DIRECTORIES("${SMARTCGMS_COMMON_DIR}")

SET(COMMON_FILES )
LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/iface/DeviceIface.cpp")

LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/rtl/referencedImpl.cpp")
LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/rtl/scgmsLib.cpp")
LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/rtl/Dynamic_Library.cpp")
LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/rtl/DeviceLib.cpp")
LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/rtl/FilterLib.cpp")
LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/rtl/UILib.cpp")
LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/rtl/hresult.c")

LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/lang/dstrings.cpp")

LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/utils/string_utils.cpp")
LIST(APPEND COMMON_FILES "${SMARTCGMS_COMMON_DIR}/utils/winapi_mapping.c")

SOURCE_GROUP("common" FILES ${COMMON_FILES})
SOURCE_GROUP("src" FILES ${SRC_FILES})
SOURCE_GROUP("ML" FILES ${SRC_ML_FILES})
SOURCE_GROUP("ML/sklearn" FILES ${SRC_SKLEARN_FILES})

ADD_LIBRARY(${PROJ} SHARED ${SRC_FILES};${COMMON_FILES};${SRC_ML_FILES};${SRC_SKLEARN_FILES})

#set include directories
include_directories("c:/Program Files (x86)/Visual Leak Detector/include/")
include_directories("../lib/frugally-deep/include/")
include_directories("../lib/eigen/")
include_directories("../lib/FunctionalPlus/include/")
include_directories("../lib/json/include/")