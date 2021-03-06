/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETFRAME_SHAPE_H
#define PROPERTYWIDGETFRAME_SHAPE_H

#include "ui_propertywidgetframe_shape.h"

#include "scribusapi.h"
#include "scrspinbox.h"
#include "pageitem.h"
#include "linkbutton.h"
#include "sclistboxpixmap.h"
//#include "scguardedptr.h"
#include "propertywidgetbase.h"
#include "sctreewidget.h"

class Autoforms;
class ScribusDoc;
class ScribusMainWindow;
class Selection;

class SCRIBUS_API PropertyWidgetFrame_Shape : public QWidget, Ui::PropertyWidgetFrame_Shape,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetFrame_Shape(QWidget* parent);
	~PropertyWidgetFrame_Shape() {};

	virtual void changeEvent(QEvent *e);

	void setCustomShapeIcon(int submode);
	void setLocked(bool isLocked);
	void setSizeLocked(bool isLocked);
	void setRoundRectEnabled(bool enabled);
	void showTextFlowMode(PageItem::TextFlowMode mode);

protected:

	bool      m_haveDoc;
	bool      m_haveItem;
	double    m_unitRatio;
	int       m_unitIndex;
	PageItem *m_item;
//	ScGuardedPtr<ScribusDoc> m_doc;

	Selection* m_tmpSelection;

private:

	PageItem* currentItemFromSelection();

protected:
	ScribusMainWindow *m_ScMW;
	
	void enableCustomShape();
	void enableEditShape();

public slots:
	void setMainWindow(ScribusMainWindow *mw);

	void setDoc(ScribusDoc *d);
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void handleSelectionChanged();
	void handleUpdateRequest(int);

	void languageChange();
	void unitChange();

private slots:

	void handleTextFlow();
	void handleShapeEdit();
	void handleShapeEditEnded();
	void handleCornerRadius();
	void handleFillRule();
	void handleNewShape(int, int, qreal *);
};

#endif
