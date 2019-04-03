#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->widget->setInteraction(QCP::iRangeZoom, true);
    ui->widget->setInteraction(QCP::iRangeDrag, true);

    ui->widget->plotLayout()->insertRow(0);
    ui->widget->plotLayout()->addElement(0, 0, new QCPTextElement(ui->widget, "Station MAP", QFont("sans", 12, QFont::Bold)));

    ui->widget->xAxis->setLabel("EASTING METER");
    ui->widget->yAxis->setLabel("NORTHING METER");

    ui->widget->replot();

    QSqlDatabase dbase;
    dbase = QSqlDatabase::addDatabase("QSQLITE");
    dbase.setDatabaseName(R"(D:\Qt\C++\rzdBFS\rzhd.db)");

    if (!dbase.open()) qDebug() << dbase.lastError().text();
        else qDebug() << "Rzd database opened ";

    auto qGPstart = new QSqlQuery();
    if( qGPstart->exec("SELECT ID_GEO_POINT_START FROM GEO_LINE") )
    {
        while(qGPstart->next())
        {
            ui->comboBox_GPstart->addItem(qGPstart->value(0).toString());
            ui->comboBox_GPend->addItem(qGPstart->value(0).toString());

            auto qGPneighbourhoods = new QSqlQuery();
            qGPneighbourhoods->prepare("SELECT ID_GEO_POINT_END FROM GEO_LINE WHERE ID_GEO_POINT_START = ?");
            qGPneighbourhoods->addBindValue(qGPstart->value(0).toInt());

            if ( qGPneighbourhoods->exec() )
            {
                while(qGPneighbourhoods->next())
                {
                    neighbourhood.push_back(qGPneighbourhoods->value(0).toInt());
                }
            }

            graph.insert( qGPstart->value(0).toInt(), neighbourhood );

            neighbourhood.clear();
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBox_GPstart_currentIndexChanged(const QString &arg1)
{
    auto qGPstartXY = new QSqlQuery();
    qGPstartXY->prepare("SELECT NORTHING_METER, EASTING_METER FROM GEO_POINT WHERE ID = ?");
    qGPstartXY->addBindValue(arg1.toInt());

    if ( qGPstartXY->exec() )
    {
        qGPstartXY->next();
        ui->lineEdit_GPstart_X1->setText(qGPstartXY->value(0).toString());
        ui->lineEdit_GPstart_Y1->setText(qGPstartXY->value(1).toString());
    }
}

void MainWindow::on_comboBox_GPend_currentIndexChanged(const QString &arg1)
{
    auto qGPendXY = new QSqlQuery();
    qGPendXY->prepare("SELECT NORTHING_METER, EASTING_METER FROM GEO_POINT WHERE ID = ?");
    qGPendXY->addBindValue(arg1.toInt());

    if ( qGPendXY->exec() )
    {
        qGPendXY->next();
        ui->lineEdit_GPend_X2->setText(qGPendXY->value(0).toString());
        ui->lineEdit_GPend_Y2->setText(qGPendXY->value(1).toString());
    }
}

void MainWindow::on_pushButton_BFS_clicked()
{
    queue.clear();
    chekedGP.clear();
    pathLenth = 0;
    find = false;

    queue.push_back(ui->comboBox_GPstart->currentText().toInt());

    while (!queue.empty())
    {
        GPsearch = queue.at(0);
        queue.removeFirst();

        if ( chekedGP.find(GPsearch) == chekedGP.end() )
        {
            if ( GPsearch == ui->comboBox_GPend->currentText().toInt() )
            {
                find = true;

                break;

            } else {

                pathLenth++;
                chekedGP.insert(GPsearch);

                for (auto i : graph.value(GPsearch) )
                {
                    queue.push_back(i);
                }
            }
        }
    }

    if (find) ui->lineEdit_BFS_result->setText( QString::number(pathLenth) );
        else ui->lineEdit_BFS_result->setText("Не нашли путь");
}

void MainWindow::on_pushButton_station_map_clicked()
{
    auto *graph1 = new QCPCurve (ui->widget->xAxis, ui->widget->yAxis);
    graph1->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::red, 0.5), QBrush(Qt::white), 5));
    graph1->setPen(QPen(QColor(120, 120, 120), 2));

    auto qGP = new QSqlQuery();
    if( qGP->exec("SELECT ID_GEO_POINT_START, ID_GEO_POINT_END FROM GEO_LINE") )
    {
        while(qGP->next())
        {
            auto qGPstartXY = new QSqlQuery();
            qGPstartXY->prepare("SELECT EASTING_METER, NORTHING_METER FROM GEO_POINT WHERE ID = ?");
            qGPstartXY->addBindValue(qGP->value(0).toInt());

            if ( qGPstartXY->exec() )
            {
                while(qGPstartXY->next())
                {
//                  GPx.push_back(qGPstartXY->value(0).toDouble());
//                  GPy.push_back(qGPstartXY->value(1).toDouble());

                    x1 = qGPstartXY->value(0).toDouble();
                    y1 = qGPstartXY->value(1).toDouble();
                }
            }

            auto qGPendXY = new QSqlQuery();
            qGPendXY->prepare("SELECT EASTING_METER, NORTHING_METER FROM GEO_POINT WHERE ID = ?");
            qGPendXY->addBindValue(qGP->value(1).toInt());

            if ( qGPendXY->exec() )
            {
                while(qGPendXY->next())
                {
//                    GPx.push_back(qGPendXY->value(0).toDouble());
//                    GPy.push_back(qGPendXY->value(1).toDouble());

                    x2 = qGPendXY->value(0).toDouble();
                    y2 = qGPendXY->value(1).toDouble();
                }
            }

            auto *line = new QCPItemLine(ui->widget);
            line->start->setCoords(x1, y1);
            line->end->setCoords(x2, y2);
            ui->widget->item(line);
        }
    }

//    graph1->setData(GPx, GPy);

    ui->widget->rescaleAxes();
    ui->widget->replot();
}
