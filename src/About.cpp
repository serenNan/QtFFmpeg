#include "About.h"

About::About(QWidget *parent) : QDialog(parent), ui(new Ui_About)
{
    ui->setupUi(this);
}

About::~About()
{
    delete ui;
}