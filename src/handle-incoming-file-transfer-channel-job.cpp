/*
* Copyright (C) 2010, 2011 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "handle-incoming-file-transfer-channel-job.h"
#include "telepathy-base-job_p.h"

#include <QTimer>

#include <KLocalizedString>
#include <KDebug>
#include <KUrl>
#include <kio/renamedialog.h>

#include <TelepathyQt4/IncomingFileTransferChannel>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingOperation>

class HandleIncomingFileTransferChannelJobPrivate : public KTelepathy::TelepathyBaseJobPrivate
{
    Q_DECLARE_PUBLIC(HandleIncomingFileTransferChannelJob)

    public:
        HandleIncomingFileTransferChannelJobPrivate();
        virtual ~HandleIncomingFileTransferChannelJobPrivate();

        Tp::IncomingFileTransferChannelPtr channel;
        QString downloadDirectory;
        qulonglong offset;
        QFile* file;

        void __k__start();
        void __k__onSetUrlOperationFinished(Tp::PendingOperation* op);
        void __k__onFileTransferChannelStateChanged(Tp::FileTransferState state, Tp::FileTransferStateChangeReason reason);
        void __k__onFileTransferChannelTransferredBytesChanged(qulonglong count);
        void __k__onAcceptFileFinished(Tp::PendingOperation* op);
        void __k__onInvalidated();

        bool transferCompleted;
};

HandleIncomingFileTransferChannelJob::HandleIncomingFileTransferChannelJob(Tp::IncomingFileTransferChannelPtr channel,
                                                                           const QString downloadDirectory,
                                                                           QObject* parent)
    : TelepathyBaseJob(*new HandleIncomingFileTransferChannelJobPrivate(), parent)
{
    kDebug();
    Q_D(HandleIncomingFileTransferChannelJob);

    if (channel.isNull())
    {
        kError() << "Channel cannot be NULL";
        setError(KTelepathy::NullChannel);
        setErrorText(i18n("Invalid channel"));
    }

    Tp::Features features = Tp::Features() << Tp::FileTransferChannel::FeatureCore;
    if (!channel->isReady(Tp::Features() << Tp::FileTransferChannel::FeatureCore))
    {
        kError() << "Channel must be ready with Tp::FileTransferChannel::FeatureCore";
        setError(KTelepathy::FeatureNotReady);
        setErrorText(i18n("Channel is not ready"));
    }

    d->channel = channel;
    d->downloadDirectory = downloadDirectory;
}

HandleIncomingFileTransferChannelJob::~HandleIncomingFileTransferChannelJob()
{
    kDebug();
}

void HandleIncomingFileTransferChannelJob::start()
{
    kDebug();
    QTimer::singleShot(0, this, SLOT(__k__start()));
}

HandleIncomingFileTransferChannelJobPrivate::HandleIncomingFileTransferChannelJobPrivate()
    : file(0)
{
    kDebug();
}

HandleIncomingFileTransferChannelJobPrivate::~HandleIncomingFileTransferChannelJobPrivate()
{
    kDebug();
}

void HandleIncomingFileTransferChannelJobPrivate::__k__start()
{
    kDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (q->error()) {
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    q->connect(channel.data(), SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
               q, SLOT(__k__onInvalidated()));
    q->connect(channel.data(),
               SIGNAL(stateChanged(Tp::FileTransferState, Tp::FileTransferStateChangeReason)),
               SLOT(__k__onFileTransferChannelStateChanged(Tp::FileTransferState, Tp::FileTransferStateChangeReason)));
    q->connect(channel.data(),
               SIGNAL(transferredBytesChanged(qulonglong)),
               SLOT(__k__onFileTransferChannelTransferredBytesChanged(qulonglong)));

    KUrl url(downloadDirectory);
    url.addPath(channel->fileName());
    url.setScheme(QLatin1String("file"));
    kDebug() << "File name:" << url;

    QFileInfo fileInfo(url.toLocalFile());
    if (fileInfo.exists()) // TODO check if it is a dir?
    {
        KIO::RenameDialog renameDialog(0,
                                       i18n("Incoming file exists"),
                                       KUrl(), //TODO
                                       url,
                                       KIO::M_OVERWRITE,
                                       fileInfo.size(),
                                       channel->size(),
                                       fileInfo.created().toTime_t(),
                                       time_t(-1),
                                       fileInfo.lastModified().toTime_t(),
                                       channel->lastModificationTime().toTime_t());
        renameDialog.exec();
        switch (renameDialog.result())
        {
            case KIO::R_CANCEL:
                // TODO Cancel file transfer and close channel
                channel->cancel();
                QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
                return;
            case KIO::R_RENAME:
                url = renameDialog.newDestUrl();
                break;
            case KIO::R_OVERWRITE:
                break;
            default:
                kWarning() << "Unknown Error";
                q->setError(KTelepathy::KTelepathyError);
                q->setErrorText(i18n("Unknown Error"));
                QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        }
    }

    // TODO check if a .part file already exists ask user if file should be
    //      resumed and set offset accordingly
    offset = 0;

    file = new QFile(url.toLocalFile(), q->parent());
    kDebug() << "Saving file as" << file->fileName();

    Tp::PendingOperation* setUrlOperation = channel->setUri(url.url());
    q->connect(setUrlOperation,
               SIGNAL(finished(Tp::PendingOperation*)),
               SLOT(__k__onSetUrlOperationFinished(Tp::PendingOperation*)));
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onSetUrlOperationFinished(Tp::PendingOperation* op)
{
    kDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (op->isError()) {
        kWarning() << "Unable to set the URI -" <<
            op->errorName() << ":" << op->errorMessage();
        q->setError(KTelepathy::SetUriFailed);
        q->setErrorText(i18n("Unable to set the URI"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    Tp::PendingOperation* acceptFileOperation = channel->acceptFile(offset, file);
    q->connect(acceptFileOperation,
               SIGNAL(finished(Tp::PendingOperation*)),
               SLOT(__k__onAcceptFileFinished(Tp::PendingOperation*)));
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onFileTransferChannelStateChanged(Tp::FileTransferState state,
                                                                          Tp::FileTransferStateChangeReason stateReason)
{
    kDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    //TODO Handle other states
    kDebug() << "File transfer channel state changed to" << state << "with reason" << stateReason;
    transferCompleted = (state == Tp::FileTransferStateCompleted);
    if (transferCompleted) {
        kDebug() << "Transfer completed, saved at" << file->fileName();
        Q_EMIT q->infoMessage(q, i18n("Transfer completed."));

        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
    }
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onFileTransferChannelTransferredBytesChanged(qulonglong count)
{
    kDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    kDebug().nospace() << "Receiving " << channel->fileName() << " - "
                       << "transferred bytes" << " = " << count << " ("
                       << ((int) (((double) count / channel->size()) * 100)) << "% done)";
    Q_EMIT q->infoMessage(q, i18n("Transferred bytes"));
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onAcceptFileFinished(Tp::PendingOperation* op)
{
    // This method is called when the "acceptFile" operation is finished,
    // therefore the file was not received yet.
    kDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (op->isError()) {
        kWarning() << "Unable to accept file -" <<
            op->errorName() << ":" << op->errorMessage();
        q->setError(KTelepathy::AcceptFileError);
        q->setErrorText(i18n("Unable to accept file"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
    }
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onInvalidated()
{
    kDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    kWarning() << "File transfer invalidated!";
    Q_EMIT q->infoMessage(q, i18n("File transfer invalidated."));

    QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
}


#include "handle-incoming-file-transfer-channel-job.moc"
