cmake_minimum_required(VERSION 3.16)

project(InvoiceApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Sql PrintSupport)

# Try to find Qt Pdf optionally
find_package(Qt6 6.5 COMPONENTS Pdf QUIET)

set(SOURCES
    main.cpp
    core/database.cpp
    core/sessionmanager.cpp
    services/authservice.cpp
    services/vendorservice.cpp
    services/invoiceservice.cpp
    services/settingsservice.cpp
    ui/loginwindow.cpp
    ui/mainwindow.cpp
    models/invoiceitemmodel.cpp
)

set(HEADERS
    core/database.h
    core/sessionmanager.h
    services/authservice.h
    services/vendorservice.h
    services/invoiceservice.h
    services/settingsservice.h
    ui/loginwindow.h
    ui/mainwindow.h
    models/user.h
    models/vendor.h
    models/invoice.h
    models/invoiceitem.h
    models/invoiceitemmodel.h
)

qt_add_executable(InvoiceApp
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(InvoiceApp PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

if (TARGET Qt6::Pdf)
    target_link_libraries(InvoiceApp PRIVATE Qt6::Pdf)
    target_compile_definitions(InvoiceApp PRIVATE HAVE_QTPDF=1)
endif()

target_include_directories(InvoiceApp PRIVATE
    ${CMAKE_SOURCE_DIR}
)
