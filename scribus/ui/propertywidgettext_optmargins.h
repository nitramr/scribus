/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_OPTMARGINS_H
#define PROPERTYWIDGET_OPTMARGINS_H

#include "ui_propertywidget_optmarginsbase.h"

#include "propertywidgetbase.h"
#include "scribusdoc.h"

class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;

class PropertyWidgetText_OptMargins : public QWidget, public Ui::PropertyWidget_OptMarginsBase,
                                  public PropertyWidgetBase
{
	Q_OBJECT

public:

	enum FlopButtonID
	{
		RealHeightID   = 0,
		FontAscentID   = 1,
		LineSpacingID  = 2,
		BaselineGridID = 3
	};

	PropertyWidgetText_OptMargins(QWidget *parent = 0);
	~PropertyWidgetText_OptMargins() {};

	void updateStyle(const ParagraphStyle& newCurrent);

protected:
	PageItem *         m_item;
	ScribusMainWindow* m_ScMW;

	void connectSignals();
	void disconnectSignals();

	void configureWidgets(void);
	void setCurrentItem(PageItem *item);

	virtual void changeEvent(QEvent *e);

public slots:
	void setMainWindow(ScribusMainWindow *mw);
	void setDoc(ScribusDoc *d);

	void handleAppModeChanged(int oldMode, int mode);
	void handleSelectionChanged();
	void handleUpdateRequest(int) {};

	void languageChange();
	void unitChange() {};

	void showOpticalMargins(const ParagraphStyle& pStyle);	
	void showFirstLinePolicy( FirstLineOffsetPolicy f );

private slots:
	void handleFirstLinePolicy(int radioFlop);
	void handleOpticalMargins();
	void handleTabs();
	void resetOpticalMargins();

};

#endif
