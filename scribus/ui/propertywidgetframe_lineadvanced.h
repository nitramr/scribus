/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETFRAME_LINEADVANCED_H
#define PROPERTYWIDGETFRAME_LINEADVANCED_H

#include "ui_propertywidgetframe_lineadvanced.h"

#include "scribusapi.h"
#include "linecombo.h"
#include "propertywidgetbase.h"
#include <QListWidgetItem>

class ArrowChooser;
class DashEditor;
class PageItem;
class ScComboBox;
class ScribusDoc;
class ScribusMainWindow;

class SCRIBUS_API PropertyWidgetFrame_LineAdvanced : public QWidget, Ui::PropertyWidgetFrame_LineAdvanced,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetFrame_LineAdvanced(QWidget* parent);
	~PropertyWidgetFrame_LineAdvanced() {};

	virtual void changeEvent(QEvent *e);

	void updateArrowStyles();

protected:

	bool      m_haveDoc;
	bool      m_haveItem;
	bool      m_lineMode;
	double    m_unitRatio;
	int       m_unitIndex;

	PageItem *m_item;
	ScribusMainWindow*       m_ScMW;

private:

	PageItem* currentItemFromSelection();
	void      updateArrowStyles(ScribusDoc *dd);

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

	void handleLineJoin();
	void handleLineEnd();
	void handleLineMode();
	void handleStartArrow(int id);
	void handleEndArrow(int id);
	void handleStartArrowScale(double sc);
	void handleEndArrowScale(double sc);
	void handleLineStyle(QListWidgetItem *c);

	void showLineValues(Qt::PenCapStyle pc, Qt::PenJoinStyle pj);

signals:
	void lineModeChanged(int);
};

#endif
