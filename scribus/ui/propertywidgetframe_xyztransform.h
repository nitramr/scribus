/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETFRAME_XYZTRANSFORM_H
#define PROPERTYWIDGETFRAME_XYZTRANSFORM_H

#include "ui_propertywidgetframe_xyztransform.h"

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

class SCRIBUS_API PropertyWidgetFrame_XYZTransform : public QWidget, public Ui::PropertiesPalette_XYZTransform,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetFrame_XYZTransform(QWidget* parent);
	~PropertyWidgetFrame_XYZTransform() {};

	virtual void changeEvent(QEvent *e);
	
	/** @brief Returns true if there is a user action going on at the moment of call. */
	bool userActionOn(); // not yet implemented!!! This is needed badly.
                         // When user releases the mouse button or arrow key, changes must be checked
                         // and if in ScribusView a groupTransaction has been started it must be also
                         // commmited

	void setLineMode(int lineMode);
	void sendRotation();

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

	void showRotation(double r);
	void showLocked(bool);
	void showFlippedH(bool);
	void showFlippedV(bool);

	void handleAppModeChanged(int oldMode, int mode);
	void handleSelectionChanged();

private slots:
	void handleRotation();
	void handleFlipH();
	void handleFlipV();
	void handleLower();
	void handleRaise();
	void handleFront();
	void handleBack();
	void handleGrouping();
	void handleUngrouping();

protected slots:
	void spinboxStartUserAction();
	void spinboxFinishUserAction();

signals:
	void updateXY(double,double);
	void setRotation(double);

protected:
	ScribusMainWindow *m_ScMW;

	bool      m_haveDoc;
	bool      m_haveItem;
	bool      m_lineMode;
	double    m_unitRatio;
	int       m_unitIndex;
	PageItem *m_item;
	
	double    m_oldRotation;

	bool _userActionOn;
	UserActionSniffer *userActionSniffer;
	void installSniffer(ScrSpinBox *spinBox);
};

#endif
