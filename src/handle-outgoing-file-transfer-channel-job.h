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

#ifndef HANDLE_OUTGOING_FILE_TRANSFER_CHANNEL_JOB_H
#define HANDLE_OUTGOING_FILE_TRANSFER_CHANNEL_JOB_H

#include <telepathy-base-job.h>

#include <TelepathyQt/Constants>
#include <TelepathyQt/Types>

namespace Tp {
    class PendingOperation;
}


class HandleOutgoingFileTransferChannelJobPrivate;
class HandleOutgoingFileTransferChannelJob : public KTp::TelepathyBaseJob
{
    Q_OBJECT
    Q_DISABLE_COPY(HandleOutgoingFileTransferChannelJob)
    Q_DECLARE_PRIVATE(HandleOutgoingFileTransferChannelJob)

//     // Our Q_PRIVATE_SLOTS who perform the real job
    Q_PRIVATE_SLOT(d_func(), void __k__start())
    Q_PRIVATE_SLOT(d_func(), void __k__onInitialOffsetDefined(qulonglong offset))
    Q_PRIVATE_SLOT(d_func(), void __k__onFileTransferChannelStateChanged(Tp::FileTransferState state, Tp::FileTransferStateChangeReason reason))
    Q_PRIVATE_SLOT(d_func(), void __k__onFileTransferChannelTransferredBytesChanged(qulonglong count))
    Q_PRIVATE_SLOT(d_func(), void __k__onProvideFileFinished(Tp::PendingOperation* op))
    Q_PRIVATE_SLOT(d_func(), void __k__onCancelOperationFinished(Tp::PendingOperation* op))
    Q_PRIVATE_SLOT(d_func(), void __k__onInvalidated())


public:
    explicit HandleOutgoingFileTransferChannelJob(Tp::OutgoingFileTransferChannelPtr channel,
                                                  QObject* parent = 0);
    virtual ~HandleOutgoingFileTransferChannelJob();

    virtual void start();
    virtual bool doKill();
};


#endif // HANDLE_OUTGOING_FILE_TRANSFER_CHANNEL_JOB_H
