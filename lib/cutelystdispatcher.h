/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CUTELYSTDISPATCHER_H
#define CUTELYSTDISPATCHER_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include "cutelystaction.h"

namespace Cutelyst {

class Context;
class CutelystController;
class CutelystDispatchType;
class CutelystDispatcherPrivate;
class CutelystDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit CutelystDispatcher(QObject *parent = 0);
    ~CutelystDispatcher();
    void setupActions();

    bool dispatch(Context *ctx);
    bool forward(Context *ctx, const QString &opname, const QStringList &arguments);
    void prepareAction(Context *ctx);
    CutelystAction* getAction(const QString &name, const QString &ns = QString()) const;
    ActionList getActions(const QString &name, const QString &ns) const;
    QHash<QString, CutelystController*> controllers() const;
    QString uriForAction(CutelystAction *action, const QStringList &captures);

private:
    void printActions();
    CutelystAction *command2Action(Context *ctx, const QString &command, const QStringList &extraParams = QStringList());
    QStringList unexcapedArgs(const QStringList &args);
    QString actionRel2Abs(Context *ctx, const QString &path);
    CutelystAction *invokeAsPath(Context *ctx, const QString &relativePath, const QStringList &args);
    QString cleanNamespace(const QString &ns) const;

protected:
    CutelystDispatcherPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystDispatcher)
};

}

#endif // CUTELYSTDISPATCHER_H
