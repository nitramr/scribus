/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETFRAME_XYZEXT_H
#define PROPERTYWIDGETFRAME_XYZEXT_H

#include "ui_propertywidgetframe_xyzext.h"

#include "scribusapi.h"
#include "scrspinbox.h"
#include "sclistboxpixmap.h"
#include "scguardedptr.h"
#include "propertywidgetbase.h"

class NameWidget;
class PageItem;
class ScribusDoc;
class Selection;
class ScribusMainWindow;

class SCRIBUS_API PropertyWidgetFrame_XYZExt : public QWidget, public Ui::PropertyWidgetFrame_XYZExt,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetFrame_XYZExt(QWidget* parent);
	~PropertyWidgetFrame_XYZExt() {};

	virtual void changeEvent(QEvent *e);
	
	/** @brief Returns true if there is a user action going on at the moment of call. */
	bool userActionOn(); // not yet implemented!!! This is needed badly.
                         // When user releases the mouse button or arrow key, changes must be checked
                         // and if in ScribusView a groupTransaction has been started it must be also
                         // commmited

private:

	PageItem* currentItemFromSelection();
	
public slots:
	void setMainWindow(ScribusMainWindow *mw);
	
	void setDoc(ScribusDoc *d);
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void languageChange();
	void unitChange();

	void showPrintingEnabled(bool);

//	void handleAppModeChanged(int oldMode, int mode);
	void handleSelectionChanged();

private slots:
	void handlePrint();
	void handleNewName();
//	void handleGrouping();
//	void handleUngrouping();

protected:
	ScribusMainWindow *m_ScMW;

	bool      m_haveDoc;
	bool      m_haveItem;
	PageItem *m_item;
//	ScGuardedPtr<ScribusDoc> m_doc;

};

#endif
