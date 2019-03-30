#include "stationmap.h"
#include "ui_stationmap.h"

stationmap::stationmap(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::stationmap)
{
    ui->setupUi(this);


}

stationmap::~stationmap()
{
    delete ui;
};
