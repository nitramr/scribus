/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETFRAME_LINE_H
#define PROPERTYWIDGETFRAME_LINE_H

#include "ui_propertywidgetframe_line.h"

#include "scribusapi.h"
#include "linecombo.h"
#include "propertywidgetbase.h"

class DashEditor;
class PageItem;
class ScComboBox;
class ScribusDoc;
class ScribusMainWindow;
//class StyleManager;

class SCRIBUS_API PropertyWidgetFrame_Line : public QWidget, Ui::PropertyWidgetFrame_Line,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetFrame_Line(QWidget* parent);
	~PropertyWidgetFrame_Line() {delete lineStyles->itemDelegate();};

	virtual void changeEvent(QEvent *e);

	void updateLineStyles();

protected:

	bool      m_haveDoc;
	bool      m_haveItem;
	double    m_unitRatio;
	int       m_unitIndex;

	PageItem *m_item;
	ScribusMainWindow*       m_ScMW;

//	StyleManager *m_styleManager;

private:

	PageItem* currentItemFromSelection();
	void      updateLineStyles(ScribusDoc *dd);

public slots:
	void setMainWindow(ScribusMainWindow *mw);

	void setDoc(ScribusDoc *d);
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void handleSelectionChanged();
	void handleUpdateRequest(int);

	void showLineWidth(double s);

	void languageChange();
	void unitChange();

private slots:

	void handleLineWidth();
	void handleLineStyle();
	void handleDashChange();
	void handleLineStyle(QListWidgetItem *c);
//	void handleLineEdit();

	void showLineValues(Qt::PenStyle p);


};

#endif
