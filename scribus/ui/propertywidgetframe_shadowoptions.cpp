/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgetframe_shadowoptions.h"

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


PropertyWidgetFrame_ShadowOptions::PropertyWidgetFrame_ShadowOptions( QWidget* parent) : QWidget(parent)
{


	m_ScMW = 0;
	m_doc = 0;
	m_item = 0;
	m_haveDoc  = false;
	m_haveItem = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	setupUi(this);


	// Blend
	QStringList modes;
	softShadowBlendMode->addItems(modes);
	softShadowBlendMode->setItemText(0, tr( "Normal"));

	// checkbox
	softShadowErase->setChecked(false);
	softShadowObjTrans->setChecked(false);

	languageChange();
	m_haveItem = false;

	connectSignals();

	m_haveItem = false;
}

void PropertyWidgetFrame_ShadowOptions::connectSignals(){

	connect(softShadowBlendMode, SIGNAL(currentIndexChanged(int)), this, SLOT(handleNewValues()));
	connect(softShadowErase, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
	connect(softShadowObjTrans, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
}

void PropertyWidgetFrame_ShadowOptions::disconnectSignals(){

	disconnect(softShadowBlendMode, SIGNAL(currentIndexChanged(int)), this, SLOT(handleNewValues()));
	disconnect(softShadowErase, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
	disconnect(softShadowObjTrans, SIGNAL(toggled(bool)), this, SLOT(handleNewValues()));
}


void PropertyWidgetFrame_ShadowOptions::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;
	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidgetFrame_ShadowOptions::setDoc(ScribusDoc *d)
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

	m_haveDoc = true;
	m_haveItem = false;

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}



void PropertyWidgetFrame_ShadowOptions::unsetDoc()
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

void PropertyWidgetFrame_ShadowOptions::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	handleSelectionChanged();
}



void PropertyWidgetFrame_ShadowOptions::setCurrentItem(PageItem *i)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if (!m_doc)
		setDoc(i->doc());
	m_haveItem = false;
	m_item = i;

	softShadowBlendMode->setCurrentIndex(i->softShadowBlendMode()); //
	softShadowErase->setChecked(i->softShadowErasedByObject()); //
	softShadowObjTrans->setChecked(i->softShadowHasObjectTransparency()); //

	m_haveItem = true;
}


/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidgetFrame_ShadowOptions::handleNewValues()
{
	if (m_haveItem)
	{

		int shadowB = softShadowBlendMode->currentIndex();
		bool shadowE = softShadowErase->isChecked();
		bool shadowT = softShadowObjTrans->isChecked();

		emit sendShadowOptions(shadowB, shadowE, shadowT);
	}
}

void PropertyWidgetFrame_ShadowOptions::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (currItem)
		setCurrentItem(currItem);
	updateGeometry();
}


PageItem* PropertyWidgetFrame_ShadowOptions::currentItemFromSelection()
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

void PropertyWidgetFrame_ShadowOptions::languageChange()
{
	disconnectSignals();

	softShadowErase->setText(tr( "Content covers\nDrop Shadow")); //
	softShadowObjTrans->setText(tr( "Inherit Object\nTransparency")); //

	softShadowBlendMode->clear(); //

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

	softShadowBlendMode->addItems(modes); //
	softShadowBlendMode->setCurrentText( tr("Normal")); //

	connectSignals();
}

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetFrame_ShadowOptions::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
	}
	else
		QWidget::changeEvent(e);
}

