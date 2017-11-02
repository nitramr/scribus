/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_textbase.h"

#include "appmodes.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
#include "units.h"

#include "sccolorfillsbox.h"
#include "sccolorpicker.h"
#include "scpopupmenu.h"

PropertyWidget_TextBase::PropertyWidget_TextBase(QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_item=0;
	m_haveDoc = false;
	m_haveItem = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	setupUi(this);

	fontSize->setPrefix( "" );

	// Setup ScColorPicker in ScPopupMenu and add it to ScColorFillsBox
	ScColorPicker * colorPicker = new ScColorPicker();
	colorPicker->setFixedSize(100,100);
	ScPopupMenu * colorPickerMenu = new ScPopupMenu(colorPicker);
	ScColorFillsBox * fontColorFillsBox = new ScColorFillsBox();
	fontColorFillsBox->setMenu(colorPickerMenu);
	verticalLayout_fontColor->insertWidget(0,fontColorFillsBox);

	// connect ScFillsBox with ScColorPicker
	connect(colorPicker, SIGNAL(setPreview(QPixmap)), fontColorFillsBox, SLOT(setPixmap(QPixmap)));
	connect(colorPicker, SIGNAL(resetColor()), fontColorFillsBox, SLOT(resetColor()));


	languageChange();

	connect(lineSpacing   , SIGNAL(valueChanged(double)), this, SLOT(handleLineSpacing()));
	connect(fonts         , SIGNAL(fontSelected(QString )), this, SLOT(handleTextFont(QString)));
	connect(fontSize      , SIGNAL(valueChanged(double)), this, SLOT(handleFontSize()));

	connect(lineSpacingModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(handleLineSpacingMode(int)));

	m_haveItem = false;
	setEnabled(false);
}

void PropertyWidget_TextBase::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int))     , this  , SLOT(handleUpdateRequest(int)));
}

void PropertyWidget_TextBase::setDoc(ScribusDoc *d)
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

	m_unitRatio   = m_doc->unitRatio();
	m_unitIndex   = m_doc->unitIndex();

	m_haveDoc  = true;
	m_haveItem = false;

	fontSize->setValues( 0.5, 2048, 2, 1);
	lineSpacing->setValues( 1, 2048, 2, 1);

	fonts->RebuildList(m_doc);

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));	
}

void PropertyWidget_TextBase::unsetDoc()
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

void PropertyWidget_TextBase::setCurrentItem(PageItem *i)
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


PageItem* PropertyWidget_TextBase::currentItemFromSelection()
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

void PropertyWidget_TextBase::handleSelectionChanged()
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


void PropertyWidget_TextBase::handleLineSpacingMode(int id)
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

void PropertyWidget_TextBase::showLineSpacing(double r)
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

void PropertyWidget_TextBase::showFontFace(const QString& newFont)
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

void PropertyWidget_TextBase::showFontSize(double s)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	fontSize->showValue(s / 10.0);
}


void PropertyWidget_TextBase::setupLineSpacingSpinbox(int mode, double value)
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

void PropertyWidget_TextBase::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	showFontFace(charStyle.font().scName());
	showFontSize(charStyle.fontSize());
}

void PropertyWidget_TextBase::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showFontFace(charStyle.font().scName());
	showFontSize(charStyle.fontSize());

	bool tmp = m_haveItem;
	m_haveItem = false;

	setupLineSpacingSpinbox(newCurrent.lineSpacingMode(), newCurrent.lineSpacing());
	lineSpacingModeCombo->setCurrentIndex(newCurrent.lineSpacingMode());

	m_haveItem = tmp;
}

void PropertyWidget_TextBase::handleLineSpacing()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetLineSpacing(lineSpacing->value(), &tempSelection);
}

void PropertyWidget_TextBase::handleFontSize()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetFontSize(qRound(fontSize->value()*10.0), &tempSelection);
}


void PropertyWidget_TextBase::handleTextFont(QString c)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->SetNewFont(c);
}

void PropertyWidget_TextBase::handleUpdateRequest(int updateFlags)
{

	// ColorWidget will handle its update itself
	/*if (updateFlags & reqColorsUpdate)
		updateColorList();*/

	if (updateFlags & reqDefFontListUpdate)
		fonts->RebuildList(0);
	if (updateFlags & reqDocFontListUpdate)
		fonts->RebuildList(m_haveDoc ? m_doc : 0);

}


void PropertyWidget_TextBase::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}

void PropertyWidget_TextBase::languageChange()
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
