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
#include "propertywidgettext_textfont.h"
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
	m_item=0;
	m_haveDoc = false;
	m_haveItem = false;

	textWidgets = new PropertyWidgetText_TextFont();

//	ScPopupMenu * popupFontFeatures = new ScPopupMenu(textAdvancedWidgets);
//	popupFontFeatures->addWidget(colorWidgets);
//	popupFontFeatures->addWidget(fontfeaturesWidget);
//	ScPopupMenu * popupOrphans = new ScPopupMenu(orphanBox);
//	ScPopupMenu * popupHyphenation = new ScPopupMenu(hyphenationWidget);


	layoutSectionText = new ScLayoutSection(tr("Text"));

	layoutSectionText->addWidget(textWidgets);

	FlowLayout *flowLayout = new FlowLayout(0,0,0);
	flowLayout->addWidget(layoutSectionText);

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


	textWidgets->setMainWindow(mw);


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

	m_doc  = d;
	m_item = NULL;

	m_haveDoc  = true;
	m_haveItem = false;


	textWidgets->setDoc(m_doc);


	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertiesContentPalette_Image::unsetDoc()
{
	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_haveDoc  = false;
	m_haveItem = false;
	m_doc      = NULL;
	m_item     = NULL;


	textWidgets->setDoc(0);

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

	handleSelectionChanged();
}

PageItem* PropertiesContentPalette_Image::currentItemFromSelection()
{
	PageItem *currentItem = NULL;

	if (m_doc)
	{
		if (m_doc->m_Selection->count() > 1)
			currentItem = m_doc->m_Selection->itemAt(0);
		else if (m_doc->m_Selection->count() == 1)
			currentItem = m_doc->m_Selection->itemAt(0);
		if (currentItem  && currentItem->isTable() && m_doc->appMode == modeEditTable)
			currentItem = currentItem->asTable()->activeCell().textFrame();
	}

	return currentItem;
}

/*********************************************************************
*
* Features
*
**********************************************************************/


void PropertiesContentPalette_Image::showFontSize(double s)
{
	textWidgets->showFontSize(s);
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

	if (currItem)
	{

		//CB We shouldn't really need to process this if our item is the same one
		//maybe we do if the item has been changed by scripter.. but that should probably
		//set some status if so.

		if (!m_doc)
			setDoc(currItem->doc());

		m_item = currItem;
		m_haveItem = true;

		if (m_item->asTextFrame() || m_item->asPathText() || m_item->asTable())
		{
			ParagraphStyle parStyle =  m_item->itemText.defaultStyle();
			if (m_doc->appMode == modeEdit || m_doc->appMode == modeEditTable)
				m_item->currentTextProps(parStyle);
			updateStyle(parStyle);
		}
	}

}


void PropertiesContentPalette_Image::unitChange()
{
	if (!m_haveDoc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;



	m_haveItem = tmp;
}

void PropertiesContentPalette_Image::languageChange()
{

	textWidgets->languageChange();

}

void PropertiesContentPalette_Image::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;


	textWidgets->updateCharStyle(charStyle);

}

void PropertiesContentPalette_Image::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;


	textWidgets->updateStyle(newCurrent);

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
