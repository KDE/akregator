/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"
#include "akregator_debug.h"
#include "akregator_options.h"
#include <kcoreaddons_version.h>
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "akregratormigrateapplication.h"
#endif
#include "mainwindow.h"
#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/akregatoruserfeedbackprovider.h"
#endif
#include <KontactInterface/PimUniqueApplication>

#include <KCrash>

#include <QDBusInterface>

namespace Akregator
{
class Application : public KontactInterface::PimUniqueApplication
{
public:
    Application(int &argc, char **argv[])
        : KontactInterface::PimUniqueApplication(argc, argv)
    {
    }

    ~Application() override = default;

    int activate(const QStringList &args, const QString &workingDir) override;

private:
    Akregator::MainWindow *mMainWindow = nullptr;
};

int Application::activate(const QStringList &args, const QString &workingDir)
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
    if (cmdArgs->isSet(QStringLiteral("feedback"))) {
        auto userFeedBack = new Akregator::AkregatorUserFeedbackProvider(nullptr);
        QTextStream(stdout) << userFeedBack->describeDataSources() << '\n';
        delete userFeedBack;
        return 0;
    }
#endif
    if (!Akregator::Application::start(args)) {
        qCWarning(AKREGATOR_LOG) << "akregator is already running, exiting.";
        exit(0);
    }
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
    AkregratorMigrateApplication migrate;
    migrate.migrate();
#endif
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
