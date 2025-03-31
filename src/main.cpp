/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"
#include "akregator_debug.h"
#include "akregator_options.h"
#include "config-akregator.h"
#include "mainwindow.h"
#include "systemsignalhandlers.h"
#include "trayicon.h"
#if AKREGATOR_WITH_KUSERFEEDBACK
#include "userfeedback/akregatoruserfeedbackprovider.h"
#endif
#include <KontactInterface/PimUniqueApplication>

#include <KCrash>

#include <QDBusInterface>

#include <KIconTheme>

#include <KStyleManager>

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

    bool eventFilter(QObject *obj, QEvent *ev) override
    {
        // The QWindow of the the Akregator::MainWindow has been observed
        // to be destroyed and recreated (asynchronously) during initialisation
        // of the article viewer.  If this is detected, then ensure that the
        // KStatusNotifierItem is informed.
        if (ev->type() == QEvent::WinIdChange && obj == mMainWindow) {
            setTrayIconAssociatedWindow();
        }
        return (false);
    }

    void setTrayIconAssociatedWindow()
    {
        // The 'associatedWindow' test below will be true if the window has been
        // destroyed, because KStatusNotifierItem stores it as a QPointer.
        if (Akregator::TrayIcon::getInstance() && !Akregator::TrayIcon::getInstance()->associatedWindow()) {
            Akregator::TrayIcon::getInstance()->setAssociatedWindow(mMainWindow->windowHandle());
        }
    }

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

            setTrayIconAssociatedWindow();
            installEventFilter(this);

            akr.call(QStringLiteral("openStandardFeedList"));
        }

        akr.call(QStringLiteral("handleCommandLine"), args);
    }

    return PimUniqueApplication::activate(args, workingDir);
}
} // namespace Akregator

int main(int argc, char **argv)
{
    KIconTheme::initTheme();
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    Akregator::Application app(argc, &argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("akregator"));

    KStyleManager::initStyle();

    app.setDesktopFileName(QStringLiteral("org.kde.akregator"));
    Akregator::AboutData about;
    app.setAboutData(about);
    KCrash::initialize();

    QCommandLineParser *cmdArgs = app.cmdArgs();
    Akregator::akregator_options(cmdArgs);

    const QStringList args = QCoreApplication::arguments();
    cmdArgs->process(args);
    about.processCommandLine(cmdArgs);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("akregator")));

#if AKREGATOR_WITH_KUSERFEEDBACK
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
    // see if we are starting with session management
    if (app.isSessionRestored()) {
        auto mainWindow = new Akregator::MainWindow();
        mainWindow->loadPart();
        if (KMainWindow::canBeRestored(1)) {
            mainWindow->restore(1, false);
        }
    }

#ifdef USE_SYSTEM_SIGNAL_HANDLERS
    SystemSignalHandlers::RegisterTerminationSignalHandlers();
#endif
    return app.exec();
}
