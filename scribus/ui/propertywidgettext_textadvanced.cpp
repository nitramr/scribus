/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgettext_textadvanced.h"

#include "appmodes.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
//#include "units.h" //
#include "langmgr.h"

//#include "sccolorfillsbox.h"
//#include "sccolorpicker.h"
//#include "scpopupmenu.h"

PropertyWidgetText_TextAdvanced::PropertyWidgetText_TextAdvanced(QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_item=0;
	m_haveDoc = false;
	m_haveItem = false;

	setupUi(this);


	languageChange();

	// Language
	connect(langCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLang(int)));

	m_haveItem = false;
	setEnabled(false);
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetText_TextAdvanced::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;
	connect(m_ScMW, SIGNAL(UpdateRequest(int))     , this  , SLOT(handleUpdateRequest(int))); // neccessary
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidgetText_TextAdvanced::setDoc(ScribusDoc *d)
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

	//connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged())); // -> Crash Signal 11
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidgetText_TextAdvanced::unsetDoc()
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

	setEnabled(false);
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidgetText_TextAdvanced::setCurrentItem(PageItem *i)
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


PageItem* PropertyWidgetText_TextAdvanced::currentItemFromSelection()
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
* Language
*
**********************************************************************/

void PropertyWidgetText_TextAdvanced::changeLang(int id)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	QStringList languageList;
	LanguageManager::instance()->fillInstalledStringList(&languageList);
	QString abrv = LanguageManager::instance()->getAbbrevFromLang(languageList.value(id),false);
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetLanguage(abrv, &tempSelection);
}


void PropertyWidgetText_TextAdvanced::showLanguage(QString w)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	QStringList lang;
	LanguageManager::instance()->fillInstalledStringList(&lang);
	QString langName = LanguageManager::instance()->getLangFromAbbrev(w, true);

	bool sigBlocked  = langCombo->blockSignals(true);
	langCombo->setCurrentIndex(lang.indexOf(langName));
	langCombo->blockSignals(sigBlocked);
}


/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidgetText_TextAdvanced::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	showLanguage(charStyle.language());
}

void PropertyWidgetText_TextAdvanced::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showLanguage(charStyle.language());

}

void PropertyWidgetText_TextAdvanced::handleSelectionChanged()
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

void PropertyWidgetText_TextAdvanced::languageChange()
{
	retranslateUi(this);

	QSignalBlocker langComboBlocker(langCombo);
	QStringList languageList;
	LanguageManager::instance()->fillInstalledStringList(&languageList);
	int oldLang = langCombo->currentIndex();
	langCombo->clear();
	langCombo->addItems(languageList);
	langCombo->setCurrentIndex(oldLang);

}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetText_TextAdvanced::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
