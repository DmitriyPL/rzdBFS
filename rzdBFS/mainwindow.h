#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include<QDebug>

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>

#include "stationmap.h"
#include "ui_stationmap.h"

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

private:
    Ui::MainWindow *ui;

    stationmap *map;

    QMap< int, QVector<int> > graph;
    QVector <int> neighbourhood;
    QList <int> queue;
    QSet <int> chekedGP;

    int GPsearch;
    int pathLenth;

    bool find;
};

#endif // MAINWINDOW_H
