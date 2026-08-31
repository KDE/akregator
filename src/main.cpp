/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
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

using namespace Qt::Literals::StringLiterals;

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

    Akregator::MainWindow *mMainWindow = nullptr;
};

int Application::activate(const QStringList &args, const QString &workingDir)
{
    if (!isSessionRestored()) {
        QDBusInterface akr(u"org.kde.akregator"_s, u"/Akregator"_s, u"org.kde.akregator.part"_s);

        QCommandLineParser *parser = cmdArgs();
        parser->process(args);

        if (!mMainWindow) {
            mMainWindow = new Akregator::MainWindow();
            mMainWindow->loadPart();
            mMainWindow->setupProgressWidgets();
            if (!parser->isSet(u"hide-mainwindow"_s)) {
                mMainWindow->show();
            }

            setTrayIconAssociatedWindow();
            installEventFilter(this);

            akr.call(u"openStandardFeedList"_s);
        }

        akr.call(u"handleCommandLine"_s, args);
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

    app.setDesktopFileName(u"org.kde.akregator"_s);
    Akregator::AboutData about;
    app.setAboutData(about);
    KCrash::initialize();

    QCommandLineParser *cmdArgs = app.cmdArgs();
    Akregator::akregator_options(cmdArgs);

    const QStringList args = QCoreApplication::arguments();
    cmdArgs->process(args);
    about.processCommandLine(cmdArgs);
    QApplication::setWindowIcon(QIcon::fromTheme(u"akregator"_s));

#if AKREGATOR_WITH_KUSERFEEDBACK
    if (cmdArgs->isSet(u"feedback"_s)) {
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
        Q_ASSERT(!app.mMainWindow);
        app.mMainWindow = new Akregator::MainWindow();
        app.mMainWindow->loadPart();
        if (KMainWindow::canBeRestored(1)) {
            app.mMainWindow->restore(1, false);
        }
        app.setTrayIconAssociatedWindow();
    }

#ifdef USE_SYSTEM_SIGNAL_HANDLERS
    SystemSignalHandlers::RegisterTerminationSignalHandlers();
#endif
    return app.exec();
}
