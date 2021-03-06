/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "appmodes.h"
#include "pageitem_table.h"
#include "propertywidgettext_textcolor.h"
#include "scribus.h"
#include "selection.h"
#include "sccolorengine.h"
#include "scpopupmenu.h"

PropertyWidgetText_TextColor::PropertyWidgetText_TextColor(QWidget* parent) : QWidget(parent)
{
	m_item = NULL;
	m_ScMW = NULL;

	setupUi(this);

	strokeColor->setPixmapType(ColorCombo::fancyPixmaps);
	strokeColorBox = new ScColorFillsBox();
	strokeLayout->insertWidget(0,strokeColorBox);
	ScPopupMenu * strokeColorMenu = new ScPopupMenu(strokeColor);
	strokeColorMenu->setBuddy(strokeColorBox);


	backColor->setPixmapType(ColorCombo::fancyPixmaps);
	backColorBox = new ScColorFillsBox();
	backLayout->insertWidget(0,backColorBox);
	ScPopupMenu * backColorMenu = new ScPopupMenu(backColor);
	backColorMenu->setBuddy(backColorBox);

	languageChange();

	strokeColor->setEnabled(false);
	strokeShade->setEnabled(false);
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetText_TextColor::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
}

void PropertyWidgetText_TextColor::connectSignals()
{
	connect(strokeColor , SIGNAL(activated(int)), this, SLOT(handleTextStroke())   , Qt::UniqueConnection);
	connect(strokeShade , SIGNAL(clicked())     , this, SLOT(handleTextShade())    , Qt::UniqueConnection);
	connect(backShade   , SIGNAL(clicked())     , this, SLOT(handleTextShade())    , Qt::UniqueConnection);
	connect(backColor   , SIGNAL(activated(int)), this, SLOT(handleTextBackground())     , Qt::UniqueConnection);
}

void PropertyWidgetText_TextColor::disconnectSignals()
{
	disconnect(strokeColor , SIGNAL(activated(int)), this, SLOT(handleTextStroke()));
	disconnect(strokeShade , SIGNAL(clicked())     , this, SLOT(handleTextShade()));
	disconnect(backShade   , SIGNAL(clicked())     , this, SLOT(handleTextShade()));
	disconnect(backColor   , SIGNAL(activated(int)), this, SLOT(handleTextBackground()));
}

void PropertyWidgetText_TextColor::configureWidgets(void)
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

void PropertyWidgetText_TextColor::setDoc(ScribusDoc *d)
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

	updateColorList();

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidgetText_TextColor::setCurrentItem(PageItem *item)
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

	m_item = item;
	if (item && m_doc.isNull())
		setDoc(item->doc());

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
	connectSignals();
}



/*********************************************************************
*
* Stroke Color
*
**********************************************************************/

void PropertyWidgetText_TextColor::handleStrokeColorBox(){

	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	QString color = strokeColor->currentColor();

	if (color == CommonStrings::tr_NoneColor)
		color = CommonStrings::None;

	if(color != CommonStrings::None){
		const ScColor& col = m_doc->PageColors[color];
		QColor tmp = ScColorEngine::getShadeColorProof(col, m_doc, strokeShade->getValue());
		strokeColorBox->setColor(tmp);
	} else strokeColorBox->resetColor();
}

void PropertyWidgetText_TextColor::handleTextStroke()
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
		m_doc->itemSelection_SetStrokeColor(strokeColor->currentColor(), &tempSelection);

		// draw color box
		handleStrokeColorBox();
	}
}

/*********************************************************************
*
* Back Color
*
**********************************************************************/

void PropertyWidgetText_TextColor::handleBackColorBox(){

	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	QString color = backColor->currentColor();

	if (color == CommonStrings::tr_NoneColor)
		color = CommonStrings::None;

	if(color != CommonStrings::None){
		const ScColor& col = m_doc->PageColors[color];
		QColor tmp = ScColorEngine::getShadeColorProof(col, m_doc, backShade->getValue());
		backColorBox->setColor(tmp);
	} else backColorBox->resetColor();
}



void PropertyWidgetText_TextColor::handleTextBackground()
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
		m_doc->itemSelection_SetBackgroundColor(backColor->currentColor(), &tempSelection);

		// draw color box
		handleBackColorBox();
	}
}


/*********************************************************************
*
* Color Helper
*
**********************************************************************/

void PropertyWidgetText_TextColor::showTextColors(QString p, QString bc, double shp, double sbc)
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	ColorList::Iterator it;
	int c = 0;

	strokeShade->setValue(qRound(shp));
	backShade->setValue(qRound(sbc));

	c = 0;
	if ((p != CommonStrings::None) && (!p.isEmpty()))
	{
		for (it = m_doc->PageColors.begin(); it != m_doc->PageColors.end(); ++it)
		{
			if (it.key() == p)
				break;
			c++;
		}
	}	
	strokeColor->setCurrentIndex(c);
	// draw color box
	handleStrokeColorBox();

	c = 0;
	if ((bc != CommonStrings::None) && (!bc.isEmpty()))
	{
		c++;
		for (it = m_doc->PageColors.begin(); it != m_doc->PageColors.end(); ++it)
		{
			if (it.key() == bc)
				break;
			c++;
		}
	}
	backColor->setCurrentIndex(c);
	// draw color box
	handleBackColorBox();
}

void PropertyWidgetText_TextColor::showTextEffects(int s)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	strokeColor->setEnabled(false);
	strokeShade->setEnabled(false);

	if ((s & 4) || (s & 256))
	{
		strokeColor->setEnabled(true);
		strokeShade->setEnabled(true);
	}
}

void PropertyWidgetText_TextColor::handleTextShade()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if (strokeShade == sender())
	{
		int b = strokeShade->getValue();
		PageItem *i2 = m_item;
		if (m_doc->appMode == modeEditTable)
			i2 = m_item->asTable()->activeCell().textFrame();
		if (i2 != NULL)
		{
			Selection tempSelection(this, false);
			tempSelection.addItem(i2, true);
			m_doc->itemSelection_SetStrokeShade(b, &tempSelection);
			// draw color box
			handleStrokeColorBox();
		}
	}
	else if (backShade == sender())
	{
		int b = backShade->getValue();
		PageItem *i2 = m_item;
		if (m_doc->appMode == modeEditTable)
			i2 = m_item->asTable()->activeCell().textFrame();
		if (i2 != NULL)
		{
			Selection tempSelection(this, false);
			tempSelection.addItem(i2, true);
			m_doc->itemSelection_SetBackgroundShade(b, &tempSelection);
			// draw color box
			handleBackColorBox();
		}
	}
}


/*********************************************************************
*
* Update helper
*
**********************************************************************/

void PropertyWidgetText_TextColor::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	setCurrentItem(currItem);
	updateGeometry();
}

void PropertyWidgetText_TextColor::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqColorsUpdate)
		updateColorList();
}

void PropertyWidgetText_TextColor::updateColorList()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (m_item)
		disconnectSignals();

	strokeColor->setColors(m_doc->PageColors, false);
	backColor->setColors(m_doc->PageColors, true);
	strokeColor->view()->setMinimumWidth(100);//strokeColor->view()->maximumViewportSize().width());// + 24);
	backColor->view()->setMinimumWidth(100);//backColor->view()->maximumViewportSize().width());// + 24);

	if (m_item)
		setCurrentItem(m_item);
}

void PropertyWidgetText_TextColor::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	showTextColors(charStyle.strokeColor(), charStyle.backColor(), charStyle.strokeShade(), charStyle.backShade());
	showTextEffects(charStyle.effects());

}

void PropertyWidgetText_TextColor::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showTextColors(charStyle.strokeColor(), charStyle.backColor(), charStyle.strokeShade(), charStyle.backShade());
	showTextEffects(charStyle.effects());

}

void PropertyWidgetText_TextColor::handleTypeStyle(int s)
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

void PropertyWidgetText_TextColor::languageChange()
{
	retranslateUi(this);
}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetText_TextColor::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}


