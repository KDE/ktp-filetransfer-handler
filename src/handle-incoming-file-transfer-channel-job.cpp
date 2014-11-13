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
#include <QUrl>
#include <QPointer>
#include <QDebug>
#include <QFileDialog>

#include <KLocalizedString>
#include <kio/renamedialog.h>
#include <kio/global.h>
#include <kjobtrackerinterface.h>

#include <TelepathyQt/IncomingFileTransferChannel>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/Contact>


class HandleIncomingFileTransferChannelJobPrivate : public KTp::TelepathyBaseJobPrivate
{
    Q_DECLARE_PUBLIC(HandleIncomingFileTransferChannelJob)

public:
    HandleIncomingFileTransferChannelJobPrivate();
    virtual ~HandleIncomingFileTransferChannelJobPrivate();

    Tp::IncomingFileTransferChannelPtr channel;
    QString downloadDirectory;
    bool askForDownloadDirectory;
    QFile* file;
    QUrl url, partUrl;
    qulonglong offset;
    bool isResuming;
    QPointer<KIO::RenameDialog> renameDialog;

    void init();
    void start();
    bool kill();
    void checkFileExists();
    void checkPartFile();
    void receiveFile();

    void __k__onRenameDialogFinished(int result);
    void __k__onResumeDialogFinished(int result);
    void __k__onSetUriOperationFinished(Tp::PendingOperation* op);
    void __k__onInitialOffsetDefined(qulonglong offset);
    void __k__onFileTransferChannelStateChanged(Tp::FileTransferState state, Tp::FileTransferStateChangeReason reason);
    void __k__onFileTransferChannelTransferredBytesChanged(qulonglong count);
    void __k__acceptFile();
    void __k__onAcceptFileFinished(Tp::PendingOperation* op);
    void __k__onCancelOperationFinished(Tp::PendingOperation* op);
    void __k__onInvalidated();
};

HandleIncomingFileTransferChannelJob::HandleIncomingFileTransferChannelJob(Tp::IncomingFileTransferChannelPtr channel,
                                                                           const QString downloadDirectory,
                                                                           bool askForDownloadDirectory,
                                                                           QObject* parent)
    : TelepathyBaseJob(*new HandleIncomingFileTransferChannelJobPrivate(), parent)
{
    qDebug();
    Q_D(HandleIncomingFileTransferChannelJob);

    d->channel = channel;
    d->downloadDirectory = downloadDirectory;
    d->askForDownloadDirectory = askForDownloadDirectory;
    d->init();
}

HandleIncomingFileTransferChannelJob::~HandleIncomingFileTransferChannelJob()
{
    qDebug();
    KIO::getJobTracker()->unregisterJob(this);
}

void HandleIncomingFileTransferChannelJob::start()
{
    qDebug();
    Q_D(HandleIncomingFileTransferChannelJob);
    d->start();
}

bool HandleIncomingFileTransferChannelJob::doKill()
{
    qDebug() << "Incoming file transfer killed.";
    Q_D(HandleIncomingFileTransferChannelJob);
    return d->kill();
}

HandleIncomingFileTransferChannelJobPrivate::HandleIncomingFileTransferChannelJobPrivate()
    : askForDownloadDirectory(true),
      file(0),
      offset(0),
      isResuming(false)
{
    qDebug();
}

HandleIncomingFileTransferChannelJobPrivate::~HandleIncomingFileTransferChannelJobPrivate()
{
    qDebug();
}

void HandleIncomingFileTransferChannelJobPrivate::init()
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (channel.isNull()) {
        qCritical() << "Channel cannot be NULL";
        q->setError(KTp::NullChannel);
        q->setErrorText(i18n("Invalid channel"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    Tp::Features features = Tp::Features() << Tp::FileTransferChannel::FeatureCore;
    if (!channel->isReady(Tp::Features() << Tp::FileTransferChannel::FeatureCore)) {
        qCritical() << "Channel must be ready with Tp::FileTransferChannel::FeatureCore";
        q->setError(KTp::FeatureNotReady);
        q->setErrorText(i18n("Channel is not ready"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    q->setCapabilities(KJob::Killable);
    q->setTotalAmount(KJob::Bytes, channel->size());
    q->setProcessedAmountAndCalculateSpeed(0);

    q->connect(channel.data(),
               SIGNAL(invalidated(Tp::DBusProxy*,QString,QString)),
               SLOT(__k__onInvalidated()));
    q->connect(channel.data(),
               SIGNAL(initialOffsetDefined(qulonglong)),
               SLOT(__k__onInitialOffsetDefined(qulonglong)));
    q->connect(channel.data(),
               SIGNAL(stateChanged(Tp::FileTransferState,Tp::FileTransferStateChangeReason)),
               SLOT(__k__onFileTransferChannelStateChanged(Tp::FileTransferState,Tp::FileTransferStateChangeReason)));
    q->connect(channel.data(),
               SIGNAL(transferredBytesChanged(qulonglong)),
               SLOT(__k__onFileTransferChannelTransferredBytesChanged(qulonglong)));
}

void HandleIncomingFileTransferChannelJobPrivate::start()
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    Q_ASSERT(!q->error());
    if (q->error()) {
        qWarning() << "Job was started in error state. Something wrong happened." << q->errorString();
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    if (askForDownloadDirectory) {
        url = QFileDialog::getSaveFileUrl(0, QString(), QUrl(QLatin1String("kfiledialog:///FileTransferLastDirectory/") + channel->fileName()));

        partUrl.setPath(url.path() + QLatin1String(".part"));
        partUrl.setScheme(QLatin1String("file"));

        checkPartFile();
        return;
    }

    checkFileExists();
}

void HandleIncomingFileTransferChannelJobPrivate::checkFileExists()
{
    Q_Q(HandleIncomingFileTransferChannelJob);

    url = downloadDirectory + QLatin1Char('/') + channel->fileName();
    url.setScheme(QLatin1String("file"));

    partUrl = url.path() + QLatin1String(".part");
    partUrl.setScheme(QLatin1String("file"));

    QFileInfo fileInfo(url.toLocalFile()); // TODO check if it is a dir?
    if (fileInfo.exists()) {
        renameDialog = new KIO::RenameDialog(0,
                                             i18n("Incoming file exists"),
                                             QUrl(), //TODO
                                             url,
                                             KIO::RenameDialog_Overwrite,
                                             fileInfo.size(),
                                             channel->size(),
                                             fileInfo.created(),
                                             QDateTime(),
                                             fileInfo.lastModified(),
                                             channel->lastModificationTime());

        q->connect(q, SIGNAL(finished(KJob*)),
                   renameDialog.data(), SLOT(reject()));

        q->connect(renameDialog.data(),
                   SIGNAL(finished(int)),
                   SLOT(__k__onRenameDialogFinished(int)));

        renameDialog.data()->show();
        return;
    }

    checkPartFile();
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onRenameDialogFinished(int result)
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (!renameDialog) {
        qWarning() << "Rename dialog was deleted during event loop.";
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    Q_ASSERT(renameDialog.data()->result() == result);

    switch (result) {
    case KIO::R_CANCEL:
        // TODO Cancel file transfer and close channel
        channel->cancel();
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    case KIO::R_RENAME:
        url = renameDialog.data()->newDestUrl();
        break;
    case KIO::R_OVERWRITE:
    {
        // Delete the old file if exists
        QFile oldFile(url.toLocalFile(), 0);
        if (oldFile.exists()) {
            oldFile.remove();
        }
    }
        break;
    default:
        qWarning() << "Unknown Error";
        q->setError(KTp::KTpError);
        q->setErrorText(i18n("Unknown Error"));
        renameDialog.data()->deleteLater();
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }
    renameDialog.data()->deleteLater();
    renameDialog.clear();
    checkPartFile();
}

void HandleIncomingFileTransferChannelJobPrivate::checkPartFile()
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    QFileInfo fileInfo(partUrl.toLocalFile());
    if (fileInfo.exists()) {
        renameDialog = new KIO::RenameDialog(0,
                                             i18n("Would you like to resume partial download?"),
                                             QUrl(),
                                             partUrl,
                                             KIO::RenameDialog_Resume,
                                             fileInfo.size(),
                                             channel->size(),
                                             fileInfo.created(),
                                             QDateTime(),
                                             fileInfo.lastModified(),
                                             channel->lastModificationTime());

        q->connect(q, SIGNAL(finished(KJob*)),
                   renameDialog.data(), SLOT(reject()));

        q->connect(renameDialog.data(),
                   SIGNAL(finished(int)),
                   SLOT(__k__onResumeDialogFinished(int)));

        renameDialog.data()->show();
        return;
    }
    receiveFile();
}


void HandleIncomingFileTransferChannelJobPrivate::__k__onResumeDialogFinished(int result)
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (!renameDialog) {
        qWarning() << "Rename dialog was deleted during event loop.";
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        return;
    }

    Q_ASSERT(renameDialog.data()->result() == result);

    switch (result) {
    case KIO::R_RESUME:
    {
        QFileInfo fileInfo(partUrl.toLocalFile());
        offset = fileInfo.size();
        isResuming = true;
        break;
    }
    case KIO::R_RENAME:
        // If the user hits rename, we use the new name as the .part file
        partUrl = renameDialog.data()->newDestUrl();
        break;
    case KIO::R_CANCEL:
        // If user hits cancel .part file will be overwritten
    default:
        break;
    }

    receiveFile();
}

void HandleIncomingFileTransferChannelJobPrivate::receiveFile()
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    // Open the .part file in append mode
    file = new QFile(partUrl.toLocalFile(), q->parent());
    file->open(isResuming ? QIODevice::Append : QIODevice::WriteOnly);

    // Create an empty file with the definitive file name
    QFile realFile(url.toLocalFile(), 0);
    realFile.open(QIODevice::WriteOnly);

    Tp::PendingOperation* setUriOperation = channel->setUri(url.url());
    q->connect(setUriOperation,
               SIGNAL(finished(Tp::PendingOperation*)),
               SLOT(__k__onSetUriOperationFinished(Tp::PendingOperation*)));
}

bool HandleIncomingFileTransferChannelJobPrivate::kill()
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (channel->state() != Tp::FileTransferStateCancelled) {
        Tp::PendingOperation *cancelOperation = channel->cancel();
        q->connect(cancelOperation,
                   SIGNAL(finished(Tp::PendingOperation*)),
                   SLOT(__k__onCancelOperationFinished(Tp::PendingOperation*)));
    } else {
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
    }

    return true;
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onSetUriOperationFinished(Tp::PendingOperation* op)
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (op->isError()) {
        // We do not want to exit if setUri failed, but we try to send the file
        // anyway. Anyway we print a message for debugging purposes.
        qWarning() << "Unable to set the URI -" << op->errorName() << ":" << op->errorMessage();
    }

    KIO::getJobTracker()->registerJob(q);
    // KWidgetJobTracker has an internal timer of 500 ms, if we don't wait here
    // when the job description is emitted it won't be ready
    // We set the description here and then whe update it if path is changed.

    QTimer::singleShot(500, q, SLOT(__k__acceptFile()));
}

void HandleIncomingFileTransferChannelJobPrivate::__k__acceptFile()
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    Q_EMIT q->description(q, i18n("Incoming file transfer"),
                          qMakePair<QString, QString>(i18n("From"), channel->targetContact()->alias()),
                          qMakePair<QString, QString>(i18n("Filename"), url.toLocalFile()));

    Tp::PendingOperation* acceptFileOperation = channel->acceptFile(offset, file);
    q->connect(acceptFileOperation,
               SIGNAL(finished(Tp::PendingOperation*)),
               SLOT(__k__onAcceptFileFinished(Tp::PendingOperation*)));
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onInitialOffsetDefined(qulonglong offset)
{
    qDebug() << "__k__onInitialOffsetDefined" << offset;
    Q_Q(HandleIncomingFileTransferChannelJob);

    // Some protocols do not support resuming file transfers, therefore we need
    // to use to this method to set the real offset
    if (isResuming && offset == 0) {
        qDebug() << "Impossible to resume file. Restarting.";
        Q_EMIT q->infoMessage(q, i18n("Impossible to resume file transfer. Restarting."));
    }

    this->offset = offset;

    file->seek(offset);
    q->setProcessedAmountAndCalculateSpeed(offset);
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onFileTransferChannelStateChanged(Tp::FileTransferState state,
                                                                                         Tp::FileTransferStateChangeReason stateReason)
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    qDebug() << "Incoming file transfer channel state changed to" << state << "with reason" << stateReason;

    switch (state) {
    case Tp::FileTransferStateNone:
        // This is bad
        qWarning() << "An unknown error occurred.";
        q->setError(KTp::TelepathyErrorError);
        q->setErrorText(i18n("An unknown error occurred"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        break;
    case Tp::FileTransferStateCompleted:
    {
        QFileInfo fileinfo(url.toLocalFile());
        if (fileinfo.exists()) {
            QFile::remove(url.toLocalFile());
        }
        file->rename(url.toLocalFile());
        file->flush();
        file->close();
        qDebug() << "Incoming file transfer completed, saved at" << file->fileName();
        Q_EMIT q->infoMessage(q, i18n("Incoming file transfer")); // [Finished] is added automatically to the notification
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
        break;
    }
    case Tp::FileTransferStateCancelled:
    {
        q->setError(KTp::FileTransferCancelled);
        q->setErrorText(i18n("Incoming file transfer was canceled."));
        // Close .part file if open
        if (file && file->isOpen()) {
            file->close();
        }
        q->kill(KJob::Quietly);
        break;
    }
    case Tp::FileTransferStateAccepted:
    case Tp::FileTransferStatePending:
    case Tp::FileTransferStateOpen:
    default:
        break;
    }
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onFileTransferChannelTransferredBytesChanged(qulonglong count)
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    qDebug().nospace() << "Receiving " << channel->fileName() << " - "
                       << "transferred bytes" << " = " << offset + count << " ("
                       << ((int)(((double)(offset + count) / channel->size()) * 100)) << "% done)";
    q->setProcessedAmountAndCalculateSpeed(offset + count);
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onAcceptFileFinished(Tp::PendingOperation* op)
{
    // This method is called when the "acceptFile" operation is finished,
    // therefore the file was not received yet.
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (op->isError()) {
        qWarning() << "Unable to accept file -" << op->errorName() << ":" << op->errorMessage();
        q->setError(KTp::AcceptFileError);
        q->setErrorText(i18n("Unable to accept file"));
        QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
    }
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onCancelOperationFinished(Tp::PendingOperation* op)
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    if (op->isError()) {
        qWarning() << "Unable to cancel file transfer - " << op->errorName() << ":" << op->errorMessage();
        q->setError(KTp::CancelFileTransferError);
        q->setErrorText(i18n("Cannot cancel incoming file transfer"));
    }

    qDebug() << "File transfer cancelled";
    QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
}

void HandleIncomingFileTransferChannelJobPrivate::__k__onInvalidated()
{
    qDebug();
    Q_Q(HandleIncomingFileTransferChannelJob);

    qWarning() << "File transfer invalidated!" << channel->invalidationMessage() << "reason" << channel->invalidationReason();
    Q_EMIT q->infoMessage(q, i18n("File transfer invalidated. %1", channel->invalidationMessage()));

    QTimer::singleShot(0, q, SLOT(__k__doEmitResult()));
}

#include "moc_handle-incoming-file-transfer-channel-job.cpp"
