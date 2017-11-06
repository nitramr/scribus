/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_GLYPHSTRETCH_H
#define PROPERTYWIDGET_GLYPHSTRETCH_H

#include "ui_propertywidget_glyphstretch.h"

#include "propertywidgetbase.h"


class CharStyle;
class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;

class PropertyWidget_GlyphStretch : public QWidget, Ui::PropertyWidget_GlyphStretch,
                                public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidget_GlyphStretch(QWidget *parent = 0);
	~PropertyWidget_GlyphStretch() {};

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

	void showTextScaleH(double e);
	void showTextScaleV(double e);

	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& newCurrent);

private slots:

	void handleMinGlyphExtension();
	void handleMaxGlyphExtension();
	void handleTextScaleH();
	void handleTextScaleV();

//	void handleHyphenate();
//	void handleDeHyphenate();

};

#endif
