/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETFRAME_SHADOWOPTIONS_H
#define PROPERTYWIDGETFRAME_SHADOWOPTIONS_H
#include "ui_propertywidgetframe_shadowoptions.h"

#include "scribusapi.h"
#include "scguardedptr.h"

class PageItem;
class ScribusDoc;
class Selection;
class ScribusMainWindow;

class SCRIBUS_API PropertyWidgetFrame_ShadowOptions : public QWidget, Ui::PropertyWidgetFrame_ShadowOptions
{
	Q_OBJECT

public:
	PropertyWidgetFrame_ShadowOptions(QWidget* parent);
	~PropertyWidgetFrame_ShadowOptions() {};

	virtual void changeEvent(QEvent *e);
	void updateColorList();

private:

	PageItem* currentItemFromSelection();
	void connectSignals();
	void disconnectSignals();

public slots:
	void setMainWindow(ScribusMainWindow *mw);

	void setDoc(ScribusDoc *d);
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void languageChange();
	void unitChange(){};
	void handleSelectionChanged();

private slots:
	void handleNewValues();

signals:
	void sendShadowOptions(int, bool, bool);

protected:

	ScribusMainWindow *m_ScMW;

	bool      m_haveDoc;
	bool      m_haveItem;
	double    m_unitRatio;
	int       m_unitIndex;

	PageItem *m_item;
	ScGuardedPtr<ScribusDoc> m_doc;
};

#endif
