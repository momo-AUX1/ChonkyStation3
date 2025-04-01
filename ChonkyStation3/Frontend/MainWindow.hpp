#pragma once

#include <common.hpp>

#include <atomic>
#include <thread>

#include <tinyfiledialogs.h>
#include <Frontend/GameWindow.hpp>
#include "Frontend/UI/ui_mainwindow.h"
#include <QtWidgets>
#include <PlayStation3.hpp>
#include <Loaders/Game/GameLoader.hpp>

#include "SettingsWidget.hpp"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    Ui::Main ui;
    SettingsWidget* settings;

    void setListItem(int row, int column, std::string str);
    void setListIcon(int row, fs::path icon);

    int curr_selection = -1;
    std::atomic<bool> is_game_running = false;
    bool ensureGameNotRunning();

    // Actions
    void launchELF();
    void loadAndLaunchGame(int idx);
    void updateBackgroundImage();

    // Exit callback
    void onExit();

    PlayStation3* ps3;
    GameLoader* game_loader;
    GameWindow* game_window;
    std::thread game_thread;
    void launchGame();
    void gameThread();

private:
    // Overrides
    void resizeEvent(QResizeEvent* event) override;
};