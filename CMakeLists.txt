cmake_minimum_required(VERSION 3.21)

project(DialogG2Panel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Network
    SerialPort
    SerialBus
)

get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
execute_process(
    COMMAND "${QT_QMAKE_EXECUTABLE}" -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_INSTALL_PLUGINS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND "${QT_QMAKE_EXECUTABLE}" -query QT_INSTALL_BINS
    OUTPUT_VARIABLE QT_INSTALL_BINS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

add_library(dialog-g2-core STATIC
    src/engine/Adl200Meter.h
    src/engine/Adl200Meter.cpp
    src/engine/Amc16zFak24Meter.h
    src/engine/Amc16zFak24Meter.cpp
    src/engine/Asj60Ld16aMonitor.h
    src/engine/Asj60Ld16aMonitor.cpp
    src/engine/WhdTemperatureHumidityController.h
    src/engine/WhdTemperatureHumidityController.cpp
    src/engine/AppConfig.h
    src/engine/AppConfig.cpp
    src/engine/CabinetSnapshot.h
    src/engine/CabinetSnapshot.cpp
    src/engine/JbdBmsProtocol.h
    src/engine/JbdBmsProtocol.cpp
    src/engine/StateEngine.h
    src/engine/StateEngine.cpp
    src/engine/TestController.h
    src/engine/TestController.cpp
    src/engine/TestJournalStore.h
    src/engine/TestJournalStore.cpp
    src/engine/TestScheduleManager.h
    src/engine/TestScheduleManager.cpp
    src/engine/StateFileStore.h
    src/engine/StateFileStore.cpp
    src/engine/Logger.h
    src/engine/Logger.cpp
    src/engine/LineManager.h
    src/engine/LineManager.cpp
    src/engine/LineOperationalMonitor.h
    src/engine/LineOperationalMonitor.cpp
    src/engine/MaintenanceChecker.h
    src/engine/MaintenanceChecker.cpp
    src/engine/ManualEmergencyController.h
    src/engine/ManualEmergencyController.cpp
    src/engine/MeteringBusController.h
    src/engine/MeteringBusController.cpp
    src/engine/ModbusBusMonitor.h
    src/engine/ModbusBusMonitor.cpp
    src/engine/ModbusRtuCodec.h
    src/engine/ModbusRtuCodec.cpp
    src/engine/ModbusController.h
    src/engine/ModbusController.cpp
)

target_link_libraries(dialog-g2-core PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::SerialPort
    Qt6::SerialBus
)

target_include_directories(dialog-g2-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

qt_add_executable(dialog-g2-engine
    src/engine_main.cpp
    src/PasswordManager.h
)

target_link_libraries(dialog-g2-engine PRIVATE
    dialog-g2-core
)

qt_add_executable(test-controller-tests
    tests/test_controller_tests.cpp
)

target_link_libraries(test-controller-tests PRIVATE
    dialog-g2-core
)

add_custom_command(TARGET dialog-g2-engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:dialog-g2-engine>
        $<TARGET_FILE_DIR:dialog-g2-engine>
    COMMAND_EXPAND_LISTS
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:dialog-g2-engine>/config"
    COMMAND ${CMAKE_COMMAND}
        -D "SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/config/app.json"
        -D "DEST=$<TARGET_FILE_DIR:dialog-g2-engine>/config/app.json"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_if_missing.cmake"
    COMMAND ${CMAKE_COMMAND}
        -D "SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/config/lines.json"
        -D "DEST=$<TARGET_FILE_DIR:dialog-g2-engine>/config/lines.json"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_if_missing.cmake"
)

qt_add_executable(dialog-g2-hmi
    src/main.cpp
    src/PanelFacade.h
    src/PanelFacade.cpp
)

qt_add_qml_module(dialog-g2-hmi
    URI DialogG2
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/pages/StartPage.qml
        qml/pages/PasswordPage.qml
        qml/pages/TestPage.qml
        qml/pages/SettingsPage.qml
        qml/pages/LineSettingsPage.qml
        qml/pages/PasswordChangePage.qml
        qml/pages/DateTimePage.qml
        qml/pages/SchedulePage.qml
        qml/pages/TestSettingsPage.qml
        qml/pages/JournalPage.qml
        qml/pages/SystemPage.qml
        qml/pages/LinesPage.qml
        qml/pages/BatteryPage.qml
    RESOURCES
        qml/assets/light-tech-logo.png
        qml/assets/Back.png
)

target_link_libraries(dialog-g2-hmi PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
)

add_custom_command(TARGET dialog-g2-hmi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:dialog-g2-hmi>
        $<TARGET_FILE_DIR:dialog-g2-hmi>
    COMMAND_EXPAND_LISTS
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:dialog-g2-hmi>/platforms"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_INSTALL_PLUGINS}/platforms/$<IF:$<CONFIG:Debug>,qwindowsd.dll,qwindows.dll>"
        "$<TARGET_FILE_DIR:dialog-g2-hmi>/platforms"
)

if (WIN32)
    add_custom_command(TARGET dialog-g2-hmi POST_BUILD
        COMMAND "${QT_INSTALL_BINS}/windeployqt.exe"
            --no-translations
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
            "$<TARGET_FILE:dialog-g2-hmi>"
    )
endif()

if (WIN32)
    set_target_properties(dialog-g2-hmi PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
