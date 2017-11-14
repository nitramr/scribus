/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "propertiescontentpalette_table.h"

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <QSignalBlocker>

#include "appmodehelper.h"
#include "appmodes.h"
#include "pageitem.h"
#include "pageitem_table.h"
#include "pageitem_textframe.h"
#include "propertiespalette_utils.h"
#include "propertywidgettable_table.h"
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

PropertiesContentPalette_Table::PropertiesContentPalette_Table( QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_haveDoc = false;
	m_haveItem = false;
	m_unitRatio = 1.0;

	tableWidget = new PropertyWidgetTable_Table();
	layoutSectionTable = new ScLayoutSection(tr("&Table"));
	layoutSectionTable->addWidget(tableWidget);


	FlowLayout *flowLayout = new FlowLayout(0,0,0);
	flowLayout->addWidget(layoutSectionTable);

	setLayout(flowLayout);

	languageChange();

	m_haveItem = false;
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertiesContentPalette_Table::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;


	tableWidget->setMainWindow(mw);

	connect(m_ScMW->appModeHelper, SIGNAL(AppModeChanged(int,int)), this, SLOT(AppModeChanged()));
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertiesContentPalette_Table::setDoc(ScribusDoc *d)
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

	tableWidget->setDoc(m_doc);

	updateColorList();

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));

	// Handle properties update when switching document
	handleSelectionChanged();
}

void PropertiesContentPalette_Table::unsetDoc()
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

	tableWidget->unsetItem();
	tableWidget->unsetDocument();

	m_haveItem = false;

}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertiesContentPalette_Table::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;

	tableWidget->unsetItem();

	handleSelectionChanged();
}

void PropertiesContentPalette_Table::setCurrentItem(PageItem *i)
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

		tableWidget->setItem(m_item);

		layoutSectionTable->setEnabled(true);


		if ((m_item->isGroup()) && (!m_item->isSingleSel))
		{
			layoutSectionTable->setVisible(false);
		}

		m_haveItem = true;

		if (!sender() || (m_doc->appMode == modeEditTable))
		{
			tableWidget->handleSelectionChanged();
		}

		if (m_item->asOSGFrame())
		{
			layoutSectionTable->setVisible(false);
		}
		if (m_item->asSymbolFrame())
		{
			layoutSectionTable->setVisible(false);
		}

}


PageItem* PropertiesContentPalette_Table::currentItemFromSelection()
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


/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertiesContentPalette_Table::AppModeChanged()
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

void PropertiesContentPalette_Table::handleSelectionChanged()
{

	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{
		layoutSectionTable->setEnabled(false);

	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem   = (itemType != -1);

		layoutSectionTable->setEnabled(false);

		switch (itemType)
		{
		case -1:
			m_haveItem = false;

			layoutSectionTable->setEnabled(false);

			break;
		case PageItem::ImageFrame:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
		case PageItem::Line:
		case PageItem::TextFrame:
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
			break;
		case PageItem::Table:
			layoutSectionTable->setEnabled(true);
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


void PropertiesContentPalette_Table::updateColorList()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	tableWidget->updateColorList();

	assert (m_doc->PageColors.document());
}

void PropertiesContentPalette_Table::languageChange()
{

	tableWidget->languageChange();

}


/*********************************************************************
*
* Event
*
**********************************************************************/

void PropertiesContentPalette_Table::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
