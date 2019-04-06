#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    Xstart(0), Ystart(0), Xend(0), Yend(0),
    Xmin(0), Ymin(0), Xmax(0), Ymax(0),
    GPsearch(0), pathLenth(0), find(false)
{
    ui->setupUi(this);

    //***** Подключаемся к БД *****

    QSqlDatabase dbase;
    dbase = QSqlDatabase::addDatabase("QSQLITE");
    //dbase.setDatabaseName("/home/pl/rzdBFS/rzhd.db");
    dbase.setDatabaseName(R"(D:\Qt\C++\rzdBFS\rzhd.db)");

    if (!dbase.open()) qDebug() << dbase.lastError().text();
    else qDebug() << "Rzd database opened ";


    //****** Формируем графф ********
    FormGraph();


    //********** Настройка параметров отображения графика  ****************
    ui->widget1->setInteraction(QCP::iRangeZoom, true);
    ui->widget1->setInteraction(QCP::iRangeDrag, true);

    ui->widget1->plotLayout()->insertRow(0);
    ui->widget1->plotLayout()->addElement(0, 0, new QCPTextElement(ui->widget1, "Station MAP", QFont("sans", 12, QFont::Bold)));

    ui->widget1->xAxis->setLabel("EASTING METER");
    ui->widget1->yAxis->setLabel("NORTHING METER");

    //********** Отрисовываем станцию **************
    DrawStationMap();

    //****** Подгонка масштаба по факт. значениям X, Y ************
    ui->widget1->xAxis->setRange(Xmin, Xmax);
    ui->widget1->yAxis->setRange(Ymin, Ymax);
    ui->widget1->replot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DrawStationMap()
{
    // ***** Прибераем за собой *********
    if (!StationMap.empty())
    {
        for (auto i : StationMap)
            delete i;

        StationMap.clear();
    }


    // *********** Для последующего масштабирования изображения
    Xstart = 0.0;
    Ystart = 0.0;

    auto qGPminXY = new QSqlQuery();
    qGPminXY->prepare("SELECT EASTING_METER, NORTHING_METER FROM GEO_POINT WHERE ID = ?");
    qGPminXY->addBindValue(ui->comboBox_GPstart->itemText(0).toInt());

    if ( qGPminXY->exec() )
    {
        qGPminXY->next();

        // Устанавливаем начальной значение X, Y для послед. масштабирования. На основании точки старта.
        Xmin = Xmax = qGPminXY->value(0).toDouble();
        Ymin = Ymax = qGPminXY->value(1).toDouble();
    }
    // ***********************


    // Ищем ВСЕ точки старта по всем GEO_LINE
    auto qGPstart = new QSqlQuery();
    if( qGPstart->exec("SELECT ID_GEO_POINT_START FROM GEO_LINE") )
    {
        while(qGPstart->next())
        {
            // Для каждой точки старта ищем координаты X, Y
            auto qGPstartXY = new QSqlQuery();
            qGPstartXY->prepare("SELECT EASTING_METER, NORTHING_METER FROM GEO_POINT WHERE ID = ?");
            qGPstartXY->addBindValue(qGPstart->value(0).toInt());

            if ( qGPstartXY->exec() )
            {
                qGPstartXY->next();
                Xstart = qGPstartXY->value(0).toDouble();
                Ystart = qGPstartXY->value(1).toDouble();
            }

            // Для каждой точки старта ищем точку конца GEO_LINE, их может быть несколько !!!
            auto qGPend = new QSqlQuery();
            qGPend->prepare("SELECT ID_GEO_POINT_END FROM GEO_LINE WHERE ID_GEO_POINT_START = ?");
            qGPend->addBindValue(qGPstart->value(0).toInt());

            if ( qGPend->exec() )
            {
                while( qGPend->next() )
                {
                    // Для каждой точки конца ищем координаты X, Y
                    auto qGPendXY = new QSqlQuery();
                    qGPendXY->prepare("SELECT EASTING_METER, NORTHING_METER FROM GEO_POINT WHERE ID = ?");
                    qGPendXY->addBindValue(qGPend->value(0).toInt());

                    if ( qGPendXY->exec() )
                    {
                        // Создаем объект line на widgite
                        auto *line = new QCPItemLine(ui->widget1);

                        qGPendXY->next();
                        Xend = qGPendXY->value(0).toDouble();
                        Yend = qGPendXY->value(1).toDouble();

                        // Задаем координаты line, Точка старта + Все точки конца, линий может получиться несколько
                        line->start->setCoords(Xstart, Ystart);
                        line->end->setCoords(Xend, Yend);

                        // Задаем стиль отображения линий
                        line->setHead(QCPLineEnding(QCPLineEnding::esSpikeArrow, 8, 10, false) );
                        line->setTail(QCPLineEnding(QCPLineEnding::esDisc, 4, 1, false) );
                        line->setPen( QPen(Qt::black, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin) );

                        // Ищем диапазон X, Y на нашей карты для масштаба
                        if (Xstart <= Xmin) Xmin = Xstart;
                        if (Xend   <= Xmin) Xmin = Xend;
                        if (Ystart <= Ymin) Ymin = Ystart;
                        if (Yend   <= Ymin) Ymin = Yend;

                        if (Xstart >= Xmax) Xmax = Xstart;
                        if (Xend   >= Xmax) Xmax = Xend;
                        if (Ystart >= Ymax) Ymax = Ystart;
                        if (Yend   >= Ymax) Ymax = Yend;

                        // Формируем нашу карту ввиде объектов и запихиваем в вектор, чтобы потом можно было ее чистить
                        StationMap.push_back(line);
                    }
                }
            }
        }
    }
}

// Отрисовываем процес поиска пути
void MainWindow::DrawBFSpath()
{
    // Создаем объект графика и настраиваем
    auto linePathSearch = new QCPItemLine(ui->widget1);
    linePathSearch->setHead(QCPLineEnding(QCPLineEnding::esDisc, 8, 10, false) );
    linePathSearch->setPen(QPen(Qt::blue, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin) );

    // Запрашиваем координаты проверяемой вершины графа и рисуем
    auto qGPsearchXY = new QSqlQuery();
    qGPsearchXY->prepare("SELECT NORTHING_METER, EASTING_METER FROM GEO_POINT WHERE ID = ?");
    qGPsearchXY->addBindValue(GPsearch);

    if ( qGPsearchXY->exec() )
    {
        qGPsearchXY->next();

        linePathSearch->start->setCoords(qGPsearchXY->value(1).toDouble()-0.1, qGPsearchXY->value(0).toDouble()-0.1); // костылек, т.к. не добавить точку
        linePathSearch->end->setCoords(qGPsearchXY->value(1).toDouble(), qGPsearchXY->value(0).toDouble());
    }

    ui->widget1->replot();

    // Формируем список объектов, что бы можно было его очистить при новом поиске, и стереть с графика
    linePathSearchQue.push_back(linePathSearch);
}

// Формируем граф станции
void MainWindow::FormGraph()
{
    // Находим все точки старта для всех GEO_LINE
    auto qGPstart = new QSqlQuery();
    if( qGPstart->exec("SELECT ID_GEO_POINT_START FROM GEO_LINE") )
    {
        while(qGPstart->next())
        {
            // Заполняем comboBox при старте приложения
            ui->comboBox_GPstart->addItem(qGPstart->value(0).toString());
            ui->comboBox_GPend->addItem(qGPstart->value(0).toString());

            // Для каждой точки старта находим все точки конца (соседей)
            auto qGPneighbourhoods = new QSqlQuery();
            qGPneighbourhoods->prepare("SELECT ID_GEO_POINT_END FROM GEO_LINE WHERE ID_GEO_POINT_START = ?");
            qGPneighbourhoods->addBindValue(qGPstart->value(0).toInt());

            if ( qGPneighbourhoods->exec() )
            {
                while(qGPneighbourhoods->next())
                {
                    // Для каждой точки старта формируем список соседей
                    neighbourhood.push_back(qGPneighbourhoods->value(0).toInt());
                }
            }

            // Формируем граф - точка старта + список соседей ( map ( точка старат, (сосед1, сосед2) ) )
            graph.insert( qGPstart->value(0).toInt(), neighbourhood );

            // Очищаем список соседей для след. точки старта
            neighbourhood.clear();
        }
    }
}

// Задаем точку старта
void MainWindow::on_comboBox_GPstart_currentIndexChanged(const QString &arg1)
{
    // Создаем объект line, что бы визуально отобразить точку старта
    auto lineTaskStart = new QCPItemLine(ui->widget1);
    lineTaskStart->setHead(QCPLineEnding(QCPLineEnding::esDisc, 8, 10, false) );
    lineTaskStart->setPen(QPen(Qt::red, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin) );

    // Если пользователь меняет точку старта, удаляем старую с графика
    if (!lineStartQue.empty())
    {
        auto d = lineStartQue.at(0);
        lineStartQue.removeFirst();
        delete  d;
        lineStartQue.clear();
    }

    // Ищем координаты точки старта для отображения
    auto qGPstartXY = new QSqlQuery();
    qGPstartXY->prepare("SELECT NORTHING_METER, EASTING_METER FROM GEO_POINT WHERE ID = ?");
    qGPstartXY->addBindValue(arg1.toInt());

    if ( qGPstartXY->exec() )
    {
        qGPstartXY->next();
        ui->lineEdit_GPstart_X1->setText(qGPstartXY->value(1).toString());
        ui->lineEdit_GPstart_Y1->setText(qGPstartXY->value(0).toString());

        // Рисуем точку старта
        lineTaskStart->start->setCoords( 4950, 5160);
        lineTaskStart->end->setCoords(qGPstartXY->value(1).toDouble(), qGPstartXY->value(0).toDouble());
    }

    ui->widget1->replot();
    // Добавляем в список, чтобы потом можно было очистить
    lineStartQue.push_back(lineTaskStart);
}

// Все аналогично on_comboBox_GPstart_currentIndexChanged
void MainWindow::on_comboBox_GPend_currentIndexChanged(const QString &arg1)
{
    auto lineTaskEnd = new QCPItemLine(ui->widget1);
    lineTaskEnd->setHead(QCPLineEnding(QCPLineEnding::esDisc, 8, 10, false) );
    lineTaskEnd->setPen(QPen(Qt::green, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin) );

    if (!lineEndQue.empty())
    {
        auto d = lineEndQue.at(0);
        lineEndQue.removeFirst();
        delete  d;
        lineEndQue.clear();
    }

    auto qGPendXY = new QSqlQuery();
    qGPendXY->prepare("SELECT NORTHING_METER, EASTING_METER FROM GEO_POINT WHERE ID = ?");
    qGPendXY->addBindValue(arg1.toInt());

    if ( qGPendXY->exec() )
    {
        qGPendXY->next();
        ui->lineEdit_GPend_X2->setText(qGPendXY->value(1).toString());
        ui->lineEdit_GPend_Y2->setText(qGPendXY->value(0).toString());

        lineTaskEnd->start->setCoords( 4950, 5160);
        lineTaskEnd->end->setCoords(qGPendXY->value(1).toDouble(), qGPendXY->value(0).toDouble());
    }

    ui->widget1->replot();
    lineEndQue.push_back(lineTaskEnd);
}


// поиск в ширину
void MainWindow::on_pushButton_BFS_clicked()
{

    // Очищаем результаты предыдущего поиска
    if (!linePathSearchQue.empty())
    {
        for (auto i : linePathSearchQue)
            delete  i;
        linePathSearchQue.clear();
    }

    // Очередь поиска
    queue.clear();

    // Проверенные элементы
    chekedGP.clear();

    // Длина пути - кол-во пройденых ребер
    pathLenth = 0;

    find = false;

    // Добавляем в очередь первый елемент, точку начала поиска заданную пользователем
    queue.push_back(ui->comboBox_GPstart->currentText().toInt());

    // Пока есть что проверять
    while (!queue.empty())
    {
        // Вынимаем из очереди поиска первыелемент
        GPsearch = queue.at(0);
        queue.removeFirst();

        // Ищем элемент в уже проверенных вершинах, если не проверяли идем дальше
        if ( chekedGP.find(GPsearch) == chekedGP.end() )
        {
            // Если нашли, конец поиска
            if ( GPsearch == ui->comboBox_GPend->currentText().toInt() ) { find = true; break; }

            // Увеличиваем путь
            pathLenth++;
            // Добавялем элемент в проверенные
            chekedGP.insert(GPsearch);

            // Добавляем проверенную точку на график
            DrawBFSpath();

            for (auto i : graph.value(GPsearch) )
            {
                // Добавляем в КОНЕЦ очереди поиска всех соседей проверенной вершины
                queue.push_back(i);
            }
        }
    }

    if (find) ui->lineEdit_BFS_result->setText( QString::number(pathLenth) );
    else ui->lineEdit_BFS_result->setText("Не нашли путь");
}
