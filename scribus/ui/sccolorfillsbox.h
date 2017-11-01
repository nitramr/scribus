#ifndef SCCOLORFILLSBOX_H
#define SCCOLORFILLSBOX_H

#include <QPushButton>
#include <QMenu>

class ScColorFillsBox : public QPushButton
{
	Q_OBJECT
public:
	ScColorFillsBox(QWidget *parent = 0);
		~ScColorFillsBox() {}



private:
	QPixmap * m_fills;
	bool m_reset;
	void paintEvent(QPaintEvent *);

public slots:
	void setPixmap(QPixmap fills);
	void resetColor();
};

#endif // SCCOLORFILLSBOX_H
