#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QMouseEvent>
#include <QObject>
#include <QSlider>

class MySlider : public QSlider {
    Q_OBJECT
  public:
    explicit MySlider(QWidget* parent = nullptr);
    ~MySlider();
    virtual void mousePressEvent(QMouseEvent* event);

  signals:
};

#endif    // MYSLIDER_H
