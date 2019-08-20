// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2019 The MMapper Authors

#include "SignalBlocker.h"

#include <cassert>
#include <QWidget>

#include "utils.h"

SignalBlocker::SignalBlocker(QObject &in)
    : obj{in}
    , wasBlocked{obj.signalsBlocked()}
{
    if (!wasBlocked) {
        obj.blockSignals(true);
    }
}

SignalBlocker::~SignalBlocker()
{
    if (!wasBlocked) {
        obj.blockSignals(false);
    }
}
