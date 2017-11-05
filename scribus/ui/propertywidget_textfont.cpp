/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_textfont.h"

#include "appmodes.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
#include "units.h"
#include "sccolorengine.h"

#include "sccolorfillsbox.h"
//#include "sccolorpicker.h"
#include "scpopupmenu.h"

PropertyWidget_TextFont::PropertyWidget_TextFont(QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_item=0;
	m_haveDoc = false;
	m_haveItem = false;
//	m_unitIndex = 0;
//	m_unitRatio = 1.0;

	setupUi(this);	

	// Text
	fontSize->setPrefix( "" );

	fillColor = new ColorCombo();
	fillColor->setPixmapType(ColorCombo::fancyPixmaps);
	ScPopupMenu * fillsColorMenu = new ScPopupMenu(fillColor);
	fillsColorBox = new ScColorFillsBox();
	fillsColorBox->setMenu(fillsColorMenu);
	verticalLayout_fontColor->insertWidget(0,fillsColorBox);

	fillShade = new ShadeButton(this);
	verticalLayout_fillShade->insertWidget(0,fillShade);

	languageChange();

	// Line Space
	connect(lineSpacing   , SIGNAL(valueChanged(double)), this, SLOT(handleLineSpacing()));
	connect(lineSpacingModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(handleLineSpacingMode(int)));

	// Text
	connect(fonts         , SIGNAL(fontSelected(QString )), this, SLOT(handleTextFont(QString)));
	connect(fontSize      , SIGNAL(valueChanged(double)), this, SLOT(handleFontSize()));


	m_haveItem = false;
	setEnabled(false);
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidget_TextFont::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int))     , this  , SLOT(handleUpdateRequest(int)));
}


void PropertyWidget_TextFont::connectSignals()
{
	// Text
	connect(fillColor   , SIGNAL(activated(int)), this, SLOT(handleTextFill())     , Qt::UniqueConnection);
	connect(fillShade   , SIGNAL(clicked())     , this, SLOT(handleTextShade())    , Qt::UniqueConnection);

}

void PropertyWidget_TextFont::disconnectSignals()
{
	// Text
	disconnect(fillColor   , SIGNAL(activated(int)), this, SLOT(handleTextFill()));
	disconnect(fillShade   , SIGNAL(clicked())     , this, SLOT(handleTextShade()));

}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidget_TextFont::setDoc(ScribusDoc *d)
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

	m_haveDoc  = true;
	m_haveItem = false;

	// set values
	updateColorList();

	fontSize->setValues( 0.5, 2048, 2, 1);
	fonts->RebuildList(m_doc);

	lineSpacing->setValues( 1, 2048, 2, 1);



	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));	
}

void PropertyWidget_TextFont::unsetDoc()
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

	setEnabled(false);
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidget_TextFont::setCurrentItem(PageItem *i)
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

	disconnectSignals();

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

	connectSignals();

}


PageItem* PropertyWidget_TextFont::currentItemFromSelection()
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
* Line Space
*
**********************************************************************/

void PropertyWidget_TextFont::handleLineSpacingMode(int id)
{
	if ((m_haveDoc) && (m_haveItem))
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(m_item, true);
		m_doc->itemSelection_SetLineSpacingMode(id, &tempSelection);
		updateStyle(((m_doc->appMode == modeEdit) || (m_doc->appMode == modeEditTable)) ? m_item->currentStyle() : m_item->itemText.defaultStyle());
		m_doc->regionsChanged()->update(QRect());
	}
}

void PropertyWidget_TextFont::showLineSpacing(double r)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	bool inEditMode = (m_doc->appMode == modeEdit || m_doc->appMode == modeEditTable);
	bool tmp = m_haveItem;
	m_haveItem = false;
	lineSpacing->showValue(r);
	const ParagraphStyle& curStyle(m_haveItem && inEditMode ? m_item->currentStyle() : m_item->itemText.defaultStyle());
	if (tmp)
	{
		setupLineSpacingSpinbox(curStyle.lineSpacingMode(), r);
		lineSpacingModeCombo->setCurrentIndex(curStyle.lineSpacingMode());
	}
	m_haveItem = tmp;
}

void PropertyWidget_TextFont::setupLineSpacingSpinbox(int mode, double value)
{
	bool blocked = lineSpacing->blockSignals(true);
	if (mode > 0)
	{
		if (mode==1)
			lineSpacing->setSpecialValueText( tr( "Auto" ) );
		if (mode==2)
			lineSpacing->setSpecialValueText( tr( "Baseline" ) );
		lineSpacing->setMinimum(0);
		lineSpacing->setValue(0);
		lineSpacing->setEnabled(false);
	}
	else
	{
		lineSpacing->setSpecialValueText("");
		lineSpacing->setMinimum(1);
		lineSpacing->setValue(value);
		lineSpacing->setEnabled(true);
	}
	lineSpacing->blockSignals(blocked);
}

void PropertyWidget_TextFont::handleLineSpacing()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetLineSpacing(lineSpacing->value(), &tempSelection);
}

/*********************************************************************
*
* Text (Font, Weight, Color, Size)
*
**********************************************************************/

void PropertyWidget_TextFont::showFontFace(const QString& newFont)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;
	if (m_item != NULL)
		fonts->RebuildList(m_doc, m_item->isAnnotation());
	fonts->setCurrentFont(newFont);
	m_haveItem = tmp;
}

void PropertyWidget_TextFont::showFontSize(double s)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	fontSize->showValue(s / 10.0);
}


void PropertyWidget_TextFont::showTextColors(QString b, double shb)
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	ColorList::Iterator it;
	int c = 0;

	fillShade->setValue(qRound(shb));	// Fills

	if ((b != CommonStrings::None) && (!b.isEmpty()))
	{
		c++;
		for (it = m_doc->PageColors.begin(); it != m_doc->PageColors.end(); ++it)
		{
			if (it.key() == b)
				break;
			c++;
		}
	}
	fillColor->setCurrentIndex(c);

	// Update Color Box
	handleFillColorBox();


}

void PropertyWidget_TextFont::handleFontSize()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetFontSize(qRound(fontSize->value()*10.0), &tempSelection);
}


void PropertyWidget_TextFont::handleTextFont(QString c)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->SetNewFont(c);
}

// Fills
void PropertyWidget_TextFont::handleFillColorBox(){

	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	QString color = fillColor->currentColor();

	if (color == CommonStrings::tr_NoneColor)
		color = CommonStrings::None;

	if(color != CommonStrings::None){
		const ScColor& col = m_doc->PageColors[color];
		QColor tmp = ScColorEngine::getShadeColorProof(col, m_doc, fillShade->getValue());
		fillsColorBox->setColor(tmp);
	} else fillsColorBox->resetColor();
}

// Fills
void PropertyWidget_TextFont::handleTextFill()
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
		m_doc->itemSelection_SetFillColor(fillColor->currentColor(), &tempSelection);

		// draw color box
		handleFillColorBox();
	}
}

void PropertyWidget_TextFont::handleTextShade()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if (fillShade == sender()) // Fills
	{
		int b = fillShade->getValue();
		PageItem *i2 = m_item;
		if (m_doc->appMode == modeEditTable)
			i2 = m_item->asTable()->activeCell().textFrame();
		if (i2 != NULL)
		{
			Selection tempSelection(this, false);
			tempSelection.addItem(i2, true);
			m_doc->itemSelection_SetFillShade(b, &tempSelection);
			// draw color box
			handleFillColorBox();
		}
	}
}


/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidget_TextFont::updateColorList()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (m_item)
		disconnectSignals();

	// Fills
	fillColor->setColors(m_doc->PageColors, true);
	fillColor->view()->setMinimumWidth(100);//fillColor->view()->maximumViewportSize().width());// + 24);

	if (m_item)
		setCurrentItem(m_item);
}


void PropertyWidget_TextFont::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	showTextColors(charStyle.fillColor(), charStyle.fillShade());
	showFontFace(charStyle.font().scName());
	showFontSize(charStyle.fontSize());
}

void PropertyWidget_TextFont::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showTextColors(charStyle.fillColor(), charStyle.fillShade());
	showFontFace(charStyle.font().scName());
	showFontSize(charStyle.fontSize());

	bool tmp = m_haveItem;
	m_haveItem = false;

	setupLineSpacingSpinbox(newCurrent.lineSpacingMode(), newCurrent.lineSpacing());
	lineSpacingModeCombo->setCurrentIndex(newCurrent.lineSpacingMode());

	m_haveItem = tmp;
}


void PropertyWidget_TextFont::handleUpdateRequest(int updateFlags)
{

	// ColorWidget will handle its update itself
	/*if (updateFlags & reqColorsUpdate)
		updateColorList();*/

	if (updateFlags & reqDefFontListUpdate)
		fonts->RebuildList(0);
	if (updateFlags & reqDocFontListUpdate)
		fonts->RebuildList(m_haveDoc ? m_doc : 0);
	if (updateFlags & reqColorsUpdate)
		updateColorList();

}

void PropertyWidget_TextFont::handleSelectionChanged()
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

void PropertyWidget_TextFont::languageChange()
{
	retranslateUi(this);

	QSignalBlocker lineSpacingModeBlocker(lineSpacingModeCombo);
	int oldLineSpacingMode = lineSpacingModeCombo->currentIndex();
	lineSpacingModeCombo->clear();
	lineSpacingModeCombo->addItem( tr("Fixed Linespacing"));
	lineSpacingModeCombo->addItem( tr("Automatic Linespacing"));
	lineSpacingModeCombo->addItem( tr("Align to Baseline Grid"));
	lineSpacingModeCombo->setCurrentIndex(oldLineSpacingMode);

}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidget_TextFont::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}


