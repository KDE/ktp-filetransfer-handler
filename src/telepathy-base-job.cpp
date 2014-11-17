/*
 * Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
 *   @Author Dario Freddi <dario.freddi@collabora.co.uk>
 * Copyright (C) 2011 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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

#include "telepathy-base-job_p.h"
#include "ktp-fth-debug.h"

#include <TelepathyQt/PendingOperation>

#include <KLocalizedString>
#include <QDebug>

using namespace KTp;

TelepathyBaseJobPrivate::TelepathyBaseJobPrivate()
    : q_ptr(0)
    , alreadyProcessed(0)
{
}

TelepathyBaseJobPrivate::~TelepathyBaseJobPrivate()
{
}

void TelepathyBaseJobPrivate::addOperation(Tp::PendingOperation *op)
{
    Q_Q(TelepathyBaseJob);

    // Add the operation to the list
    operations << op;

    // Attach the operation to our listener
    q->connect(op, SIGNAL(finished(Tp::PendingOperation*)), q, SLOT(__k__tpOperationFinished(Tp::PendingOperation*)));
}

TelepathyBaseJob::TelepathyBaseJob(TelepathyBaseJobPrivate& dd, QObject* parent)
    : KJob(parent)
    , d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

TelepathyBaseJob::~TelepathyBaseJob()
{
    delete d_ptr;
}

void TelepathyBaseJob::setProcessedAmountAndCalculateSpeed(qulonglong amount)
{
    qCDebug(KTP_FTH_MODULE) << amount;
    Q_D(TelepathyBaseJob);

    //If the transfer is starting
    if (amount == 0) {
        d->time = QTime::currentTime();
    }

    //If a least 1 second has passed since last update
    int secondsSinceLastTime = d->time.secsTo(QTime::currentTime());
    if (secondsSinceLastTime > 0) {
        float speed = (amount - d->alreadyProcessed) / secondsSinceLastTime;
        emitSpeed(speed);

        d->time = QTime::currentTime();
        d->alreadyProcessed = amount;
    }
    setProcessedAmount(Bytes, amount);
}

void TelepathyBaseJobPrivate::__k__tpOperationFinished(Tp::PendingOperation* op)
{
    // First of all check if the operation is in our list
    if (!operations.contains(op)) {
        // WTF?
        // TODO: This should never happen, should we do something?
        return;
    }

    if (op->isError()) {
        // Ouch. Add it to the error roster
        telepathyErrors << qMakePair(op->errorName(), op->errorMessage());
    }

    // Remove it from the list
    operations.removeOne(op);

    // Ok, are we done yet?
    if (operations.isEmpty()) {
        // It looks like we are. Let's pass the ball to doEmitResult.
        __k__doEmitResult();
    }
}

void TelepathyBaseJobPrivate::__k__doEmitResult()
{
    qCDebug(KTP_FTH_MODULE);
    Q_Q(TelepathyBaseJob);

    // Before streaming out: are there any telepathy errors?
    if (!telepathyErrors.isEmpty()) {
        // Hmm, bad stuff. Let's handle them here.
        // FIXME: Maybe there's a better formatting for this specific error string?

        QString errorMessage = i18np("Telepathy reported an error while performing the requested operation:",
                                     "Telepathy reported %1 errors while performing the requested operation:",
                                     telepathyErrors.size());

        QList< QPair< QString, QString > >::const_iterator i;
        for (i = telepathyErrors.constBegin(); i != telepathyErrors.constEnd(); ++i) {
            errorMessage.append(QLatin1Char('\n'));
            errorMessage.append(i18nc("The following format is: ' - <error name>: <error message>'", " - %1: %2",
                                      (*i).first, (*i).second));
        }

        // Ok, let's set the errors now
        q->setError(KTp::TelepathyErrorError);
        q->setErrorText(errorMessage);
    }

    // The job has been finished
    q->emitResult();
}

#include "moc_telepathy-base-job.cpp"
