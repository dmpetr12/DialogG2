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
    SerialPort
    SerialBus
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
    Qt6::SerialPort
    Qt6::SerialBus
)

target_include_directories(dialog-g2-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

qt_add_executable(dialog-g2-engine
    src/engine_main.cpp
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
    src/CabinetState.h
    src/CabinetState.cpp
)

qt_add_qml_module(dialog-g2-hmi
    URI DialogG2
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(dialog-g2-hmi PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

if (WIN32)
    set_target_properties(dialog-g2-hmi PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
