include(FetchContent)
FetchContent_Declare(
  yrshell
  GIT_REPOSITORY https://github.com/brymatpc/YRShell.git
  GIT_TAG        origin/20250404_Cmake
)

FetchContent_MakeAvailable(yrshell)