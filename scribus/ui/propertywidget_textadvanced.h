/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_TEXTADVANCED_H
#define PROPERTYWIDGET_TEXTADVANCED_H

#include "ui_propertywidget_textadvanced.h"
//#include "scguardedptr.h"
#include "propertywidgetbase.h"

class PageItem;
class CharStyle;
class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;

class PropertyWidget_TextAdvanced : public QWidget, Ui::PropertyWidget_TextAdvanced,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidget_TextAdvanced(QWidget* parent = 0);
	~PropertyWidget_TextAdvanced() {}

	virtual void changeEvent(QEvent *e);


	/** @brief Returns true if there is a user action going on at the moment of call. */
	bool userActionOn(); // not yet implemented!!! This is needed badly.
						 // When user releases the mouse button or arrow key, changes must be checked
						 // and if in ScribusView a groupTransaction has been started it must be also
						 // commmited
protected:

	bool   m_haveDoc;
	bool   m_haveItem;

	PageItem *m_item;
	ScribusMainWindow*       m_ScMW;

private:
	PageItem* currentItemFromSelection();


public slots:
	void setMainWindow(ScribusMainWindow *mw);

	void setDoc(ScribusDoc *d);
	void unsetDoc();

	void languageChange();
	void changeLang(int id);

	void showLanguage(QString w);
	void setCurrentItem(PageItem *i);

	/// update TB values:
	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& newCurrent);

	void handleSelectionChanged();

};

#endif
