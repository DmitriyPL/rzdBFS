#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include<QDebug>

#include "qcustomplot.h"

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_comboBox_GPstart_currentIndexChanged(const QString &arg1);

    void on_comboBox_GPend_currentIndexChanged(const QString &arg1);

    void on_pushButton_BFS_clicked();

    void on_pushButton_station_map_clicked();

private:
    Ui::MainWindow *ui;

    QMap< int, QVector<int> > graph;
    QVector <int> neighbourhood;
    QVector <double> GPx, GPy;
    QList <int> queue;
    QSet <int> chekedGP;

    double x1, y1, x2, y2;

    int GPsearch;
    int pathLenth;


    bool find;
};

#endif // MAINWINDOW_H
