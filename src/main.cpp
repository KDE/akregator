/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"
#include "akregator_debug.h"
#include "akregator_options.h"
#include "mainwindow.h"
#include "trayicon.h"
#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/akregatoruserfeedbackprovider.h"
#endif
#include <KontactInterface/PimUniqueApplication>

#include <KCrash>

#include <QDBusInterface>

#define HAVE_KICONTHEME __has_include(<KIconTheme>)
#if HAVE_KICONTHEME
#include <KIconTheme>
#endif

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif

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
            if (Akregator::TrayIcon::getInstance() && !Akregator::TrayIcon::getInstance()->associatedWindow()) {
                Akregator::TrayIcon::getInstance()->setAssociatedWindow(mMainWindow->windowHandle());
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
#if HAVE_KICONTHEME && (KICONTHEMES_VERSION >= QT_VERSION_CHECK(6, 3, 0))
    KIconTheme::initTheme();
#endif
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    Akregator::Application app(argc, &argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("akregator"));

    KCrash::initialize();
#if HAVE_STYLE_MANAGER
    KStyleManager::initStyle();
#else // !HAVE_STYLE_MANAGER
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setStyle(QStringLiteral("breeze"));
#endif // defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#endif // HAVE_STYLE_MANAGER

    app.setDesktopFileName(QStringLiteral("org.kde.akregator"));
    Akregator::AboutData about;
    app.setAboutData(about);

    QCommandLineParser *cmdArgs = app.cmdArgs();
    Akregator::akregator_options(cmdArgs);

    const QStringList args = QCoreApplication::arguments();
    cmdArgs->process(args);
    about.processCommandLine(cmdArgs);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("akregator")));

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
