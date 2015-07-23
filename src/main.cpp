/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "aboutdata.h"
#include "mainwindow.h"
#include "akregator_options.h"
#include "utils.h"
#include "akregator_debug.h"
#include "akregratormigrateapplication.h"
#include <KLocalizedString>
#include <kontactinterface/pimuniqueapplication.h>
#include <QtDBus/QtDBus>

namespace Akregator
{

class Application : public KontactInterface::PimUniqueApplication
{
public:
    Application(int &argc, char **argv[])
        : KontactInterface::PimUniqueApplication(argc, argv)
        , mMainWindow(0)
    {}

    ~Application() {}

    int activate(const QStringList &args) Q_DECL_OVERRIDE;

private:
    Akregator::MainWindow *mMainWindow;
};

int Application::activate(const QStringList &args)
{
    if (!isSessionRestored()) {
        QDBusInterface akr(QStringLiteral("org.kde.akregator"), QStringLiteral("/Akregator"), QStringLiteral("org.kde.akregator.part"));

        QCommandLineParser *parser = cmdArgs();
        parser->process(args);

        if (!mMainWindow) {
            mMainWindow = new Akregator::MainWindow();
            mMainWindow->loadPart();
            mMainWindow->setupProgressWidgets();
            if (!parser->isSet(QStringLiteral("hide-mainwindow"))) {
                mMainWindow->show();
            }
            akr.call(QStringLiteral("openStandardFeedList"));
        }

        akr.call(QStringLiteral("handleCommandLine"), args);
    }
    return PimUniqueApplication::activate(args);
}

} // namespace Akregator

int main(int argc, char **argv)
{
    Akregator::Application app(argc, &argv);
    Akregator::AboutData about;
    app.setAboutData(about);

    QCommandLineParser *cmdArgs = app.cmdArgs();
    Akregator::akregator_options(cmdArgs);

    const QStringList args = QCoreApplication::arguments();
    cmdArgs->process(args);
    about.processCommandLine(cmdArgs);

    if (!Akregator::Application::start(args)) {
        qCWarning(AKREGATOR_LOG) << "akregator is already running, exiting.";
        exit(0);
    }
    AkregratorMigrateApplication migrate;
    migrate.migrate();

    // start knotifyclient if not already started. makes it work for people who doesn't use full kde, according to kmail devels
    //KNotifyClient::startDaemon();

    // see if we are starting with session management
    if (app.isSessionRestored()) {
#undef RESTORE
#define RESTORE(type) { int n = 1;\
        while (KMainWindow::canBeRestored(n)){\
            (new type)->restore(n, false);\
            n++;}}

        RESTORE(Akregator::MainWindow);
    }

    return app.exec();
}

