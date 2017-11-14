/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTIESCONTENTPALETTE_TABLE_H
#define PROPERTIESCONTENTPALETTE_TABLE_H



#include "scribusapi.h"
#include "scrpalettebase.h"
#include "scrspinbox.h"

#include "scguardedptr.h"
#include "sctextstruct.h"

#include "sclayoutsection.h" // Indigo Section
#include "flowlayout.h"

class PageItem;
class PropertyWidgetTable_Table;
class ScribusDoc;
class ScribusMainWindow;
class Selection;

class SCRIBUS_API PropertiesContentPalette_Table : public QWidget
{
	Q_OBJECT

public:
	PropertiesContentPalette_Table(QWidget* parent = 0);
	~PropertiesContentPalette_Table() {}

	virtual void changeEvent(QEvent *e);
	void updateColorList();
	
	/** @brief Returns true if there is a user action going on at the moment of call. */
	bool userActionOn(); // not yet implemented!!! This is needed badly.
                         // When user releases the mouse button or arrow key, changes must be checked
                         // and if in ScribusView a groupTransaction has been started it must be also
                         // commmited
protected:

	bool   m_haveDoc;
	bool   m_haveItem;
	double    m_unitRatio;
	int       m_unitIndex;

	PageItem *m_item;
	ScribusMainWindow*       m_ScMW;
	ScGuardedPtr<ScribusDoc> m_doc;

	ScLayoutSection *layoutSectionTable;

	PropertyWidgetTable_Table* tableWidget;

private:
	PageItem* currentItemFromSelection();

	
public slots:
	void AppModeChanged();

	void setMainWindow(ScribusMainWindow *mw);
	
	void setDoc(ScribusDoc *d);
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void handleSelectionChanged();

	void languageChange();
	void unitChange(){};


};

#endif
