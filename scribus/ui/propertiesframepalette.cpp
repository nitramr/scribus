/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "propertiesframepalette.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QObject>
#include <QPoint>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QTimer>
#include <QToolBox>
#include <QToolTip>
#include <QTransform>
#include <QVBoxLayout>
#include <QValidator>
#include <QWidget>

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "appmodehelper.h"
#include "appmodes.h"
#include "arrowchooser.h"
#include "autoform.h"
#include "basepointwidget.h"
#include "colorcombo.h"
#include "colorlistbox.h"
#include "commonstrings.h"
#include "colorpalette.h"
#include "colorpicker.h"
#include "dasheditor.h"
#include "pageitem_table.h"
#include "propertiespalette_group.h"
#include "propertywidgetframe_line.h"
#include "propertywidgetframe_lineadvanced.h"
#include "propertywidgetframe_shadow.h"
#include "propertywidgetframe_shadowoptions.h"
#include "propertywidgetframe_shape.h"
#include "propertiespalette_utils.h"
#include "propertywidgetframe_xyz.h"
#include "propertywidgetframe_xyzext.h"
#include "propertywidgetframe_xyztransform.h"
#include "scribus.h"
#include "scribusview.h"
#include "selection.h"
#include "undomanager.h"
#include "units.h"
#include "util_math.h"
#include "scpopupmenu.h"


PropertiesFramePalette::PropertiesFramePalette( QWidget* parent) : ScDockPalette( parent, "PropertiesFramePalette", 0)
{
	undoManager = UndoManager::instance();
	m_ScMW=0;
//	m_doc=0;
	m_haveDoc = false;
	m_haveItem = false;
	m_unitRatio = 1.0;

	setObjectName(QString::fromLocal8Bit("PropertiesFramePalette"));

	QFont f(font());
	f.setPointSize(f.pointSize()-1);
	setFont(f);

	xyzPal = new PropertyWidgetFrame_XYZ(this);
	xyzExtPal = new PropertyWidgetFrame_XYZExt(this);
	xyzTransformPal = new PropertyWidgetFrame_XYZTransform(this);
	shadowPal = new PropertyWidgetFrame_Shadow(this);
	shadowOptionsPal = new PropertyWidgetFrame_ShadowOptions(this);
	shapePal = new PropertyWidgetFrame_Shape(this);
	groupPal = new PropertiesPalette_Group(this);
	linePal = new PropertyWidgetFrame_Line(this);
	lineAdvancedPal = new PropertyWidgetFrame_LineAdvanced(this);
	colorPal = new ColorPalette(this);
	transparencyPal = new PropertyWidgetFrame_Transparency(this);

	colorPicker = new ColorPicker(this);


	ScPopupMenu * popupXYZ = new ScPopupMenu(xyzExtPal);
	layoutSectionXYZ = new ScLayoutSection(tr("XYZ"), popupXYZ);
	layoutSectionXYZ->addWidget(xyzPal);
	layoutSectionXYZ->addWidget(xyzTransformPal);

	ScPopupMenu * popupFontFeatures = new ScPopupMenu(shadowOptionsPal);
	layoutSectionDropShadow = new ScLayoutSection(tr("Drop Shadow"), popupFontFeatures, true);
	layoutSectionDropShadow->addWidget(shadowPal);
	layoutSectionDropShadow->setVisible(false);

	layoutSectionShape = new ScLayoutSection(tr("&Shape"));
	layoutSectionShape->addWidget(shapePal);
	layoutSectionShape->setVisible(false);

	layoutSectionGroup = new ScLayoutSection(tr("&Group"));
	layoutSectionGroup->addWidget(groupPal);
	layoutSectionGroup->setVisible(false);

	ScPopupMenu * popupLineAdvanced = new ScPopupMenu(lineAdvancedPal);
	layoutSectionLine = new ScLayoutSection(tr("&Line"), popupLineAdvanced);
	layoutSectionLine->addWidget(linePal);
	layoutSectionLine->setVisible(false);

	layoutSectionColor = new ScLayoutSection(tr("&Colors"));
	layoutSectionColor->addWidget(colorPal);
	layoutSectionColor->addWidget(colorPicker);
	layoutSectionColor->setVisible(false);

	layoutSectionTransparency = new ScLayoutSection(tr("&Transparency"));
	layoutSectionTransparency->addWidget(transparencyPal);
	layoutSectionTransparency->setVisible(false);


	FlowLayout *flowLayout = new FlowLayout(0,0,0);
	flowLayout->addWidget(layoutSectionXYZ);
	flowLayout->addWidget(layoutSectionDropShadow);
	flowLayout->addWidget(layoutSectionShape);
	flowLayout->addWidget(layoutSectionGroup);
	flowLayout->addWidget(layoutSectionLine);
	flowLayout->addWidget(layoutSectionColor);
	flowLayout->addWidget(layoutSectionTransparency);

	QWidget * paletteHolder = new QWidget();
	paletteHolder->setLayout(flowLayout);

	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidget(paletteHolder);
	m_scrollArea->setMinimumWidth(this->width());
	m_scrollArea->setWidgetResizable(true);

	setWidget( m_scrollArea );


	languageChange();

	connect(linePal, SIGNAL(lineModeChanged(int)), this, SLOT(NewLineMode(int)));
	connect(lineAdvancedPal, SIGNAL(lineModeChanged(int)), this, SLOT(NewLineMode(int)));

	connect(groupPal, SIGNAL(shapeChanged(int)) , this, SLOT(handleNewShape(int)));
	connect(groupPal, SIGNAL(shapeEditStarted()), this, SLOT(handleShapeEdit()));

	connect(shadowOptionsPal, SIGNAL(sendShadowOptions(int,bool,bool)), shadowPal, SLOT(setShadowOptions(int,bool,bool)));
	connect(layoutSectionDropShadow, SIGNAL(toggleState(bool)), shadowPal, SLOT(setShadowOn(bool)));
	connect(shadowPal, SIGNAL(shadowOn(bool)), layoutSectionDropShadow, SLOT(setToggleOn(bool)));

	connect(xyzTransformPal, SIGNAL(updateXY(double,double)), xyzPal, SLOT(showXY(double,double)));
	connect(xyzTransformPal, SIGNAL(setRotation(double)), xyzPal, SLOT(setRotation(double)));

	m_haveItem = false;

}


/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertiesFramePalette::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	// For some reason, the mapToGlobal() / mapFromGlobal() roundtrip
	// performed below does not give always good results, causing PP to
	// not display in some situations. Moreover the reparenting is useless 
	// as PP is already created with ScribusMainWindow as parent.
	/*QPoint p1 = mapToGlobal(pos());
	QPoint p2 = m_ScMW->mapFromGlobal(p1);
	setParent(m_ScMW);
	move(p2);*/

	this->xyzPal->setMainWindow(mw);
	this->xyzTransformPal->setMainWindow(mw);
	this->xyzExtPal->setMainWindow(mw);
	this->shadowPal->setMainWindow(mw);
	this->shadowOptionsPal->setMainWindow(mw);
	this->shapePal->setMainWindow(mw);
	this->groupPal->setMainWindow(mw);
	this->linePal->setMainWindow(mw);
	this->lineAdvancedPal->setMainWindow(mw);
	this->colorPal->setMainWindow(mw);
	this->transparencyPal->setMainWindow(mw);
	this->colorPicker->setMainWindow(mw);

	//connect(this->colorPal, SIGNAL(gradientChanged()), m_ScMW, SLOT(updtGradFill()));
	//connect(this->colorPal, SIGNAL(strokeGradientChanged()), m_ScMW, SLOT(updtGradStroke()));
	connect(m_ScMW->appModeHelper, SIGNAL(AppModeChanged(int,int)), this, SLOT(AppModeChanged()));
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertiesFramePalette::setDoc(ScribusDoc *d)
{
	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_doc = d;
	m_item = NULL;
	setEnabled(!m_doc->drawAsPreview);

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();
	m_haveDoc = true;
	m_haveItem = false;

	xyzPal->setDoc(m_doc);
	xyzTransformPal->setDoc(m_doc);
	xyzExtPal->setDoc(m_doc);
	shadowPal->setDoc(m_doc);
	shadowOptionsPal->setDoc(m_doc);
	shapePal->setDoc(m_doc);
	groupPal->setDoc(m_doc);
	linePal->setDoc(m_doc);
	lineAdvancedPal->setDoc(m_doc);
	transparencyPal->setDoc(m_doc);
	colorPal->setDoc(m_doc);
	colorPicker->setDoc(m_doc);

	updateColorList();

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));

	// Handle properties update when switching document
	handleSelectionChanged();
}

void PropertiesFramePalette::unsetDoc()
{
	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}
	setEnabled(true);
	m_haveDoc = false;
	m_haveItem = false;
	m_doc=NULL;
	m_item = NULL;

	xyzPal->unsetDoc();
	xyzTransformPal->unsetDoc();
	xyzExtPal->unsetDoc();
	shadowPal->unsetDoc();
	shadowOptionsPal->unsetDoc();
	shapePal->unsetDoc();
	groupPal->unsetDoc();
	linePal->unsetDoc();
	lineAdvancedPal->unsetDoc();
	colorPal->unsetDoc();
	colorPicker->unsetDoc();
	transparencyPal->unsetDoc();

	m_haveItem = false;

	layoutSectionXYZ->setEnabled(false);
	layoutSectionDropShadow->setVisible(false);
	layoutSectionShape->setVisible(false);
	layoutSectionGroup->setVisible(false);
	layoutSectionLine->setVisible(false);
	layoutSectionColor->setVisible(false);
	layoutSectionTransparency->setVisible(false);
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertiesFramePalette::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;

	colorPal->unsetItem();
	colorPicker->unsetItem();
	transparencyPal->unsetItem();
	shapePal->unsetItem();
	groupPal->unsetItem();
	shadowPal->unsetItem();
	shadowOptionsPal->unsetItem();
	linePal->unsetItem();
	lineAdvancedPal->unsetItem();

	handleSelectionChanged();
}


PageItem* PropertiesFramePalette::currentItemFromSelection()
{
	PageItem *currentItem = NULL;

	if (m_doc)
	{
		if (m_doc->m_Selection->count() > 0)
			currentItem = m_doc->m_Selection->itemAt(0);
	/*	if (m_doc->m_Selection->count() > 1)
		{
			int lowestItem = 999999;
			for (int a=0; a<m_doc->m_Selection->count(); ++a)
			{
				currentItem = m_doc->m_Selection->itemAt(a);
				lowestItem = qMin(lowestItem, m_doc->Items->indexOf(currentItem));
			}
			currentItem = m_doc->Items->at(lowestItem);
		}
		else if (m_doc->m_Selection->count() == 1)
		{
			currentItem = m_doc->m_Selection->itemAt(0);
		} */
	}

	return currentItem;
}

void PropertiesFramePalette::setCurrentItem(PageItem *i)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	//CB We shouldn't really need to process this if our item is the same one
	//maybe we do if the item has been changed by scripter.. but that should probably
	//set some status if so.
	//FIXME: This won't work until when a canvas deselect happens, m_item must be NULL.
	//if (m_item == i)
	//	return;

	if (!i)
	{
		unsetItem();
		return;
	}

	if (!m_doc)
		setDoc(i->doc());

	disconnect(linePal , SIGNAL(lineModeChanged(int)), this, SLOT(NewLineMode(int)));
	disconnect(lineAdvancedPal , SIGNAL(lineModeChanged(int)), this, SLOT(NewLineMode(int)));

	m_haveItem = false;
	m_item = i;

	setTextFlowMode(m_item->textFlowMode());

	connect(linePal , SIGNAL(lineModeChanged(int)), this, SLOT(NewLineMode(int)));
	connect(lineAdvancedPal , SIGNAL(lineModeChanged(int)), this, SLOT(NewLineMode(int)));

//CB replaces old emits from PageItem::emitAllToGUI()
	setLocked(i->locked());

	layoutSectionDropShadow->setEnabled(true);
	layoutSectionShape->setEnabled(true);
	layoutSectionGroup->setEnabled(true);
	layoutSectionLine->setEnabled(true);
	layoutSectionColor->setEnabled(true);
	layoutSectionTransparency->setEnabled(true);

	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		layoutSectionXYZ->setEnabled(true);
		layoutSectionDropShadow->setVisible(true);
		layoutSectionShape->setVisible(false);
		layoutSectionGroup->setVisible(true);
		layoutSectionLine->setVisible(false);
		layoutSectionColor->setVisible(false);
	}
	else
		layoutSectionGroup->setVisible(false);

	m_haveItem = true;

	if (!sender() || (m_doc->appMode == modeEditTable))
	{
		xyzPal->handleSelectionChanged();
		xyzTransformPal->handleSelectionChanged();
		xyzExtPal->handleSelectionChanged();
		transparencyPal->handleSelectionChanged();
		shadowPal->handleSelectionChanged();
		shadowOptionsPal->handleSelectionChanged();
		shapePal->handleSelectionChanged();
		groupPal->handleSelectionChanged();
		linePal->handleSelectionChanged();
		lineAdvancedPal->handleSelectionChanged();
		colorPal->handleSelectionChanged();
		colorPicker->handleSelectionChanged();

	}

	if (m_item->asOSGFrame())
	{
		layoutSectionXYZ->setEnabled(true);
		layoutSectionDropShadow->setVisible(true);
		layoutSectionShape->setVisible(true);
		layoutSectionGroup->setVisible(false);
		layoutSectionLine->setVisible(false);
		layoutSectionColor->setVisible(true);
		layoutSectionTransparency->setVisible(false);
	}
	if (m_item->asSymbolFrame())
	{
		layoutSectionXYZ->setEnabled(true);
		layoutSectionDropShadow->setVisible(true);
		layoutSectionShape->setVisible(false);
		layoutSectionGroup->setVisible(true);
		layoutSectionLine->setVisible(false);
		layoutSectionColor->setVisible(false);
		layoutSectionTransparency->setVisible(false);
	}
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void  PropertiesFramePalette::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{
		layoutSectionXYZ->setEnabled(true);
		layoutSectionDropShadow->setVisible(true);
		layoutSectionLine->setVisible(true);
		layoutSectionColor->setVisible(true);
		layoutSectionTransparency->setVisible(true);

		if (m_haveItem && m_item)
		{
			if ((m_item->isGroup()) && (!m_item->isSingleSel))
				layoutSectionGroup->setVisible(true);
			}
	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem   = (itemType != -1);

		layoutSectionXYZ->setEnabled(true);
		layoutSectionColor->setVisible(true);
		layoutSectionTransparency->setVisible(true);

		switch (itemType)
		{
		case -1:
			m_haveItem = false;

			layoutSectionXYZ->setEnabled(false);
			layoutSectionDropShadow->setVisible(false);
			layoutSectionShape->setVisible(false);
			layoutSectionGroup->setVisible(false);
			layoutSectionLine->setVisible(false);
			layoutSectionColor->setVisible(false);
			layoutSectionTransparency->setVisible(false);

			//colorPal->showGradient(0);
			break;
		case PageItem::ImageFrame:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
			if (currItem->asOSGFrame())
			{
				layoutSectionXYZ->setEnabled(true);
				layoutSectionDropShadow->setVisible(true);
				layoutSectionShape->setVisible(true);
				layoutSectionGroup->setVisible(false);
				layoutSectionLine->setVisible(false);
				layoutSectionColor->setVisible(true);
				layoutSectionTransparency->setVisible(false);

			}
			else
			{
				layoutSectionXYZ->setEnabled(true);
				layoutSectionDropShadow->setVisible(true);
				layoutSectionShape->setVisible(true);
				layoutSectionLine->setVisible(true);
			}
			break;
		case PageItem::Line:
			layoutSectionDropShadow->setVisible(true);
			layoutSectionShape->setVisible(false);
			layoutSectionLine->setVisible(true);
			break;
		case PageItem::TextFrame:
		case PageItem::ItemType1:
		case PageItem::ItemType3:
		case PageItem::Polygon:
		case PageItem::RegularPolygon:
		case PageItem::Arc:
		case PageItem::PolyLine:
		case PageItem::Spiral:
		case PageItem::PathText:
			layoutSectionDropShadow->setVisible(true);
			layoutSectionShape->setVisible(true);
			layoutSectionLine->setVisible(true);
			break;
		case PageItem::Symbol:
		case PageItem::Group:
			layoutSectionDropShadow->setVisible(true);
			layoutSectionShape->setVisible(false);
			layoutSectionGroup->setVisible(true);
			layoutSectionLine->setVisible(false);
			layoutSectionColor->setVisible(false);
			layoutSectionTransparency->setVisible(false);
			break;
		case PageItem::Table:
			layoutSectionDropShadow->setVisible(true);
			layoutSectionShape->setVisible(true);
			layoutSectionGroup->setVisible(false);
			layoutSectionLine->setVisible(false);
			layoutSectionColor->setVisible(false);
			layoutSectionTransparency->setVisible(false);
			break;
		}
	}

	updateGeometry();
	update();

	if (currItem)
	{
		setCurrentItem(currItem);
	}
}

void PropertiesFramePalette::unitChange()
{
	if (!m_haveDoc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;
	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();

	xyzPal->unitChange();
	xyzTransformPal->unitChange();
	xyzExtPal->unitChange();
	shadowPal->unitChange();
	shadowOptionsPal->unitChange();
	shapePal->unitChange();
	groupPal->unitChange();
	linePal->unitChange();
	lineAdvancedPal->unitChange();
	colorPal->unitChange();
	colorPicker->unitChange();
	transparencyPal->unitChange();

	m_haveItem = tmp;
}

void PropertiesFramePalette::languageChange()
{
	layoutSectionXYZ->setTitle(tr("X, Y, &Z"));
	layoutSectionDropShadow->setTitle(tr("Drop Shadow"));
	layoutSectionShape->setTitle(tr("&Shape"));
	layoutSectionGroup->setTitle(tr("&Group"));
	layoutSectionLine->setTitle(tr("&Line"));
	layoutSectionColor->setTitle(tr("&Colors"));
	layoutSectionTransparency->setTitle(tr("&Transparency"));

	xyzPal->languageChange();
	xyzTransformPal->languageChange();
	xyzExtPal->languageChange();
	shadowPal->languageChange();
	shadowOptionsPal->languageChange();
	shapePal->languageChange();
	groupPal->languageChange();
	colorPal->languageChange();
	colorPicker->languageChange();
	linePal->languageChange();
	lineAdvancedPal->languageChange();
}

void PropertiesFramePalette::AppModeChanged()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		if (m_item->isTable())
		{
			if (m_doc->appMode == modeEditTable)
				connect(m_item->asTable(), SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
			else
				disconnect(m_item->asTable(), SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		}
	}
}

/*********************************************************************
*
* Features
*
**********************************************************************/

void PropertiesFramePalette::NewLineMode(int mode)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	xyzTransformPal->setLineMode(mode);
	xyzTransformPal->sendRotation();
	xyzPal->setLineMode(mode);
	xyzPal->showWH(m_item->width(), m_item->height());

	repaint();
}

void PropertiesFramePalette::handleNewShape(int frameType)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		if ((m_item->itemType() == PageItem::PolyLine) || (m_item->itemType() == PageItem::PathText))
			return;
		shapePal->setRoundRectEnabled(frameType == 0);
	}
}


void PropertiesFramePalette::updateColorList()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	groupPal->updateColorList();
	colorPal->updateColorList();
//	colorPicker->updateColorList();
	transparencyPal->updateColorList();
	shadowPal->updateColorList();

	assert (m_doc->PageColors.document());
}

bool PropertiesFramePalette::userActionOn()
{
	bool userActionOn = false;
	userActionOn  = xyzPal->userActionOn();
	userActionOn  |= xyzTransformPal->userActionOn();
	return userActionOn;
}





void PropertiesFramePalette::setGradientEditMode(bool on)
{
	colorPal->gradEditButton->setChecked(on);
}

void PropertiesFramePalette::endPatchAdd()
{
	colorPal->endPatchAdd();
}

void PropertiesFramePalette::updateColorSpecialGradient()
{

	groupPal->updateColorSpecialGradient();
	colorPal->updateColorSpecialGradient();
	transparencyPal->updateColorSpecialGradient();

}

void PropertiesFramePalette::setLocked(bool isLocked)
{
	shapePal->setLocked(isLocked);
}

void PropertiesFramePalette::handleShapeEdit()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
		shapePal->setRoundRectEnabled(false);
}

void PropertiesFramePalette::setTextFlowMode(PageItem::TextFlowMode mode)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning() || !m_haveItem)
		return;
	shapePal->showTextFlowMode(mode);
	groupPal->showTextFlowMode(mode);
}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertiesFramePalette::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	if(e->type() == QEvent::Resize){
		m_scrollArea->setMinimumWidth(this->width());
		return;
	}
	ScDockPalette::changeEvent(e);
}

void PropertiesFramePalette::closeEvent(QCloseEvent *closeEvent)
{
	if (m_ScMW && !m_ScMW->scriptIsRunning())
	{
		if ((m_haveDoc) && (m_haveItem))
		{
			if (colorPal->gradEditButton->isChecked())
			{
				m_ScMW->view->requestMode(modeNormal);
				m_ScMW->view->RefreshGradient(m_item);
			}
		}
	}
	ScDockPalette::closeEvent(closeEvent);
}
