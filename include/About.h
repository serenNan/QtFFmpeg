#pragma once
#include "ui_About.h"
#include <QDialog>

class About : public QDialog
{
    Q_OBJECT
    
public:
    About(QWidget* parent = nullptr);
    ~About();

private:
    Ui_About* ui;
};