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
#include <KCmdLineArgs>
#include <KDebug>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/FileTransferChannel>


int main(int argc, char* argv[])
{
    KAboutData aboutData("ktp-filetransfer-handler",
                         "ktp-filetransfer-handler",
                         ki18n("Telepathy File Transfer Handler"),
                         KTP_FILETRANSFER_HANDLER_VERSION,
                         ki18n("Handles your Telepathy file transfers"),
                         KAboutData::License_GPL_V2,
                         ki18n("Copyright (C) 2010, 2011, 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>"));
    aboutData.addAuthor(ki18n("Daniele E. Domenichelli"),
                        ki18n("Developer"),
                        "daniele.domenichelli@gmail.com",
                        "http://blogs.fsfe.org/drdanz/",
                        "drdanz");
    aboutData.addCredit(ki18n("Alin M Elena"), ki18n("Contributor"), "alinm.elena@gmail.com");
    aboutData.addCredit(ki18n("Dario Freddi"), ki18n("Contributor"), "dario.freddi@collabora.com");
    aboutData.addCredit(ki18n("David Edmundson"), ki18n("Contributor"), "kde@davidedmundson.co.uk");
    aboutData.addCredit(ki18n("George Kiagiadakis"), ki18n("Contributor"), "george.kiagiadakis@collabora.com");
    aboutData.addCredit(ki18n("Martin Klapetek"), ki18n("Contributor"), "martin.klapetek@gmail.com");
    aboutData.addCredit(ki18n("Andrea Scarpino"), ki18n("Contributor"), "andrea@archlinux.org");

    aboutData.setProductName("telepathy/filetransfer");
    aboutData.setTranslator(ki18nc("NAME OF TRANSLATORS", "Your names"),
                            ki18nc("EMAIL OF TRANSLATORS", "Your emails"));
    aboutData.setProgramIconName(QLatin1String("telepathy-kde"));
    aboutData.setHomepage("http://community.kde.org/KTp");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KTp::TelepathyHandlerApplication app;

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
        kDebug() << "File Transfer Handler already running. Exiting";
        return 1;
    }

    return app.exec();
}
