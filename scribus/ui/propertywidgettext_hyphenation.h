/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETTEXT_HYPHENATION_H
#define PROPERTYWIDGETTEXT_HYPHENATION_H

#include "ui_propertywidgettext_hyphenationbase.h"
#include "propertywidgetbase.h"


class CharStyle;
class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;

class PropertyWidgetText_Hyphenation : public QWidget, Ui::PropertyWidget_HyphenationBase,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetText_Hyphenation(QWidget *parent = 0);
	~PropertyWidgetText_Hyphenation() {}

protected:
	void connectSignals();
	void disconnectSignals();
	PageItem* m_item;
	ScribusMainWindow* m_ScMW;

	void configureWidgets(void);
	void setCurrentItem(PageItem *item);
	virtual void changeEvent(QEvent *e);

public slots:
	void setMainWindow(ScribusMainWindow *mw);
	void setDoc(ScribusDoc *d);
	void handleSelectionChanged();
	void languageChange();
	void unitChange() {};
	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& paraStyle);

private slots:
	void handleWordMin(int minWord);
	void handleConsecutiveLines(int consecutiveLines);
	void handleHyphenChar(const QString& hyphenText);
};

#endif // PROPERTYWIDGET_HYPHENATION_H
