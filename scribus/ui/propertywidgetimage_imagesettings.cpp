/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "propertywidgetimage_imagesettings.h"

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <QSignalBlocker>

#include "commonstrings.h"
#include "pageitem.h"
#include "propertiespalette_utils.h"
#include "sccolorengine.h"
#include "sccombobox.h"
#include "scribuscore.h"
#include "scraction.h"
#include "scribusview.h"
#include "selection.h"
#include "units.h"
#include "undomanager.h"
#include "util.h"
#include "util_math.h"

PropertyWidgetImage_ImageSettings::PropertyWidgetImage_ImageSettings( QWidget* parent) : QWidget(parent)
{
	m_ScMW = 0;
	m_doc  = 0;
	m_haveDoc    = false;
	m_haveItem   = false;
	m_item       = 0;

	setupUi(this);

	languageChange();


	// Colo Management
	connect(inputProfiles      , SIGNAL(activated(const QString&)), this, SLOT(handleProfile(const QString&)));
	connect(renderIntent       , SIGNAL(activated(int))      , this, SLOT(handleIntent()));

	// Compression
	connect(compressionMethod  , SIGNAL(activated(int))      , this, SLOT(handleCompressionMethod()));
	connect(compressionQuality , SIGNAL(activated(int))      , this, SLOT(handleCompressionQuality()));
}



/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetImage_ImageSettings::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this  , SLOT(handleUpdateRequest(int)));
}


/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidgetImage_ImageSettings::setDoc(ScribusDoc *d)
{
	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_doc  = d;
	m_item = NULL;
	m_haveDoc  = true;
	m_haveItem = false;

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}


void PropertyWidgetImage_ImageSettings::unsetDoc()
{
	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_haveDoc  = false;
	m_haveItem = false;
	m_doc   = NULL;
	m_item  = NULL;

	setEnabled(false);
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidgetImage_ImageSettings::setCurrentItem(PageItem *item)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	//CB We shouldn't really need to process this if our item is the same one
	//maybe we do if the item has been changed by scripter.. but that should probably
	//set some status if so.
	//FIXME: This won't work until when a canvas deselect happens, m_item must be NULL.
	//if (m_item == i)
	//	return;

	if (!m_doc)
		setDoc(item->doc());

	m_haveItem = false;
	m_item = item;

	if (m_item->asImageFrame())
	{

		compressionMethod->setCurrentIndex(m_item->OverrideCompressionMethod ? m_item->CompressionMethodIndex + 1 : 0);
		compressionQuality->setCurrentIndex(m_item->OverrideCompressionQuality ? m_item->CompressionQualityIndex + 1 : 0);

	}
	m_haveItem = true;


	if (m_item->asImageFrame())
	{
		updateProfileList();
	}
	if (m_item->asOSGFrame())
	{
		setEnabled(false);
	}
	if (m_item->asSymbolFrame())
	{
		setEnabled(false);
	}
}

void PropertyWidgetImage_ImageSettings::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	handleSelectionChanged();
}

PageItem* PropertyWidgetImage_ImageSettings::currentItemFromSelection()
{
	PageItem *currentItem = NULL;

	if (m_doc)
	{
		if (m_doc->m_Selection->count() > 1)
		{
			currentItem = m_doc->m_Selection->itemAt(0);
		}
		else if (m_doc->m_Selection->count() == 1)
		{
			currentItem = m_doc->m_Selection->itemAt(0);
		}
	}

	return currentItem;
}


void PropertyWidgetImage_ImageSettings::showCMSOptions()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if (m_haveItem)
		updateProfileList();
	else if (m_doc)
		colorMgmtGroup->setVisible(ScCore->haveCMS() && m_doc->cmsSettings().CMSinUse);
}


/*********************************************************************
*
* Features Color Management
*
**********************************************************************/

void PropertyWidgetImage_ImageSettings::handleProfile(const QString& prn)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_SetColorProfile(inputProfiles->currentText());
}

void PropertyWidgetImage_ImageSettings::handleIntent()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_SetRenderIntent(renderIntent->currentIndex());
}

void PropertyWidgetImage_ImageSettings::updateProfileList()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if (m_haveDoc)
	{
		if (ScCore->haveCMS() && m_doc->cmsSettings().CMSinUse)
			colorMgmtGroup->show();
		else
		{
			colorMgmtGroup->hide();
			return;
		}

		inputProfiles->blockSignals(true);
		renderIntent->blockSignals(true);

		inputProfiles->clear();
		if (m_haveItem)
		{
			if (m_item->pixm.imgInfo.colorspace == ColorSpaceCMYK)
			{
				ProfilesL::Iterator itP;
				ProfilesL::Iterator itPend = ScCore->InputProfilesCMYK.end();
				for (itP = ScCore->InputProfilesCMYK.begin(); itP != itPend; ++itP)
				{
					inputProfiles->addItem(itP.key());
					if (itP.key() == m_item->IProfile)
						inputProfiles->setCurrentIndex(inputProfiles->count()-1);
				}
				if (!ScCore->InputProfilesCMYK.contains(m_item->IProfile))
				{
					inputProfiles->addItem(m_item->IProfile);
					inputProfiles->setCurrentIndex(inputProfiles->count()-1);
				}
				else
				{
					if (!m_item->EmProfile.isEmpty())
						inputProfiles->addItem(m_item->EmProfile);
				}
			}
			else
			{
				ProfilesL::Iterator itP;
				ProfilesL::Iterator itPend=ScCore->InputProfiles.end();
				for (itP = ScCore->InputProfiles.begin(); itP != itPend; ++itP)
				{
					inputProfiles->addItem(itP.key());
					if (itP.key() == m_item->IProfile)
						inputProfiles->setCurrentIndex(inputProfiles->count()-1);
				}
				if (!ScCore->InputProfiles.contains(m_item->IProfile))
				{
					inputProfiles->addItem(m_item->IProfile);
					inputProfiles->setCurrentIndex(inputProfiles->count()-1);
				}
				else
				{
					if (!m_item->EmProfile.isEmpty())
						inputProfiles->addItem(m_item->EmProfile);
				}
			}
			renderIntent->setCurrentIndex(m_item->IRender);
		}

		inputProfiles->blockSignals(false);
		renderIntent->blockSignals(false);
	}
}


 
/*********************************************************************
*
* Features Compression
*
**********************************************************************/

void PropertyWidgetImage_ImageSettings::handleCompressionMethod()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_SetCompressionMethod(compressionMethod->currentIndex() - 1);
}

void PropertyWidgetImage_ImageSettings::handleCompressionQuality()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_SetCompressionQuality(compressionQuality->currentIndex() - 1);
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidgetImage_ImageSettings::languageChange()
{
	retranslateUi(this);


	QSignalBlocker renderIntentBlocker(renderIntent);
	int oldRenderI = renderIntent->currentIndex();
	renderIntent->clear();
	renderIntent->addItem( tr("Perceptual"));
	renderIntent->addItem( tr("Relative Colorimetric"));
	renderIntent->addItem( tr("Saturation"));
	renderIntent->addItem( tr("Absolute Colorimetric"));
	renderIntent->setCurrentIndex(oldRenderI);

	QSignalBlocker compressionMethodBlocker(compressionMethod);
	int oldCompressionMethod = compressionMethod->currentIndex();
	compressionMethod->clear();
	compressionMethod->addItem( tr( "Global" ) );
	compressionMethod->addItem( tr( "Automatic" ) );
	compressionMethod->addItem( tr( "Lossy - JPEG" ) );
	compressionMethod->addItem( tr( "Lossless - Zip" ) );
	compressionMethod->addItem( tr( "None" ) );
	compressionMethod->setCurrentIndex(oldCompressionMethod);

	QSignalBlocker compressionQualityBlocker(compressionQuality);
	int oldCompressionQuality = compressionQuality->currentIndex();
	compressionQuality->clear();
	compressionQuality->addItem( tr( "Global" ) );
	compressionQuality->addItem( tr( "Maximum" ) );
	compressionQuality->addItem( tr( "High" ) );
	compressionQuality->addItem( tr( "Medium" ) );
	compressionQuality->addItem( tr( "Low" ) );
	compressionQuality->addItem( tr( "Minimum" ) );
	compressionQuality->setCurrentIndex(oldCompressionQuality);

}


void PropertyWidgetImage_ImageSettings::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqCmsOptionsUpdate)
		showCMSOptions();
}


void PropertyWidgetImage_ImageSettings::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{
		setEnabled(false);
	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem = (itemType != -1);

		switch (itemType)
		{
		case -1:
			setEnabled(false);
			break;
		case PageItem::ImageFrame:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
			setEnabled(currItem->asOSGFrame() == NULL);
			break;
		case PageItem::TextFrame:
		case PageItem::Line:
		case PageItem::Arc:
		case PageItem::ItemType1:
		case PageItem::ItemType3:
		case PageItem::Polygon:
		case PageItem::RegularPolygon:
		case PageItem::PolyLine:
		case PageItem::PathText:
		case PageItem::Symbol:
			setEnabled(false);
			break;
		}
	}
	if (currItem)
	{
		setCurrentItem(currItem);
	}
}


/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetImage_ImageSettings::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
