/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETFRAME_XYZ_H
#define PROPERTYWIDGETFRAME_XYZ_H

#include "ui_propertywidgetframe_xyz.h"

#include "scribusapi.h"
#include "scrspinbox.h"
#include "linkbutton.h"
#include "sclistboxpixmap.h"
#include "propertywidgetbase.h"

class BasePointWidget;
class PageItem;
class ScribusDoc;
class Selection;
class ScribusMainWindow;
class UserActionSniffer;

class SCRIBUS_API PropertyWidgetFrame_XYZ : public QWidget, public Ui::PropertiesPalette_XYZ,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetFrame_XYZ(QWidget* parent);
	~PropertyWidgetFrame_XYZ() {};

	virtual void changeEvent(QEvent *e);
	
	/** @brief Returns true if there is a user action going on at the moment of call. */
	bool userActionOn(); // not yet implemented!!! This is needed badly.
                         // When user releases the mouse button or arrow key, changes must be checked
                         // and if in ScribusView a groupTransaction has been started it must be also
                         // commmited

	void setLineMode(int lineMode);

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

	void showXY(double x, double y);
	void showWH(double x, double y);
	void showLocked(bool);
	void showSizeLocked(bool);

	void handleAppModeChanged(int oldMode, int mode);
	void handleSelectionChanged();

	void setRotation(double rotation);

private slots:
	void handleNewX();
	void handleNewY();
	void handleNewW();
	void handleNewH();
	void handleBasePoint(int m);
	void handleLock();
	void handleLockSize();

protected slots:
	//virtual void reject();
	void spinboxStartUserAction();
	void spinboxFinishUserAction();
	void updateSpinBoxConstants();

protected:
	ScribusMainWindow *m_ScMW;

	bool      m_haveDoc;
	bool      m_haveItem;
	bool      m_lineMode;
	double    m_unitRatio;
	int       m_unitIndex;
	PageItem *m_item;

	double	m_rotation;

	bool _userActionOn;
	UserActionSniffer *userActionSniffer;
	void installSniffer(ScrSpinBox *spinBox);
};

#endif
