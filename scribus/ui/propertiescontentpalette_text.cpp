/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "propertiescontentpalette_text.h"

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
#include "propertywidgettext_distance.h"
#include "propertywidgettext_fontfeatures.h"
#include "propertywidgettext_glyphspace.h"
#include "propertywidgettext_glyphstretch.h"
#include "propertywidgettext_hyphenate.h"
#include "propertywidgettext_hyphenation.h"
#include "propertywidgettext_optmargins.h"
#include "propertywidgettext_orphans.h"
#include "propertywidgettext_pareffect.h"
#include "propertywidgettext_pathtext.h"
#include "propertywidgettext_textadvanced.h"
#include "propertywidgettext_textalignment.h"
#include "propertywidgettext_textfont.h"
#include "propertywidgettext_textcolor.h"
#include "propertywidgettext_textstyles.h"
#include "scpopupmenu.h"
#include "scraction.h"
#include "scribuscore.h"
#include "selection.h"
#include "tabmanager.h"
#include "undomanager.h"
#include "units.h"
#include "util.h"
#include "util_math.h"
#include "langmgr.h"

PropertiesContentPalette_Text::PropertiesContentPalette_Text( QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_haveDoc = false;

	textWidgets = new PropertyWidgetText_TextFont();
	textAdvancedWidgets = new PropertyWidgetText_TextAdvanced();
	textAlignmentWidgets = new PropertyWidgetText_TextAlignment();
	colorWidgets = new PropertyWidgetText_TextColor();
	orphanBox = new PropertyWidgetText_Orphans();
	parEffectWidgets = new PropertyWidgetText_ParEffect();
	distanceWidgets = new PropertyWidgetText_Distance();
	optMargins = new PropertyWidgetText_OptMargins();
	hyphenateWidget = new PropertyWidgetText_Hyphenate();
	hyphenationWidget = new PropertyWidgetText_Hyphenation();
	charSpaceWidgets = new PropertyWidgetText_GlyphSpace();
	charStretchWidgets = new PropertyWidgetText_GlyphStretch();
	fontfeaturesWidget = new PropertyWidgetText_FontFeatures();
	pathTextWidgets = new PropertyWidgetText_PathText();
	textStylesWidgets = new PropertyWidgetText_TextStyles();

	ScPopupMenu * popupFontFeatures = new ScPopupMenu(textAdvancedWidgets);
	popupFontFeatures->addWidget(colorWidgets);
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
	layoutSectionText->addWidget(textStylesWidgets);

	layoutSectionParagraph->addWidget(distanceWidgets);
	layoutSectionParagraph->addWidget(optMargins);

	layoutSectionCharacter->addWidget(charSpaceWidgets);
	layoutSectionCharacter->addWidget(charStretchWidgets);
	layoutSectionCharacter->addWidget(hyphenateWidget);

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

	connect(fontfeaturesWidget, SIGNAL(needsRelayout()), popupFontFeatures, SLOT(updateSize()));

}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertiesContentPalette_Text::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	charSpaceWidgets->setMainWindow(mw);
	charStretchWidgets->setMainWindow(mw);
	fontfeaturesWidget->setMainWindow(mw);
	textWidgets->setMainWindow(mw);
	textAlignmentWidgets->setMainWindow(mw);
	colorWidgets->setMainWindow(mw);
	distanceWidgets->setMainWindow(mw);
	hyphenateWidget->setMainWindow(mw);
	hyphenationWidget->setMainWindow(mw);
	parEffectWidgets->setMainWindow(mw);
	optMargins->setMainWindow(mw);
	pathTextWidgets->setMainWindow(mw);
	textAdvancedWidgets->setMainWindow(mw);
	textStylesWidgets->setMainWindow(mw);


}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertiesContentPalette_Text::setDoc(ScribusDoc *d)
{
	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	m_doc  = d;

	charSpaceWidgets->setDoc(m_doc);
	charStretchWidgets->setDoc(m_doc);
	fontfeaturesWidget->setDoc(m_doc);
	textWidgets->setDoc(m_doc);
	textAlignmentWidgets->setDoc(m_doc);
	colorWidgets->setDoc(m_doc);
	distanceWidgets->setDoc(m_doc);
	parEffectWidgets->setDoc(m_doc);
	hyphenateWidget->setDoc(m_doc);
	hyphenationWidget->setDoc(m_doc);
	optMargins->setDoc(m_doc);
	orphanBox->setDoc(m_doc);
	pathTextWidgets->setDoc(m_doc);
	textAdvancedWidgets->setDoc(m_doc);
	textStylesWidgets->setDoc(m_doc);

}

void PropertiesContentPalette_Text::unsetDoc()
{

	m_doc      = NULL;

	charSpaceWidgets->setDoc(0);
	charStretchWidgets->setDoc(0);
	fontfeaturesWidget->setDoc(0);
	textWidgets->setDoc(0);
	textAlignmentWidgets->setDoc(0);
	colorWidgets->setDoc(0);
	distanceWidgets->setDoc(0);
	hyphenateWidget->setDoc(0);
	hyphenationWidget->setDoc(0);
	optMargins->setDoc(0);
	orphanBox->setDoc(0);
	parEffectWidgets->setDoc(0);
	pathTextWidgets->setDoc(0);
	textStylesWidgets->setDoc(0);
	textAdvancedWidgets->setDoc(0);

}


/*********************************************************************
*
* Features
*
**********************************************************************/

void PropertiesContentPalette_Text::showLanguage(QString w)
{

	textAdvancedWidgets->showLanguage(w);

}

void PropertiesContentPalette_Text::showFontSize(double s)
{
	textWidgets->showFontSize(s);
}

void PropertiesContentPalette_Text::showAlignment(int e)
{
	textAlignmentWidgets->showAlignment(e);
}

void PropertiesContentPalette_Text::showDirection(int e)
{
	textAlignmentWidgets->showDirection(e);

}

void PropertiesContentPalette_Text::showCharStyle(const QString& name)
{
	textStylesWidgets->showCharStyle(name);
}

void PropertiesContentPalette_Text::showParStyle(const QString& name)
{
	textStylesWidgets->showParStyle(name);
}


/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertiesContentPalette_Text::handleSelectionChanged()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	charSpaceWidgets->handleSelectionChanged();
	charStretchWidgets->handleSelectionChanged();
	fontfeaturesWidget->handleSelectionChanged();
	colorWidgets->handleSelectionChanged();
	textAlignmentWidgets->handleSelectionChanged();
	optMargins->handleSelectionChanged();
	orphanBox->handleSelectionChanged();
	parEffectWidgets->handleSelectionChanged();
	hyphenationWidget->handleSelectionChanged();
	textWidgets->handleSelectionChanged();
	textStylesWidgets->handleSelectionChanged();
	textAdvancedWidgets->handleSelectionChanged();
}


void PropertiesContentPalette_Text::unitChange()
{
	if (!m_haveDoc)
		return;

	charSpaceWidgets->unitChange();
	charStretchWidgets->unitChange();
	hyphenateWidget->unitChange();
	fontfeaturesWidget->unitChange();
	textAlignmentWidgets->unitChange();
	colorWidgets->unitChange();
	distanceWidgets->unitChange();
	optMargins->unitChange();
	pathTextWidgets->unitChange();
	parEffectWidgets->unitChange();

}

void PropertiesContentPalette_Text::languageChange()
{

	textWidgets->languageChange();
	colorWidgets->languageChange();
	textAlignmentWidgets->languageChange();
	orphanBox->languageChange();
	distanceWidgets->languageChange();
	optMargins->languageChange();
	charSpaceWidgets->languageChange();
	charStretchWidgets->languageChange();
	pathTextWidgets->languageChange();
	fontfeaturesWidget->languageChange();
	hyphenateWidget->languageChange();
	hyphenationWidget->languageChange();
	textStylesWidgets->languageChange();
	textAdvancedWidgets->languageChange();
}

void PropertiesContentPalette_Text::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	charSpaceWidgets->updateCharStyle(charStyle);
	charStretchWidgets->updateCharStyle(charStyle);
	fontfeaturesWidget->updateCharStyle(charStyle);
	colorWidgets->updateCharStyle(charStyle);
	textAlignmentWidgets->updateCharStyle(charStyle);
	hyphenationWidget->updateCharStyle(charStyle);
	textWidgets->updateCharStyle(charStyle);
	textAdvancedWidgets->updateCharStyle(charStyle);
}

void PropertiesContentPalette_Text::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	charSpaceWidgets->updateStyle(newCurrent);
	charStretchWidgets->updateStyle(newCurrent);
	fontfeaturesWidget->updateStyle(newCurrent);
	colorWidgets->updateStyle(newCurrent);
	textAlignmentWidgets->updateStyle(newCurrent);
	optMargins->updateStyle(newCurrent);
	orphanBox->updateStyle (newCurrent);
	parEffectWidgets->updateStyle(newCurrent);
	hyphenationWidget->updateStyle(newCurrent);
	textWidgets->updateStyle(newCurrent);
	textStylesWidgets->updateStyle(newCurrent);
	textAdvancedWidgets->updateStyle(newCurrent);
}

void PropertiesContentPalette_Text::updateParagraphStyles()
{
	textStylesWidgets->updateParagraphStyles();
	parEffectWidgets->updateCharStyles();
}


void PropertiesContentPalette_Text::updateColorList()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	colorWidgets->updateColorList();
	textWidgets->updateColorList();
}



/*********************************************************************
*
* Event
*
**********************************************************************/

void PropertiesContentPalette_Text::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
