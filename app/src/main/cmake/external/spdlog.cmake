include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    message("CMAKE VERSION_LESS 3.14")
endif()

set(SPDLOG_GIT_TAG  v1.11.0)  # 指定版本
set(SPDLOG_GIT_URL  https://github.com/gabime/spdlog.git)  # 指定git仓库地址

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY    ${SPDLOG_GIT_URL}
        GIT_TAG           ${SPDLOG_GIT_TAG}
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(spdlog)
FetchContent_GetProperties(spdlog SOURCE_DIR SPDLOG_INCLUDE_DIR)
include_directories(${SPDLOG_INCLUDE_DIR}/spdlog/include)
message("SPDLOG_INCLUDE_DIR: ${SPDLOG_INCLUDE_DIR}")
