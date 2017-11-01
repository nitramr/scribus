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
#include "colorcombo.h"
#include "colorlistbox.h"
#include "commonstrings.h"
#include "fontcombo.h"
#include "iconmanager.h"
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
#include "propertywidget_textbase.h"
#include "propertywidget_textcolor.h"
#include "sccombobox.h"
#include "scfonts.h"
#include "scpopupmenu.h"
#include "scraction.h"
#include "scribuscore.h"
#include "selection.h"
#include "spalette.h"
#include "styleselect.h"
#include "tabmanager.h"
#include "undomanager.h"
#include "units.h"
#include "util.h"
#include "util_math.h"
#include "langmgr.h"

//using namespace std;

PropertiesPalette_Text::PropertiesPalette_Text( QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_item=0;
	m_haveDoc = false;
	m_haveItem = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;


//	fontSize->setPrefix( "" );
//	fontSizeLabel->setPixmap(IconManager::instance()->loadPixmap("zeichen.png"));
//	lineSpacingLabel->setPixmap(IconManager::instance()->loadPixmap("linespacing2.png"));

//	paraStyleLabel->setBuddy(paraStyleCombo);
//	paraStyleClear->setIcon(IconManager::instance()->loadPixmap("16/edit-clear.png"));
//	charStyleLabel->setBuddy(charStyleCombo);
//	charStyleClear->setIcon(IconManager::instance()->loadPixmap("16/edit-clear.png"));

	textWidgets = new PropertyWidget_TextBase();

	colorWidgets = new PropertyWidget_TextColor();
	//colorWidgetsItem = textTree->addItem( colorWidgets, tr("Color && Effects") );

	flopBox = new PropertyWidget_Flop();
	//flopItem = textTree->addItem( flopBox, tr("First Line Offset"));

	orphanBox = new PropertyWidget_Orphans();
	//orphanItem = textTree->addItem(orphanBox, tr("Orphans and Widows"));

	parEffectWidgets = new PropertyWidget_ParEffect();
	//parEffectItem = textTree->addItem(parEffectWidgets, tr("Paragraph Effects"));

	distanceWidgets = new PropertyWidget_Distance();
	//distanceItem = textTree->addItem(distanceWidgets, tr("Columns && Text Distances"));

	//<< Optical Margins
	optMargins = new PropertyWidget_OptMargins();
	//optMarginsItem = textTree->addItem(optMargins, tr("Optical Margins"));
	//>> Optical Margins

	hyphenationWidget = new PropertyWidget_Hyphenation();
	//hyphenationWidgetItem = textTree->addItem(hyphenationWidget, tr("Hyphenation"));

	//<<Advanced Settings
	advancedWidgets = new PropertyWidget_Advanced();
	//advancedWidgetsItem = textTree->addItem(advancedWidgets, tr("Advanced Settings"));

	//>>Advanced Settings
	fontfeaturesWidget = new PropertyWidget_FontFeatures();
	//fontfeaturesWidgetItem = textTree->addItem(fontfeaturesWidget, tr("Font Features"));

	pathTextWidgets = new PropertyWidget_PathText();
	//pathTextItem = textTree->addItem(pathTextWidgets, tr("Path Text Properties"));

	ScPopupMenu * popupFontFeatures = new ScPopupMenu(fontfeaturesWidget);


	layoutSectionText = new ScLayoutSection(tr("Text"),popupFontFeatures);
	layoutSectionParagraph = new ScLayoutSection(tr("Paragraph"));
	layoutSectionCharacter = new ScLayoutSection(tr("Character"));
	layoutSectionLists = new ScLayoutSection(tr("Lists"));
	layoutSectionTextPath = new ScLayoutSection(tr("Text Path"));

	layoutSectionText->addWidget(textWidgets);
	layoutSectionText->addWidget(colorWidgets);
	layoutSectionText->addWidget(parEffectWidgets);

	layoutSectionParagraph->addWidget(distanceWidgets);
	layoutSectionParagraph->addWidget(flopBox);
	layoutSectionParagraph->addWidget(optMargins);
	layoutSectionParagraph->addWidget(orphanBox);

	layoutSectionCharacter->addWidget(advancedWidgets);
	layoutSectionCharacter->addWidget(hyphenationWidget);

	layoutSectionTextPath->addWidget(pathTextWidgets);

	FlowLayout *flowLayout = new FlowLayout(0,0,0);
	flowLayout->addWidget(layoutSectionText);
	flowLayout->addWidget(layoutSectionParagraph);
	flowLayout->addWidget(layoutSectionCharacter);
	flowLayout->addWidget(layoutSectionLists);
	flowLayout->addWidget(layoutSectionTextPath);

	setLayout(flowLayout);



	languageChange();

//	connect(lineSpacing   , SIGNAL(valueChanged(double)), this, SLOT(handleLineSpacing()));
//	connect(fonts         , SIGNAL(fontSelected(QString )), this, SLOT(handleTextFont(QString)));
//	connect(fontSize      , SIGNAL(valueChanged(double)), this, SLOT(handleFontSize()));
//	connect(textAlignment , SIGNAL(State(int))   , this, SLOT(handleAlignment(int)));
//	connect(textDirection , SIGNAL(State(int))   , this, SLOT(handleDirection(int)));
//	connect(textWidgets->charStyleClear, SIGNAL(clicked()), this, SLOT(doClearCStyle()));
//	connect(textWidgets->paraStyleClear, SIGNAL(clicked()), this, SLOT(doClearPStyle()));

	connect(flopBox->flopGroup, SIGNAL(buttonClicked( int )), this, SLOT(handleFirstLinePolicy(int)));

//	connect(lineSpacingModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(handleLineSpacingMode(int)));
//	connect(langCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLang(int)));

	connect(fontfeaturesWidget, SIGNAL(needsRelayout()), this, SLOT(updateTreeLayout()));
	connect(parEffectWidgets,   SIGNAL(needsRelayout()), this, SLOT(updateTreeLayout()));

	m_haveItem = false;
	setEnabled(false);
}

void PropertiesPalette_Text::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	advancedWidgets->setMainWindow(mw);
	fontfeaturesWidget->setMainWindow(mw);
	colorWidgets->setMainWindow(mw);
	distanceWidgets->setMainWindow(mw);
	hyphenationWidget->setMainWindow(mw);
	parEffectWidgets->setMainWindow(mw);
	optMargins->setMainWindow(mw);
	pathTextWidgets->setMainWindow(mw);

	connect(m_ScMW, SIGNAL(UpdateRequest(int))     , this  , SLOT(handleUpdateRequest(int)));

//	connect(paraStyleCombo, SIGNAL(newStyle(const QString&)), m_ScMW, SLOT(setNewParStyle(const QString&)), Qt::UniqueConnection);
//	connect(charStyleCombo, SIGNAL(newStyle(const QString&)), m_ScMW, SLOT(setNewCharStyle(const QString&)), Qt::UniqueConnection);
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

	m_unitRatio   = m_doc->unitRatio();
	m_unitIndex   = m_doc->unitIndex();

	m_haveDoc  = true;
	m_haveItem = false;

//	fontSize->setValues( 0.5, 2048, 2, 1);
//	lineSpacing->setValues( 1, 2048, 2, 1);

	advancedWidgets->setDoc(m_doc);
	fontfeaturesWidget->setDoc(m_doc);
	textWidgets->setDoc(m_doc);
	colorWidgets->setDoc(m_doc);
	distanceWidgets->setDoc(m_doc);
	parEffectWidgets->setDoc(m_doc);
	flopBox->setDoc(m_doc);
	hyphenationWidget->setDoc(m_doc);
	optMargins->setDoc(m_doc);
	orphanBox->setDoc(m_doc);
	pathTextWidgets->setDoc(m_doc);

//	fonts->RebuildList(m_doc);
//	paraStyleCombo->setDoc(m_doc);
//	charStyleCombo->setDoc(m_doc);

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
	// ColorWidget will handle its update itself
	/*if (updateFlags & reqColorsUpdate)
		updateColorList();*/
	if (updateFlags & reqCharStylesUpdate)
	{
		//charStyleCombo->updateFormatList();
		parEffectWidgets->updateCharStyles();
	}
	if (updateFlags & reqParaStylesUpdate)
		//paraStyleCombo->updateFormatList();
	if (updateFlags & reqDefFontListUpdate)
		//fonts->RebuildList(0);
	if (updateFlags & reqDocFontListUpdate)
		//fonts->RebuildList(m_haveDoc ? m_doc : 0);
	if (updateFlags & reqStyleComboDocUpdate)
	{
		//paraStyleCombo->setDoc(m_haveDoc ? m_doc : 0);
		//charStyleCombo->setDoc(m_haveDoc ? m_doc : 0);
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

	textWidgets->setCurrentItem(i);

	showFirstLinePolicy(m_item->firstLineOffset());

	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		setEnabled(false);
	}

	m_haveItem = true;

	if (!sender())
	{
		colorWidgets->handleSelectionChanged();
		distanceWidgets->handleSelectionChanged();
		parEffectWidgets->handleSelectionChanged();
	}

	if (m_item->asTextFrame() || m_item->asPathText() || m_item->asTable())
	{
		ParagraphStyle parStyle =  m_item->itemText.defaultStyle();
		if (m_doc->appMode == modeEdit || m_doc->appMode == modeEditTable)
			m_item->currentTextProps(parStyle);
		textWidgets->updateStyle(parStyle);
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
	colorWidgets->unitChange();
	distanceWidgets->unitChange();
	flopBox->unitChange();
	optMargins->unitChange();
	pathTextWidgets->unitChange();
	parEffectWidgets->unitChange();

	m_haveItem = tmp;
}

void PropertiesPalette_Text::handleLineSpacingMode(int id)
{
	if ((m_haveDoc) && (m_haveItem))
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(m_item, true);
		m_doc->itemSelection_SetLineSpacingMode(id, &tempSelection);
	//	updateStyle(((m_doc->appMode == modeEdit) || (m_doc->appMode == modeEditTable)) ? m_item->currentStyle() : m_item->itemText.defaultStyle());
		m_doc->regionsChanged()->update(QRect());
	}
}

void PropertiesPalette_Text::changeLang(int id)
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

void PropertiesPalette_Text::showLineSpacing(double r)
{
	textWidgets->showLineSpacing(r);

//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool inEditMode = (m_doc->appMode == modeEdit || m_doc->appMode == modeEditTable);
//	bool tmp = m_haveItem;
//	m_haveItem = false;
//	lineSpacing->showValue(r);
//	const ParagraphStyle& curStyle(m_haveItem && inEditMode ? m_item->currentStyle() : m_item->itemText.defaultStyle());
//	if (tmp)
//	{
//		setupLineSpacingSpinbox(curStyle.lineSpacingMode(), r);
//		lineSpacingModeCombo->setCurrentIndex(curStyle.lineSpacingMode());
//	}
//	m_haveItem = tmp;
}

void PropertiesPalette_Text::showFontFace(const QString& newFont)
{
	textWidgets->showFontFace(newFont);

//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool tmp = m_haveItem;
//	m_haveItem = false;
//	if (m_item != NULL)
//		fonts->RebuildList(m_doc, m_item->isAnnotation());
//	fonts->setCurrentFont(newFont);
//	m_haveItem = tmp;
}

void PropertiesPalette_Text::showFontSize(double s)
{
	textWidgets->showFontSize(s);

//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	fontSize->showValue(s / 10.0);
}

void PropertiesPalette_Text::showLanguage(QString w)
{
	textWidgets->showLanguage(w);

//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	QStringList lang;
//	LanguageManager::instance()->fillInstalledStringList(&lang);
//	QString langName = LanguageManager::instance()->getLangFromAbbrev(w, true);

//	bool sigBlocked  = langCombo->blockSignals(true);
//	langCombo->setCurrentIndex(lang.indexOf(langName));
//	langCombo->blockSignals(sigBlocked);
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

//	bool blocked = lineSpacing->blockSignals(true);
//	if (mode > 0)
//	{
//		if (mode==1)
//			lineSpacing->setSpecialValueText( tr( "Auto" ) );
//		if (mode==2)
//			lineSpacing->setSpecialValueText( tr( "Baseline" ) );
//		lineSpacing->setMinimum(0);
//		lineSpacing->setValue(0);
//		lineSpacing->setEnabled(false);
//	}
//	else
//	{
//		lineSpacing->setSpecialValueText("");
//		lineSpacing->setMinimum(1);
//		lineSpacing->setValue(value);
//		lineSpacing->setEnabled(true);
//	}
//	lineSpacing->blockSignals(blocked);
}

void PropertiesPalette_Text::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	advancedWidgets->updateCharStyle(charStyle);
	fontfeaturesWidget->updateCharStyle(charStyle);
	colorWidgets->updateCharStyle(charStyle);
	hyphenationWidget->updateCharStyle(charStyle);
	textWidgets->updateCharStyle(charStyle);

//	showFontFace(charStyle.font().scName());
//	showFontSize(charStyle.fontSize());
//	showLanguage(charStyle.language());
}

void PropertiesPalette_Text::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

//	const CharStyle& charStyle = newCurrent.charStyle();

	advancedWidgets->updateStyle(newCurrent);
	fontfeaturesWidget->updateStyle(newCurrent);
	colorWidgets->updateStyle(newCurrent);
	optMargins->updateStyle(newCurrent);
	orphanBox->updateStyle (newCurrent);
	parEffectWidgets->updateStyle(newCurrent);
	hyphenationWidget->updateStyle(newCurrent);

	textWidgets->updateStyle(newCurrent);
//	showFontFace(charStyle.font().scName());
//	showFontSize(charStyle.fontSize());
//	showLanguage(charStyle.language());

//	showParStyle(newCurrent.parent());
//	showCharStyle(charStyle.parent());

//	bool tmp = m_haveItem;
//	m_haveItem = false;

//	setupLineSpacingSpinbox(newCurrent.lineSpacingMode(), newCurrent.lineSpacing());
//	lineSpacingModeCombo->setCurrentIndex(newCurrent.lineSpacingMode());
//	textAlignment->setStyle(newCurrent.alignment(), newCurrent.direction());
//	textDirection->setStyle(newCurrent.direction());

//7	m_haveItem = tmp;
}

void PropertiesPalette_Text::updateCharStyles()
{
	textWidgets->updateCharStyles();
//	charStyleCombo->updateFormatList();
	parEffectWidgets->updateCharStyles();
}

void PropertiesPalette_Text::updateParagraphStyles()
{
	textWidgets->updateParagraphStyles();
//	paraStyleCombo->updateFormatList();
//	charStyleCombo->updateFormatList();
	parEffectWidgets->updateCharStyles();
}

void PropertiesPalette_Text::updateTextStyles()
{
	textWidgets->updateTextStyles();
//	paraStyleCombo->updateFormatList();
//	charStyleCombo->updateFormatList();
}

void PropertiesPalette_Text::updateTreeLayout()
{
	//textTree->doItemsLayout();
}

void PropertiesPalette_Text::showAlignment(int e)
{
	textWidgets->showAlignment(e);

//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool tmp = m_haveItem;
//	m_haveItem = false;
//	textAlignment->setEnabled(true);
//	textAlignment->setStyle(e, textDirection->getStyle());
//	m_haveItem = tmp;
}

void PropertiesPalette_Text::showDirection(int e)
{
	textWidgets->showDirection(e);

//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool tmp = m_haveItem;
//	m_haveItem = false;
//	textDirection->setEnabled(true);
//	textDirection->setStyle(e);
//	m_haveItem = tmp;
}

void PropertiesPalette_Text::showCharStyle(const QString& name)
{
	textWidgets->showCharStyle(name);

//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool blocked = charStyleCombo->blockSignals(true);
//	charStyleCombo->setFormat(name);
//	charStyleCombo->blockSignals(blocked);
}

void PropertiesPalette_Text::showParStyle(const QString& name)
{
	textWidgets->showParStyle(name);

//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool blocked = paraStyleCombo->blockSignals(true);
//	paraStyleCombo->setFormat(name);
//	paraStyleCombo->blockSignals(blocked);
}

void PropertiesPalette_Text::handleLineSpacing()
{
	textWidgets->handleLineSpacing();

//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_SetLineSpacing(lineSpacing->value(), &tempSelection);
}

void PropertiesPalette_Text::handleFontSize()
{
	textWidgets->handleFontSize();

//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_SetFontSize(qRound(fontSize->value()*10.0), &tempSelection);
}

void PropertiesPalette_Text::handleAlignment(int a)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetAlignment(a, &tempSelection);
	if (m_item->isPathText())
		pathTextWidgets->handleSelectionChanged();
}

void PropertiesPalette_Text::handleDirection(int d)
{
	textWidgets->handleDirection(d);

//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_SetDirection(d, &tempSelection);
//	// If current text alignment is left or right, change it to match direction
//	if (d == ParagraphStyle::RTL && textAlignment->selectedId() == ParagraphStyle::Leftaligned)
//	{
//		m_doc->itemSelection_SetAlignment(ParagraphStyle::Rightaligned, &tempSelection);
//		textAlignment->setTypeStyle(ParagraphStyle::Rightaligned);
//	}
//	else if (d == ParagraphStyle::LTR && textAlignment->selectedId() == ParagraphStyle::Rightaligned)
//	{
//		m_doc->itemSelection_SetAlignment(ParagraphStyle::Leftaligned, &tempSelection);
//		textAlignment->setTypeStyle(ParagraphStyle::Leftaligned);
//	}
}

void PropertiesPalette_Text::handleTextFont(QString c)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->SetNewFont(c);
}

void PropertiesPalette_Text::doClearCStyle()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning() || !m_haveDoc || !m_haveItem)
		return;
	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_EraseCharStyle(&tempSelection);
}


void PropertiesPalette_Text::doClearPStyle()
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
//	retranslateUi(this);

//	textTree->setItemText(colorWidgetsItem, tr("Color && Effects"));
//	textTree->setItemText(flopItem, tr("First Line Offset"));
//	textTree->setItemText(hyphenationWidgetItem, tr("Hyphenation"));
//	textTree->setItemText(orphanItem, tr("Orphans and Widows"));
//	textTree->setItemText(parEffectItem, tr("Paragraph Effects"));
//	textTree->setItemText(distanceItem, tr("Columns && Text Distances"));
//	textTree->setItemText(optMarginsItem, tr("Optical Margins"));
//	textTree->setItemText(advancedWidgetsItem, tr("Advanced Settings"));
//	textTree->setItemText(fontfeaturesWidgetItem, tr("Font Features"));
//	textTree->setItemText(pathTextItem, tr("Path Text Properties"));

//	QSignalBlocker lineSpacingModeBlocker(lineSpacingModeCombo);
//	int oldLineSpacingMode = lineSpacingModeCombo->currentIndex();
//	lineSpacingModeCombo->clear();
//	lineSpacingModeCombo->addItem( tr("Fixed Linespacing"));
//	lineSpacingModeCombo->addItem( tr("Automatic Linespacing"));
//	lineSpacingModeCombo->addItem( tr("Align to Baseline Grid"));
//	lineSpacingModeCombo->setCurrentIndex(oldLineSpacingMode);

//	QSignalBlocker langComboBlocker(langCombo);
//	QStringList languageList;
//	LanguageManager::instance()->fillInstalledStringList(&languageList);
//	int oldLang = langCombo->currentIndex();
//	langCombo->clear();
//	langCombo->addItems(languageList);
//	langCombo->setCurrentIndex(oldLang);

	textWidgets->languageChange();

	colorWidgets->languageChange();
	flopBox->languageChange();
	orphanBox->languageChange();
	distanceWidgets->languageChange();
	optMargins->languageChange();
	advancedWidgets->languageChange();
	pathTextWidgets->languageChange();
	fontfeaturesWidget->languageChange();
	hyphenationWidget->languageChange();

//	textAlignment->languageChange();
//	textDirection->languageChange();
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
