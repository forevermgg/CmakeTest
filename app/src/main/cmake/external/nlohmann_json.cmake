include(FetchContent)
find_package(Git REQUIRED)
FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.7.3
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(json)
FetchContent_GetProperties(json SOURCE_DIR JSON_INCLUDE_DIR)
include_directories(${JSON_INCLUDE_DIR}/include/)
message("JSON_INCLUDE_DIR: ${JSON_INCLUDE_DIR}")
