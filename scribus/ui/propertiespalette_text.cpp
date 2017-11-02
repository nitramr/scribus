/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "propertiespalette_text.h"

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <QSignalBlocker>

#include "appmodes.h"
//#include "commonstrings.h"
#include "pageitem.h"
#include "pageitem_table.h"
#include "pageitem_textframe.h"
#include "propertiespalette_utils.h"
#include "propertywidget_advanced.h"
#include "propertywidget_distance.h"
#include "propertywidget_flop.h"
#include "propertywidget_fontfeatures.h"
#include "propertywidget_hyphenation.h"
#include "propertywidget_optmargins.h"
#include "propertywidget_orphans.h"
#include "propertywidget_pareffect.h"
#include "propertywidget_pathtext.h"
#include "propertywidget_textadvanced.h"
#include "propertywidget_textalignment.h"
#include "propertywidget_textbase.h"
#include "propertywidget_textcolor.h"
#include "scpopupmenu.h"
#include "scraction.h"
#include "scribuscore.h"
#include "selection.h"
//#include "spalette.h"
//#include "styleselect.h"
#include "tabmanager.h"
#include "undomanager.h"
#include "units.h"
#include "util.h"
#include "util_math.h"
#include "langmgr.h"

PropertiesPalette_Text::PropertiesPalette_Text( QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_item=0;
	m_haveDoc = false;
	m_haveItem = false;
//	m_unitIndex = 0;
//	m_unitRatio = 1.0;

	textWidgets = new PropertyWidget_TextBase();
	textAdvancedWidgets = new PropertyWidget_TextAdvanced();
	textAlignmentWidgets = new PropertyWidget_TextAlignment();
	colorWidgets = new PropertyWidget_TextColor();
	flopBox = new PropertyWidget_Flop();
	orphanBox = new PropertyWidget_Orphans();
	parEffectWidgets = new PropertyWidget_ParEffect();
	distanceWidgets = new PropertyWidget_Distance();
	optMargins = new PropertyWidget_OptMargins();
	hyphenationWidget = new PropertyWidget_Hyphenation();
	advancedWidgets = new PropertyWidget_Advanced();
	fontfeaturesWidget = new PropertyWidget_FontFeatures();
	pathTextWidgets = new PropertyWidget_PathText();


	ScPopupMenu * popupFontFeatures = new ScPopupMenu(textAdvancedWidgets);
	popupFontFeatures->addWidget(fontfeaturesWidget);
	ScPopupMenu * popupOrphans = new ScPopupMenu(orphanBox);
	ScPopupMenu * popupHyphenation = new ScPopupMenu(hyphenationWidget);


	layoutSectionText = new ScLayoutSection(tr("Text"),popupFontFeatures);
	layoutSectionParagraph = new ScLayoutSection(tr("Paragraph"), popupOrphans);
	layoutSectionCharacter = new ScLayoutSection(tr("Character"), popupHyphenation);
	layoutSectionLists = new ScLayoutSection(tr("Lists"));
	layoutSectionTextPath = new ScLayoutSection(tr("Text Path"));

	layoutSectionText->addWidget(textWidgets);
	layoutSectionText->addWidget(textAlignmentWidgets);
	layoutSectionText->addWidget(colorWidgets);

	layoutSectionParagraph->addWidget(distanceWidgets);
	layoutSectionParagraph->addWidget(flopBox);
	layoutSectionParagraph->addWidget(optMargins);

	layoutSectionCharacter->addWidget(advancedWidgets);

	layoutSectionLists->addWidget(parEffectWidgets);

	layoutSectionTextPath->addWidget(pathTextWidgets);

	FlowLayout *flowLayout = new FlowLayout(0,0,0);
	flowLayout->addWidget(layoutSectionText);
	flowLayout->addWidget(layoutSectionParagraph);
	flowLayout->addWidget(layoutSectionCharacter);
	flowLayout->addWidget(layoutSectionLists);
	flowLayout->addWidget(layoutSectionTextPath);

	setLayout(flowLayout);



	languageChange();

	connect(flopBox->flopGroup, SIGNAL(buttonClicked( int )), this, SLOT(handleFirstLinePolicy(int)));

	connect(textAlignmentWidgets, SIGNAL(handleAlignment()), this, SLOT(handleAlignment()));

	m_haveItem = false;
	setEnabled(false);
}

void PropertiesPalette_Text::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	advancedWidgets->setMainWindow(mw);
	fontfeaturesWidget->setMainWindow(mw);
	textWidgets->setMainWindow(mw);
	textAdvancedWidgets->setMainWindow(mw);
	textAlignmentWidgets->setMainWindow(mw);
	colorWidgets->setMainWindow(mw);
	distanceWidgets->setMainWindow(mw);
	hyphenationWidget->setMainWindow(mw);
	parEffectWidgets->setMainWindow(mw);
	optMargins->setMainWindow(mw);
	pathTextWidgets->setMainWindow(mw);


	connect(m_ScMW, SIGNAL(UpdateRequest(int))     , this  , SLOT(handleUpdateRequest(int)));
}

void PropertiesPalette_Text::setDoc(ScribusDoc *d)
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

//	m_unitRatio   = m_doc->unitRatio();
//	m_unitIndex   = m_doc->unitIndex();

	m_haveDoc  = true;
	m_haveItem = false;

	advancedWidgets->setDoc(m_doc);
	fontfeaturesWidget->setDoc(m_doc);
	textWidgets->setDoc(m_doc);
	textAdvancedWidgets->setDoc(m_doc);
	textAlignmentWidgets->setDoc(m_doc);
	colorWidgets->setDoc(m_doc);
	distanceWidgets->setDoc(m_doc);
	parEffectWidgets->setDoc(m_doc);
	flopBox->setDoc(m_doc);
	hyphenationWidget->setDoc(m_doc);
	optMargins->setDoc(m_doc);
	orphanBox->setDoc(m_doc);
	pathTextWidgets->setDoc(m_doc);

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertiesPalette_Text::unsetDoc()
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

	advancedWidgets->setDoc(0);
	fontfeaturesWidget->setDoc(0);
	textWidgets->setDoc(0);
	textAdvancedWidgets->setDoc(0);
	textAlignmentWidgets->setDoc(0);
	colorWidgets->setDoc(0);
	distanceWidgets->setDoc(0);
	flopBox->setDoc(0);
	hyphenationWidget->setDoc(NULL);
	optMargins->setDoc(0);
	orphanBox->setDoc(0);
	parEffectWidgets->setDoc(0);
	pathTextWidgets->setDoc(0);

	m_haveItem = false;

	setEnabled(false);
}

void PropertiesPalette_Text::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	colorWidgets->setCurrentItem(m_item);
	textAlignmentWidgets->setCurrentItem(m_item);
	handleSelectionChanged();
}

PageItem* PropertiesPalette_Text::currentItemFromSelection()
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

void PropertiesPalette_Text::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1 )
	{
		setEnabled(false);
		flopBox->flopRealHeight->setChecked(true);
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

void PropertiesPalette_Text::handleUpdateRequest(int updateFlags)
{
	textWidgets->handleUpdateRequest(updateFlags);

	// ColorWidget will handle its update itself
	/*if (updateFlags & reqColorsUpdate)
		updateColorList();*/
	if (updateFlags & reqCharStylesUpdate)
	{
		parEffectWidgets->updateCharStyles();
	}
	if (updateFlags & reqStyleComboDocUpdate)
	{
		parEffectWidgets->setDoc(m_haveDoc ? m_doc : 0);
	}
}

void PropertiesPalette_Text::setCurrentItem(PageItem *i)
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


	showFirstLinePolicy(m_item->firstLineOffset());

	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		setEnabled(false);
	}

	m_haveItem = true;

	if (!sender())
	{
//		textWidgets->setCurrentItem(m_item);
//		textAdvancedWidgets->setCurrentItem(m_item);
		textWidgets->handleSelectionChanged();
		textAdvancedWidgets->handleSelectionChanged();
		colorWidgets->handleSelectionChanged();
		textAlignmentWidgets->handleSelectionChanged();
		distanceWidgets->handleSelectionChanged();
		parEffectWidgets->handleSelectionChanged();
	}

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

void PropertiesPalette_Text::unitChange()
{
	if (!m_haveDoc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;

	advancedWidgets->unitChange();
	fontfeaturesWidget->unitChange();
	textAlignmentWidgets->unitChange();
	colorWidgets->unitChange();
	distanceWidgets->unitChange();
	flopBox->unitChange();
	optMargins->unitChange();
	pathTextWidgets->unitChange();
	parEffectWidgets->unitChange();

	m_haveItem = tmp;
}


void PropertiesPalette_Text::changeLang(int id)
{
	textAdvancedWidgets->changeLang(id);
}

void PropertiesPalette_Text::showLanguage(QString w)
{

	textAdvancedWidgets->showLanguage(w);

}


void PropertiesPalette_Text::showLineSpacing(double r)
{
	textWidgets->showLineSpacing(r);

}

void PropertiesPalette_Text::showFontFace(const QString& newFont)
{
	textWidgets->showFontFace(newFont);

}

void PropertiesPalette_Text::showFontSize(double s)
{
	textWidgets->showFontSize(s);
}



void PropertiesPalette_Text::showFirstLinePolicy( FirstLineOffsetPolicy f )
{
	if(f == FLOPFontAscent)
		flopBox->flopFontAscent->setChecked(true);
	else if(f == FLOPLineSpacing)
		flopBox->flopLineSpacing->setChecked(true);
	else if (f == FLOPRealGlyphHeight)
		flopBox->flopRealHeight->setChecked(true); //It’s historical behaviour.
	else // if (f == FLOPBaseline)
		flopBox->flopBaselineGrid->setChecked(true);
}

void PropertiesPalette_Text::setupLineSpacingSpinbox(int mode, double value)
{
	textWidgets->setupLineSpacingSpinbox(mode, value);
}

void PropertiesPalette_Text::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	advancedWidgets->updateCharStyle(charStyle);
	fontfeaturesWidget->updateCharStyle(charStyle);
	colorWidgets->updateCharStyle(charStyle);
	textAlignmentWidgets->updateCharStyle(charStyle);
	hyphenationWidget->updateCharStyle(charStyle);
	textWidgets->updateCharStyle(charStyle);
	textAdvancedWidgets->updateCharStyle(charStyle);
}

void PropertiesPalette_Text::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	advancedWidgets->updateStyle(newCurrent);
	fontfeaturesWidget->updateStyle(newCurrent);
	colorWidgets->updateStyle(newCurrent);
	textAlignmentWidgets->updateStyle(newCurrent);
	optMargins->updateStyle(newCurrent);
	orphanBox->updateStyle (newCurrent);
	parEffectWidgets->updateStyle(newCurrent);
	hyphenationWidget->updateStyle(newCurrent);
	textWidgets->updateStyle(newCurrent);
	textAdvancedWidgets->updateStyle(newCurrent);
}

void PropertiesPalette_Text::updateCharStyles()
{
	textWidgets->updateCharStyles();
	parEffectWidgets->updateCharStyles();
}

void PropertiesPalette_Text::updateParagraphStyles()
{
	textWidgets->updateParagraphStyles();
	parEffectWidgets->updateCharStyles();
}

void PropertiesPalette_Text::updateTextStyles()
{
	textWidgets->updateTextStyles();
}

void PropertiesPalette_Text::showAlignment(int e)
{
	textAlignmentWidgets->showAlignment(e);
}

void PropertiesPalette_Text::showDirection(int e)
{
	textAlignmentWidgets->showDirection(e);

}

void PropertiesPalette_Text::showCharStyle(const QString& name)
{
	textWidgets->showCharStyle(name);
}

void PropertiesPalette_Text::showParStyle(const QString& name)
{
	textWidgets->showParStyle(name);
}


void PropertiesPalette_Text::handleAlignment()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (m_item->isPathText())
		pathTextWidgets->handleSelectionChanged();
}


void PropertiesPalette_Text::updateColorList()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	colorWidgets->updateColorList();
}

void PropertiesPalette_Text::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}

void PropertiesPalette_Text::languageChange()
{

	textWidgets->languageChange();
	textAdvancedWidgets->languageChange();
	colorWidgets->languageChange();
	textAlignmentWidgets->languageChange();
	flopBox->languageChange();
	orphanBox->languageChange();
	distanceWidgets->languageChange();
	optMargins->languageChange();
	advancedWidgets->languageChange();
	pathTextWidgets->languageChange();
	fontfeaturesWidget->languageChange();
	hyphenationWidget->languageChange();
}

void PropertiesPalette_Text::handleFirstLinePolicy(int radioFlop)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning() || !m_haveDoc || !m_haveItem)
		return;
	if( radioFlop == PropertyWidget_Flop::RealHeightID)
		m_item->setFirstLineOffset(FLOPRealGlyphHeight);
	else if( radioFlop == PropertyWidget_Flop::FontAscentID)
		m_item->setFirstLineOffset(FLOPFontAscent);
	else if( radioFlop == PropertyWidget_Flop::LineSpacingID)
		m_item->setFirstLineOffset(FLOPLineSpacing);
	else if( radioFlop == PropertyWidget_Flop::BaselineGridID)
		m_item->setFirstLineOffset(FLOPBaselineGrid);
	m_item->update();
	if (m_doc->appMode == modeEditTable)
		m_item->parentTable()->update();
	else
		m_item->update();
	m_doc->regionsChanged()->update(QRect());
}
