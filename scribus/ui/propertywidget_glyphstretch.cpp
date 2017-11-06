/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_glyphstretch.h"

#include "appmodes.h"
#include "iconmanager.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
#include "units.h"

PropertyWidget_GlyphStretch::PropertyWidget_GlyphStretch(QWidget* parent) : QWidget(parent)
{
	m_item = NULL;
	m_ScMW = NULL;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	setupUi(this);

	// Glyph Scale
	scaleH->setValues(10, 400, 2, 100 );
	scaleHLabel->setPixmap(IconManager::instance()->loadPixmap("textscaleh.png"));

	scaleV->setValues(10, 400, 2, 100 );
	scaleVLabel->setPixmap(IconManager::instance()->loadPixmap("textscalev.png"));

	minGlyphExtensionLabel->setBuddy(minGlyphExtSpinBox);
	maxGlyphExtensionLabel->setBuddy(maxGlyphExtSpinBox);

	languageChange();
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidget_GlyphStretch::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;
}

void PropertyWidget_GlyphStretch::connectSignals()
{
	// Glyph Scale
	connect(scaleH  , SIGNAL(valueChanged(double)), this, SLOT(handleTextScaleH()));
	connect(scaleV  , SIGNAL(valueChanged(double)), this, SLOT(handleTextScaleV()));
	connect(minGlyphExtSpinBox     , SIGNAL(valueChanged(double)), this, SLOT(handleMinGlyphExtension()) );
	connect(maxGlyphExtSpinBox     , SIGNAL(valueChanged(double)), this, SLOT(handleMaxGlyphExtension()) );

//	// Hyphenation
//	connect( hyphenateButton, SIGNAL(pressed()), this, SLOT(handleHyphenate()) );
//	connect( dehyphenateButton, SIGNAL(pressed()), this, SLOT(handleDeHyphenate()) );

}

void PropertyWidget_GlyphStretch::disconnectSignals()
{
	// Glyph Scale
	disconnect(scaleH  , SIGNAL(valueChanged(double)), this, SLOT(handleTextScaleH()));
	disconnect(scaleV  , SIGNAL(valueChanged(double)), this, SLOT(handleTextScaleV()));
	disconnect(minGlyphExtSpinBox     , SIGNAL(valueChanged(double)), this, SLOT(handleMinGlyphExtension()) );
	disconnect(maxGlyphExtSpinBox     , SIGNAL(valueChanged(double)), this, SLOT(handleMaxGlyphExtension()) );

//	// Hyphenation
//	disconnect( hyphenateButton, SIGNAL(pressed()), this, SLOT(handleHyphenate()) );
//	disconnect( dehyphenateButton, SIGNAL(pressed()), this, SLOT(handleDeHyphenate()) );
}

void PropertyWidget_GlyphStretch::configureWidgets(void)
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

void PropertyWidget_GlyphStretch::setDoc(ScribusDoc *d)
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

	// Glyph Scale
	minGlyphExtSpinBox->setValues(90, 110, 2, 100);
	maxGlyphExtSpinBox->setValues(90, 110, 2, 100);

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidget_GlyphStretch::setCurrentItem(PageItem *item)
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
* Hyphenation
*
**********************************************************************/

//void PropertyWidget_GlyphStretch::handleHyphenate(){

//	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;

//	m_doc->itemSelection_DoHyphenate();

//}

//void PropertyWidget_GlyphStretch::handleDeHyphenate(){

//	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;

//	m_doc->itemSelection_DoDeHyphenate();
//}


/*********************************************************************
*
* Glyph Scale
*
**********************************************************************/

void PropertyWidget_GlyphStretch::showTextScaleH(double e)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	scaleH->showValue(e / 10.0);
}

void PropertyWidget_GlyphStretch::showTextScaleV(double e)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	scaleV->showValue(e / 10.0);
}



void PropertyWidget_GlyphStretch::handleMinGlyphExtension()
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
		newStyle.setMinGlyphExtension(minGlyphExtSpinBox->value() / 100.0);
		m_doc->itemSelection_ApplyParagraphStyle(newStyle, &tempSelection);
	}
}

void PropertyWidget_GlyphStretch::handleMaxGlyphExtension()
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
		newStyle.setMaxGlyphExtension(maxGlyphExtSpinBox->value() / 100.0);
		m_doc->itemSelection_ApplyParagraphStyle(newStyle, &tempSelection);
	}
}

void PropertyWidget_GlyphStretch::handleTextScaleH()
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
		m_doc->itemSelection_SetScaleH(qRound(scaleH->value() * 10), &tempSelection);
	}
}

void PropertyWidget_GlyphStretch::handleTextScaleV()
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
		m_doc->itemSelection_SetScaleV(qRound(scaleV->value() * 10), &tempSelection);
	}
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidget_GlyphStretch::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	showTextScaleH(charStyle.scaleH());
	showTextScaleV(charStyle.scaleV());

}

void PropertyWidget_GlyphStretch::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showTextScaleH(charStyle.scaleH());
	showTextScaleV(charStyle.scaleV());

	minGlyphExtSpinBox->showValue(newCurrent.minGlyphExtension() * 100.0);
	maxGlyphExtSpinBox->showValue(newCurrent.maxGlyphExtension() * 100.0);
}

void PropertyWidget_GlyphStretch::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	setCurrentItem(currItem);
	updateGeometry();
}

void PropertyWidget_GlyphStretch::languageChange()
{
	retranslateUi(this);
}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidget_GlyphStretch::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
