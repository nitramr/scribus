/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_GLYPHSPACE_H
#define PROPERTYWIDGET_GLYPHSPACE_H

#include "ui_propertywidget_glyphspace.h"

#include "propertywidgetbase.h"


class CharStyle;
class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;

class PropertyWidget_GlyphSpace : public QWidget, Ui::PropertyWidget_GlyphSpace,
                                public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidget_GlyphSpace(QWidget *parent = 0);
	~PropertyWidget_GlyphSpace() {};

protected:
	void connectSignals();
	void disconnectSignals();

	double m_unitRatio;
	int    m_unitIndex;

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

	void showBaseLineOffset(double e);
	void showTracking(double e);

	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& newCurrent);

private slots:
	void handleBaselineOffset();
	void handleMinWordTracking();
	void handleNormWordTracking();
	void handleTracking();

};

#endif
