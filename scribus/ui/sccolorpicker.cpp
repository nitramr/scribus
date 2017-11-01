#include "sccolorpicker.h"
#include "ui_sccolorpicker.h"
#include <QPainter>

ScColorPicker::ScColorPicker(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ScColorPicker)
{
	ui->setupUi(this);

	this->connect(ui->submitColor, SIGNAL(clicked(bool)), this, SLOT(submitButtonPress()));

}

ScColorPicker::~ScColorPicker()
{
	delete ui;
}


// Slots
void ScColorPicker::submitButtonPress(){

	//tmp color
	QPixmap *fills = new QPixmap(30,24);
	fills->fill(Qt::transparent);

	int red = qrand() % 255;
	int green = qrand() % 255;
	int blue = qrand() % 255;
	int alpha = qrand() % 255;

	QPainter paint(fills);
	paint.setBrush(QBrush(QColor(red,green,blue,alpha)));
	paint.drawRect(fills->rect());

	emit setPreview(QPixmap(*fills));

}
