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
#include "akregator_debug.h"
#include "akregratormigrateapplication.h"
#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/akregatoruserfeedbackprovider.h"
#endif
#include <KontactInterface/PimUniqueApplication>

#include <KCrash>

#include <QDBusInterface>

namespace Akregator {
class Application : public KontactInterface::PimUniqueApplication
{
public:
    Application(int &argc, char **argv[])
        : KontactInterface::PimUniqueApplication(argc, argv)
        , mMainWindow(nullptr)
    {
    }

    ~Application() override
    {
    }

    int activate(const QStringList &args, const QString &workingDir) override;

private:
    Akregator::MainWindow *mMainWindow = nullptr;
};

int Application::activate(const QStringList &args, const QString &workingDir)
{
    if (!isSessionRestored()) {
        QDBusInterface akr(QStringLiteral("org.kde.akregator"), QStringLiteral("/Akregator"),
                           QStringLiteral("org.kde.akregator.part"));

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
    return PimUniqueApplication::activate(args, workingDir);
}
} // namespace Akregator

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    Akregator::Application app(argc, &argv);
    KLocalizedString::setApplicationDomain("akregator");

    KCrash::initialize();

    app.setDesktopFileName(QStringLiteral("org.kde.akregator"));
    Akregator::AboutData about;
    app.setAboutData(about);

    QCommandLineParser *cmdArgs = app.cmdArgs();
    Akregator::akregator_options(cmdArgs);

    const QStringList args = QCoreApplication::arguments();
    cmdArgs->process(args);
    about.processCommandLine(cmdArgs);

#ifdef WITH_KUSERFEEDBACK
    if(cmdArgs->isSet(QStringLiteral("feedback"))) {
        Akregator::AkregatorUserFeedbackProvider *userFeedBack = new Akregator::AkregatorUserFeedbackProvider(nullptr);
        QTextStream(stdout) << userFeedBack->describeDataSources() << '\n';
        delete userFeedBack;
        return 0;
    }
#endif
    if (!Akregator::Application::start(args)) {
        qCWarning(AKREGATOR_LOG) << "akregator is already running, exiting.";
        exit(0);
    }
    AkregratorMigrateApplication migrate;
    migrate.migrate();

    // see if we are starting with session management
    if (app.isSessionRestored()) {
        auto mainWindow = new Akregator::MainWindow();
        mainWindow->loadPart();
        if (KMainWindow::canBeRestored(1)) {
            mainWindow->restore(1, false);
        }
    }

    return app.exec();
}
