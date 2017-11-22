/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTIESFRAMEPALETTE_H
#define PROPERTIESFRAMEPALETTE_H

#include <QListWidgetItem>
#include <QLineEdit>
#include <QScrollArea>

class QCloseEvent;
class QEvent;
class QFocusEvent;
class QToolBox;
class QVBoxLayout;
class QWidget;

#include "scribusapi.h"
#include "scdockpalette.h"
#include "scguardedptr.h"
#include "scrspinbox.h"
#include "pageitem.h"
#include "linkbutton.h"
#include "linecombo.h"
#include "scguardedptr.h"
#include "sclistboxpixmap.h"
#include "scpage.h"
#include "spalette.h"
#include "sclayoutsection.h" // Indigo Section
#include "flowlayout.h"
#include "propertywidgetbase.h"

class ColorCombo;
class ColorPalette;
class NameWidget;
class PropertiesPalette_Group;
class PropertyWidgetFrame_Line;
class PropertyWidgetFrame_LineAdvanced;
class PropertyWidgetFrame_Shadow;
class PropertyWidgetFrame_ShadowOptions;
class PropertyWidgetFrame_Shape;
class PropertyWidgetFrame_XYZ;
class PropertyWidgetFrame_XYZExt;
class PropertyWidgetFrame_XYZTransform;
class ScComboBox;
class ScribusDoc;
class ScribusMainWindow;
class Selection;
class UndoManager;
class PropertyWidgetFrame_Transparency;


class SCRIBUS_API PropertiesFramePalette : public ScDockPalette,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertiesFramePalette(QWidget* parent);
	~PropertiesFramePalette() {}

	virtual void changeEvent(QEvent *e);
	virtual void closeEvent(QCloseEvent *closeEvent);

	void endPatchAdd();
	void updateColorSpecialGradient();
	void updateColorList();
	void setGradientEditMode(bool);
	void setTextFlowMode(PageItem::TextFlowMode mode);

	/** @brief Returns true if there is a user action going on at the moment of call. */
	bool userActionOn(); // not yet implemented!!! This is needed badly.
                         // When user releases the mouse button or arrow key, changes must be checked
                         // and if in ScribusView a groupTransaction has been started it must be also
                         // commmited

	PropertiesPalette_Group* groupPal;
	PropertyWidgetFrame_Line*  linePal;
	PropertyWidgetFrame_LineAdvanced*  lineAdvancedPal;
	PropertyWidgetFrame_Shadow* shadowPal;
	PropertyWidgetFrame_ShadowOptions* shadowOptionsPal;
	PropertyWidgetFrame_Shape* shapePal;
	PropertyWidgetFrame_XYZ*   xyzPal;
	PropertyWidgetFrame_XYZExt*   xyzExtPal;
	PropertyWidgetFrame_XYZTransform*   xyzTransformPal;
	ColorPalette *colorPal;
	PropertyWidgetFrame_Transparency *transparencyPal;



private:

	PageItem* currentItemFromSelection();
	QScrollArea *m_scrollArea;
	
public slots:
	void setMainWindow(ScribusMainWindow *mw);
	
	void setDoc(ScribusDoc *d);
	void AppModeChanged();
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void handleSelectionChanged();
	
	void unitChange();
	void languageChange();

	void setLocked(bool);

private slots:

	void NewLineMode(int mode);
	void handleNewShape(int frameType);
	void handleShapeEdit();

protected:
	ScribusMainWindow *m_ScMW;

	bool      m_haveDoc;
	bool      m_haveItem;
	double    m_unitRatio;
	int       m_unitIndex;
	PageItem* m_item;
	UndoManager *undoManager;
	
//	ScGuardedPtr<ScribusDoc> m_doc;

	ScLayoutSection *layoutSectionXYZ;
	ScLayoutSection *layoutSectionDropShadow;
	ScLayoutSection *layoutSectionShape;
	ScLayoutSection *layoutSectionGroup;
	ScLayoutSection *layoutSectionLine;
	ScLayoutSection *layoutSectionColor;
	ScLayoutSection *layoutSectionTransparency;

};

#endif
