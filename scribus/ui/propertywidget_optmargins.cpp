/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_optmargins.h"

#include "appmodehelper.h"
#include "appmodes.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
#include "units.h"
#include "tabmanager.h"
#include "iconmanager.h"

PropertyWidget_OptMargins::PropertyWidget_OptMargins(QWidget* parent) : QWidget(parent)
{
	m_item = 0;
	m_ScMW = 0;

	setupUi(this);

	// First Line Offset
	flopRealHeight->setIcon(IconManager::instance()->loadPixmap("16/flob-align-realheight.png"));
	flopRealHeight->setCheckable(true);
	flopRealHeight->setChecked(true);
	flopFontAscent->setIcon(IconManager::instance()->loadPixmap("16/flob-align-fontascent.png"));
	flopFontAscent->setCheckable(true);
	flopLineSpacing->setIcon(IconManager::instance()->loadPixmap("16/flob-align-linespace.png"));
	flopLineSpacing->setCheckable(true);
	flopBaselineGrid->setIcon(IconManager::instance()->loadPixmap("16/flob-align-baseline.png"));
	flopBaselineGrid->setCheckable(true);

	flopGroup->setId(flopRealHeight,  RealHeightID);
	flopGroup->setId(flopFontAscent,  FontAscentID);
	flopGroup->setId(flopLineSpacing, LineSpacingID);
	flopGroup->setId(flopBaselineGrid, BaselineGridID);


	// Optical Margins
	optMarginRadioBoth->setIcon(IconManager::instance()->loadPixmap("16/optmar-align-both.png"));
	optMarginRadioBoth->setCheckable(true);
	optMarginRadioLeft->setIcon(IconManager::instance()->loadPixmap("16/optmar-align-left.png"));
	optMarginRadioLeft->setCheckable(true);
	optMarginRadioRight->setIcon(IconManager::instance()->loadPixmap("16/optmar-align-right.png"));
	optMarginRadioRight->setCheckable(true);
	optMarginRadioNone->setIcon(IconManager::instance()->loadPixmap("16/optmar-align-none.png"));
	optMarginRadioNone->setCheckable(true);

	languageChange();

	connect(flopGroup, SIGNAL(buttonClicked( int )), this, SLOT(handleFirstLinePolicy(int)));
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidget_OptMargins::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(m_ScMW->appModeHelper, SIGNAL(AppModeChanged(int, int)), this, SLOT(handleAppModeChanged(int, int)));
	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this  , SLOT(handleUpdateRequest(int)));
}

void PropertyWidget_OptMargins::connectSignals()
{
	//Optical Margins
	connect(optMarginRadioNone  , SIGNAL(clicked()), this, SLOT(handleOpticalMargins()) );
	connect(optMarginRadioBoth  , SIGNAL(clicked()), this, SLOT(handleOpticalMargins()) );
	connect(optMarginRadioLeft  , SIGNAL(clicked()), this, SLOT(handleOpticalMargins()) );
	connect(optMarginRadioRight , SIGNAL(clicked()), this, SLOT(handleOpticalMargins()) );
	connect(optMarginResetButton, SIGNAL(clicked()), this, SLOT(resetOpticalMargins()) );

	//Tabs
	connect(tabsButton    , SIGNAL(clicked())           , this, SLOT(handleTabs()), Qt::UniqueConnection);
}

void PropertyWidget_OptMargins::disconnectSignals()
{
	//Optical Margins
	disconnect(optMarginRadioNone  , SIGNAL(clicked()), this, SLOT(handleOpticalMargins()) );
	disconnect(optMarginRadioBoth  , SIGNAL(clicked()), this, SLOT(handleOpticalMargins()) );
	disconnect(optMarginRadioLeft  , SIGNAL(clicked()), this, SLOT(handleOpticalMargins()) );
	disconnect(optMarginRadioRight , SIGNAL(clicked()), this, SLOT(handleOpticalMargins()) );
	disconnect(optMarginResetButton, SIGNAL(clicked()), this, SLOT(resetOpticalMargins()) );

	//Tabs
	disconnect(tabsButton    , SIGNAL(clicked())           , this, SLOT(handleTabs()));
}

void PropertyWidget_OptMargins::configureWidgets(void)
{
	bool enabled = false;
	if (m_item && m_doc)
	{
		PageItem_TextFrame* textItem = m_item->asTextFrame();
		if (m_doc->appMode == modeEditTable)
			textItem = m_item->asTable()->activeCell().textFrame();

		enabled  = (m_item->isPathText() || (textItem != NULL));
		enabled &= (m_doc->m_Selection->count() == 1);

		showFirstLinePolicy(m_item->firstLineOffset()); // flopBox
	}
	setEnabled(enabled);
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidget_OptMargins::setDoc(ScribusDoc *d)
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

void PropertyWidget_OptMargins::setCurrentItem(PageItem *item)
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
		if (m_item->asTextFrame() || m_item->asPathText())
		{
			ParagraphStyle parStyle =  m_item->itemText.defaultStyle();
			if (m_doc->appMode == modeEdit || m_doc->appMode == modeEditTable)
				m_item->currentTextProps(parStyle);
			showOpticalMargins(parStyle);
		}

		connectSignals();
	}
}


/*********************************************************************
*
* Optical Margin
*
**********************************************************************/

void PropertyWidget_OptMargins::showOpticalMargins(const ParagraphStyle & pStyle)
{
	ParagraphStyle::OpticalMarginType omt(static_cast<ParagraphStyle::OpticalMarginType>(pStyle.opticalMargins()));
	bool blocked = optMarginRadioBoth->blockSignals(true);
	if (omt == ParagraphStyle::OM_Default)
		optMarginRadioBoth->setChecked(true);
	else if (omt == ParagraphStyle::OM_LeftHangingPunct)
		optMarginRadioLeft->setChecked(true);
	else if (omt == ParagraphStyle::OM_RightHangingPunct)
		optMarginRadioRight->setChecked(true);
	else
		optMarginRadioNone->setChecked(true);
	optMarginRadioBoth->blockSignals(blocked);
}

void PropertyWidget_OptMargins::updateStyle(const ParagraphStyle& newCurrent)
{
	showOpticalMargins(newCurrent);
}

void PropertyWidget_OptMargins::handleOpticalMargins()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	int omt(ParagraphStyle::OM_None);
	if (optMarginRadioBoth->isChecked())
		omt = ParagraphStyle::OM_Default;
	else if (optMarginRadioLeft->isChecked())
		omt = ParagraphStyle::OM_LeftHangingPunct;
	else if (optMarginRadioRight->isChecked())
		omt = ParagraphStyle::OM_RightHangingPunct;

	PageItem *item = m_item;
	if (m_doc->appMode == modeEditTable)
		item = m_item->asTable()->activeCell().textFrame();
	if (item != NULL)
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(item, true);
		m_doc->itemSelection_SetOpticalMargins(omt, &tempSelection);
	}
}

void PropertyWidget_OptMargins::resetOpticalMargins()
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	PageItem *item = m_item;
	if (m_doc->appMode == modeEditTable)
		item = m_item->asTable()->activeCell().textFrame();
	if (item != NULL)
	{
		Selection tempSelection(this, false);
		tempSelection.addItem(item, true);
		m_doc->itemSelection_resetOpticalMargins(&tempSelection);
	}
}


/*********************************************************************
*
* FirstLine
*
**********************************************************************/

void PropertyWidget_OptMargins::showFirstLinePolicy( FirstLineOffsetPolicy f )
{
	if(f == FLOPFontAscent)
		flopFontAscent->setChecked(true);
	else if(f == FLOPLineSpacing)
		flopLineSpacing->setChecked(true);
	else if (f == FLOPRealGlyphHeight)
		flopRealHeight->setChecked(true); //It’s historical behaviour.
	else // if (f == FLOPBaseline)
		flopBaselineGrid->setChecked(true);
}


void PropertyWidget_OptMargins::handleFirstLinePolicy(int radioFlop)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning()/* || !m_haveDoc*/)
		return;
	if( radioFlop == RealHeightID)
		m_item->setFirstLineOffset(FLOPRealGlyphHeight);
	else if( radioFlop == FontAscentID)
		m_item->setFirstLineOffset(FLOPFontAscent);
	else if( radioFlop == LineSpacingID)
		m_item->setFirstLineOffset(FLOPLineSpacing);
	else if( radioFlop == BaselineGridID)
		m_item->setFirstLineOffset(FLOPBaselineGrid);
	m_item->update();
	if (m_doc->appMode == modeEditTable)
		m_item->parentTable()->update();
	else
		m_item->update();
	m_doc->regionsChanged()->update(QRect());
}


/*********************************************************************
*
* Tabs
*
**********************************************************************/

void PropertyWidget_OptMargins::handleTabs()
{
	if (m_doc && m_item)
	{
		PageItem_TextFrame *tItem = m_item->asTextFrame();
		if (tItem == 0)
			return;
		const ParagraphStyle& style(m_doc->appMode == modeEdit ? tItem->currentStyle() : tItem->itemText.defaultStyle());
		TabManager *dia = new TabManager(this, m_doc->unitIndex(), style.tabValues(), tItem->columnWidth());
		if (dia->exec())
		{
			if (m_doc->appMode != modeEdit)
			{
				ParagraphStyle newStyle(m_item->itemText.defaultStyle());
				newStyle.setTabValues(dia->tmpTab);
				Selection tempSelection(this, false);
				tempSelection.addItem(m_item, true);
				m_doc->itemSelection_ApplyParagraphStyle(newStyle, &tempSelection);
			}
			else
			{
				ParagraphStyle newStyle;
				newStyle.setTabValues(dia->tmpTab);
				m_doc->itemSelection_ApplyParagraphStyle(newStyle);
			}
			m_item->update();
		}
		delete dia;
	}
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidget_OptMargins::handleAppModeChanged(int oldMode, int mode)
{
	if (oldMode == modeEditTable || mode == modeEditTable)
	{
		setCurrentItem(m_item);
	}
}

void PropertyWidget_OptMargins::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();

	if (m_doc->m_Selection->count() > 1 )
	{
		flopRealHeight->setChecked(true);
	}

	setCurrentItem(currItem);
	updateGeometry();
}

void PropertyWidget_OptMargins::languageChange()
{
	retranslateUi(this);
}


/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidget_OptMargins::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
