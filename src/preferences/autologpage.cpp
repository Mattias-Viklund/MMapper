// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2019 The MMapper Authors
// Author: Mattias Viklund <devmew@exedump.com> (Mew_)

#include "autologpage.h"

#include <QSpinBox>
#include <QString>
#include <QtGui>
#include <QtWidgets>

#include "../configuration/configuration.h"
#include "ui_autologpage.h"

AutoLogPage::AutoLogPage(QWidget *const parent)
    : QWidget(parent)
    , ui(new Ui::AutoLogPage)
{
    ui->setupUi(this);

    connect(ui->autoLogCheckBox,
            &QCheckBox::stateChanged,
            this,
            &AutoLogPage::autoLogCheckBoxChanged);
    connect(ui->selectAutoLogLocationButton,
            &QAbstractButton::clicked,
            this,
            &AutoLogPage::selectLogLocationButtonClicked);
    connect(ui->autoLogMaxBytes,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &AutoLogPage::maxLogBytesChanged);
    connect(ui->notifyWhenLogsReachCheckBox,
            QOverload<int>::of(&QCheckBox::stateChanged),
            this,
            &AutoLogPage::notifyWhenLogsReachCheckBoxChanged);
    connect(ui->notifyWhenLogsReachSize,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &AutoLogPage::notifyWhenLogsReachSizeChanged);
}

AutoLogPage::~AutoLogPage()
{
    delete ui;
}

void AutoLogPage::loadConfig()
{
    const auto &config = getConfig().autoLog;
    ui->autoLogCheckBox->setChecked(config.autoLog);
    ui->autoLogLocation->setText(config.autoLogDirectory);
    ui->autoLogMaxBytes->setValue(config.autoLogMaxBytes / 1000000);
    ui->notifyWhenLogsReachSize->setValue(config.notifyWhenLogsReachSize);
    ui->notifyWhenLogsReachCheckBox->setChecked(config.notifyWhenLogsReach);
}

void AutoLogPage::selectLogLocationButtonClicked(int /*unused*/)
{
    auto &config = setConfig().autoLog;
    QString logDirectory = QFileDialog::getExistingDirectory(this,
                                                             "Choose log location ...",
                                                             config.autoLogDirectory);

    if (!logDirectory.isEmpty()) {
        ui->autoLogLocation->setText(logDirectory);
        config.autoLogDirectory = logDirectory;
    }
}

void AutoLogPage::autoLogCheckBoxChanged(int /*unused*/)
{
    setConfig().autoLog.autoLog = ui->autoLogCheckBox->isChecked();
}

void AutoLogPage::maxLogBytesChanged(int /*unused*/)
{
    setConfig().autoLog.autoLogMaxBytes = ui->autoLogMaxBytes->value()
                                          * 1000000; // Convert megabytes to bytes.
}

void AutoLogPage::notifyWhenLogsReachCheckBoxChanged(int /*unused*/)
{
    setConfig().autoLog.notifyWhenLogsReach = ui->notifyWhenLogsReachCheckBox->isChecked();
}

void AutoLogPage::notifyWhenLogsReachSizeChanged(int /*unused*/)
{
    setConfig().autoLog.notifyWhenLogsReachSize = ui->notifyWhenLogsReachSize->value();
}
