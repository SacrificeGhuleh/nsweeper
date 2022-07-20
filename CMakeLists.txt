cmake_minimum_required (VERSION 3.10)
project(nsweeper VERSION 1.0.0)

option(IMTUI_ALL_WARNINGS            "imtui: enable all compiler warnings" ON)
option(IMTUI_ALL_WARNINGS_3RD_PARTY  "imtui: enable all compiler warnings in 3rd party libs" OFF)

option(IMTUI_SANITIZE_THREAD         "imtui: enable thread sanitizer" OFF)
option(IMTUI_SANITIZE_ADDRESS        "imtui: enable address sanitizer" OFF)
option(IMTUI_SANITIZE_UNDEFINED      "imtui: enable undefined sanitizer" OFF)

option(IMTUI_SUPPORT_NCURSES         "imtui: support for libncurses" ON)
option(IMTUI_SUPPORT_CURL            "imtui: support for libcurl" OFF)

option(IMTUI_BUILD_EXAMPLES          "imtui: build examples" OFF)

add_subdirectory(lib)

add_subdirectory(src)