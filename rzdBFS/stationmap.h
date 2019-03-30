#ifndef STATIONMAP_H
#define STATIONMAP_H

#include <QWidget>
#include <QPrinter>

namespace Ui {
class stationmap;
}

class stationmap : public QWidget
{
    Q_OBJECT

public:
    explicit stationmap(QWidget *parent = nullptr);
    ~stationmap();

private:
    Ui::stationmap *ui;
};

#endif // STATIONMAP_H
