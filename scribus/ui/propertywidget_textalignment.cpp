/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_textalignment.h"
#include "appmodes.h"
#include "iconmanager.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "selection.h"

PropertyWidget_TextAlignment::PropertyWidget_TextAlignment(QWidget* parent) : QWidget(parent)
{
	m_item = NULL;
	m_ScMW = NULL;
	m_haveItem = false;
	m_haveDoc = false;

	setupUi(this);


	effectsLayout->setAlignment( Qt::AlignLeft );

	languageChange();

	// Alignment
	connect(textAlignment , SIGNAL(State(int))   , this, SLOT(handleAlignment(int)));
	connect(textDirection , SIGNAL(State(int))   , this, SLOT(handleDirection(int)));
	// Alignment End

}

void PropertyWidget_TextAlignment::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;
}

void PropertyWidget_TextAlignment::setDoc(ScribusDoc *d)
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

	if (m_doc.isNull())
	{
		disconnectSignals();
		return;
	}

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidget_TextAlignment::setCurrentItem(PageItem *item)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	//CB We shouldn't really need to process this if our item is the same one
	//maybe we do if the item has been changed by scripter.. but that should probably
	//set some status if so.
	//FIXME: This won't work until when a canvas deselect happens, m_item must be NULL.
	//if (m_item == i)
	//	return;

	disconnectSignals();
	m_haveItem = false;

	m_item = item;
	if (item && m_doc.isNull())
		setDoc(item->doc());

	m_haveItem = true;

	configureWidgets();

	if (m_item == NULL)
		return;
	if (!m_item->isTable() && !m_item->isTextFrame() && !m_item->asPathText())
		return;

	if (m_item->asTextFrame() || m_item->asPathText() || m_item->asTable())
	{
		ParagraphStyle parStyle =  m_item->itemText.defaultStyle();
		if (m_doc->appMode == modeEdit || m_doc->appMode == modeEditTable)
			m_item->currentTextProps(parStyle);
		updateStyle(parStyle);
	}

	//Alignment
	PageItem_TextFrame *textItem = m_item->asTextFrame();
	if (m_doc->appMode == modeEditTable)
		textItem = m_item->asTable()->activeCell().textFrame();
	if (!textItem) return;

	verticalAlign->setCurrentIndex(textItem->verticalAlignment());
		// Alignment End

	connectSignals();
}

void PropertyWidget_TextAlignment::connectSignals()
{
	connect(textEffects, SIGNAL(State(int))      , this, SLOT(handleTypeStyle(int)), Qt::UniqueConnection);
	connect(textEffects->ShadowVal->Xoffset  , SIGNAL(valueChanged(double)), this, SLOT(handleShadowOffs()), Qt::UniqueConnection);
	connect(textEffects->ShadowVal->Yoffset  , SIGNAL(valueChanged(double)), this, SLOT(handleShadowOffs()), Qt::UniqueConnection);
	connect(textEffects->OutlineVal->LWidth  , SIGNAL(valueChanged(double)), this, SLOT(handleOutlineWidth()), Qt::UniqueConnection);
	connect(textEffects->UnderlineVal->LPos  , SIGNAL(valueChanged(double)), this, SLOT(handleUnderline()) , Qt::UniqueConnection);
	connect(textEffects->UnderlineVal->LWidth, SIGNAL(valueChanged(double)), this, SLOT(handleUnderline()) , Qt::UniqueConnection);
	connect(textEffects->StrikeVal->LPos     , SIGNAL(valueChanged(double)), this, SLOT(handleStrikeThru()), Qt::UniqueConnection);
	connect(textEffects->StrikeVal->LWidth   , SIGNAL(valueChanged(double)), this, SLOT(handleStrikeThru()), Qt::UniqueConnection);

	connect(verticalAlign , SIGNAL(activated(int))      , this, SLOT(handleVAlign()), Qt::UniqueConnection);
}

void PropertyWidget_TextAlignment::disconnectSignals()
{
	disconnect(textEffects, SIGNAL(State(int))      , this, SLOT(handleTypeStyle(int)));
	disconnect(textEffects->ShadowVal->Xoffset  , SIGNAL(valueChanged(double)), this, SLOT(handleShadowOffs()));
	disconnect(textEffects->ShadowVal->Yoffset  , SIGNAL(valueChanged(double)), this, SLOT(handleShadowOffs()));
	disconnect(textEffects->OutlineVal->LWidth  , SIGNAL(valueChanged(double)), this, SLOT(handleOutlineWidth()));
	disconnect(textEffects->UnderlineVal->LPos  , SIGNAL(valueChanged(double)), this, SLOT(handleUnderline()));
	disconnect(textEffects->UnderlineVal->LWidth, SIGNAL(valueChanged(double)), this, SLOT(handleUnderline()));
	disconnect(textEffects->StrikeVal->LPos     , SIGNAL(valueChanged(double)), this, SLOT(handleStrikeThru()));
	disconnect(textEffects->StrikeVal->LWidth   , SIGNAL(valueChanged(double)), this, SLOT(handleStrikeThru()));

	disconnect(verticalAlign , SIGNAL(activated(int))      , this, SLOT(handleVAlign()));
}

void PropertyWidget_TextAlignment::configureWidgets(void)
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

void PropertyWidget_TextAlignment::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	setCurrentItem(currItem);
	updateGeometry();
}


void PropertyWidget_TextAlignment::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	showOutlineW  (charStyle.outlineWidth());
	showShadowOffset(charStyle.shadowXOffset(), charStyle.shadowYOffset());
	showTextEffects(charStyle.effects());
	showStrikeThru(charStyle.strikethruOffset()  , charStyle.strikethruWidth());
	showUnderline (charStyle.underlineOffset(), charStyle.underlineWidth());
}

void PropertyWidget_TextAlignment::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showOutlineW  (charStyle.outlineWidth());
	showShadowOffset(charStyle.shadowXOffset(), charStyle.shadowYOffset());
	showTextEffects(charStyle.effects());
	showStrikeThru(charStyle.strikethruOffset()  , charStyle.strikethruWidth());
	showUnderline (charStyle.underlineOffset(), charStyle.underlineWidth());

	//Alignment
	textAlignment->setStyle(newCurrent.alignment(), newCurrent.direction());
	textDirection->setStyle(newCurrent.direction());
	// Alignment End
}

void PropertyWidget_TextAlignment::showOutlineW(double x)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	textEffects->OutlineVal->LWidth->showValue(x / 10.0);
}

void PropertyWidget_TextAlignment::showShadowOffset(double x, double y)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	textEffects->ShadowVal->Xoffset->showValue(x / 10.0);
	textEffects->ShadowVal->Yoffset->showValue(y / 10.0);
}

void PropertyWidget_TextAlignment::showStrikeThru(double p, double w)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	textEffects->StrikeVal->LPos->showValue(p / 10.0);
	textEffects->StrikeVal->LWidth->showValue(w / 10.0);
}


void PropertyWidget_TextAlignment::showTextEffects(int s)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	textEffects->setStyle(s);
}

void PropertyWidget_TextAlignment::handleOutlineWidth()
{
	int x = qRound(textEffects->OutlineVal->LWidth->value() * 10.0);
	if ((m_doc) && (m_item))
	{
		PageItem *i2 = m_item;
		if (m_doc->appMode == modeEditTable)
			i2 = m_item->asTable()->activeCell().textFrame();
		if (i2 != NULL)
		{
			Selection tempSelection(this, false);
			tempSelection.addItem(i2, true);
			m_doc->itemSelection_SetOutlineWidth(x, &tempSelection);
		}
	}
}

void PropertyWidget_TextAlignment::handleShadowOffs()
{
	if ((m_doc) && (m_item))
	{
		int x = qRound(textEffects->ShadowVal->Xoffset->value() * 10.0);
		int y = qRound(textEffects->ShadowVal->Yoffset->value() * 10.0);
		PageItem *i2 = m_item;
		if (m_doc->appMode == modeEditTable)
			i2 = m_item->asTable()->activeCell().textFrame();
		if (i2 != NULL)
		{
			Selection tempSelection(this, false);
			tempSelection.addItem(i2, true);
			m_doc->itemSelection_SetShadowOffsets(x, y, &tempSelection);
		}
	}
}

void PropertyWidget_TextAlignment::handleStrikeThru()
{
	if ((m_doc) && (m_item))
	{
		int x = qRound(textEffects->StrikeVal->LPos->value() * 10.0);
		int y = qRound(textEffects->StrikeVal->LWidth->value() * 10.0);
		PageItem *i2 = m_item;
		if (m_doc->appMode == modeEditTable)
			i2 = m_item->asTable()->activeCell().textFrame();
		if (i2 != NULL)
		{
			Selection tempSelection(this, false);
			tempSelection.addItem(i2, true);
			m_doc->itemSelection_SetStrikethru(x, y, &tempSelection);
		}
	}
}


void PropertyWidget_TextAlignment::handleTypeStyle(int s)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	PageItem *i2 = m_item;
	if (m_doc->appMode == modeEditTable)
		i2 = m_item->asTable()->activeCell().textFrame();
	if (i2 != NULL)
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(i2, true);
		m_doc->itemSelection_SetEffects(s, &tempSelection);
		m_ScMW->setStyleEffects(s);
	}
}

void PropertyWidget_TextAlignment::showUnderline(double p, double w)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	textEffects->UnderlineVal->LPos->showValue(p / 10.0);
	textEffects->UnderlineVal->LWidth->showValue(w / 10.0);
}

void PropertyWidget_TextAlignment::handleUnderline()
{
	if ((m_doc) && (m_item))
	{
		int x = qRound(textEffects->UnderlineVal->LPos->value() * 10.0);
		int y = qRound(textEffects->UnderlineVal->LWidth->value() * 10.0);
		PageItem *i2 = m_item;
		if (m_doc->appMode == modeEditTable)
			i2 = m_item->asTable()->activeCell().textFrame();
		if (i2 != NULL)
		{
			Selection tempSelection(this, false);
			tempSelection.addItem(i2, true);
			m_doc->itemSelection_SetUnderline(x, y, &tempSelection);
		}
	}
}


// Alignment & Direction Start



void PropertyWidget_TextAlignment::showAlignment(int e)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;
	textAlignment->setEnabled(true);
	textAlignment->setStyle(e, textDirection->getStyle());
	m_haveItem = tmp;
}

void PropertyWidget_TextAlignment::showDirection(int e)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;
	textDirection->setEnabled(true);
	textDirection->setStyle(e);
	m_haveItem = tmp;
}

void PropertyWidget_TextAlignment::handleDirection(int d)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetDirection(d, &tempSelection);
	// If current text alignment is left or right, change it to match direction
	if (d == ParagraphStyle::RTL && textAlignment->selectedId() == ParagraphStyle::Leftaligned)
	{
		m_doc->itemSelection_SetAlignment(ParagraphStyle::Rightaligned, &tempSelection);
		textAlignment->setTypeStyle(ParagraphStyle::Rightaligned);
	}
	else if (d == ParagraphStyle::LTR && textAlignment->selectedId() == ParagraphStyle::Rightaligned)
	{
		m_doc->itemSelection_SetAlignment(ParagraphStyle::Leftaligned, &tempSelection);
		textAlignment->setTypeStyle(ParagraphStyle::Leftaligned);
	}
}

void PropertyWidget_TextAlignment::handleVAlign()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	PageItem *textItem = m_item;
	if (m_doc->appMode == modeEditTable)
		textItem = m_item->asTable()->activeCell().textFrame();
	if (textItem != NULL)
	{
		textItem->setVerticalAlignment(verticalAlign->currentIndex());
		textItem->update();
		if (m_doc->appMode == modeEditTable)
			m_item->asTable()->update();
		m_doc->regionsChanged()->update(QRect());
	}
}


void PropertyWidget_TextAlignment::handleAlignment(int a)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetAlignment(a, &tempSelection);
//	if (m_item->isPathText())
//		pathTextWidgets->handleSelectionChanged();
}

// Alignment & Direction End




void PropertyWidget_TextAlignment::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}

void PropertyWidget_TextAlignment::languageChange()
{
	retranslateUi(this);
	textEffects->languageChange();


	QSignalBlocker verticalAlignBlocker(verticalAlign);
	int oldAlignIndex = verticalAlign->currentIndex();
	verticalAlign->clear();
	verticalAlign->addItem( tr("Top"));
	verticalAlign->addItem( tr("Middle"));
	verticalAlign->addItem( tr("Bottom"));
	verticalAlign->setCurrentIndex(oldAlignIndex);

	textAlignment->languageChange();
	textDirection->languageChange();

}
