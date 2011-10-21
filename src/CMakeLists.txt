set (telepathy_kde_common_internals_SRCS
     ../common/telepathy-handler-application.cpp
)

set (telepathy_kde_filetransfer_handler_SRCS
     main.cpp
     filetransfer-handler.cpp
     telepathy-base-job.cpp
     handle-incoming-file-transfer-channel-job.cpp
     handle-outgoing-file-transfer-channel-job.cpp
     ${telepathy_kde_common_internals_SRCS}
)

kde4_add_executable(telepathy-kde-filetransfer-handler ${telepathy_kde_filetransfer_handler_SRCS})

target_link_libraries(telepathy-kde-filetransfer-handler
            ${KDE4_KDECORE_LIBS}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KIO_LIBS}
            ${TELEPATHY_QT4_LIBRARIES}
)

configure_file(org.freedesktop.Telepathy.Client.KDE.FileTransferHandler.service.in
        ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KDE.FileTransferHandler.service)

install(TARGETS telepathy-kde-filetransfer-handler DESTINATION ${LIBEXEC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.KDE.FileTransferHandler.service
        DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
install(FILES KDE.FileTransferHandler.client DESTINATION ${SHARE_INSTALL_PREFIX}/telepathy/clients/)

