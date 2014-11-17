/*
 * Copyright (C) 2010, 2011 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filetransfer-handler.h"
#include "version.h"

#include <KTp/telepathy-handler-application.h>

#include <KAboutData>
#include <KLocalizedString>

#include <QDebug>
#include <QIcon>

#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/FileTransferChannel>


int main(int argc, char* argv[])
{
    KAboutData aboutData("ktp-filetransfer-handler",
                         i18n("Telepathy File Transfer Handler"),
                         KTP_FILETRANSFER_HANDLER_VERSION,
                         i18n("Handles your Telepathy file transfers"),
                         KAboutLicense::GPL_V2,
                         i18n("Copyright (C) 2010, 2011, 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>"));
    aboutData.addAuthor(i18n("Daniele E. Domenichelli"),
                        i18n("Developer"),
                        "daniele.domenichelli@gmail.com",
                        "http://blogs.fsfe.org/drdanz/",
                        "drdanz");
    aboutData.addCredit(i18n("Alin M Elena"), i18n("Contributor"), "alinm.elena@gmail.com");
    aboutData.addCredit(i18n("Dario Freddi"), i18n("Contributor"), "dario.freddi@collabora.com");
    aboutData.addCredit(i18n("David Edmundson"), i18n("Contributor"), "kde@davidedmundson.co.uk");
    aboutData.addCredit(i18n("George Kiagiadakis"), i18n("Contributor"), "george.kiagiadakis@collabora.com");
    aboutData.addCredit(i18n("Martin Klapetek"), i18n("Contributor"), "martin.klapetek@gmail.com");
    aboutData.addCredit(i18n("Andrea Scarpino"), i18n("Contributor"), "andrea@archlinux.org");
    aboutData.addCredit(i18n("Dan Vrátil"), i18n("Contributor"), "dvratil@redhat.com");


    aboutData.setProductName("telepathy/filetransfer");
    aboutData.setHomepage("http://community.kde.org/KTp");

    KAboutData::setApplicationData(aboutData);

    // This is a very very very ugly hack that attempts to solve the following problem:
    // D-Bus service activated applications inherit the environment of dbus-daemon.
    // Normally, in KDE, startkde sets these environment variables. However, the session's
    // dbus-daemon is started before this happens, which means that dbus-daemon does NOT
    // have these variables in its environment and therefore all service-activated UIs
    // think that they are not running in KDE. This causes Qt not to load the KDE platform
    // plugin, which leaves the UI in a sorry state, using a completely wrong theme,
    // wrong colors, etc...
    // See also:
    // - https://bugs.kde.org/show_bug.cgi?id=269861
    // - https://bugs.kde.org/show_bug.cgi?id=267770
    // - https://git.reviewboard.kde.org/r/102194/
    // Here we are just going to assume that kde-telepathy is always used in KDE and
    // not anywhere else. This is probably the best that we can do.
    setenv("KDE_FULL_SESSION", "true", 0);
    setenv("KDE_SESSION_VERSION", "5", 0);
    KTp::TelepathyHandlerApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("telepathy-kde")));

    Tp::AccountFactoryPtr accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus());

    Tp::ConnectionFactoryPtr  connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus());

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);
    channelFactory->addFeaturesForIncomingFileTransfers(Tp::FileTransferChannel::FeatureCore);
    channelFactory->addFeaturesForOutgoingFileTransfers(Tp::FileTransferChannel::FeatureCore);

    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create();
    contactFactory->addFeature(Tp::Contact::FeatureAlias);

    Tp::ClientRegistrarPtr registrar = Tp::ClientRegistrar::create(accountFactory,
                                                                   connectionFactory,
                                                                   channelFactory,
                                                                   contactFactory);

    Tp::SharedPtr<FileTransferHandler> fth = Tp::SharedPtr<FileTransferHandler>(new FileTransferHandler(&app));
    if (!registrar->registerClient(Tp::AbstractClientPtr(fth), QLatin1String("KTp.FileTransferHandler"))) {
        qWarning() << "File Transfer Handler already running. Exiting";
        return 1;
    }

    return app.exec();
}
