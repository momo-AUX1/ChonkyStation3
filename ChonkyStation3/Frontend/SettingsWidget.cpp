#include "SettingsWidget.hpp"


SettingsWidget::SettingsWidget(PlayStation3* ps3, QWidget* parent) : QWidget(parent, Qt::Window), ps3(ps3) {
    ui.setupUi(this);

    // Initialize settings
    
    // System
    ui.nickname->setText(QString::fromStdString(ps3->settings.system.nickname));

    // LLE
    ui.partialLv2->setChecked(ps3->settings.lle.partialLv2LLE);
    ui.sys_fs->setChecked(ps3->settings.lle.sys_fs);
    ui.cellResc->setChecked(ps3->settings.lle.cellResc);
    ui.cellPngDec->setChecked(ps3->settings.lle.cellPngDec);
    ui.cellSpurs->setChecked(ps3->settings.lle.cellSpurs);
    ui.cellSpursJq->setChecked(ps3->settings.lle.cellSpursJq);

    // Debug
    ui.disableSPU->setChecked(ps3->settings.debug.disableSPU);
    ui.enableSPUAfterPC->setText(QString::fromStdString(ps3->settings.debug.enableSPUAfterPC));
    ui.spuThreadToEnable->setText(QString::fromStdString(ps3->settings.debug.spuThreadToEnable));

    // Setup events
    connect(ui.apply, &QPushButton::clicked, this, [this, ps3]() {
        ps3->settings.system.nickname   = ui.nickname->text().toStdString();

        ps3->settings.lle.sys_fs        = ui.sys_fs->isChecked();
        ps3->settings.lle.cellResc      = ui.cellResc->isChecked();
        ps3->settings.lle.cellPngDec    = ui.cellPngDec->isChecked();
        ps3->settings.lle.cellSpurs     = ui.cellSpurs->isChecked();
        ps3->settings.lle.cellSpursJq   = ui.cellSpursJq->isChecked();

        ps3->settings.debug.disableSPU          = ui.disableSPU->isChecked();
        ps3->settings.debug.enableSPUAfterPC    = ui.enableSPUAfterPC->text().toStdString();
        ps3->settings.debug.spuThreadToEnable   = ui.spuThreadToEnable->text().toStdString();

        ps3->settings.save();
    });

    setWindowTitle("Settings");
    hide();
}