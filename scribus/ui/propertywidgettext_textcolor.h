/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_TEXTCOLOR_H
#define PROPERTYWIDGET_TEXTCOLOR_H

#include "ui_propertywidget_textcolorbase.h"

#include "propertywidgetbase.h"
#include "sccolorfillsbox.h"


class ScribusDoc;
class ScribusMainWindow;

class PropertyWidgetText_TextColor : public QWidget, Ui::PropertyWidget_TextColorBase,
	                             public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetText_TextColor(QWidget *parent = 0);
	~PropertyWidgetText_TextColor() {};

	void updateColorList();
	void setCurrentItem(PageItem *i);

protected:
	PageItem *         m_item;
	ScribusMainWindow* m_ScMW;
	ScColorFillsBox * strokeColorBox;
	ScColorFillsBox * backColorBox;

	void connectSignals();
	void disconnectSignals();

	void configureWidgets(void);

	virtual void changeEvent(QEvent *e);

public slots:
	void setMainWindow(ScribusMainWindow *mw);
	void setDoc(ScribusDoc *d);

	void handleSelectionChanged();
	void handleUpdateRequest(int);

	void languageChange();
	void unitChange() {};

	void showTextColors(QString p, QString bc, double shp, double sbc);
	void showTextEffects(int s);

	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& newCurrent);

private slots:
	void handleStrokeColorBox();
	void handleBackColorBox();
	void handleTextShade();
	void handleTextStroke();
	void handleTextBackground();
	void handleTypeStyle(int s);
};

#endif
