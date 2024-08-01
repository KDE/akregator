/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once
#include "activitiesmanager.h"
#include "akregator_export.h"
#include <QWidget>
class QListView;
class QCheckBox;
namespace Akregator
{
class AKREGATOR_EXPORT ConfigureActivitiesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigureActivitiesWidget(QWidget *parent = nullptr);
    ~ConfigureActivitiesWidget() override;

    [[nodiscard]] ActivitiesManager::ActivitySettings activitiesSettings() const;
    void setActivitiesSettings(const ActivitiesManager::ActivitySettings &activitySettings);

private:
    QListView *const mListView;
    QCheckBox *const mEnableActivitiesSupport;
};
}
