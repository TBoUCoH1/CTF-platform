cmake_minimum_required(VERSION 3.16)
project(CTFApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Network
    REQUIRED
)

# Исходные файлы
set(SOURCES
    main.cpp
    CTFApp.cpp
    HintDialog.cpp
    ChallengeManager.cpp
)

set(HEADERS
    CTFApp.h
    HintDialog.h
    LeaderboardStyle.h
    ChallengeManager.h
)

# Создание приложения
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Линкование библиотек Qt
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# Инструкции по установке (опционально)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
# Куда устанавливать бинарник
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
