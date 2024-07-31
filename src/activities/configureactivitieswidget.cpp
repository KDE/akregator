/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configureactivitieswidget.h"
#include <KLocalizedString>
#include <PlasmaActivities/ActivitiesModel>
#include <QCheckBox>
#include <QItemDelegate>
#include <QLabel>
#include <QListView>
#include <QVBoxLayout>

using namespace Qt::Literals::StringLiterals;
using namespace Akregator;
namespace
{
// Code stolen from plasma-vault
class CheckboxDelegate : public QItemDelegate
{
public:
    CheckboxDelegate(QObject *parent)
        : QItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        auto wholeRect = option.rect;

        // Drawing the checkbox
        auto checkRect = wholeRect;
        checkRect.setWidth(checkRect.height());
        drawCheck(painter, option, checkRect, option.state & QStyle::State_Selected ? Qt::Checked : Qt::Unchecked);

        // Drawing the text
        auto textRect = wholeRect;
        textRect.setLeft(textRect.left() + 8 + textRect.height());
        drawDisplay(painter, option, textRect, index.data(Qt::DisplayRole).toString());
    }

    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return QSize(100, 22);
    }
};
}

ConfigureActivitiesWidget::ConfigureActivitiesWidget(QWidget *parent)
    : QWidget{parent}
    , mListView(new QListView(this))
    , mEnableActivitiesSupport(new QCheckBox(i18nc("@option:check", "Limit to the selected activities:"), this))
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("mainLayout"_L1);
    mainLayout->setContentsMargins({});

    auto label = new QLabel(i18nc("@label:textbox",
                                  "If you limit this account to activity, it will be shown in Ruqola only when you are in those activities. Furthermore, when "
                                  "you switch to an activity it should not be available in, it will automatically be hidden."),
                            this);
    label->setWordWrap(true);
    label->setObjectName("label"_L1);
    mainLayout->addWidget(label);

    mEnableActivitiesSupport->setObjectName("mEnableActivitiesSupport"_L1);
    mainLayout->addWidget(mEnableActivitiesSupport);
    mEnableActivitiesSupport->setChecked(false);

    mListView->setObjectName("mListView"_L1);
    mListView->setModel(new KActivities::ActivitiesModel(this));
    mListView->setItemDelegate(new CheckboxDelegate(this));
    mainLayout->addWidget(mListView);
    mListView->setSelectionMode(QAbstractItemView::MultiSelection);
    mListView->setEnabled(false);
    connect(mEnableActivitiesSupport, &QCheckBox::clicked, mListView, &QListView::setEnabled);
}

ConfigureActivitiesWidget::~ConfigureActivitiesWidget() = default;

ActivitiesManager::ActivitySettings ConfigureActivitiesWidget::activitiesSettings() const
{
    if (!mEnableActivitiesSupport->isChecked()) {
        return {};
    }
    const auto selection = mListView->selectionModel();
    const auto selected = selection->selectedIndexes();
    QStringList selectedActivities;
    selectedActivities.reserve(selected.count());
    for (const auto &selectedIndex : selected) {
        selectedActivities << selectedIndex.data(KActivities::ActivitiesModel::ActivityId).toString();
    }
    const ActivitiesManager::ActivitySettings activities{selectedActivities, true};
    return activities;
}

void ConfigureActivitiesWidget::setActivitiesSettings(const ActivitiesManager::ActivitySettings &activitySettings)
{
    auto model = mListView->model();
    auto selection = mListView->selectionModel();
    selection->clearSelection();

    bool listIsEmpty{activitySettings.activities.isEmpty()};
    mListView->setEnabled(activitySettings.enabled);
    mEnableActivitiesSupport->setChecked(activitySettings.enabled);
    bool hasFoundActivities = false;
    if (!listIsEmpty) {
        for (int row = 0; row < model->rowCount(); ++row) {
            const auto index = model->index(row, 0);
            const auto activity = model->data(index, KActivities::ActivitiesModel::ActivityId).toString();

            if (activitySettings.contains(activity)) {
                selection->select(index, QItemSelectionModel::Select);
                hasFoundActivities = true;
            }
        }
        // Make sure to disable it if we don't find activities => it was removed
        if (!hasFoundActivities) {
            mListView->setEnabled(false);
            mEnableActivitiesSupport->setChecked(false);
        }
    }
}

#include "moc_configureactivitieswidget.cpp"
