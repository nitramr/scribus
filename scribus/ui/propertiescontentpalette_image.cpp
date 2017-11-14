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

	m_doc = d;

	imageWidget->setDoc(m_doc);
	imageSettingsWidget->setDoc(m_doc);
}

void PropertiesContentPalette_Image::unsetDoc()
{

	imageWidget->unsetItem();
	imageWidget->unsetDoc();
	imageSettingsWidget->unsetItem();
	imageSettingsWidget->unsetDoc();

}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertiesContentPalette_Image::unsetItem()
{

	imageWidget->unsetItem();
	imageSettingsWidget->unsetItem();

	handleSelectionChanged();
}

void PropertiesContentPalette_Image::setCurrentItem(PageItem *i)
{

	imageWidget->setCurrentItem(i);
	imageSettingsWidget->setCurrentItem(i);

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

	imageWidget->handleSelectionChanged();
	imageSettingsWidget->handleSelectionChanged();
}


void PropertiesContentPalette_Image::unitChange()
{

	imageWidget->unitChange();

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
