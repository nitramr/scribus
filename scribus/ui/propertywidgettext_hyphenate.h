/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETTEXT_HYPHENATE_H
#define PROPERTYWIDGETTEXT_HYPHENATE_H

#include "ui_propertywidgettext_hyphenate.h"

#include "propertywidgetbase.h"


class CharStyle;
class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;

class PropertyWidgetText_Hyphenate : public QWidget, Ui::PropertyWidget_Hyphenate,
                                public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetText_Hyphenate(QWidget *parent = 0);
	~PropertyWidgetText_Hyphenate() {};

protected:
	void connectSignals();
	void disconnectSignals();

//	double m_unitRatio;
//	int    m_unitIndex;

	PageItem*          m_item;
	ScribusMainWindow* m_ScMW;

	void configureWidgets(void);
	void setCurrentItem(PageItem *i);

	virtual void changeEvent(QEvent *e);

public slots:
	void setMainWindow(ScribusMainWindow *mw);
	void setDoc(ScribusDoc *d);

	void handleSelectionChanged();

	void languageChange();
	void unitChange() {};

private slots:

	void handleHyphenate();
	void handleDeHyphenate();

};

#endif
