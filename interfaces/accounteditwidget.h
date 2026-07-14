/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "akregatorinterfaces_export.h"
#include <QWidget>

namespace Akregator
{

/**
 * Base class for the account configuration form of an AccountPlugin.
 * The widget is embedded into the "Add Online Account" wizard (and can be
 * reused by plugin-specific dialogs).
 */
class AKREGATORINTERFACES_EXPORT AccountEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountEditWidget(QWidget *parent = nullptr);
    ~AccountEditWidget() override;

    /** @return true if the current input is complete enough to create the account. */
    [[nodiscard]] virtual bool isValid() const = 0;

    /** Create the account from the current input. @return false if it failed
     *  (e.g. duplicate account name); the wizard stays open in that case. */
    virtual bool apply() = 0;

Q_SIGNALS:
    /** Emitted whenever the result of isValid() may have changed. */
    void validityChanged();
};

} // namespace Akregator
