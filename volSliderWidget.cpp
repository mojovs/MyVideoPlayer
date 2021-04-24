#include "volSliderWidget.h"

#include "ui_volSliderWidget.h"

volSliderWidget::volSliderWidget(QWidget* parent) : QWidget(parent), ui(new Ui::volSliderWidget) { ui->setupUi(this); }

volSliderWidget::~volSliderWidget() { delete ui; }

void volSliderWidget::on_verticalSlider_sliderReleased() {
    this->value = ui->verticalSlider->value();    //得到滑动条的值
    emit setVol(value);
}
