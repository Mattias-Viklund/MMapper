// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2019 The MMapper Authors
// Author: Nils Schimmelmann <nschimme@gmail.com> (Jahara)

#include "groupmanagerpage.h"

#include <QSet>
#include <QString>
#include <QtGui>
#include <QtWidgets>

#include "../configuration/configuration.h"
#include "../pandoragroup/CGroup.h"
#include "../pandoragroup/groupauthority.h"
#include "../pandoragroup/mmapper2group.h"
#include "AnsiColorDialog.h"
#include "ansicombo.h"
#include "ui_groupmanagerpage.h"

GroupManagerPage::GroupManagerPage(Mmapper2Group *gm, QWidget *parent)
    : QWidget(parent)
    , m_groupManager(gm)
    , ui(new Ui::GroupManagerPage)
{
    ui->setupUi(this);
    auto authority = m_groupManager->getAuthority();
    connect(this, &GroupManagerPage::refresh, authority, &GroupAuthority::refresh);
    connect(authority, &GroupAuthority::secretRefreshed, this, [this](const GroupSecret &secret) {
        ui->secretLineEdit->setText(secret);
        ui->refreshButton->setEnabled(true);
    });

    // Character Section
    connect(ui->charName, &QLineEdit::editingFinished, this, &GroupManagerPage::charNameTextChanged);
    connect(ui->changeColor, &QAbstractButton::clicked, this, &GroupManagerPage::changeColorClicked);
    connect(ui->refreshButton, &QAbstractButton::clicked, this, [this]() {
        ui->refreshButton->setEnabled(false);
        // Refreshing the SSL certificate asynchronously
        emit refresh();
    });

    // Authorized Secrets Section
    connect(ui->authorizationCheckBox, &QCheckBox::clicked, this, [this](const bool checked) {
        ui->allowedComboBox->setEnabled(checked);
        if (!checked) {
            ui->allowedComboBox->setCurrentText("");
        }
        setConfig().groupManager.requireAuth = checked;
        allowedSecretsChanged();
    });
    connect(ui->allowedComboBox,
            &QComboBox::editTextChanged,
            this,
            &GroupManagerPage::allowedSecretsChanged);
    connect(authority->getItemModel(), &QAbstractItemModel::dataChanged, this, [this]() {
        allowedSecretsChanged();
    });
    connect(ui->allowSecret, &QPushButton::pressed, this, [this]() {
        auto authority = m_groupManager->getAuthority();
        auto secret = ui->allowedComboBox->currentText().simplified().toLatin1();
        authority->add(secret);
    });
    connect(ui->revokeSecret, &QPushButton::pressed, this, [this]() {
        auto authority = m_groupManager->getAuthority();
        auto secret = ui->allowedComboBox->currentText().simplified().toLatin1();
        authority->remove(secret);
    });

    // Host Section
    connect(ui->localHost, &QLabel::linkActivated, this, [](const QString &link) {
        QDesktopServices::openUrl(QUrl::fromEncoded(link.toLatin1()));
    });
    connect(ui->localPort,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &GroupManagerPage::localPortValueChanged);
    connect(ui->shareSelfCheckBox,
            &QCheckBox::stateChanged,
            this,
            &GroupManagerPage::shareSelfChanged);
    connect(ui->localPort,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &GroupManagerPage::localPortValueChanged);
    connect(ui->shareSelfCheckBox,
            &QCheckBox::stateChanged,
            this,
            &GroupManagerPage::shareSelfChanged);
    connect(ui->lockGroupCheckBox, &QCheckBox::stateChanged, this, [this]() {
        setConfig().groupManager.lockGroup = ui->lockGroupCheckBox->isChecked();
    });

    connect(ui->remoteHost,
            &QComboBox::editTextChanged,
            this,
            &GroupManagerPage::remoteHostTextChanged);

    // Group Tells Section
    connect(ui->groupTellColorPushButton, &QPushButton::pressed, this, [this]() {
        QString ansiString = AnsiColorDialog::getColor(getConfig().groupManager.groupTellColor,
                                                       this);
        AnsiCombo::makeWidgetColoured(ui->groupTellColorLabel, ansiString, false);
        setConfig().groupManager.groupTellColor = ansiString;
    });
    connect(ui->groupTellColorAnsi256RadioButton, &QAbstractButton::toggled, this, [this]() {
        setConfig().groupManager.useGroupTellAnsi256Color = ui->groupTellColorAnsi256RadioButton
                                                                ->isChecked();
    });

    // Other Sections
    connect(ui->rulesWarning,
            &QCheckBox::stateChanged,
            this,
            &GroupManagerPage::rulesWarningChanged);

    // Inform Group Manager of changes
    connect(this, &GroupManagerPage::updatedSelf, m_groupManager, &Mmapper2Group::updateSelf);
}

GroupManagerPage::~GroupManagerPage()
{
    delete ui;
}

void GroupManagerPage::loadConfig()
{
    const Configuration::GroupManagerSettings &settings = getConfig().groupManager;
    const auto authority = m_groupManager->getAuthority();
    const auto &itemModel = authority->getItemModel();
    ui->charName->setText(settings.charName);
    QPixmap charColorPixmap(16, 16);
    charColorPixmap.fill(settings.color);
    ui->changeColor->setIcon(QIcon(charColorPixmap));
    ui->secretLineEdit->setText(authority->getSecret());
    ui->secretLineEdit->setEnabled(!NO_OPEN_SSL);
    ui->refreshButton->setEnabled(!NO_OPEN_SSL);
    ui->authorizationCheckBox->setChecked(settings.requireAuth);
    ui->authorizationCheckBox->setEnabled(!NO_OPEN_SSL);
    ui->allowSecret->setEnabled(false);
    ui->revokeSecret->setEnabled(false);
    ui->allowedComboBox->setEnabled(settings.requireAuth);
    ui->allowedComboBox->setModel(itemModel);
    ui->allowedComboBox->setEditText("");
    allowedSecretsChanged();

    ui->localPort->setValue(settings.localPort);
    ui->shareSelfCheckBox->setChecked(settings.shareSelf);
    ui->lockGroupCheckBox->setChecked(settings.lockGroup);

    // Client Section
    const auto oldText = ui->remoteHost->currentText();
    ui->remoteHost->clear();
    QSet<QString> contacts;
    for (int i = 0; i < itemModel->rowCount(); i++) {
        // Pre-populate entries from Authorized Contacts
        const auto key = itemModel->index(i, 0).data(Qt::DisplayRole).toByteArray();
        const auto ip = authority->getMetadata(key, GroupMetadataEnum::IP_ADDRESS);
        const auto port = authority->getMetadata(key, GroupMetadataEnum::PORT).toInt();
        if (ip.isEmpty() || port <= 0)
            continue;
        const auto remoteHostText = QString("%1:%2").arg(ip).arg(port);
        if (contacts.contains(remoteHostText))
            continue;
        contacts.insert(remoteHostText);
        ui->remoteHost->addItem(remoteHostText);
        const auto name = authority->getMetadata(key, GroupMetadataEnum::NAME);
        ui->remoteHost->setItemData(i, name.isEmpty() ? "Unknown" : name, Qt::ToolTipRole);
    }
    const auto remoteHostText
        = QString("%1:%2").arg(settings.host.constData()).arg(settings.remotePort);
    if (!contacts.contains(remoteHostText)) {
        // Add the entry from config if it wasn't already prepopulated
        ui->remoteHost->addItem(remoteHostText);
        ui->remoteHost->setItemData(ui->remoteHost->count() - 1, "Unknown", Qt::ToolTipRole);
        ui->remoteHost->setCurrentIndex(ui->remoteHost->count() - 1);
    } else {
        for (int i = 0; i < ui->remoteHost->count(); i++) {
            const auto itemText = ui->remoteHost->itemText(i);
            if (remoteHostText.compare(itemText, Qt::CaseInsensitive) == 0) {
                ui->remoteHost->setCurrentIndex(i);
                break;
            }
        }
    }
    if (int index = ui->remoteHost->findText(oldText) && !oldText.isEmpty())
        ui->remoteHost->setCurrentIndex(index);

    ui->groupTellColorAnsi256RadioButton->setChecked(settings.useGroupTellAnsi256Color);
    AnsiCombo::makeWidgetColoured(ui->groupTellColorLabel, settings.groupTellColor, false);

    ui->rulesWarning->setChecked(settings.rulesWarning);
}

void GroupManagerPage::charNameTextChanged()
{
    // REVISIT: Remove non-valid characters (numbers, punctuation, etc)
    const QByteArray newName = ui->charName->text().toLatin1().simplified();

    // Correct any UTF-8 characters that we do not understand
    QString newNameStr = QString::fromLatin1(newName);

    // Force first character to be uppercase
    if (newNameStr.length() > 0)
        newNameStr.replace(0, 1, newNameStr[0].toUpper());

    // Apply corrections back to the input field
    if (ui->charName->text() != newNameStr) {
        ui->charName->setText(newNameStr);
    }

    setConfig().groupManager.charName = newNameStr.toLatin1();
    emit updatedSelf();
}

void GroupManagerPage::changeColorClicked()
{
    QColor &savedColor = setConfig().groupManager.color;
    const QColor newColor = QColorDialog::getColor(savedColor, this);
    if (newColor.isValid() && newColor != savedColor) {
        QPixmap charColorPixmap(16, 16);
        charColorPixmap.fill(newColor);
        ui->changeColor->setIcon(QIcon(charColorPixmap));
        savedColor = newColor;

        emit updatedSelf();
    }
}

void GroupManagerPage::allowedSecretsChanged()
{
    static constexpr const int SHA1_LENGTH = 40;
    const auto authority = m_groupManager->getAuthority();
    const auto secretText = ui->allowedComboBox->currentText().simplified();
    bool correctLength = secretText.length() == SHA1_LENGTH;
    bool isSelf = secretText.compare(authority->getSecret(), Qt::CaseInsensitive) == 0;
    bool alreadyPresent = authority->validSecret(secretText.toLatin1());

    bool enableAllowSecret = correctLength && !alreadyPresent && !isSelf;
    if (ui->allowSecret->hasFocus() && !enableAllowSecret)
        ui->allowedComboBox->setFocus();
    ui->allowSecret->setEnabled(enableAllowSecret);

    bool enableRevokeSecret = correctLength && alreadyPresent && !isSelf;
    if (ui->revokeSecret->hasFocus() && !enableRevokeSecret)
        ui->allowedComboBox->setFocus();
    ui->revokeSecret->setEnabled(enableRevokeSecret);

    if (correctLength && alreadyPresent) {
        const auto key = secretText.toLatin1();
        const auto lastLogin = authority->getMetadata(key, GroupMetadataEnum::LAST_LOGIN);
        QString line;
        if (lastLogin.isEmpty()) {
            line = "<i>Never seen before</i>";
        } else {
            const auto name = authority->getMetadata(key, GroupMetadataEnum::NAME);
            const auto ip = authority->getMetadata(key, GroupMetadataEnum::IP_ADDRESS);
            const auto port = authority->getMetadata(key, GroupMetadataEnum::PORT).toInt();
            line = QString("<i>Last seen %1%2 from %3</i>")
                       .arg(lastLogin)
                       .arg(name.isEmpty() ? "" : QString(" as '%2'").arg(name))
                       .arg(port > 0 ? QString("%1:%2").arg(ip).arg(port) : ip);
        }
        ui->secretMetadataLabel->setText(line);
    } else {
        ui->secretMetadataLabel->setText("");
    }

    ui->secretCountLabel->setText(QString("<i>%1 contact%2 found</i>")
                                      .arg(ui->allowedComboBox->count())
                                      .arg(ui->allowedComboBox->count() == 1 ? "" : "s"));
}

void GroupManagerPage::remoteHostTextChanged()
{
    const auto currentText = ui->remoteHost->currentText().simplified().toLatin1();
    const auto parts = currentText.split(':');
    if (parts.size() == 2) {
        const auto currentHost = parts[0];
        auto &savedHost = setConfig().groupManager.host;
        if (currentHost != savedHost) {
            savedHost = currentHost;
        }
        auto &savedRemotePort = setConfig().groupManager.remotePort;
        const auto currentRemotePort = static_cast<quint16>(QString(parts[1]).toInt());
        if (currentRemotePort != savedRemotePort) {
            savedRemotePort = currentRemotePort;
        }
    }
}

void GroupManagerPage::localPortValueChanged(int /*unused*/)
{
    auto &savedLocalPort = setConfig().groupManager.localPort;
    const auto currentLocalPort = static_cast<quint16>(ui->localPort->value());
    if (currentLocalPort != savedLocalPort) {
        savedLocalPort = currentLocalPort;
    }
}

void GroupManagerPage::rulesWarningChanged(int /*unused*/)
{
    setConfig().groupManager.rulesWarning = ui->rulesWarning->isChecked();
}

void GroupManagerPage::shareSelfChanged(int /*unused*/)
{
    setConfig().groupManager.shareSelf = ui->shareSelfCheckBox->isChecked();
}
