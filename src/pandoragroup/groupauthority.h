#pragma once
/************************************************************************
**
** Authors:   Nils Schimmelmann <nschimme@gmail.com> (Jahara)
**
** This file is part of the MMapper project.
** Maintained by Nils Schimmelmann <nschimme@gmail.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the:
** Free Software Foundation, Inc.
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
************************************************************************/

#ifndef GROUPCERTIFICATE_H
#define GROUPCERTIFICATE_H

#include <QObject>
#include <QSslCertificate>
#include <QSslKey>
#include <QStringListModel>

using GroupSecret = QByteArray;

static constexpr const auto GROUP_ORGANIZATION = "MUME";
static constexpr const auto GROUP_ORGANIZATIONAL_UNIT = "MMapper";
static constexpr const auto GROUP_COMMON_NAME = "GroupManager";

class GroupAuthority : public QObject
{
    Q_OBJECT
public:
    explicit GroupAuthority(QObject *parent = nullptr);

public slots:
    void refresh();

public:
    GroupSecret getSecret() const;
    QSslCertificate getLocalCertificate() const { return certificate; }
    QSslKey getPrivateKey() const { return key; }

public:
    QAbstractItemModel *getItemModel() { return &model; }
    bool add(const GroupSecret &);
    bool remove(const GroupSecret &);
    bool isAuthorized(const GroupSecret &) const;

signals:
    void secretRevoked(const GroupSecret &);
    void secretRefreshed(const GroupSecret &);

private:
    QStringListModel model;
    QSslCertificate certificate;
    QSslKey key;
};

#endif // GROUPCERTIFICATE_H
