/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#include "propertywidgettext_hyphenation.h"

#include "appmodes.h"
#include "pageitem_table.h"
#include "iconmanager.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"

PropertyWidgetText_Hyphenation::PropertyWidgetText_Hyphenation(QWidget* parent)
	: QWidget(parent)
{
	m_item = NULL;
	m_ScMW = NULL;

	setupUi(this);

	layout()->setAlignment( Qt::AlignTop );
	languageChange();
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetText_Hyphenation::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;
}

void PropertyWidgetText_Hyphenation::connectSignals()
{
	connect(smallestWordSpinBox,        SIGNAL(valueChanged(int)), this, SLOT(handleWordMin(int)));
	connect(maxConsecutiveCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(handleConsecutiveLines(int)));
	connect(hyphenCharLineEdit,         SIGNAL(textChanged(const QString&)), this, SLOT(handleHyphenChar(const QString&)));
}

void PropertyWidgetText_Hyphenation::disconnectSignals()
{
	disconnect(smallestWordSpinBox,        SIGNAL(valueChanged(int)), this, SLOT(handleWordMin(int)));
	disconnect(maxConsecutiveCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(handleConsecutiveLines(int)));
	disconnect(hyphenCharLineEdit,         SIGNAL(textChanged(const QString&)), this, SLOT(handleHyphenChar(const QString&)));
}

void PropertyWidgetText_Hyphenation::configureWidgets(void)
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

void PropertyWidgetText_Hyphenation::setDoc(ScribusDoc *d)
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

void PropertyWidgetText_Hyphenation::setCurrentItem(PageItem *item)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (item && m_doc.isNull())
		setDoc(item->doc());

	m_item = item;

	disconnectSignals();
	configureWidgets();

	if (m_item)
	{
		if (m_item->asTextFrame() || m_item->asPathText() || m_item->asTable())
		{
			ParagraphStyle parStyle =  m_item->itemText.defaultStyle();
			if (m_doc->appMode == modeEdit)
				m_item->currentTextProps(parStyle);
			else if (m_doc->appMode == modeEditTable)
				m_item->asTable()->activeCell().textFrame()->currentTextProps(parStyle);
			updateStyle(parStyle);
		}
		connectSignals();
	}
}


/*********************************************************************
*
* Hyphenation
*
**********************************************************************/

void PropertyWidgetText_Hyphenation::handleWordMin(int minWord)
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetHyphenWordMin(minWord, &tempSelection);
}

void PropertyWidgetText_Hyphenation::handleConsecutiveLines(int consecutiveLines)
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	Selection tempSelection(this, false);
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetHyphenConsecutiveLines(consecutiveLines, &tempSelection);
}

void PropertyWidgetText_Hyphenation::handleHyphenChar(const QString& hyphenText)
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	Selection tempSelection(this, false);
	uint hyphenChar;
	if (hyphenText.isEmpty())
		hyphenChar = 0;
	else
		hyphenChar = hyphenText.toUcs4()[0];
	qDebug() << hyphenChar;
	tempSelection.addItem(m_item, true);
	m_doc->itemSelection_SetHyphenChar(hyphenChar, &tempSelection);
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidgetText_Hyphenation::updateCharStyle(const CharStyle& charStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	smallestWordSpinBox->setValue(charStyle.hyphenWordMin());
	uint hyphenChar = charStyle.hyphenChar();
	QString hyphenText;
	if (hyphenChar)
		hyphenText = QString::fromUcs4(&hyphenChar, 1);
	hyphenCharLineEdit->setText(hyphenText);
}

void PropertyWidgetText_Hyphenation::updateStyle(const ParagraphStyle& paraStyle)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	const CharStyle& charStyle = paraStyle.charStyle();
	updateCharStyle(charStyle);

	maxConsecutiveCountSpinBox->setValue(paraStyle.hyphenConsecutiveLines());
}

void PropertyWidgetText_Hyphenation::handleSelectionChanged()
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	setCurrentItem(currItem);
	updateGeometry();
	repaint();
}

void PropertyWidgetText_Hyphenation::languageChange()
{
	retranslateUi(this);
}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetText_Hyphenation::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}

