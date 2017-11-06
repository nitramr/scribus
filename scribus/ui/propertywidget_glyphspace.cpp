/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_glyphspace.h"

#include "appmodes.h"
#include "iconmanager.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
#include "units.h"

PropertyWidget_GlyphSpace::PropertyWidget_GlyphSpace(QWidget* parent) : QWidget(parent)
{
	m_item = NULL;
	m_ScMW = NULL;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	setupUi(this);

	// Space
	textBase->setValue( 0 );
	textBaseLabel->setPixmap(IconManager::instance()->loadPixmap("textbase.png"));
	trackingLabel->setPixmap(IconManager::instance()->loadPixmap("textkern.png"));

	minWordTrackingLabel->setBuddy(minWordTrackingSpinBox);
	normWordTrackingLabel->setBuddy(normWordTrackingSpinBox);

	languageChange();
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidget_GlyphSpace::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;
}

void PropertyWidget_GlyphSpace::connectSignals()
{
	// Space
	connect(textBase, SIGNAL(valueChanged(double)), this, SLOT(handleBaselineOffset()));
	connect(tracking, SIGNAL(valueChanged(double)), this, SLOT(handleTracking()));
	connect(minWordTrackingSpinBox , SIGNAL(valueChanged(double)), this, SLOT(handleMinWordTracking()) );
	connect(normWordTrackingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(handleNormWordTracking()) );
}

void PropertyWidget_GlyphSpace::disconnectSignals()
{
	// Space
	disconnect(textBase, SIGNAL(valueChanged(double)), this, SLOT(handleBaselineOffset()));
	disconnect(tracking, SIGNAL(valueChanged(double)), this, SLOT(handleTracking()));
	disconnect(minWordTrackingSpinBox , SIGNAL(valueChanged(double)), this, SLOT(handleMinWordTracking()) );
	disconnect(normWordTrackingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(handleNormWordTracking()) );
}

void PropertyWidget_GlyphSpace::configureWidgets(void)
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

void PropertyWidget_GlyphSpace::setDoc(ScribusDoc *d)
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

	m_unitRatio   = m_doc->unitRatio();
	m_unitIndex   = m_doc->unitIndex();

	// Space
	tracking->setValues( -300, 300, 2, 0);
	minWordTrackingSpinBox->setValues(1, 100, 2, 100);
	normWordTrackingSpinBox->setValues(1, 2000, 2, 100);

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidget_GlyphSpace::setCurrentItem(PageItem *item)
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
		if (m_item->asTextFrame() || m_item->asPathText() || m_item->asTable())
		{
			ParagraphStyle parStyle =  m_item->itemText.defaultStyle();
			if (m_doc->appMode == modeEdit || m_doc->appMode == modeEditTable)
				m_item->currentTextProps(parStyle);
			updateStyle(parStyle);
		}
		connectSignals();
	}
}


/*********************************************************************
*
* Space
*
**********************************************************************/



void PropertyWidget_GlyphSpace::showBaseLineOffset(double e)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	textBase->showValue(e / 10.0);
}


void PropertyWidget_GlyphSpace::showTracking(double e)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	tracking->showValue(e / 10.0);
}

void PropertyWidget_GlyphSpace::handleBaselineOffset()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	PageItem *i2 = m_item;
	if (m_doc->appMode == modeEditTable)
		i2 = m_item->asTable()->activeCell().textFrame();
	if (i2 != NULL)
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(i2, true);
		m_doc->itemSelection_SetBaselineOffset(qRound(textBase->value() * 10), &tempSelection);
	}
}

void PropertyWidget_GlyphSpace::handleMinWordTracking()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	PageItem *i2 = m_item;
	if (m_doc->appMode == modeEditTable)
		i2 = m_item->asTable()->activeCell().textFrame();
	if (i2 != NULL)
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(i2, true);
		ParagraphStyle newStyle;
		newStyle.setMinWordTracking(minWordTrackingSpinBox->value() / 100.0);
		m_doc->itemSelection_ApplyParagraphStyle(newStyle, &tempSelection);
	}
}

void PropertyWidget_GlyphSpace::handleNormWordTracking()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	PageItem *i2 = m_item;
	if (m_doc->appMode == modeEditTable)
		i2 = m_item->asTable()->activeCell().textFrame();
	if (i2 != NULL)
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(i2, true);
		ParagraphStyle newStyle;
		newStyle.charStyle().setWordTracking(normWordTrackingSpinBox->value() / 100.0);
		m_doc->itemSelection_ApplyParagraphStyle(newStyle, &tempSelection);
	}
}

void PropertyWidget_GlyphSpace::handleTracking()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	PageItem *i2 = m_item;
	if (m_doc->appMode == modeEditTable)
		i2 = m_item->asTable()->activeCell().textFrame();
	if (i2 != NULL)
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(i2, true);
		m_doc->itemSelection_SetTracking(qRound(tracking->value() * 10.0), &tempSelection);
	}
}


/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidget_GlyphSpace::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	showTracking(charStyle.tracking());
	showBaseLineOffset(charStyle.baselineOffset());

	normWordTrackingSpinBox->showValue(charStyle.wordTracking() * 100.0);
}

void PropertyWidget_GlyphSpace::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showTracking(charStyle.tracking());
	showBaseLineOffset(charStyle.baselineOffset());

	minWordTrackingSpinBox->showValue(newCurrent.minWordTracking() * 100.0);
	normWordTrackingSpinBox->showValue(newCurrent.charStyle().wordTracking() * 100.0);
}

void PropertyWidget_GlyphSpace::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	setCurrentItem(currItem);
	updateGeometry();
}

void PropertyWidget_GlyphSpace::languageChange()
{
	retranslateUi(this);
}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidget_GlyphSpace::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
