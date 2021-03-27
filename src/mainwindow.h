/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <kio/job.h>
#include <kparts/mainwindow.h>

#include <QPointer>

class KSqueezedTextLabel;

namespace Akregator
{
class Part;
class MainWindow;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 */
class MainWindow : public KParts::MainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /**
     * Creates the progress widget in the status bar and the ProgressDialog
     * and connects them.
     */
    void setupProgressWidgets();

    /**
     Loads the part
     @return Whether the part has been successfully created or not.
     */
    bool loadPart();

public Q_SLOTS:
    void slotClearStatusText();
    void slotSetStatusBarText(const QString &c) override;

protected:
    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfigGroup &) override;

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(const KConfigGroup &) override;

    /**
     * Reimplemented to say if app will be running in system tray if necessary
     */
    bool queryClose() override;

protected Q_SLOTS:
    void slotQuit();

private Q_SLOTS:

    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void applyNewToolbarConfig();
    void slotOnShutdown();

private:
    QPointer<Akregator::Part> m_part;
    KSqueezedTextLabel *const m_statusLabel;
};
} // namespace Akregator

