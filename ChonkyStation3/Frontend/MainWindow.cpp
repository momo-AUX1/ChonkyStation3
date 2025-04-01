#include "MainWindow.hpp"

MainWindow::MainWindow() : QMainWindow() {
    ps3 = new PlayStation3();
    game_loader = new GameLoader(ps3);
    game_window = new GameWindow();
    settings = new SettingsWidget(ps3);
    
    // Qt6 UI
    ui.setupUi(this);

    // Setup menubar buttons
    connect(ui.actionOpen_ELF, &QAction::triggered, this, &MainWindow::launchELF);
    
    connect(ui.actionSystem, &QAction::triggered, this, [this]() {
        settings->ui.tabWidget->setCurrentIndex(0);
        settings->show();
    });
    
    connect(ui.actionLLE, &QAction::triggered, this, [this]() {
        settings->ui.tabWidget->setCurrentIndex(1);
        settings->show();
    });

    connect(ui.actionDebug, &QAction::triggered, this, [this]() {
        settings->ui.tabWidget->setCurrentIndex(2);
        settings->show();
        });

    int row = 0;
    int column = 0;

    // Setup the table widget
    ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui.tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui.tableWidget->setRowCount(game_loader->games.size());
    ui.tableWidget->setColumnCount(4);
    for (int i = 0; i < ui.tableWidget->rowCount(); i++)
        ui.tableWidget->setRowHeight(i, 60);
    ui.tableWidget->setColumnWidth(0, 100);
    ui.tableWidget->setColumnWidth(1, 3000);
    ui.tableWidget->setColumnWidth(2, 100);
    ui.tableWidget->setColumnWidth(3, 100);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.tableWidget->setHorizontalHeaderLabels(QStringList({ "Icon", "Title", "ID", "Version"}));

    connect(ui.tableWidget, &QTableWidget::cellClicked, this, [this](int row, int column) {
        curr_selection = row;
        updateBackgroundImage();
    });
    
    connect(ui.tableWidget, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        curr_selection = row;
        loadAndLaunchGame(row);
    });

    // Populate table
    for (int i = 0; i < game_loader->games.size(); i++) {
        setListIcon(i, ps3->fs.guestPathToHost(game_loader->games[i].content_path / "ICON0.PNG"));
        setListItem(i, 1, game_loader->games[i].title);
        setListItem(i, 2, game_loader->games[i].id);
        if (game_loader->games[i].sfo.strings.contains("VERSION"))
            setListItem(i, 3, (char*)game_loader->games[i].sfo.strings["VERSION"].c_str());
        else
            setListItem(i, 3, "Unknown");
    }

    QPalette palette;
    palette.setColor(QPalette::Highlight, QColor(0, 0, 0, 0));
    setPalette(palette);

    resize(1280, 720);
    setWindowTitle("ChonkyStation3");
    show();
}

void MainWindow::setListItem(int row, int column, std::string str) {
    QTableWidgetItem* item = new QTableWidgetItem();
    QWidget* widget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    QLabel* label = new QLabel(widget);

    layout->setAlignment(Qt::AlignVCenter);
    label->setStyleSheet("color: white; font-size: 16px;");

    label->setText(QString::fromStdString(str));
    layout->addWidget(label);
    widget->setLayout(layout);

    ui.tableWidget->setItem(row, column, item);
    ui.tableWidget->setCellWidget(row, column, widget);
}

void MainWindow::setListIcon(int row, fs::path icon) {
    QTableWidgetItem* item = new QTableWidgetItem();
    QWidget* widget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    QLabel* label = new QLabel(widget);
    
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);
    if (fs::exists(icon)) {
        QImage image = QImage(QString::fromStdString(icon.generic_string())).scaled(100, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(QPixmap::fromImage(image));
    }
    else {
        label->setText("No icon");
    }

    layout->addWidget(label);
    widget->setLayout(layout);

    ui.tableWidget->setItem(row, 0, item);
    ui.tableWidget->setCellWidget(row, 0, widget);
}

bool MainWindow::ensureGameNotRunning() {
    if (is_game_running) {
        QMessageBox* dialog = new QMessageBox();
        dialog->setText("Close the current game before launching a new one!");
        dialog->exec();
    }
    return is_game_running;
}

void MainWindow::launchELF() {
    if (ensureGameNotRunning()) return;

    const char* filters[1] = { "*.elf" };
    const char* path;
    if (path = tinyfd_openFileDialog("Select a Playstation 3 ELF", "", 1, filters, "ELF File", false)) {
        ps3->elf_path = path;
        launchGame();
    }
}

void MainWindow::loadAndLaunchGame(int idx) {
    if (ensureGameNotRunning()) return;
    Helpers::debugAssert(idx >= 0 && idx < game_loader->games.size(), "MainWindow::loadAndLaunchGame: invalid idx");

    ps3->loadGame(game_loader->games[idx]);
    launchGame();
}

void MainWindow::updateBackgroundImage() {
    if (!game_loader) return;
    if (curr_selection < 0 || curr_selection >= game_loader->games.size()) return;

    fs::path local_content_path = ps3->fs.guestPathToHost(game_loader->games[curr_selection].content_path);
    QPalette palette;
    if (fs::exists(local_content_path / "PIC1.PNG")) {
        fs::path image_path = local_content_path / "PIC1.PNG";
        QImage image = QImage(QString::fromStdString(image_path.generic_string()));
        QPixmap pix = QPixmap::fromImage(image).scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap final_pix = QPixmap(size());
        final_pix.fill(Qt::transparent);
        QPainter painter = QPainter(&final_pix);
        painter.setOpacity(0.3);
        painter.drawPixmap(0, 0, pix);
        palette.setBrush(QPalette::Base, QBrush(final_pix));
    }

    palette.setColor(QPalette::Highlight, QColor(0, 0, 0, 0));
    setPalette(palette);
}

void MainWindow::launchGame() {
    ps3->init();
    game_thread = std::thread(&MainWindow::gameThread, this);
    game_thread.detach();
    is_game_running = true;
}

void MainWindow::gameThread() {
    game_window->run(ps3);

    is_game_running = false;
    delete ps3;
    ps3 = new PlayStation3();
}

void MainWindow::onExit() {}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateBackgroundImage();
}