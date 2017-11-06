/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgettext_hyphenate.h"

#include "appmodes.h"
#include "iconmanager.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
#include "units.h"

PropertyWidgetText_Hyphenate::PropertyWidgetText_Hyphenate(QWidget* parent) : QWidget(parent)
{
	m_item = NULL;
	m_ScMW = NULL;

	setupUi(this);

	languageChange();
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetText_Hyphenate::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;
}

void PropertyWidgetText_Hyphenate::connectSignals()
{
	// Hyphenation
	connect( hyphenateButton, SIGNAL(pressed()), this, SLOT(handleHyphenate()) );
	connect( dehyphenateButton, SIGNAL(pressed()), this, SLOT(handleDeHyphenate()) );

}

void PropertyWidgetText_Hyphenate::disconnectSignals()
{
	// Hyphenation
	disconnect( hyphenateButton, SIGNAL(pressed()), this, SLOT(handleHyphenate()) );
	disconnect( dehyphenateButton, SIGNAL(pressed()), this, SLOT(handleDeHyphenate()) );
}

void PropertyWidgetText_Hyphenate::configureWidgets(void)
{
	bool enabled = false;
	if (m_item && m_doc)
	{
		if (m_item->asPathText() || m_item->asTextFrame() || m_item->asTable())
			enabled = true;
		if ((m_item->isGroup()) && (!m_item->isSingleSel))
			enabled = false;
		if (m_item->asOSGFrame() || m_item->asSymbolFrame())
			enabled = false;
		if (m_doc->m_Selection->count() > 1)
			enabled = false;
	}
	setEnabled(enabled);
}



/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidgetText_Hyphenate::setDoc(ScribusDoc *d)
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

	if (m_doc.isNull())
	{
		disconnectSignals();
		return;
	}

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidgetText_Hyphenate::setCurrentItem(PageItem *item)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	//CB We shouldn't really need to process this if our item is the same one
	//maybe we do if the item has been changed by scripter.. but that should probably
	//set some status if so.
	//FIXME: This won't work until when a canvas deselect happens, m_item must be NULL.
	//if (m_item == i)
	//	return;

	if (item && m_doc.isNull())
		setDoc(item->doc());

	m_item = item;

	disconnectSignals();
	configureWidgets();

	if (m_item)
	{
		connectSignals();
	}
}


/*********************************************************************
*
* Hyphenation
*
**********************************************************************/

void PropertyWidgetText_Hyphenate::handleHyphenate(){

	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	m_doc->itemSelection_DoHyphenate();

}

void PropertyWidgetText_Hyphenate::handleDeHyphenate(){

	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	m_doc->itemSelection_DoDeHyphenate();
}



/*********************************************************************
*
* Update Helper
*
**********************************************************************/


void PropertyWidgetText_Hyphenate::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	setCurrentItem(currItem);
	updateGeometry();
}

void PropertyWidgetText_Hyphenate::languageChange()
{
	retranslateUi(this);
}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetText_Hyphenate::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
