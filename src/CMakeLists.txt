include_directories(${CMAKE_BINARY_DIR})

set(ktp_filetransfer_handler_SRCS
    main.cpp
    filetransfer-handler.cpp
    telepathy-base-job.cpp
    handle-incoming-file-transfer-channel-job.cpp
    handle-outgoing-file-transfer-channel-job.cpp
    ktp-fth-debug.cpp
)

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

add_executable(ktp-filetransfer-handler ${ktp_filetransfer_handler_SRCS})

target_link_libraries(ktp-filetransfer-handler
            KTp::CommonInternals
            KF5::CoreAddons
            KF5::I18n
            KF5::KIOWidgets
            KF5::KIOFileWidgets
            KF5::ConfigCore
            Qt5::Core
            Qt5::DBus
            Qt5::Widgets
)

configure_file(org.freedesktop.Telepathy.Client.KTp.FileTransferHandler.service.in
        ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.FileTransferHandler.service)

install(TARGETS ktp-filetransfer-handler DESTINATION ${KDE_INSTALL_LIBEXECDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.FileTransferHandler.service
        DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
install(FILES KTp.FileTransferHandler.client DESTINATION ${KDE_INSTALL_DATAROOTDIR}/telepathy/clients/)

