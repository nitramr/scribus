/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETTEXT_TEXTALIGNMENT_H
#define PROPERTYWIDGETTEXT_TEXTALIGNMENT_H

#include "ui_propertywidgettext_textalignment.h"

#include "propertywidgetbase.h"

class ScribusDoc;
class ScribusMainWindow;
class ParagraphStyle;
class CharStyle;

class PropertyWidgetText_TextAlignment : public QWidget, Ui::PropertyWidget_TextAlignment,
	                             public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetText_TextAlignment(QWidget *parent = 0);
	~PropertyWidgetText_TextAlignment() {};

	void setCurrentItem(PageItem *i);

protected:
	PageItem *         m_item;
	ScribusMainWindow* m_ScMW;
	bool   m_haveDoc;
	bool   m_haveItem;

	void connectSignals();
	void disconnectSignals();

	void configureWidgets(void);

	virtual void changeEvent(QEvent *e);

public slots:
	void setMainWindow(ScribusMainWindow *mw);
	void setDoc(ScribusDoc *d);

	void handleSelectionChanged();

	void languageChange();
	void unitChange() {};

	void showOutlineW(double x);
	void showShadowOffset(double x, double y);
	void showStrikeThru(double p, double w);
	void showTextEffects(int s);
	void showUnderline(double p, double w);

	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& newCurrent);

	void showAlignment(int e);
	void showDirection(int e);
	void handleAlignment(int a);
	void handleDirection(int d);

private slots:
	void handleOutlineWidth();
	void handleShadowOffs();
	void handleStrikeThru();
	void handleTypeStyle(int s);
	void handleUnderline();

	void handleVAlign();

signals:
	void handleAlignment();
};

#endif
