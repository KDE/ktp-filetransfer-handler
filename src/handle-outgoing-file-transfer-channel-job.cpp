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


#include "handle-outgoing-file-transfer-channel-job.h"
#include "telepathy-base-job_p.h"

#include <QTimer>

#include <KLocalizedString>
#include <KDebug>
#include <KUrl>

#include <TelepathyQt4/OutgoingFileTransferChannel>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingOperation>

class HandleOutgoingFileTransferChannelJobPrivate : public KTelepathy::TelepathyBaseJobPrivate
{
    Q_DECLARE_PUBLIC(HandleOutgoingFileTransferChannelJob)

    public:
        HandleOutgoingFileTransferChannelJobPrivate();
        virtual ~HandleOutgoingFileTransferChannelJobPrivate();

        Tp::OutgoingFileTransferChannelPtr channel;
        QFile* file;

        void __k__start();
        void __k__onFileTransferChannelStateChanged(Tp::FileTransferState state, Tp::FileTransferStateChangeReason reason);
        void __k__provideFile();
        void __k__onFileTransferChannelTransferredBytesChanged(qulonglong count);
        void __k__onProvideFileFinished(Tp::PendingOperation* op);
        void __k__onInvalidated();
};

HandleOutgoingFileTransferChannelJob::HandleOutgoingFileTransferChannelJob(Tp::OutgoingFileTransferChannelPtr channel,
                                                                           QObject* parent)
    : TelepathyBaseJob(*new HandleOutgoingFileTransferChannelJobPrivate(), parent)
{
    kDebug();
    Q_D(HandleOutgoingFileTransferChannelJob);

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

    connect(channel.data(),
            SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
            SLOT(__k__onInvalidated()));

    d->channel = channel;
}

HandleOutgoingFileTransferChannelJob::~HandleOutgoingFileTransferChannelJob()
{
    kDebug();
}

void HandleOutgoingFileTransferChannelJob::start()
{
    kDebug();
    QTimer::singleShot(0, this, SLOT(__k__start()));
}

HandleOutgoingFileTransferChannelJobPrivate::HandleOutgoingFileTransferChannelJobPrivate()
    : file(0)
{
    kDebug();
}

HandleOutgoingFileTransferChannelJobPrivate::~HandleOutgoingFileTransferChannelJobPrivate()
{
    kDebug();
}

void HandleOutgoingFileTransferChannelJobPrivate::__k__start()
{
    kDebug();
    Q_Q(HandleOutgoingFileTransferChannelJob);

    if (q->error()) {
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    q->connect(channel.data(),
               SIGNAL(stateChanged(Tp::FileTransferState, Tp::FileTransferStateChangeReason)),
               SLOT(__k__onFileTransferChannelStateChanged(Tp::FileTransferState, Tp::FileTransferStateChangeReason)));
    q->connect(channel.data(),
               SIGNAL(transferredBytesChanged(qulonglong)),
               SLOT(__k__onFileTransferChannelTransferredBytesChanged(qulonglong)));

    if (channel->state() == Tp::FileTransferStateAccepted) {
        QTimer::singleShot(0, q, SLOT(__k__provideFile()));
    }
}

void HandleOutgoingFileTransferChannelJobPrivate::__k__onFileTransferChannelStateChanged(Tp::FileTransferState state,
                                                                                         Tp::FileTransferStateChangeReason stateReason)
{
    kDebug();
    Q_Q(HandleOutgoingFileTransferChannelJob);

    kDebug() << "File transfer channel state changed to" << state << "with reason" << stateReason;

    switch (state)
    {
        case Tp::FileTransferStateNone:
            // This is bad
            kWarning() << "An error occurred.";
            q->setError(KTelepathy::TelepathyErrorError);
            q->setErrorText(i18n("An error occurred"));
            QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        case Tp::FileTransferStateCompleted:
            kDebug() << "Transfer completed";
            Q_EMIT q->infoMessage(q, i18n("Transfer completed"));
            QTimer::singleShot(0, q, SLOT(__k__doEmitResult())); //TODO here?
            break;
        case Tp::FileTransferStateCancelled:
            kWarning() << "Transfer was canceled.";
            q->setError(KTelepathy::FileTransferCancelled); //TODO
            q->setErrorText(i18n("Transfer was canceled."));
            QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
            break;
        case Tp::FileTransferStateAccepted:
            QTimer::singleShot(0, q, SLOT(__k__provideFile()));
            break;
        case Tp::FileTransferStatePending:
        case Tp::FileTransferStateOpen:
        default:
            break;
    }
}

void HandleOutgoingFileTransferChannelJobPrivate::__k__provideFile()
{
    kDebug();
    Q_Q(HandleOutgoingFileTransferChannelJob);
    KUrl uri(channel->uri());
    if (uri.isEmpty())
    {
        qWarning() << "URI property missing";
        q->setError(KTelepathy::UriPropertyMissing);
        q->setErrorText(i18n("URI property is missing"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
    }
    if (uri.scheme() != QLatin1String("file"))
    {
        // TODO handle this!
        qWarning() << "Not a local file";
        q->setError(KTelepathy::NotALocalFile);
        q->setErrorText(i18n("This is not a local file"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
    }

    file = new QFile(uri.toLocalFile(), q->parent());
    kDebug() << "Providing file" << file->fileName();

    Tp::PendingOperation* provideFileOperation = channel->provideFile(file);
    q->connect(provideFileOperation, SIGNAL(finished(Tp::PendingOperation*)),
               q, SLOT(__k__onProvideFileFinished(Tp::PendingOperation*)));
}

void HandleOutgoingFileTransferChannelJobPrivate::__k__onFileTransferChannelTransferredBytesChanged(qulonglong count)
{
    kDebug();
    Q_Q(HandleOutgoingFileTransferChannelJob);

    kDebug().nospace() << "Sending " << channel->fileName() << " - "
                       << "Transferred bytes = " << count << " ("
                       << ((int) (((double) count / channel->size()) * 100)) << "% done)";
    Q_EMIT q->infoMessage(q, i18n("Transferred bytes"));
}

void HandleOutgoingFileTransferChannelJobPrivate::__k__onProvideFileFinished(Tp::PendingOperation* op)
{
    // This method is called when the "provideFile" operation is finished,
    // therefore the file was not sent yet.
    kDebug();
    Q_Q(HandleOutgoingFileTransferChannelJob);

    if (op->isError()) {
        kWarning() << "Unable to provide file - " <<
            op->errorName() << ":" << op->errorMessage();
        q->setError(KTelepathy::ProvideFileError);
        q->setErrorText(i18n("Cannot provide file"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
    }
}

void HandleOutgoingFileTransferChannelJobPrivate::__k__onInvalidated()
{
    kDebug();
    Q_Q(HandleOutgoingFileTransferChannelJob);

    kWarning() << "File transfer invalidated!";
    Q_EMIT q->infoMessage(q, i18n("File transfer invalidated."));

    QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
}


#include "handle-outgoing-file-transfer-channel-job.moc"
