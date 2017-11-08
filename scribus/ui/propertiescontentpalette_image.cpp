/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "propertiescontentpalette_image.h"

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <QSignalBlocker>

#include "appmodes.h"
#include "pageitem.h"
#include "pageitem_table.h"
#include "pageitem_textframe.h"
#include "propertiespalette_utils.h"
#include "propertywidgetimage_image.h"
#include "propertywidgetimage_imagesettings.h"
#include "scpopupmenu.h"
#include "scraction.h"
#include "scribuscore.h"
#include "selection.h"
#include "tabmanager.h"
#include "undomanager.h"
#include "units.h"
#include "util.h"
#include "util_math.h"
#include "langmgr.h"

PropertiesContentPalette_Image::PropertiesContentPalette_Image( QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_haveDoc = false;
	m_haveItem = false;
	m_unitRatio = 1.0;

	imageWidget = new PropertyWidgetImage_Image();

//	ScPopupMenu * popupFontFeatures = new ScPopupMenu(textAdvancedWidgets);
//	popupFontFeatures->addWidget(colorWidgets);
//	popupFontFeatures->addWidget(fontfeaturesWidget);
//	ScPopupMenu * popupOrphans = new ScPopupMenu(orphanBox);
//	ScPopupMenu * popupHyphenation = new ScPopupMenu(hyphenationWidget);


	layoutSectionImage = new ScLayoutSection(tr("Image"));
	layoutSectionImage->addWidget(imageWidget);

	imageSettingsWidget = new PropertyWidgetImage_ImageSettings();
	layoutSectionImageSettings = new ScLayoutSection(tr("Image Settings"));
	layoutSectionImageSettings->addWidget(imageSettingsWidget);

	FlowLayout *flowLayout = new FlowLayout(0,0,0);
	flowLayout->addWidget(layoutSectionImage);
	flowLayout->addWidget(layoutSectionImageSettings);

	setLayout(flowLayout);

	languageChange();

	m_haveItem = false;
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertiesContentPalette_Image::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;


	imageWidget->setMainWindow(mw);
	imageSettingsWidget->setMainWindow(mw);


}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertiesContentPalette_Image::setDoc(ScribusDoc *d)
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

	imageWidget->setDoc(m_doc);
	imageSettingsWidget->setDoc(m_doc);


	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));

	// Handle properties update when switching document
	handleSelectionChanged();
}

void PropertiesContentPalette_Image::unsetDoc()
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

	imageWidget->unsetItem();
	imageWidget->unsetDoc();
	imageSettingsWidget->unsetItem();
	imageSettingsWidget->unsetDoc();

	m_haveItem = false;

}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertiesContentPalette_Image::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;

	imageWidget->unsetItem();
	imageSettingsWidget->unsetItem();

	handleSelectionChanged();
}

void PropertiesContentPalette_Image::setCurrentItem(PageItem *i)
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

	m_haveItem = false;
	m_item = i;

	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
//		imageWidget->setEnabled(false);
//		imageSettingsWidget->setEnabled(false);
		layoutSectionImage->setEnabled(false);
		layoutSectionImageSettings->setEnabled(false);
	}

	m_haveItem = true;



	if (!sender() || (m_doc->appMode == modeEditTable))
	{
		imageWidget->handleSelectionChanged();
		imageSettingsWidget->handleSelectionChanged();
	}

	if (m_item->asOSGFrame())
	{
//		imageWidget->setEnabled(false);
//		imageSettingsWidget->setEnabled(false);
		layoutSectionImage->setEnabled(false);
		layoutSectionImageSettings->setEnabled(false);
	}
	if (m_item->asSymbolFrame())
	{
//		imageWidget->setEnabled(false);
//		imageSettingsWidget->setEnabled(false);
		layoutSectionImage->setEnabled(false);
		layoutSectionImageSettings->setEnabled(false);
	}

}


PageItem* PropertiesContentPalette_Image::currentItemFromSelection()
{
	PageItem *currentItem = NULL;

	if (m_doc)
	{
		if (m_doc->m_Selection->count() > 0)
			currentItem = m_doc->m_Selection->itemAt(0);

	}

	return currentItem;
}


/*********************************************************************
*
* Features
*
**********************************************************************/

void PropertiesContentPalette_Image::showScaleAndOffset(double scx, double scy, double x, double y){

	imageWidget->showScaleAndOffset(scx,scy,x,y);

}


/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertiesContentPalette_Image::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{

	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem   = (itemType != -1);

		switch (itemType)
		{
		case PageItem::ImageFrame:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
			if (currItem->asOSGFrame())
			{
				//TabStack->setItemEnabled(idImageItem, false);
//				imageWidget->setEnabled(false);
//				imageSettingsWidget->setEnabled(false);
				layoutSectionImage->setEnabled(false);
				layoutSectionImageSettings->setEnabled(false);
			}
			else
			{
				//TabStack->setItemEnabled(idImageItem, true);
//				imageWidget->setEnabled(true);
//				imageSettingsWidget->setEnabled(true);
				layoutSectionImage->setEnabled(true);
				layoutSectionImageSettings->setEnabled(true);
			}
			break;
		case -1:
		case PageItem::TextFrame:
		case PageItem::Line:
		case PageItem::ItemType1:
		case PageItem::ItemType3:
		case PageItem::Polygon:
		case PageItem::RegularPolygon:
		case PageItem::Arc:
		case PageItem::PolyLine:
		case PageItem::Spiral:
		case PageItem::PathText:
		case PageItem::Symbol:
		case PageItem::Group:
		case PageItem::Table:
			//TabStack->setItemEnabled(idImageItem, false);
			layoutSectionImage->setEnabled(false);
			layoutSectionImageSettings->setEnabled(false);
//			imageWidget->setEnabled(false);
//			imageSettingsWidget->setEnabled(false);
			break;
		}
	}
	updateGeometry();
	update();
}


void PropertiesContentPalette_Image::unitChange()
{
	if (!m_haveDoc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;

	imageWidget->unitChange();

	m_haveItem = tmp;
}

void PropertiesContentPalette_Image::languageChange()
{

	imageWidget->languageChange();
	imageSettingsWidget->languageChange();

}

bool PropertiesContentPalette_Image::userActionOn()
{
	bool userActionOn = false;
	userActionOn = imageWidget->userActionOn();
//	userActionOn |= imageSettingsWidget->userActionOn();
	return userActionOn;
}


/*********************************************************************
*
* Event
*
**********************************************************************/

void PropertiesContentPalette_Image::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
