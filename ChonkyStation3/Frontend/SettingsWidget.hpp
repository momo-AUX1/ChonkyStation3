#pragma once

#include <common.hpp>

#include <Frontend/GameWindow.hpp>
#include "Frontend/UI/ui_settings.h"
#include <QtWidgets>
#include <PlayStation3.hpp>


class SettingsWidget : public QWidget {
    Q_OBJECT

public:
    SettingsWidget(PlayStation3* ps3, QWidget* parent = nullptr);
    PlayStation3* ps3;
    Ui::Settings ui;
};