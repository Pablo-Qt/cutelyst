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

#include "cutelystapplication_p.h"

#include "cutelystenginehttp.h"
#include "cutelystengineuwsgi.h"
#include "context_p.h"
#include "cutelystrequest.h"
#include "cutelystresponse.h"

#include <iostream>

#include <QStringList>
#include <QDebug>

using namespace std;
using namespace Cutelyst;

void cuteOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[debug] %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "[warn] %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[crit] %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "[fatal] %s\n", localMsg.constData());
        abort();
    }
}

CutelystApplication::CutelystApplication(int &argc, char **argv) :
    QCoreApplication(argc, argv),
    d_ptr(new CutelystApplicationPrivate)
{
    qInstallMessageHandler(cuteOutput);
}

CutelystApplication::~CutelystApplication()
{
    delete d_ptr;
}

void CutelystApplication::registerPlugin(CutelystPlugin::Plugin *plugin)
{
    Q_D(CutelystApplication);

    QString pluginName = plugin->metaObject()->className();

    if (!d->plugins.contains(plugin)) {
        d->plugins << plugin;
    } else {
        qWarning() << Q_FUNC_INFO << "Failed to register plugin" << pluginName << plugin;
    }
}

bool CutelystApplication::parseArgs()
{
    Q_D(CutelystApplication);

    QStringList args = QCoreApplication::arguments();
    if (args.contains(QLatin1String("--about")) ||
            args.contains(QLatin1String("--sobre")) ||
            args.contains(QLatin1String("/sobre"))) {
        cout << QCoreApplication::applicationName().toStdString() << endl
             << QCoreApplication::applicationVersion().toStdString() << endl
             << QCoreApplication::organizationName().toStdString() << endl
             << QCoreApplication::organizationDomain().toStdString() << endl
             << "Qt: " << QT_VERSION_STR << endl;
        return false;
    }

    return true;
}

int CutelystApplication::printError()
{
    return 1;
}

bool CutelystApplication::setup(CutelystEngine *engine)
{
    Q_D(CutelystApplication);

    d->dispatcher = new CutelystDispatcher(this);
    d->dispatcher->setupActions();

    if (engine) {
        d->engine = engine;
    } else {
        d->engine = new CutelystEngineHttp(this);
    }
    connect(d->engine, &CutelystEngine::handleRequest,
            this, &CutelystApplication::handleRequest);

    foreach (CutelystPlugin::Plugin *plugin, d->plugins) {
        if (!plugin->setup(this)) {
            qWarning() << Q_FUNC_INFO << "Failed to setup plugin" << plugin;
            return false;
        }
    }

    return d->engine->init();
}

void CutelystApplication::handleRequest(CutelystRequest *req, CutelystResponse *resp)
{
    Q_D(CutelystApplication);

    ContextPrivate *priv = new ContextPrivate;
    priv->engine = d->engine;
    priv->dispatcher = d->dispatcher;
    foreach (CutelystPlugin::Plugin *plugin, d->plugins) {
        priv->plugins.insert(plugin, QVariantHash());
    }

    Context *c = new Context(priv);
    connect(c, &Context::beforePrepareAction, this, &CutelystApplication::beforePrepareAction);
    connect(c, &Context::afterPrepareAction, this, &CutelystApplication::afterPrepareAction);
    connect(c, &Context::beforeDispatch, this, &CutelystApplication::beforeDispatch);
    connect(c, &Context::afterDispatch, this, &CutelystApplication::afterDispatch);
    c->handleRequest(req, resp);
    delete c;
}
