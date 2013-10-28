/*
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author Dario Freddi <dario.freddi@collabora.co.uk>
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

#ifndef LIBKTP_TELEPATHY_BASE_JOB_P_H
#define LIBKTP_TELEPATHY_BASE_JOB_P_H

#include "telepathy-base-job.h"

#include <QTime>

namespace Tp
{
class PendingOperation;
}

namespace KTp
{

class TelepathyBaseJobPrivate
{
    Q_DECLARE_PUBLIC(TelepathyBaseJob)

protected:
    TelepathyBaseJob* q_ptr;

public:
    TelepathyBaseJobPrivate();
    virtual ~TelepathyBaseJobPrivate();

    QTime time;
    qulonglong alreadyProcessed;
    QList< Tp::PendingOperation* > operations;
    QList< QPair< QString, QString > > telepathyErrors;

    void addOperation(Tp::PendingOperation* op);

    // Operation Q_PRIVATE_SLOTS
    void __k__tpOperationFinished(Tp::PendingOperation* op);
    void __k__doEmitResult();
};

} // namespace KTp

#endif // LIBKTP_TELEPATHY_BASE_JOB_P_H
