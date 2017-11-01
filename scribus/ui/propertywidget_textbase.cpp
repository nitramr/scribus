/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidget_textbase.h"

#include "appmodes.h"
#include "iconmanager.h"
#include "pageitem_table.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
#include "units.h"
#include "langmgr.h"


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

//	fontSize->setPrefix( "" );
//	fontSizeLabel->setPixmap(IconManager::instance()->loadPixmap("zeichen.png"));
//	lineSpacingLabel->setPixmap(IconManager::instance()->loadPixmap("linespacing2.png"));

//	paraStyleLabel->setBuddy(paraStyleCombo);
//	paraStyleClear->setIcon(IconManager::instance()->loadPixmap("16/edit-clear.png"));
//	charStyleLabel->setBuddy(charStyleCombo);
//	charStyleClear->setIcon(IconManager::instance()->loadPixmap("16/edit-clear.png"));


//	languageChange();

//	connect(lineSpacing   , SIGNAL(valueChanged(double)), this, SLOT(handleLineSpacing()));
//	connect(fonts         , SIGNAL(fontSelected(QString )), this, SLOT(handleTextFont(QString)));
//	connect(fontSize      , SIGNAL(valueChanged(double)), this, SLOT(handleFontSize()));
//	connect(textAlignment , SIGNAL(State(int))   , this, SLOT(handleAlignment(int)));
//	connect(textDirection , SIGNAL(State(int))   , this, SLOT(handleDirection(int)));
//	connect(charStyleClear, SIGNAL(clicked()), this, SLOT(doClearCStyle()));
//	connect(paraStyleClear, SIGNAL(clicked()), this, SLOT(doClearPStyle()));

//	connect(lineSpacingModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(handleLineSpacingMode(int)));
//	connect(langCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLang(int)));

//	m_haveItem = false;
//	setEnabled(false);
}

void PropertyWidget_TextBase::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

//	connect(m_ScMW, SIGNAL(UpdateRequest(int))     , this  , SLOT(handleUpdateRequest(int)));

//	connect(paraStyleCombo, SIGNAL(newStyle(const QString&)), m_ScMW, SLOT(setNewParStyle(const QString&)), Qt::UniqueConnection);
//	connect(charStyleCombo, SIGNAL(newStyle(const QString&)), m_ScMW, SLOT(setNewCharStyle(const QString&)), Qt::UniqueConnection);
}

void PropertyWidget_TextBase::setDoc(ScribusDoc *d)
{

	//PropertyWidgetBase::setDoc(d);

//	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
//		return;

//	if (m_doc)
//	{
//		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
//		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
//	}

//	m_doc  = d;
//	m_item = NULL;

//	m_unitRatio   = m_doc->unitRatio();
//	m_unitIndex   = m_doc->unitIndex();

//	m_haveDoc  = true;
//	m_haveItem = false;

//	fontSize->setValues( 0.5, 2048, 2, 1);
//	lineSpacing->setValues( 1, 2048, 2, 1);

//	fonts->RebuildList(m_doc);
//	paraStyleCombo->setDoc(m_doc);
//	charStyleCombo->setDoc(m_doc);

//	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
//	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidget_TextBase::unsetDoc()
{
//	if (m_doc)
//	{
//		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
//		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
//	}

//	m_haveDoc  = false;
//	m_haveItem = false;
//	m_doc      = NULL;
//	m_item     = NULL;

//	paraStyleCombo->setDoc(0);
//	charStyleCombo->setDoc(0);


//	m_haveItem = false;

//	setEnabled(false);
}

void PropertyWidget_TextBase::setCurrentItem(PageItem *i)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	//CB We shouldn't really need to process this if our item is the same one
//	//maybe we do if the item has been changed by scripter.. but that should probably
//	//set some status if so.
//	//FIXME: This won't work until when a canvas deselect happens, m_item must be NULL.
//	//if (m_item == i)
//	//	return;

//	if (!m_doc)
//		setDoc(i->doc());

//	m_haveItem = false;
//	m_item = i;
}


void PropertyWidget_TextBase::handleLineSpacingMode(int id)
{
//	if ((m_haveDoc) && (m_haveItem))
//	{
//		Selection tempSelection(this, false);
//		tempSelection.addItem(m_item, true);
//		m_doc->itemSelection_SetLineSpacingMode(id, &tempSelection);
//	//	updateStyle(((m_doc->appMode == modeEdit) || (m_doc->appMode == modeEditTable)) ? m_item->currentStyle() : m_item->itemText.defaultStyle());
//		m_doc->regionsChanged()->update(QRect());
//	}
}

void PropertyWidget_TextBase::changeLang(int id)
{
//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	QStringList languageList;
//	LanguageManager::instance()->fillInstalledStringList(&languageList);
//	QString abrv = LanguageManager::instance()->getAbbrevFromLang(languageList.value(id),false);
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_SetLanguage(abrv, &tempSelection);
}

void PropertyWidget_TextBase::showLineSpacing(double r)
{
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

void PropertyWidget_TextBase::showFontFace(const QString& newFont)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool tmp = m_haveItem;
//	m_haveItem = false;
//	if (m_item != NULL)
//		fonts->RebuildList(m_doc, m_item->isAnnotation());
//	fonts->setCurrentFont(newFont);
//	m_haveItem = tmp;
}

void PropertyWidget_TextBase::showFontSize(double s)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	fontSize->showValue(s / 10.0);
}

void PropertyWidget_TextBase::showLanguage(QString w)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	QStringList lang;
//	LanguageManager::instance()->fillInstalledStringList(&lang);
//	QString langName = LanguageManager::instance()->getLangFromAbbrev(w, true);

//	bool sigBlocked  = langCombo->blockSignals(true);
//	langCombo->setCurrentIndex(lang.indexOf(langName));
//	langCombo->blockSignals(sigBlocked);
}


void PropertyWidget_TextBase::setupLineSpacingSpinbox(int mode, double value)
{
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

void PropertyWidget_TextBase::updateCharStyle(const CharStyle& charStyle)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;

//	showFontFace(charStyle.font().scName());
//	showFontSize(charStyle.fontSize());
//	showLanguage(charStyle.language());
}

void PropertyWidget_TextBase::updateStyle(const ParagraphStyle& newCurrent)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;

//	const CharStyle& charStyle = newCurrent.charStyle();

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

//	m_haveItem = tmp;
}

void PropertyWidget_TextBase::updateCharStyles()
{
//	charStyleCombo->updateFormatList();
}

void PropertyWidget_TextBase::updateParagraphStyles()
{
//	paraStyleCombo->updateFormatList();
//	charStyleCombo->updateFormatList();
}

void PropertyWidget_TextBase::updateTextStyles()
{
//	paraStyleCombo->updateFormatList();
//	charStyleCombo->updateFormatList();
}


void PropertyWidget_TextBase::showAlignment(int e)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool tmp = m_haveItem;
//	m_haveItem = false;
//	textAlignment->setEnabled(true);
//	textAlignment->setStyle(e, textDirection->getStyle());
//	m_haveItem = tmp;
}

void PropertyWidget_TextBase::showDirection(int e)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool tmp = m_haveItem;
//	m_haveItem = false;
//	textDirection->setEnabled(true);
//	textDirection->setStyle(e);
//	m_haveItem = tmp;
}

void PropertyWidget_TextBase::showCharStyle(const QString& name)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool blocked = charStyleCombo->blockSignals(true);
//	charStyleCombo->setFormat(name);
//	charStyleCombo->blockSignals(blocked);
}

void PropertyWidget_TextBase::showParStyle(const QString& name)
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	bool blocked = paraStyleCombo->blockSignals(true);
//	paraStyleCombo->setFormat(name);
//	paraStyleCombo->blockSignals(blocked);
}

void PropertyWidget_TextBase::handleLineSpacing()
{
//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_SetLineSpacing(lineSpacing->value(), &tempSelection);
}

void PropertyWidget_TextBase::handleFontSize()
{
//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_SetFontSize(qRound(fontSize->value()*10.0), &tempSelection);
}

void PropertyWidget_TextBase::handleAlignment(int a)
{
//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_SetAlignment(a, &tempSelection);
////	if (m_item->isPathText())
////		pathTextWidgets->handleSelectionChanged();
}

void PropertyWidget_TextBase::handleDirection(int d)
{
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

void PropertyWidget_TextBase::handleTextFont(QString c)
{
//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	m_ScMW->SetNewFont(c);
}

void PropertyWidget_TextBase::doClearCStyle()
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning() || !m_haveDoc || !m_haveItem)
//		return;
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_EraseCharStyle(&tempSelection);
}


void PropertyWidget_TextBase::doClearPStyle()
{
//	if (!m_ScMW || m_ScMW->scriptIsRunning() || !m_haveDoc || !m_haveItem)
//		return;
//	Selection tempSelection(this, false);
//	tempSelection.addItem(m_item, true);
//	m_doc->itemSelection_ClearBulNumStrings(&tempSelection);
//	m_doc->itemSelection_EraseParagraphStyle(&tempSelection);
//	CharStyle emptyCStyle;
//	m_doc->itemSelection_SetCharStyle(emptyCStyle, &tempSelection);
}


void PropertyWidget_TextBase::changeEvent(QEvent *e)
{
//	if (e->type() == QEvent::LanguageChange)
//	{
//		languageChange();
//		return;
//	}
//	QWidget::changeEvent(e);
}

void PropertyWidget_TextBase::languageChange()
{
//	retranslateUi(this);

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

//	textAlignment->languageChange();
//	textDirection->languageChange();
}

