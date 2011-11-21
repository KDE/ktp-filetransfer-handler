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

#include <KTelepathy/telepathy-handler-application.h>

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/FileTransferChannel>


int main(int argc, char* argv[])
{
    KAboutData aboutData("telepathy-kde-filetransfer-handler",
                         "telepathy-filetransfer-handler",
                         ki18n("Telepathy File Transfer Handler"),
                         "0.2.60");
    aboutData.addAuthor(ki18n("Daniele E. Domenichelli"), ki18n("Developer"), "daniele.domenichelli@gmail.com");
    aboutData.setProductName("telepathy/filetransfer");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KTelepathy::TelepathyHandlerApplication app;

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
    if(!registrar->registerClient(Tp::AbstractClientPtr(fth), QLatin1String("KDE.FileTransferHandler"))) {
        kDebug() << "File Transfer Handler already running. Exiting";
        return 1;
    }

    return app.exec();
}
