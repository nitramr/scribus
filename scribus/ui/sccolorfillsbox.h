#ifndef SCCOLORFILLSBOX_H
#define SCCOLORFILLSBOX_H

#include "scribusapi.h"
#include "scribusstructs.h"

#include <QPushButton>
#include <QMenu>

class SCRIBUS_API ScColorFillsBox : public QPushButton
{
	Q_OBJECT
public:
	ScColorFillsBox(QWidget *parent = 0);
		~ScColorFillsBox() {};



private:
	QPixmap * m_fills;
	bool m_reset;
	void paintEvent(QPaintEvent *);

public slots:
	void setPixmap(QPixmap fillsPixmap);
	void setColor(QColor fillsColor);
	void resetColor();
};

#endif // SCCOLORFILLSBOX_H
