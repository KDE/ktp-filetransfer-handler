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

#ifndef LIBKTP_TELEPATHY_BASE_JOB_H
#define LIBKTP_TELEPATHY_BASE_JOB_H

#include <kdemacros.h>
#include <KJob>

namespace KTp
{

/**
 * This enum defines the error code for a job spawned by one of the functions inside \c TelepathyBridge
 */
enum JobError {
    /** Taken from KJob. No errors occurred */
    NoError = 0,
    /** Taken from KJob. The job was killed */
    KilledJobError = 1,
    /** The operation supplied is invalid. This, most of the times, represents an error internal to TelepathyBridge */
    InvalidOperationError = 101,
    /**
     * This error means that there has been one or more errors while mapping Nepomuk resources to Telepathy contacts.
     * For specific operations, this means that:
     * <ul>
     * <li>If the operation is being done upon a contact, the Nepomuk resource provided could not be
     *     mapped to an existing Tp::Contact.
     * </li>
     * <li>If the operation is being done upon a metacontact, none of the \c PersonContact belonging to the metacontact
     *     could be mapped to an existing Tp::Contact.
     * </li>
     * </ul>
     */
    NoContactsFoundError = 102,
    /** None of the specified \c RemovalModes were available for the contact(s) in question */
    NoRemovalModesAvailableError = 103,
    /** No valid Telepathy accounts were found to match the specified contacts and/or resources */
    NoAccountsFoundError = 104,
    /** The protocol of the account on which the request is being done is not capable to carry on the requested action */
    ProtocolNotCapableError = 105,
    /** The operation requested the account to be online, but it is not */
    AccountNotOnlineError = 106,
    /** An operation attempted to create a nepomuk resource which already exists */
    ResourceAlreadyExistsError = 107,
    /** A channel is null */
    NullChannel = 108,
    /** A required Tp::Feature is not ready */
    FeatureNotReady = 109,
    /** Cannot accept file */
    AcceptFileError = 111,
    /** File transfer cancelled */
    FileTransferCancelled = 112,
    /** URI Property is missing */
    UriPropertyMissing = 113,
    /** Non a local file */
    NotALocalFile = 114,
    /** Cannot provide file */
    ProvideFileError = 115,
    /** Cannot cancel file transfer */
    CancelFileTransferError = 116,
    /** Telepathy triggered an error */
    TelepathyErrorError = 200,
    /** KTp Error */
    KTpError = 300
};

class TelepathyBaseJobPrivate;
class KDE_EXPORT TelepathyBaseJob : public KJob
{
    Q_OBJECT
    Q_DISABLE_COPY(TelepathyBaseJob)
    Q_DECLARE_PRIVATE(TelepathyBaseJob)

    Q_PRIVATE_SLOT(d_func(), void __k__tpOperationFinished(Tp::PendingOperation*))
    Q_PRIVATE_SLOT(d_func(), void __k__doEmitResult())

protected:
    explicit TelepathyBaseJob(TelepathyBaseJobPrivate &dd, QObject *parent = 0);
    virtual ~TelepathyBaseJob();

    TelepathyBaseJobPrivate * const d_ptr;
};

} // namespace KTp

#endif // LIBKTP_TELEPATHY_BASE_JOB_H
