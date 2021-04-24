#ifndef VOLSLIDERWIDGET_H
#define VOLSLIDERWIDGET_H

#include <QWidget>

namespace Ui {
    class volSliderWidget;
}

class volSliderWidget : public QWidget {
    Q_OBJECT

  public:
    int value = 0;    //音量大小
    explicit volSliderWidget(QWidget* parent = nullptr);
    ~volSliderWidget();

  signals:
    void setVol(int value);
  private slots:

    void on_verticalSlider_sliderReleased();

  private:
    Ui::volSliderWidget* ui;
};

#endif    // VOLSLIDERWIDGET_H
