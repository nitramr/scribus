#include "sccolorfillsbox.h"

#include <QPainter>

ScColorFillsBox::ScColorFillsBox(QWidget *parent) : QPushButton(parent)
{

	this->setFixedWidth(30);
	this->setMaximumHeight(30);
	this->adjustSize();

	m_reset = true;

	m_fills = new QPixmap(this->size());
	m_fills->fill(Qt::white);


}


void ScColorFillsBox::setColor(QColor fillsColor){

	QPixmap *fills = new QPixmap(this->size());
	fills->fill(Qt::transparent);

	QPainter paint(fills);
	paint.setBrush(QBrush(fillsColor));
	paint.drawRect(fills->rect());

	setPixmap(*fills);
}

void ScColorFillsBox::setPixmap(QPixmap fillsPixmap){
	m_fills = new QPixmap(fillsPixmap);
	m_reset = false;
	update();

}

void ScColorFillsBox::resetColor(){
	m_reset = true;
	update();
}


void ScColorFillsBox::paintEvent(QPaintEvent *){

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QRect rect(0,0, this->width(), this->height());

	QBrush brushBackground, brushPattern, brushPixmap;
	brushBackground = QBrush(Qt::white);
	brushPattern = QBrush(Qt::lightGray);
	brushPixmap = QBrush(*m_fills);

	painter.fillRect(rect, brushBackground);

	// draw fills
	if(m_reset){
		painter.setPen(QColor(255,0,0));
		painter.drawLine(QPoint(0,0),QPoint(this->width(),this->height()));
		painter.drawLine(QPoint(0,this->height()),QPoint(this->width(),0));
	}else{
		int cellSize = 6;
		int cellSizeX = this->width() / (this->width() / cellSize);
		int cellSizeY = this->height() / (this->height() / cellSize);

		for(int y = 0; y < cellSize; y++)
			for(int x = y % 2; x < cellSize; x+=2)
				painter.fillRect(QRect(x * cellSizeX, y * cellSizeY, cellSizeX, cellSizeY),brushPattern);

		painter.fillRect(rect, brushPixmap);

	}

	// draw frame
	painter.setPen(QColor(128,128,128));
	painter.drawRect(rect);

}
