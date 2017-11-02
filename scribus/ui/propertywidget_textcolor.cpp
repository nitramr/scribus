/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "appmodes.h"
#include "iconmanager.h"
#include "pageitem_table.h"
#include "propertywidget_textcolor.h"
#include "scribus.h"
#include "selection.h"

PropertyWidget_TextColor::PropertyWidget_TextColor(QWidget* parent) : QWidget(parent)
{
	m_item = NULL;
	m_ScMW = NULL;

	setupUi(this);

	fillLayout->setAlignment( Qt::AlignLeft );
	fillColor->setPixmapType(ColorCombo::fancyPixmaps);
	fillIcon->setPixmap(IconManager::instance()->loadPixmap("16/color-fill.png"));
	fillIcon->setScaledContents( false );
	fillShadeLabel->setPixmap(IconManager::instance()->loadPixmap("shade.png"));

	strokeLayout->setAlignment( Qt::AlignLeft );
	strokeColor->setPixmapType(ColorCombo::fancyPixmaps);
	strokeIcon->setPixmap(IconManager::instance()->loadPixmap("16/color-stroke.png"));
	strokeIcon->setScaledContents( false );
	strokeShadeLabel->setPixmap(IconManager::instance()->loadPixmap("shade.png"));

	backLayout->setAlignment( Qt::AlignLeft );
	backColor->setPixmapType(ColorCombo::fancyPixmaps);
	backIcon->setPixmap(IconManager::instance()->loadPixmap("16/color-fill.png"));
	backIcon->setScaledContents( false );
	backShadeLabel->setPixmap(IconManager::instance()->loadPixmap("shade.png"));

	languageChange();

	strokeIcon->setEnabled(false);
	strokeColor->setEnabled(false);
	strokeShade->setEnabled(false);
}

void PropertyWidget_TextColor::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
}

void PropertyWidget_TextColor::setDoc(ScribusDoc *d)
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

void PropertyWidget_TextColor::setCurrentItem(PageItem *item)
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

void PropertyWidget_TextColor::connectSignals()
{
	connect(fillColor   , SIGNAL(activated(int)), this, SLOT(handleTextFill())     , Qt::UniqueConnection);
	connect(strokeColor , SIGNAL(activated(int)), this, SLOT(handleTextStroke())   , Qt::UniqueConnection);
	connect(fillShade   , SIGNAL(clicked())     , this, SLOT(handleTextShade())    , Qt::UniqueConnection);
	connect(strokeShade , SIGNAL(clicked())     , this, SLOT(handleTextShade())    , Qt::UniqueConnection);
	connect(backShade   , SIGNAL(clicked())     , this, SLOT(handleTextShade())    , Qt::UniqueConnection);
	connect(backColor   , SIGNAL(activated(int)), this, SLOT(handleTextBackground())     , Qt::UniqueConnection);
}

void PropertyWidget_TextColor::disconnectSignals()
{
	disconnect(fillColor   , SIGNAL(activated(int)), this, SLOT(handleTextFill()));
	disconnect(strokeColor , SIGNAL(activated(int)), this, SLOT(handleTextStroke()));
	disconnect(fillShade   , SIGNAL(clicked())     , this, SLOT(handleTextShade()));
	disconnect(strokeShade , SIGNAL(clicked())     , this, SLOT(handleTextShade()));
	disconnect(backShade   , SIGNAL(clicked())     , this, SLOT(handleTextShade()));
	disconnect(backColor   , SIGNAL(activated(int)), this, SLOT(handleTextBackground()));
}

void PropertyWidget_TextColor::configureWidgets(void)
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

void PropertyWidget_TextColor::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	setCurrentItem(currItem);
	updateGeometry();
}

void PropertyWidget_TextColor::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqColorsUpdate)
		updateColorList();
}

void PropertyWidget_TextColor::updateColorList()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (m_item)
		disconnectSignals();

	fillColor->setColors(m_doc->PageColors, true);
	strokeColor->setColors(m_doc->PageColors, false);
	backColor->setColors(m_doc->PageColors, true);
	fillColor->view()->setMinimumWidth(fillColor->view()->maximumViewportSize().width());// + 24);
	strokeColor->view()->setMinimumWidth(strokeColor->view()->maximumViewportSize().width());// + 24);
	backColor->view()->setMinimumWidth(backColor->view()->maximumViewportSize().width());// + 24);

	//if (m_item)
		setCurrentItem(m_item);
}

void PropertyWidget_TextColor::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	showTextColors(charStyle.strokeColor(), charStyle.fillColor(), charStyle.backColor(), charStyle.strokeShade(), charStyle.fillShade(), charStyle.backShade());
	showTextEffects(charStyle.effects());

}

void PropertyWidget_TextColor::updateStyle(const ParagraphStyle& newCurrent)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = newCurrent.charStyle();

	showTextColors(charStyle.strokeColor(), charStyle.fillColor(), charStyle.backColor(), charStyle.strokeShade(), charStyle.fillShade(), charStyle.backShade());
	showTextEffects(charStyle.effects());

}

void PropertyWidget_TextColor::showTextColors(QString p, QString b, QString bc, double shp, double shb, double sbc)
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	ColorList::Iterator it;
	int c = 0;
	fillShade->setValue(qRound(shb));
	strokeShade->setValue(qRound(shp));
	backShade->setValue(qRound(sbc));
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
}

void PropertyWidget_TextColor::showTextEffects(int s)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	strokeIcon->setEnabled(false);
	strokeColor->setEnabled(false);
	strokeShade->setEnabled(false);

	if ((s & 4) || (s & 256))
	{
		strokeIcon->setEnabled(true);
		strokeColor->setEnabled(true);
		strokeShade->setEnabled(true);
	}
}


void PropertyWidget_TextColor::handleTextFill()
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
	}
}

void PropertyWidget_TextColor::handleTextStroke()
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
	}
}

void PropertyWidget_TextColor::handleTextBackground()
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
	}
}

void PropertyWidget_TextColor::handleTextShade()
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
		}
	}
	else if (fillShade == sender())
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
		}
	}
}

void PropertyWidget_TextColor::handleTypeStyle(int s)
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


void PropertyWidget_TextColor::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}

void PropertyWidget_TextColor::languageChange()
{
	retranslateUi(this);
}
