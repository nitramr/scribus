/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETTEXT_DISTANCE_H
#define PROPERTYWIDGETTEXT_DISTANCE_H

#include "ui_propertywidgettext_distancebase.h"
#include "propertywidgetbase.h"


class ScribusDoc;
class ScribusMainWindow;

class PropertyWidgetText_Distance : public QWidget, public Ui::PropertyWidget_DistanceBase,
                                public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetText_Distance(QWidget *parent = 0);
	~PropertyWidgetText_Distance() {};

protected:
	void connectSignals();
	void disconnectSignals();

	PageItem *         m_item;
	ScribusMainWindow* m_ScMW;

	double m_unitRatio;
	int    m_unitIndex;

	void configureWidgets(void);
	void setCurrentItem(PageItem *item);

	virtual void changeEvent(QEvent *e);

public slots:
	void setMainWindow(ScribusMainWindow *mw);
	void setDoc(ScribusDoc *d);

	void handleAppModeChanged(int oldMode, int mode);
	void handleSelectionChanged();
	void handleUpdateRequest(int);

	void languageChange();
	void unitChange();

	void showColumns(int r, double g);
	void showTextDistances(double left, double top, double bottom, double right);

private slots:
	void handleColumns();
	void handleColumnGap();
	void handleGapSwitch();
	void handleTextDistances();
};

#endif
