/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_FLOP_H
#define PROPERTYWIDGET_FLOP_H

#include "ui_propertywidget_flopbase.h"
#include "scguardedptr.h"
#include "scribusdoc.h"

class PageItem;
class ScribusDoc;
class ScribusMainWindow;

class PropertyWidget_Flop : public QWidget, public Ui::PropertyWidget_FlopBase
{
	Q_OBJECT

public:

	enum FlopButtonID
    {
        RealHeightID   = 0,
        FontAscentID   = 1,
        LineSpacingID  = 2,
		BaselineGridID = 3
    };

	PropertyWidget_Flop(QWidget *parent = 0);
	~PropertyWidget_Flop() {};


protected:
	bool   m_haveDoc;
	PageItem *m_item;

	ScGuardedPtr<ScribusDoc> m_doc;
	ScribusMainWindow*       m_ScMW;

	virtual void changeEvent(QEvent *e);

public slots:

	void showFirstLinePolicy( FirstLineOffsetPolicy f );
	void handleFirstLinePolicy(int radioFlop);

	void languageChange();
	void unitChange() {};
};

#endif
