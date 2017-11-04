/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_flop.h"

#include "scribus.h"
#include "scribusdoc.h"
#include "appmodes.h"
#include "units.h"
#include "pageitem_table.h"


PropertyWidget_Flop::PropertyWidget_Flop(QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_item=0;
	m_haveDoc = false;

	setupUi(this);

	//layout()->setAlignment( Qt::AlignTop );
	
	flopRealHeight->setChecked(true);

	flopGroup->setId(flopRealHeight,  RealHeightID);
	flopGroup->setId(flopFontAscent,  FontAscentID);
	flopGroup->setId(flopLineSpacing, LineSpacingID);
	flopGroup->setId(flopBaselineGrid, BaselineGridID);

	languageChange();

	connect(flopGroup, SIGNAL(buttonClicked( int )), this, SLOT(handleFirstLinePolicy(int)));
}



void PropertyWidget_Flop::showFirstLinePolicy( FirstLineOffsetPolicy f )
{
	if(f == FLOPFontAscent)
		this->flopFontAscent->setChecked(true);
	else if(f == FLOPLineSpacing)
		this->flopLineSpacing->setChecked(true);
	else if (f == FLOPRealGlyphHeight)
		this->flopRealHeight->setChecked(true); //It’s historical behaviour.
	else // if (f == FLOPBaseline)
		this->flopBaselineGrid->setChecked(true);
}


void PropertyWidget_Flop::handleFirstLinePolicy(int radioFlop)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning() || !m_haveDoc)
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

void PropertyWidget_Flop::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}

void PropertyWidget_Flop::languageChange()
{
	retranslateUi(this);
}
