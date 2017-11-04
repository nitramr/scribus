#ifndef SCLAYOUTSEGMENT_H
#define SCLAYOUTSEGMENT_H

#include "scribusapi.h"
#include "scribusstructs.h"

#include <QWidget>
#include <QLayout>

class SCRIBUS_API ScLayoutSegment : public QWidget
{

	Q_OBJECT

private:
	QVBoxLayout * m_layout;

public:
	ScLayoutSegment(QWidget *parent = 0);
	~ScLayoutSegment() {};
//	void addWidget(QWidget * item, int row, int column);
//	void addGridLayout(QGridLayout*gridLayout);
//	QGridLayout *getLayout();
};

#endif // SCLAYOUTSEGMENT_H
