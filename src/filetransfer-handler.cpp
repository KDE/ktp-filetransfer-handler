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
#include "handle-incoming-file-transfer-channel-job.h"
#include "handle-outgoing-file-transfer-channel-job.h"

#include <TelepathyQt4/ChannelClassSpecList>
#include <TelepathyQt4/IncomingFileTransferChannel>
#include <TelepathyQt4/OutgoingFileTransferChannel>

#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KJob>
// #include <KNotification>
#include <kio/global.h>
#include <kjobtrackerinterface.h>
#include <KDebug>

#include <QApplication>


FileTransferHandler::FileTransferHandler(QObject *parent)
    : QObject(parent),
      Tp::AbstractClientHandler(Tp::ChannelClassSpecList() << Tp::ChannelClassSpec::incomingFileTransfer()
                                                           << Tp::ChannelClassSpec::outgoingFileTransfer())
{
}


FileTransferHandler::~FileTransferHandler()
{
}

bool FileTransferHandler::bypassApproval() const
{
    return false;
}

void FileTransferHandler::handleChannels(const Tp::MethodInvocationContextPtr<> &context,
                                         const Tp::AccountPtr &account,
                                         const Tp::ConnectionPtr &connection,
                                         const QList<Tp::ChannelPtr> &channels,
                                         const QList<Tp::ChannelRequestPtr> &requestsSatisfied,
                                         const QDateTime &userActionTime,
                                         const Tp::AbstractClientHandler::HandlerInfo &handlerInfo)
{
    Q_UNUSED(account);
    Q_UNUSED(connection);
    Q_UNUSED(requestsSatisfied);
    Q_UNUSED(userActionTime);
    Q_UNUSED(handlerInfo);

    foreach(const Tp::ChannelPtr &channel, channels) {
        kDebug() << "Handling new file transfer";


        KJob* job = NULL;
        if (Tp::IncomingFileTransferChannelPtr incomingFileTransferChannel = Tp::IncomingFileTransferChannelPtr::dynamicCast(channel)) {
            m_jobCount.fetchAndAddOrdered(1);
            context->setFinished();

            kDebug() << "Incoming File Transfer:";
            kDebug() << channel->immutableProperties();

            KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("ktelepathyrc"));
            KConfigGroup filetransferConfig = config->group(QLatin1String("File Transfers"));

            QString downloadDirectory = filetransferConfig.readPathEntry(QLatin1String("downloadDirectory"),
                    QDir::homePath() + QLatin1String("/") + i18nc("This is the download directory in user's home", "Downloads"));
            kDebug() << "Download directory:" << downloadDirectory;
            // TODO Check if directory exists

            job = new HandleIncomingFileTransferChannelJob(incomingFileTransferChannel, downloadDirectory, this);
        } else if (Tp::OutgoingFileTransferChannelPtr outgoingFileTransferChannel = Tp::OutgoingFileTransferChannelPtr::dynamicCast(channel)) {
            if (outgoingFileTransferChannel->uri().isEmpty())
            {
                context->setFinishedWithError(QLatin1String(TELEPATHY_QT4_ERROR_INCONSISTENT),
                                              QLatin1String("Cannot handle outgoing file transfer without URI"));
            }
            m_jobCount.fetchAndAddOrdered(1);
            context->setFinished();

            kDebug() << "Outgoing File Transfer:";
            kDebug() << channel->immutableProperties();

            job = new HandleOutgoingFileTransferChannelJob(outgoingFileTransferChannel, this);
        } else {
            context->setFinishedWithError(QLatin1String(TELEPATHY_QT4_ERROR_INCONSISTENT),
                                          QLatin1String("Unknown channel type"));
            kDebug() << "If you are reading this, then telepathy is broken";
        }

        if (job) {
            KIO::getJobTracker()->registerJob(job);
            connect(job,
                    SIGNAL(infoMessage(KJob*, QString, QString)),
                    SLOT(onInfoMessage(KJob*, QString, QString)));
            connect(job,
                    SIGNAL(result(KJob*)),
                    SLOT  (handleResult(KJob*)));
            job->start();
        }
    }
}

void FileTransferHandler::onInfoMessage(KJob* job, const QString &plain, const QString &rich)
{
    Q_UNUSED(job);
    Q_UNUSED(rich);
    kDebug() << plain;

//    KNotification::event("newOfferFileTransferEvent", plain, rich);
}

void FileTransferHandler::handleResult(KJob* job)
{
    kDebug();
    if ( job->error() ) {
        kWarning() << job->errorString();
        // TODO do something;
    }

    QTimer::singleShot(2000, this, SLOT(onTimeout()));
}
