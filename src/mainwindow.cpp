/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "mainwindow.h"
#include "akregator_part.h"
#include "akregatorconfig.h"
#include "trayicon.h"
#include <Libkdepim/ProgressManager>
#include <Libkdepim/ProgressStatusBarWidget>
#include <Libkdepim/StatusbarProgressWidget>
#include <PimCommon/BroadcastStatus>

#include <KActionCollection>
#include <KConfig>
#include <KEditToolBar>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KSharedConfig>
#include <KShortcutsDialog>
#include <KSqueezedTextLabel>
#include <KStandardAction>
#include <KToggleFullScreenAction>
#include <KToolBar>
#include <KXMLGUIFactory>
#include <QApplication>
#include <QFontDatabase>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QToolButton>

using namespace Akregator;

MainWindow::MainWindow(QWidget *parent)
    : KParts::MainWindow(parent)
    , m_part()
    , m_statusLabel(new KSqueezedTextLabel(this))
{
    // set the shell's ui resource file
    setXMLFile(QStringLiteral("akregator_shell.rc"));

    KStandardAction::keyBindings(this, &MainWindow::optionsConfigureKeys, actionCollection()); // options_configure_keybinding
    KStandardAction::configureToolbars(this, &MainWindow::optionsConfigureToolbars, actionCollection()); // options_configure_toolbars

    toolBar()->show();
    statusBar()->show();

    const int statH = fontMetrics().height() + 2;
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
    m_statusLabel->setMinimumWidth(0);
    m_statusLabel->setFixedHeight(statH);
    statusBar()->addWidget(m_statusLabel, 1);

    KStandardAction::quit(qApp, &QApplication::quit, actionCollection());
    mShowMenuBarAction = KStandardAction::showMenubar(this, &MainWindow::slotToggleMenubar, actionCollection());
    mShowMenuBarAction->setChecked(Settings::showMenuBar());
    slotToggleMenubar(true);

    if (menuBar()) {
        mHamburgerMenu = KStandardAction::hamburgerMenu(nullptr, nullptr, actionCollection());
        mHamburgerMenu->setShowMenuBarAction(mShowMenuBarAction);
        mHamburgerMenu->setMenuBar(menuBar());
        connect(mHamburgerMenu, &KHamburgerMenu::aboutToShowMenu, this, [this]() {
            updateHamburgerMenu();
            // Immediately disconnect. We only need to run this once, but on demand.
            // NOTE: The nullptr at the end disconnects all connections between
            // q and mHamburgerMenu's aboutToShowMenu signal.
            disconnect(mHamburgerMenu, &KHamburgerMenu::aboutToShowMenu, this, nullptr);
        });
    }

    mShowFullScreenAction = KStandardAction::fullScreen(nullptr, nullptr, this, actionCollection());
    actionCollection()->setDefaultShortcut(mShowFullScreenAction, Qt::Key_F11);
    connect(mShowFullScreenAction, &QAction::toggled, this, &MainWindow::slotFullScreen);

    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();

    connect(PimCommon::BroadcastStatus::instance(), &PimCommon::BroadcastStatus::statusMsg, this, &MainWindow::slotSetStatusBarText);

    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::slotOnShutdown);
}

void MainWindow::slotFullScreen(bool t)
{
    KToggleFullScreenAction::setFullScreen(this, t);
    QMenuBar *mb = menuBar();
    if (t) {
        auto b = new QToolButton(mb);
        b->setDefaultAction(mShowFullScreenAction);
        b->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored));
        b->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
        mb->setCornerWidget(b, Qt::TopRightCorner);
        b->setVisible(true);
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    } else {
        QWidget *w = mb->cornerWidget(Qt::TopRightCorner);
        if (w) {
            w->deleteLater();
        }
    }
}

void MainWindow::slotToggleMenubar(bool dontShowWarning)
{
    if (menuBar()) {
        if (mShowMenuBarAction->isChecked()) {
            menuBar()->show();
        } else {
            if (!dontShowWarning && (!toolBar()->isVisible() || !toolBar()->actions().contains(mHamburgerMenu))) {
                const QString accel = mShowMenuBarAction->shortcut().toString();
                KMessageBox::information(this,
                                         i18n("<qt>This will hide the menu bar completely."
                                              " You can show it again by typing %1.</qt>",
                                              accel),
                                         i18nc("@title:window", "Hide menu bar"),
                                         QStringLiteral("HideMenuBarWarning"));
            }
            menuBar()->hide();
        }
        Settings::setShowMenuBar(mShowMenuBarAction->isChecked());
    }
}

void MainWindow::updateHamburgerMenu()
{
    auto menu = new QMenu(this);
    menu->addAction(actionCollection()->action(KStandardAction::name(KStandardAction::Quit)));
    mHamburgerMenu->setMenu(menu);
}

bool MainWindow::loadPart()
{
    if (m_part) {
        return true;
    }
    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    const KPluginMetaData md(QStringLiteral("akregatorpart"));
    const auto result = KPluginFactory::instantiatePlugin<KParts::Part>(md, this);
    if (result) {
        m_part = static_cast<Akregator::Part *>(result.plugin);
    } else {
        KMessageBox::error(this, i18n("Could not find the Akregator part; please check your installation.\n%1", result.errorString));
        return false;
    }

    m_part->setObjectName(QLatin1StringView("akregator_part"));
    setCentralWidget(m_part->widget());

    connect(m_part.data(), &Part::setWindowCaption, this, qOverload<const QString &>(&KMainWindow::setCaption));

    createGUI(m_part);
    setAutoSaveSettings();
    return true;
}

void MainWindow::setupProgressWidgets()
{
    auto progressStatusBarWidget = new KPIM::ProgressStatusBarWidget(statusBar(), this);
    m_statusbarProgress = progressStatusBarWidget->littleProgress();
    statusBar()->addPermanentWidget(m_statusbarProgress, 0);

    // The progress widget blocks a large part of the status bar, but is empty
    // most of the time. Hide it when unused, allowing a longer status label.
    m_hideProgressTimer = new QTimer(this);
    m_hideProgressTimer->setSingleShot(true);
    connect(m_hideProgressTimer, &QTimer::timeout, m_statusbarProgress, &QWidget::hide);

    connect(KPIM::ProgressManager::instance(), &KPIM::ProgressManager::progressItemAdded, this, &MainWindow::updateStatusbarProgressVisibility);
    connect(KPIM::ProgressManager::instance(), &KPIM::ProgressManager::progressItemCompleted, this, &MainWindow::updateStatusbarProgressVisibility);
    m_statusbarProgress->hide();
}

void MainWindow::updateStatusbarProgressVisibility()
{
    if (KPIM::ProgressManager::instance()->isEmpty()) {
        m_hideProgressTimer->start(5000);
    } else {
        m_hideProgressTimer->stop();
        m_statusbarProgress->show();
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::saveProperties(KConfigGroup &config)
{
    if (!m_part) {
        if (!loadPart()) {
            return;
        }
    }

    m_part->saveProperties(config);
    config.writeEntry("docked", isHidden());
}

void MainWindow::readProperties(const KConfigGroup &config)
{
    if (!m_part) {
        if (!loadPart()) {
            return;
        }
    }
    m_part->readProperties(config);
    setVisible(!Settings::showTrayIcon() || !config.readEntry("docked", false));
}

// TODO: move to part?
void MainWindow::optionsConfigureKeys()
{
    KShortcutsDialog dlg(this);

    dlg.addCollection(actionCollection());
    if (m_part) {
        dlg.addCollection(m_part->actionCollection());
    }

    if (dlg.configure()) {
        if (m_part) {
            // FIXME
            // m_part->updateQuickSearchLineText();
        }
    }
}

// TODO: move to part?
void MainWindow::optionsConfigureToolbars()
{
    KConfigGroup grp(KSharedConfig::openConfig().data()->group(autoSaveGroup()));
    saveMainWindowSettings(grp);
    QPointer<KEditToolBar> dlg = new KEditToolBar(factory());
    connect(dlg.data(), &KEditToolBar::newToolBarConfig, this, &MainWindow::applyNewToolbarConfig);
    dlg->exec();
    delete dlg;
}

// TODO: move to part?
void MainWindow::applyNewToolbarConfig()
{
    applyMainWindowSettings(KSharedConfig::openConfig()->group(autoSaveGroup()));
}

void MainWindow::slotQuit()
{
    qApp->quit();
}

void MainWindow::slotOnShutdown()
{
    delete m_part;
}

bool MainWindow::queryClose()
{
    if (qApp->isSavingSession()) {
        return true;
    }
    if (!TrayIcon::getInstance()) {
        delete m_part; // delete that here instead of dtor to ensure nested khtmlparts are deleted before singleton objects like KHTMLPageCache
        return true;
    }

    hide();
    return false;
}

void MainWindow::slotClearStatusText()
{
    m_statusLabel->setText(QString());
}

void MainWindow::slotSetStatusBarText(const QString &text)
{
    m_statusLabel->setText(text);
}

#include "moc_mainwindow.cpp"
