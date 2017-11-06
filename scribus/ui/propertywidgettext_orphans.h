/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_ORPHANS_H
#define PROPERTYWIDGET_ORPHANS_H

#include "ui_propertywidget_orphansbase.h"

#include "propertywidgetbase.h"


class ParagraphStyle;


class PropertyWidgetText_Orphans : public QWidget, Ui::PropertyWidget_OrphansBase,
		public PropertyWidgetBase
{
	Q_OBJECT

public:

	PropertyWidgetText_Orphans(QWidget *parent = 0);
	~PropertyWidgetText_Orphans() {};

	void updateStyle(const ParagraphStyle& newCurrent);

protected:
	virtual void changeEvent(QEvent *e);

public slots:
	void languageChange();
	void handleKeepLinesStart();
	void handleKeepLinesEnd();
	void handleKeepTogether();
	void handleKeepWithNext();

private:
	void connectSignals();
	void disconnectSignals();
};

#endif
