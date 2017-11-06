/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_TEXTSTYLES_H
#define PROPERTYWIDGET_TEXTSTYLES_H

#include "ui_propertywidget_textstyles.h"
//#include "scrspinbox.h"
//#include "scguardedptr.h"
#include "propertywidgetbase.h"

class PageItem;
class CharStyle;
class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;
class ScComboBox;

class PropertyWidgetText_TextStyles : public QWidget, Ui::PropertyWidget_TextStyles,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetText_TextStyles(QWidget* parent = 0);
	~PropertyWidgetText_TextStyles() {}

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

	void showCharStyle(const QString& name);
	void showParStyle(const QString& name);
	void setCurrentItem(PageItem *i);

	/// update TB values:
	void updateStyle(const ParagraphStyle& newCurrent);

	void updateCharStyles();
	void updateParagraphStyles();
	void updateTextStyles();

	void handleSelectionChanged();
	void handleUpdateRequest(int updateFlags);

private slots:

	void doClearCStyle();
	void doClearPStyle();

};

#endif
