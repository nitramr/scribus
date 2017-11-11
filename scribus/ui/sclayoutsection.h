#ifndef SCLAYOUTSECTION_H
#define SCLAYOUTSECTION_H

#include "scribusapi.h"
#include "scribusstructs.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "flowlayout.h"

class SCRIBUS_API ScLayoutSectionHeader : public QWidget
{
	Q_OBJECT
public:
	ScLayoutSectionHeader(QString text, QWidget *menu = 0, bool toggle = false, QWidget *parent = 0);
	~ScLayoutSectionHeader() {};

	void setToggleOn(bool isOn);
	void setTitle(QString text);

private:
	void paintEvent(QPaintEvent *);
	QLabel *m_caption;
	QPushButton *m_btnExtended;
	QPushButton *m_btnOnOff;
	bool protect;

signals:
	void toggleState(bool);

public slots:
	void setToggleState();

private slots:
	void protectSwitch();


};


class SCRIBUS_API ScLayoutSection : public QWidget
{
	Q_OBJECT
public:
	ScLayoutSection(QString text, QWidget *menu = 0, bool toggle = false, QWidget *parent = 0);
	~ScLayoutSection() {};

	void addWidget(QWidget * item);

	void setTitle(QString text);

private:
	FlowLayout *m_flowLayout;
	ScLayoutSectionHeader * m_header;


signals:
	void toggleState(bool);

public slots:

	void setToggleOn(bool isOn);

private slots:
	void setToggleIsOn(bool);


};

#endif // SCLAYOUTSECTION_H
