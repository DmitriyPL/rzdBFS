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

    void DrawStationMap();
    void DrawBFSpath();
    void FormGraph();

private slots:

    void on_comboBox_GPstart_currentIndexChanged(const QString &arg1);

    void on_comboBox_GPend_currentIndexChanged(const QString &arg1);

    void on_pushButton_BFS_clicked();

private:
    Ui::MainWindow *ui;

    QMap< int, QVector<int> > graph;
    QVector <int> neighbourhood;
    QVector <double> GPx, GPy;
    QList <int> queue;
    QSet <int> chekedGP;

    double Xstart, Ystart, Xend, Yend;
    double Xmin, Ymin, Xmax, Ymax;

    QList <QCPItemLine*> lineStartQue, lineEndQue, linePathSearchQue;
    QVector <QCPItemLine*> StationMap;

    int GPsearch;
    int pathLenth;

    bool find;
};

#endif // MAINWINDOW_H
