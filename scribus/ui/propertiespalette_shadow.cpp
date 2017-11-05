/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertiespalette_shadow.h"

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include "commonstrings.h"
#include "sccolorengine.h"
#include "pageitem.h"
#include "propertiespalette_utils.h"

#include "scribuscore.h"
#include "scraction.h"

#include "selection.h"
#include "units.h"
#include "undomanager.h"
#include "util.h"
#include "util_math.h"

#include "scpopupmenu.h"



PropertiesPalette_Shadow::PropertiesPalette_Shadow( QWidget* parent) : QWidget(parent)
{


	m_ScMW = 0;
	m_doc = 0;
	m_item = 0;
	m_haveDoc  = false;
	m_haveItem = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	setupUi(this);

	hasSoftShadow->setChecked(false);

	softShadowXOffset->setNewUnit(0);
	softShadowXOffset->setDecimals(2);
	softShadowXOffset->setMinimum(-200.0);
	softShadowXOffset->setMaximum(200.00);
	softShadowXOffset->setValue(5.0);

	softShadowYOffset->setNewUnit(0);
	softShadowYOffset->setDecimals(2);
	softShadowYOffset->setMinimum(-200.0);
	softShadowYOffset->setMaximum(200.00);
	softShadowYOffset->setValue(5.0);

	softShadowBlurRadius->setNewUnit(0);
	softShadowBlurRadius->setDecimals(1);
	softShadowBlurRadius->setMinimum(-20.0);
	softShadowBlurRadius->setMaximum(20.00);
	softShadowBlurRadius->setValue(2.0);

	softShadowColor->setCurrentText( tr( "Black"));

	softShadowShade->setNewUnit(7);
	softShadowShade->setDecimals(0);
	softShadowShade->setMinimum(0);
	softShadowShade->setMaximum(100);
	softShadowShade->setValue(100);

	softShadowOpacity->setNewUnit(7);
	softShadowOpacity->setDecimals(0);
	softShadowOpacity->setMinimum(0);
	softShadowOpacity->setMaximum(100);
	softShadowOpacity->setValue(100.0);

	QStringList modes;
	softShadowBlendMode->addItems(modes);
	softShadowBlendMode->setItemText(0, tr( "Normal"));

	softShadowErase->setChecked(false);

	softShadowObjTrans->setChecked(false);

	ScPopupMenu * shadowColorMenu = new ScPopupMenu(softShadowColor);

	shadowColor = new ScColorFillsBox();
	shadowColor->setMenu(shadowColorMenu);
	verticalLayoutColor->insertWidget(0,shadowColor);

	languageChange();
	m_haveItem = false;

	connectSignals();

	m_haveItem = false;
}

void PropertiesPalette_Shadow::connectSignals(){

	connect(softShadowBlendMode, SIGNAL(currentIndexChanged(int)), this, SLOT(handleNewValues()));
	connect(softShadowBlurRadius, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	connect(softShadowColor, SIGNAL(currentIndexChanged(int)), this, SLOT(handleNewValues()));
	connect(softShadowOpacity, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	connect(softShadowShade, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	connect(softShadowXOffset, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	connect(softShadowYOffset, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	connect(hasSoftShadow, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
	connect(softShadowErase, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
	connect(softShadowObjTrans, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
}

void PropertiesPalette_Shadow::disconnectSignals(){

	disconnect(softShadowBlendMode, SIGNAL(currentIndexChanged(int)), this, SLOT(handleNewValues()));
	disconnect(softShadowBlurRadius, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	disconnect(softShadowColor, SIGNAL(currentIndexChanged(int)), this, SLOT(handleNewValues()));
	disconnect(softShadowOpacity, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	disconnect(softShadowShade, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	disconnect(softShadowXOffset, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	disconnect(softShadowYOffset, SIGNAL(valueChanged(double)), this, SLOT(handleNewValues()));
	disconnect(hasSoftShadow, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
	disconnect(softShadowErase, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
	disconnect(softShadowObjTrans, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
}


void PropertiesPalette_Shadow::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;
	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertiesPalette_Shadow::setDoc(ScribusDoc *d)
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
	int precision = unitGetPrecisionFromIndex(m_unitIndex);
	double maxXYWHVal =  200 * m_unitRatio;
	double minXYVal   = -200 * m_unitRatio;

	m_haveDoc = true;
	m_haveItem = false;

	softShadowXOffset->setNewUnit(m_unitIndex);
	softShadowXOffset->setValue(precision);
	softShadowXOffset->setDecimals(minXYVal);
	softShadowXOffset->setMinimum(minXYVal);
	softShadowXOffset->setMaximum(maxXYWHVal);

	softShadowYOffset->setNewUnit(m_unitIndex);
	softShadowYOffset->setValue(precision);
	softShadowYOffset->setDecimals(minXYVal);
	softShadowYOffset->setMinimum(minXYVal);
	softShadowYOffset->setMaximum(maxXYWHVal);

	softShadowBlurRadius->setNewUnit(m_unitIndex);
	softShadowBlurRadius->setDecimals(1);
	softShadowBlurRadius->setMinimum(0.0);
	softShadowBlurRadius->setMaximum(20.0);
	softShadowBlurRadius->setValue(5);

	softShadowShade->setValue(100);
	softShadowShade->setDecimals(0);
	softShadowShade->setMinimum(0);
	softShadowShade->setMaximum(100);

	softShadowOpacity->setValue(100);
	softShadowOpacity->setDecimals(0);
	softShadowOpacity->setMinimum(0);
	softShadowOpacity->setMaximum(100);

	updateColorList();

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}



void PropertiesPalette_Shadow::unsetDoc()
{
	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}
	m_haveDoc  = false;
	m_haveItem = false;
	m_doc   = NULL;
	m_item  = NULL;
	setEnabled(false);
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertiesPalette_Shadow::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	handleSelectionChanged();
}



void PropertiesPalette_Shadow::setCurrentItem(PageItem *i)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if (!m_doc)
		setDoc(i->doc());
	m_haveItem = false;
	m_item = i;

	hasSoftShadow->setChecked(i->hasSoftShadow());
	softShadowXOffset->setValue(i->softShadowXOffset() * m_unitRatio);
	softShadowYOffset->setValue(i->softShadowYOffset() * m_unitRatio);
	softShadowBlurRadius->setValue(i->softShadowBlurRadius() * m_unitRatio);
//	softShadowShade->setValue(i->softShadowShade());
	softShadowOpacity->setValue(qRound(100 - (i->softShadowOpacity() * 100)));
	softShadowBlendMode->setCurrentIndex(i->softShadowBlendMode());
	softShadowErase->setChecked(i->softShadowErasedByObject());
	softShadowObjTrans->setChecked(i->softShadowHasObjectTransparency());

	// Update Color List + Shade
	showColor(i->softShadowColor(), i->softShadowShade());


	// Update Color Box
	handleFillColorBox();

	m_haveItem = true;
	updateSpinBoxConstants(); // unused
}

/*********************************************************************
*
* Color
*
**********************************************************************/

void PropertiesPalette_Shadow::handleFillColorBox(){

	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	QString color = softShadowColor->currentColor();

	if (color == CommonStrings::tr_NoneColor)
		color = CommonStrings::None;

	if(color != CommonStrings::None){
		const ScColor& col = m_doc->PageColors[color];
		QColor tmp = ScColorEngine::getShadeColorProof(col, m_doc, softShadowShade->value());
		shadowColor->setColor(tmp);
	} else shadowColor->resetColor();
}


void PropertiesPalette_Shadow::showColor(QString b, double sb)
{
	if (!m_doc || !m_item || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	ColorList::Iterator it;
	int c = 0;

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
	softShadowColor->setCurrentIndex(c);
	softShadowShade->setValue(sb);
}



/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertiesPalette_Shadow::updateColorList()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	softShadowColor->setColors(m_doc->PageColors);
}

void PropertiesPalette_Shadow::handleNewValues()
{
	if (m_haveItem)
	{

		double x = softShadowXOffset->value() / m_unitRatio;
		double y = softShadowYOffset->value() / m_unitRatio;
		double r = softShadowBlurRadius->value() / m_unitRatio;
		int b = softShadowBlendMode->currentIndex();
		double o = (100 - softShadowOpacity->value()) / 100.0;
		int s = softShadowShade->value();

		QString color = softShadowColor->currentColor();
		if (color == CommonStrings::tr_NoneColor)
			color = CommonStrings::None;

		if (m_haveDoc)
		{
			m_doc->itemSelection_SetSoftShadow(hasSoftShadow->isChecked(), color, x, y, r, s, o, b, softShadowErase->isChecked(), softShadowObjTrans->isChecked());

			// draw color box
			handleFillColorBox();

		}
	}
}

void PropertiesPalette_Shadow::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (currItem)
		setCurrentItem(currItem);
	updateGeometry();
}

void PropertiesPalette_Shadow::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqColorsUpdate)
		updateColorList();
}

PageItem* PropertiesPalette_Shadow::currentItemFromSelection()
{
	PageItem *currentItem = NULL;
	if (m_doc)
	{
		if (m_doc->m_Selection->count() > 1)
			currentItem = m_doc->m_Selection->itemAt(0);
		else if (m_doc->m_Selection->count() == 1)
			currentItem = m_doc->m_Selection->itemAt(0);
	}
	return currentItem;
}

void PropertiesPalette_Shadow::languageChange()
{
	disconnectSignals();

	labelShadowXOffset->setText(tr( "X-Offset:"));
	labelShadowYOffset->setText(tr( "Y-Offset:"));
	labelShadowBlurRadius->setText(tr( "Blur:"));
	labelShadowColor->setText(tr( "Color:"));
	labelShadowShade->setText(tr( "Shade:"));
	labelShadowOpacity->setText(tr( "Opacity:"));
	hasSoftShadow->setText(tr( "Has Drop Shadow"));
	softShadowErase->setText(tr( "Content covers\nDrop Shadow"));
	softShadowObjTrans->setText(tr( "Inherit Object\nTransparency"));

	softShadowBlendMode->clear();

	QStringList modes;
	modes.append( tr("Normal"));
	modes.append( tr("Darken"));
	modes.append( tr("Lighten"));
	modes.append( tr("Multiply"));
	modes.append( tr("Screen"));
	modes.append( tr("Overlay"));
	modes.append( tr("Hard Light"));
	modes.append( tr("Soft Light"));
	modes.append( tr("Difference"));
	modes.append( tr("Exclusion"));
	modes.append( tr("Color Dodge"));
	modes.append( tr("Color Burn"));
	modes.append( tr("Hue"));
	modes.append( tr("Saturation"));
	modes.append( tr("Color"));
	modes.append( tr("Luminosity"));

	softShadowBlendMode->addItems(modes);
	softShadowBlendMode->setCurrentText( tr("Normal"));

	connectSignals();
}

void PropertiesPalette_Shadow::unitChange()
{
	if (!m_haveDoc)
		return;
	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();

	bool sigBlocked1 = softShadowXOffset->blockSignals(true);
	bool sigBlocked2 = softShadowYOffset->blockSignals(true);
	bool sigBlocked3 = softShadowBlurRadius->blockSignals(true);
//	bool sigBlocked4 = this->model()->blockSignals(true);

	softShadowYOffset->setNewUnit(m_unitIndex);
	softShadowXOffset->setNewUnit(m_unitIndex);
	softShadowBlurRadius->setNewUnit(m_unitIndex);

	softShadowXOffset->blockSignals(sigBlocked1);
	softShadowYOffset->blockSignals(sigBlocked2);
	softShadowBlurRadius->blockSignals(sigBlocked3);
//	this->model()->blockSignals(sigBlocked4);
}

void PropertiesPalette_Shadow::updateSpinBoxConstants()
{
	if (!m_haveDoc)
		return;
	if(m_doc->m_Selection->count()==0)
		return;
}


/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertiesPalette_Shadow::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
	}
	else
		QWidget::changeEvent(e);
}

