#include "myslider.h"

MySlider::MySlider(QWidget* parent) : QSlider(parent) {}

MySlider::~MySlider() {}

void MySlider::mousePressEvent(QMouseEvent* event) {
    double pos = event->pos().x() / (double)width();
    setValue(pos * this->maximum());
    QSlider::mousePressEvent(event);    //父类重载处理
}
