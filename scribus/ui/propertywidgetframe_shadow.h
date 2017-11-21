/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETFRAME_SHADOW_H
#define PROPERTYWIDGETFRAME_SHADOW_H
#include "ui_propertywidgetframe_shadow.h"

#include "scribusapi.h"
//#include "scguardedptr.h"
#include "propertywidgetbase.h"

#include "sccolorfillsbox.h"

class PageItem;
class ScribusDoc;
class Selection;
class ScribusMainWindow;

class SCRIBUS_API PropertyWidgetFrame_Shadow : public QWidget, Ui::PropertyWidgetFrame_Shadow,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetFrame_Shadow(QWidget* parent);
	~PropertyWidgetFrame_Shadow() {};

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
	void unitChange();
	void handleSelectionChanged();
	void handleUpdateRequest(int);

	void setShadowOn(bool isOn);
	void setShadowOptions(int blendMode, bool erase, bool objTrans);

	void handleNewValues();

private slots:	
	void handleFillColorBox();
	void showColor(QString b, double sb);

protected slots:
	void updateSpinBoxConstants();

signals:
	void shadowOn(bool);

protected:

	ScribusMainWindow *m_ScMW;

	bool      m_haveDoc;
	bool      m_haveItem;
	double    m_unitRatio;
	int       m_unitIndex;

	PageItem *m_item;
	//ScGuardedPtr<ScribusDoc> m_doc;
	ScColorFillsBox * shadowColor;

	bool shadow;
	int shadowB;
	bool shadowE;
	bool shadowT;
};

#endif
