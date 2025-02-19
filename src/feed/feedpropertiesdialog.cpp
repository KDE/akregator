/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "feedpropertiesdialog.h"
#if HAVE_ACTIVITY_SUPPORT
#include <PimCommonActivities/ConfigureActivitiesWidget>
#endif
#include "akregatorconfig.h"

#include <KLocalizedString>
#include <QComboBox>
#include <QTabBar>

#include <KLocalization>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <kernel.h>
using namespace Akregator;

FeedPropertiesWidget::FeedPropertiesWidget(QWidget *parent)
    : QWidget(parent)
#if HAVE_ACTIVITY_SUPPORT
    , mConfigureActivitiesWidget(new PimCommonActivities::ConfigureActivitiesWidget(this))
#endif
{
    setupUi(this);

    tabWidget->tabBar()->setExpanding(true);

#if HAVE_ACTIVITY_SUPPORT
    if (Kernel::self()->activitiesManager()->enabled()) {
        tabWidget->addTab(mConfigureActivitiesWidget, i18n("Activities"));
    } else {
        mConfigureActivitiesWidget->hide();
    }
#endif

    connect(cb_updateInterval, &QCheckBox::toggled, updateSpinBox, &QSpinBox::setEnabled);
    connect(cb_updateInterval, &QCheckBox::toggled, updateComboBox, &QComboBox::setEnabled);
    connect(cb_updateInterval, &QCheckBox::toggled, updateLabel, &QLabel::setEnabled);
    connect(cb_updateInterval, &QCheckBox::toggled, this, &FeedPropertiesWidget::slotUpdateCheckBoxToggled);
    connect(updateComboBox, &QComboBox::activated, this, &FeedPropertiesWidget::slotUpdateComboBoxActivated);
    connect(updateSpinBox, &QSpinBox::valueChanged, this, &FeedPropertiesWidget::slotUpdateComboBoxLabels);
    connect(rb_limitArticleAge, &QRadioButton::toggled, sb_maxArticleAge, &QSpinBox::setEnabled);
    connect(rb_limitArticleNumber, &QRadioButton::toggled, sb_maxArticleNumber, &QSpinBox::setEnabled);
}

FeedPropertiesWidget::~FeedPropertiesWidget() = default;

void FeedPropertiesWidget::slotUpdateComboBoxActivated(int index)
{
    updateSpinBox->setEnabled(index != Never);
}

void FeedPropertiesWidget::slotUpdateComboBoxLabels(int value)
{
    updateComboBox->setItemText(FeedPropertiesWidget::Minutes, i18np("Minute", "Minutes", value));
    updateComboBox->setItemText(FeedPropertiesWidget::Hours, i18np("Hour", "Hours", value));
    updateComboBox->setItemText(FeedPropertiesWidget::Days, i18np("Day", "Days", value));
}

void FeedPropertiesWidget::slotUpdateCheckBoxToggled(bool enabled)
{
    updateSpinBox->setEnabled(enabled && updateComboBox->currentIndex() != Never);
}

#if HAVE_ACTIVITY_SUPPORT
PimCommonActivities::ConfigureActivitiesWidget *FeedPropertiesWidget::configureActivitiesWidget() const
{
    return mConfigureActivitiesWidget;
}

ActivitiesManager::ActivitySettings FeedPropertiesWidget::activitySettings() const
{
    return mConfigureActivitiesWidget->activitiesSettings();
}

void FeedPropertiesWidget::setActivitiesSettings(const ActivitiesManager::ActivitySettings &activitySettings)
{
    mConfigureActivitiesWidget->setActivitiesSettings(activitySettings);
}
#endif

FeedPropertiesDialog::FeedPropertiesDialog(QWidget *parent, const QString &name)
    : QDialog(parent)
    , mFeedPropertiesWidget(new FeedPropertiesWidget(this))
{
    setObjectName(name);
    setWindowTitle(i18nc("@title:window", "Feed Properties"));
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins({});

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                  style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                                  style()->pixelMetric(QStyle::PM_LayoutBottomMargin),
                                  style()->pixelMetric(QStyle::PM_LayoutRightMargin));
    buttonBox->setProperty("_breeze_force_frame", true);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FeedPropertiesDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FeedPropertiesDialog::reject);
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    setModal(true);

    mainLayout->addWidget(mFeedPropertiesWidget);
    mainLayout->addWidget(buttonBox);
    mFeedPropertiesWidget->feedNameEdit->setFocus();

    mFeedPropertiesWidget->updateComboBox->insertItem(FeedPropertiesWidget::Minutes, i18np("Minute", "Minutes", 0));
    mFeedPropertiesWidget->updateComboBox->insertItem(FeedPropertiesWidget::Hours, i18np("Hour", "Hours", 0));
    mFeedPropertiesWidget->updateComboBox->insertItem(FeedPropertiesWidget::Days, i18np("Day", "Days", 0));
    mFeedPropertiesWidget->updateComboBox->insertItem(FeedPropertiesWidget::Never, i18nc("never fetch new articles", "Never"));

    KLocalization::setupSpinBoxFormatString(mFeedPropertiesWidget->sb_maxArticleAge, ki18np("%v day", "%v days"));
    KLocalization::setupSpinBoxFormatString(mFeedPropertiesWidget->sb_maxArticleNumber, ki18np("%v article", "%v articles"));

    connect(mFeedPropertiesWidget->feedNameEdit, &QLineEdit::textChanged, this, &FeedPropertiesDialog::slotSetWindowTitle);
}

FeedPropertiesDialog::~FeedPropertiesDialog() = default;

void FeedPropertiesDialog::accept()
{
    m_feed->setNotificationMode(false);
    m_feed->setTitle(feedName());
    m_feed->setXmlUrl(url());
    m_feed->setCustomFetchIntervalEnabled(autoFetch());
    if (autoFetch()) {
        m_feed->setFetchInterval(fetchInterval());
    }
    m_feed->setArchiveMode(archiveMode());
    m_feed->setMaxArticleAge(maxArticleAge());
    m_feed->setMaxArticleNumber(maxArticleNumber());
    m_feed->setMarkImmediatelyAsRead(markImmediatelyAsRead());
    m_feed->setUseNotification(useNotification());
    m_feed->setLoadLinkedWebsite(loadLinkedWebsite());
    m_feed->setNotificationMode(true);
    m_feed->setComment(comment());
#if HAVE_ACTIVITY_SUPPORT
    m_feed->setActivities(activitySettings().activities);
    m_feed->setActivityEnabled(activitySettings().enabled);
#endif
    QDialog::accept();
}

void FeedPropertiesDialog::slotSetWindowTitle(const QString &title)
{
    setWindowTitle(title.isEmpty() ? i18nc("@title:window", "Feed Properties") : i18nc("@title:window", "Properties of %1", title));
    mOkButton->setEnabled(!title.trimmed().isEmpty());
}

void FeedPropertiesDialog::setFeed(Feed *feed)
{
    m_feed = feed;
    if (!feed) {
        return;
    }

    setFeedName(feed->title());
    setUrl(feed->xmlUrl());
    setAutoFetch(feed->useCustomFetchInterval());
    if (feed->useCustomFetchInterval()) {
        setFetchInterval(feed->fetchInterval());
    } else {
        setFetchInterval(Settings::autoFetchInterval());
    }
    setArchiveMode(feed->archiveMode());
    setMaxArticleAge(feed->maxArticleAge());
    setMaxArticleNumber(feed->maxArticleNumber());
    setMarkImmediatelyAsRead(feed->markImmediatelyAsRead());
    setUseNotification(feed->useNotification());
    setLoadLinkedWebsite(feed->loadLinkedWebsite());
    setComment(feed->comment());
    slotSetWindowTitle(feedName());
#if HAVE_ACTIVITY_SUPPORT
    setActivitiesSettings({feed->activities(), feed->activityEnabled()});
#endif
}

#if HAVE_ACTIVITY_SUPPORT
ActivitiesManager::ActivitySettings FeedPropertiesDialog::activitySettings() const
{
    return mFeedPropertiesWidget->activitySettings();
}

void FeedPropertiesDialog::setActivitiesSettings(const ActivitiesManager::ActivitySettings &activitySettings)
{
    mFeedPropertiesWidget->setActivitiesSettings(activitySettings);
}
#endif

QString FeedPropertiesDialog::comment() const
{
    return mFeedPropertiesWidget->commentEdit->text();
}

void FeedPropertiesDialog::setComment(const QString &comment)
{
    mFeedPropertiesWidget->commentEdit->setText(comment);
}

QString FeedPropertiesDialog::feedName() const
{
    return mFeedPropertiesWidget->feedNameEdit->text();
}

QString FeedPropertiesDialog::url() const
{
    return mFeedPropertiesWidget->urlEdit->text();
}

bool FeedPropertiesDialog::autoFetch() const
{
    return mFeedPropertiesWidget->cb_updateInterval->isChecked();
}

int FeedPropertiesDialog::fetchInterval() const
{
    switch (mFeedPropertiesWidget->updateComboBox->currentIndex()) {
    case FeedPropertiesWidget::Minutes:
        return mFeedPropertiesWidget->updateSpinBox->value();
    case FeedPropertiesWidget::Hours:
        return mFeedPropertiesWidget->updateSpinBox->value() * 60;
    case FeedPropertiesWidget::Days: {
        constexpr int value = 60 * 24;
        return mFeedPropertiesWidget->updateSpinBox->value() * value;
    }
    case FeedPropertiesWidget::Never:
    default:
        return -1; // never
    }
}

Feed::ArchiveMode FeedPropertiesDialog::archiveMode() const
{
    // i could check the button group's int, but order could change...
    if (mFeedPropertiesWidget->rb_globalDefault->isChecked()) {
        return Feed::globalDefault;
    }

    if (mFeedPropertiesWidget->rb_keepAllArticles->isChecked()) {
        return Feed::keepAllArticles;
    }

    if (mFeedPropertiesWidget->rb_limitArticleAge->isChecked()) {
        return Feed::limitArticleAge;
    }

    if (mFeedPropertiesWidget->rb_limitArticleNumber->isChecked()) {
        return Feed::limitArticleNumber;
    }

    if (mFeedPropertiesWidget->rb_disableArchiving->isChecked()) {
        return Feed::disableArchiving;
    }

    // in a perfect world, this is never reached

    return Feed::globalDefault;
}

int FeedPropertiesDialog::maxArticleAge() const
{
    return mFeedPropertiesWidget->sb_maxArticleAge->value();
}

int FeedPropertiesDialog::maxArticleNumber() const
{
    return mFeedPropertiesWidget->sb_maxArticleNumber->value();
}

void FeedPropertiesDialog::setArchiveMode(Feed::ArchiveMode mode)
{
    switch (mode) {
    case Feed::globalDefault:
        mFeedPropertiesWidget->rb_globalDefault->setChecked(true);
        break;
    case Feed::keepAllArticles:
        mFeedPropertiesWidget->rb_keepAllArticles->setChecked(true);
        break;
    case Feed::disableArchiving:
        mFeedPropertiesWidget->rb_disableArchiving->setChecked(true);
        break;
    case Feed::limitArticleAge:
        mFeedPropertiesWidget->rb_limitArticleAge->setChecked(true);
        break;
    case Feed::limitArticleNumber:
        mFeedPropertiesWidget->rb_limitArticleNumber->setChecked(true);
    }
}

void FeedPropertiesDialog::setFeedName(const QString &title)
{
    mFeedPropertiesWidget->feedNameEdit->setText(title);
}

void FeedPropertiesDialog::setUrl(const QString &url)
{
    mFeedPropertiesWidget->urlEdit->setText(url);
}

void FeedPropertiesDialog::setAutoFetch(bool customFetchEnabled)
{
    mFeedPropertiesWidget->cb_updateInterval->setChecked(customFetchEnabled);
    mFeedPropertiesWidget->updateComboBox->setEnabled(customFetchEnabled);

    if (mFeedPropertiesWidget->updateSpinBox->value() > -1) {
        mFeedPropertiesWidget->updateSpinBox->setEnabled(customFetchEnabled);
    } else {
        mFeedPropertiesWidget->updateSpinBox->setEnabled(false);
    }
}

void FeedPropertiesDialog::setFetchInterval(int interval)
{
    if (interval == -1) { // never update
        mFeedPropertiesWidget->updateSpinBox->setValue(0);
        mFeedPropertiesWidget->updateSpinBox->setDisabled(true);
        mFeedPropertiesWidget->updateComboBox->setCurrentIndex(FeedPropertiesWidget::Never);
        return;
    }

    if (interval == 0) {
        mFeedPropertiesWidget->updateSpinBox->setValue(0);
        mFeedPropertiesWidget->updateSpinBox->setEnabled(mFeedPropertiesWidget->cb_updateInterval->isChecked());
        mFeedPropertiesWidget->updateComboBox->setCurrentIndex(FeedPropertiesWidget::Minutes);
        return;
    }

    constexpr int value = 60 * 24;
    if (interval % (value) == 0) {
        mFeedPropertiesWidget->updateSpinBox->setValue(interval / (value));
        mFeedPropertiesWidget->updateSpinBox->setEnabled(mFeedPropertiesWidget->cb_updateInterval->isChecked());
        mFeedPropertiesWidget->updateComboBox->setCurrentIndex(FeedPropertiesWidget::Days);
        return;
    }

    if (interval % 60 == 0) {
        mFeedPropertiesWidget->updateSpinBox->setValue(interval / 60);
        mFeedPropertiesWidget->updateSpinBox->setEnabled(mFeedPropertiesWidget->cb_updateInterval->isChecked());
        mFeedPropertiesWidget->updateComboBox->setCurrentIndex(FeedPropertiesWidget::Hours);
        return;
    }

    mFeedPropertiesWidget->updateSpinBox->setValue(interval);
    mFeedPropertiesWidget->updateSpinBox->setEnabled(mFeedPropertiesWidget->cb_updateInterval->isChecked());
    mFeedPropertiesWidget->updateComboBox->setCurrentIndex(FeedPropertiesWidget::Minutes);
}

void FeedPropertiesDialog::setMaxArticleAge(int age)
{
    mFeedPropertiesWidget->sb_maxArticleAge->setValue(age);
}

void FeedPropertiesDialog::setMaxArticleNumber(int number)
{
    mFeedPropertiesWidget->sb_maxArticleNumber->setValue(number);
}

void FeedPropertiesDialog::setMarkImmediatelyAsRead(bool enabled)
{
    mFeedPropertiesWidget->checkBox_markRead->setChecked(enabled);
}

bool FeedPropertiesDialog::markImmediatelyAsRead() const
{
    return mFeedPropertiesWidget->checkBox_markRead->isChecked();
}

void FeedPropertiesDialog::setUseNotification(bool enabled)
{
    mFeedPropertiesWidget->checkBox_useNotification->setChecked(enabled);
}

bool FeedPropertiesDialog::useNotification() const
{
    return mFeedPropertiesWidget->checkBox_useNotification->isChecked();
}

bool FeedPropertiesDialog::loadLinkedWebsite() const
{
    return mFeedPropertiesWidget->checkBox_loadWebsite->isChecked();
}

void FeedPropertiesDialog::setLoadLinkedWebsite(bool enabled)
{
    mFeedPropertiesWidget->checkBox_loadWebsite->setChecked(enabled);
}

void FeedPropertiesDialog::selectFeedName()
{
    mFeedPropertiesWidget->feedNameEdit->selectAll();
}

#include "moc_feedpropertiesdialog.cpp"
