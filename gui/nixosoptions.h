#ifndef NIXOSOPTIONS_H
#define NIXOSOPTIONS_H

#include <QWidget>

namespace Ui {
class NixOSOptions;
}

class NixOSOptions : public QWidget
{
    Q_OBJECT

public:
    explicit NixOSOptions(QWidget *parent = 0);
    ~NixOSOptions();

private:
    Ui::NixOSOptions *ui;
};

#endif // NIXOSOPTIONS_H
