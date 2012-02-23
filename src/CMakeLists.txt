set (ktp_filetransfer_handler_SRCS
     main.cpp
     filetransfer-handler.cpp
     telepathy-base-job.cpp
     handle-incoming-file-transfer-channel-job.cpp
     handle-outgoing-file-transfer-channel-job.cpp
)

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

kde4_add_executable(ktp-filetransfer-handler ${ktp_filetransfer_handler_SRCS})

target_link_libraries(ktp-filetransfer-handler
            ${KDE4_KDECORE_LIBS}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KIO_LIBS}
            ${TELEPATHY_QT4_LIBRARIES}
            ${KTP_LIBRARIES}
)

configure_file(org.freedesktop.Telepathy.Client.KTp.FileTransferHandler.service.in
        ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.FileTransferHandler.service)

install(TARGETS ktp-filetransfer-handler DESTINATION ${LIBEXEC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KTp.FileTransferHandler.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES KTp.FileTransferHandler.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)

