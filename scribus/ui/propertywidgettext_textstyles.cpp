/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgettext_textstyles.h"

#include "appmodes.h"
#include "iconmanager.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"

#include "sccolorfillsbox.h"
#include "scpopupmenu.h"

PropertyWidgetText_TextStyles::PropertyWidgetText_TextStyles(QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_item=0;
	m_haveDoc = false;
	m_haveItem = false;

	setupUi(this);

	paraStyleLabel->setBuddy(paraStyleCombo);
	paraStyleClear->setIcon(IconManager::instance()->loadPixmap("16/edit-clear.png"));
	charStyleLabel->setBuddy(charStyleCombo);
	charStyleClear->setIcon(IconManager::instance()->loadPixmap("16/edit-clear.png"));


	languageChange();

	connect(charStyleClear, SIGNAL(clicked()), this, SLOT(doClearCStyle()));
	connect(paraStyleClear, SIGNAL(clicked()), this, SLOT(doClearPStyle()));

	m_haveItem = false;
	setEnabled(false);
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetText_TextStyles::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int))     , this  , SLOT(handleUpdateRequest(int)));

	connect(paraStyleCombo, SIGNAL(newStyle(const QString&)), m_ScMW, SLOT(setNewParStyle(const QString&)), Qt::UniqueConnection);
	connect(charStyleCombo, SIGNAL(newStyle(const QString&)), m_ScMW, SLOT(setNewCharStyle(const QString&)), Qt::UniqueConnection);
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidgetText_TextStyles::setDoc(ScribusDoc *d)
{

	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		//disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_doc  = d;
	m_item = NULL;

	m_haveDoc  = true;
	m_haveItem = false;

	paraStyleCombo->setDoc(m_doc);
	charStyleCombo->setDoc(m_doc);

//	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));  // -> Crash Signal 11
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));	
}

void PropertyWidgetText_TextStyles::unsetDoc()
{
	if (m_doc)
	{
		//disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_haveDoc  = false;
	m_haveItem = false;
	m_doc      = NULL;
	m_item     = NULL;

	paraStyleCombo->setDoc(0);
	charStyleCombo->setDoc(0);


	m_haveItem = false;

	setEnabled(false);
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidgetText_TextStyles::setCurrentItem(PageItem *i)
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
		setDoc(i->doc());

	m_haveItem = false;
	m_item = i;

	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		setEnabled(false);
	}

	m_haveItem = true;


	if (m_item->asTextFrame() || m_item->asPathText() || m_item->asTable())
	{
		ParagraphStyle parStyle =  m_item->itemText.defaultStyle();
		if (m_doc->appMode == modeEdit || m_doc->appMode == modeEditTable)
			m_item->currentTextProps(parStyle);
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


PageItem* PropertyWidgetText_TextStyles::currentItemFromSelection()
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
* Styles
*
**********************************************************************/

void PropertyWidgetText_TextStyles::updateCharStyles()
{
	charStyleCombo->updateFormatList();
}

void PropertyWidgetText_TextStyles::updateParagraphStyles()
{
	paraStyleCombo->updateFormatList();
	charStyleCombo->updateFormatList();
}

void PropertyWidgetText_TextStyles::updateTextStyles()
{
	paraStyleCombo->updateFormatList();
	charStyleCombo->updateFormatList();
}


void PropertyWidgetText_TextStyles::showCharStyle(const QString& name)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	bool blocked = charStyleCombo->blockSignals(true);
	charStyleCombo->setFormat(name);
	charStyleCombo->blockSignals(blocked);
}

void PropertyWidgetText_TextStyles::showParStyle(const QString& name)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	bool blocked = paraStyleCombo->blockSignals(true);
	paraStyleCombo->setFormat(name);
	paraStyleCombo->blockSignals(blocked);
}

void PropertyWidgetText_TextStyles::doClearCStyle()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning() || !m_haveDoc || !m_haveItem)
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_EraseCharStyle(&tempSelection);
}


void PropertyWidgetText_TextStyles::doClearPStyle()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning() || !m_haveDoc || !m_haveItem)
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_ClearBulNumStrings(&tempSelection);
	m_doc->itemSelection_EraseParagraphStyle(&tempSelection);
	CharStyle emptyCStyle;
	m_doc->itemSelection_SetCharStyle(emptyCStyle, &tempSelection);
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidgetText_TextStyles::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1 )
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
			m_haveItem = false;
			setEnabled(false);
			break;
		case PageItem::TextFrame:
		case PageItem::PathText:
			setEnabled(true);
			break;
		case PageItem::Table:
			setEnabled(m_doc->appMode == modeEditTable);
			break;
		default:
			setEnabled(false);
			break;
		}
	}
	if (currItem)
	{
		setCurrentItem(currItem);
	}
	updateGeometry();
	//repaint();
}



void PropertyWidgetText_TextStyles::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showParStyle(newCurrent.parent());
	showCharStyle(charStyle.parent());

}



void PropertyWidgetText_TextStyles::handleUpdateRequest(int updateFlags)
{

	// ColorWidget will handle its update itself
	/*if (updateFlags & reqColorsUpdate)
		updateColorList();*/
	if (updateFlags & reqCharStylesUpdate)
	{
		charStyleCombo->updateFormatList();
	}
	if (updateFlags & reqParaStylesUpdate)
		paraStyleCombo->updateFormatList();
	if (updateFlags & reqStyleComboDocUpdate)
	{
		paraStyleCombo->setDoc(m_haveDoc ? m_doc : 0);
		charStyleCombo->setDoc(m_haveDoc ? m_doc : 0);
	}
}

void PropertyWidgetText_TextStyles::languageChange()
{
	retranslateUi(this);

}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetText_TextStyles::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}


