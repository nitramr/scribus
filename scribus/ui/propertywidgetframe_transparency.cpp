/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/***************************************************************************
                          transparencypalette.cpp  -  description
                             -------------------
    begin                : Tue Nov 17 2009
    copyright            : (C) 2009 by Franz Schmid
    email                : Franz.Schmid@altmuehlnet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "propertywidgetframe_transparency.h"
#include "appmodes.h"
#include "sccolorengine.h"
#include "scpainter.h"
#include "scpattern.h"
#include "scribus.h"
#include "scribusview.h"
#include "iconmanager.h"
#include "util.h"
#include "util_math.h"

PropertyWidgetFrame_Transparency::PropertyWidgetFrame_Transparency(QWidget* parent) : QWidget(parent)
{
	m_ScMW = 0;
	m_item = 0;
	m_haveDoc   = false;
	m_haveItem  = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	patternList = NULL;
	TGradDia = NULL;
	TGradDia = new GradientVectorDialog(this->parentWidget());
	TGradDia->hide();
	setupUi(this);

	editLineSelector->setIcon(IconManager::instance()->loadIcon("16/color-stroke.png"));
	editFillSelector->setIcon(IconManager::instance()->loadIcon("16/color-fill.png"));
	editFillSelector->setChecked(true);
	strokeOpacity->setDecimals(0);
	fillOpacity->setDecimals(0);
	editFillSelectorButton();

	connect(editLineSelector, SIGNAL(clicked()), this, SLOT(editLineSelectorButton()));
	connect(editFillSelector, SIGNAL(clicked()), this, SLOT(editFillSelectorButton()));
	connect(strokeOpacity, SIGNAL(valueChanged(double)), this, SLOT(slotTransS(double)));
	connect(fillOpacity, SIGNAL(valueChanged(double)), this, SLOT(slotTransF(double)));
	connect(blendModeFill, SIGNAL(activated(int)), this, SIGNAL(NewBlend(int)));
	connect(blendModeStroke, SIGNAL(activated(int)), this, SIGNAL(NewBlendS(int)));
	connect(namedGradient, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
	connect(gradEdit, SIGNAL(gradientChanged()), this, SIGNAL(gradientChanged()));
	connect(gradEditButton, SIGNAL(clicked()), this, SLOT(editGradientVector()));
	connect(TGradDia, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)), this, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)));
	connect(TGradDia, SIGNAL(paletteShown(bool)), this, SLOT(setActiveGradDia(bool)));
	connect(gradientType, SIGNAL(activated(int)), this, SLOT(slotGradType(int)));
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotGrad(int)));
	connect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	connect(editPatternProps, SIGNAL(clicked()), this, SLOT(changePatternProps()));
	connect(transpCalcGradient, SIGNAL(clicked()), this, SLOT(switchGradientMode()));
	connect(transpCalcPattern, SIGNAL(clicked()), this, SLOT(switchPatternMode()));
	connect(usePatternInverted, SIGNAL(clicked()), this, SLOT(switchPatternMode()));

	connect(this , SIGNAL(editGradient())          , this, SLOT(handleGradientEdit()));
	connect(this , SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)), this, SLOT(handleSpecialGradient(double, double, double, double, double, double, double, double )));
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetFrame_Transparency::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;
	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
	connect(this, SIGNAL(gradientChanged()), this, SLOT(handleGradientChanged()));
}

void PropertyWidgetFrame_Transparency::connectSignals()
{
	connect(gradEdit, SIGNAL(gradientChanged()), this, SIGNAL(gradientChanged()));
	connect(namedGradient, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotGrad(int)));
	connect(gradientType, SIGNAL(activated(int)), this, SLOT(slotGradType(int)));
	connect(transpCalcGradient, SIGNAL(clicked()), this, SLOT(switchGradientMode()));
	connect(transpCalcPattern, SIGNAL(clicked()), this, SLOT(switchPatternMode()));
	connect(usePatternInverted, SIGNAL(clicked()), this, SLOT(switchPatternMode()));
}

void PropertyWidgetFrame_Transparency::disconnectSignals()
{
	disconnect(gradEdit, SIGNAL(gradientChanged()), this, SIGNAL(gradientChanged()));
	disconnect(namedGradient, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
	disconnect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotGrad(int)));
	disconnect(gradientType, SIGNAL(activated(int)), this, SLOT(slotGradType(int)));
	disconnect(transpCalcGradient, SIGNAL(clicked()), this, SLOT(switchGradientMode()));
	disconnect(transpCalcPattern, SIGNAL(clicked()), this, SLOT(switchPatternMode()));
	disconnect(usePatternInverted, SIGNAL(clicked()), this, SLOT(switchPatternMode()));
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidgetFrame_Transparency::setDoc(ScribusDoc* d)
{

	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		disconnect(this, SIGNAL(NewTrans(double)), 0, 0);
		disconnect(this, SIGNAL(NewTransS(double)), 0, 0);
		disconnect(this, SIGNAL(NewGradient(int)), 0, 0);
		disconnect(this, SIGNAL(NewBlend(int)), 0, 0);
		disconnect(this, SIGNAL(NewBlendS(int)), 0, 0);
		disconnect(this, SIGNAL(NewPattern(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), 0, 0);

		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
		disconnect(m_doc->scMW(), SIGNAL(UpdateRequest(int)), this, 0);
	}

	m_doc  = d;
	m_item = NULL;

	if(!m_doc)
		return;

	m_unitRatio   = m_doc->unitRatio();
	m_unitIndex   = m_doc->unitIndex();

		gradEdit->setColors(d->PageColors);

		updateColorList();

		m_haveDoc = true;
		m_haveItem = false;

		connect(this, SIGNAL(NewTrans(double)), m_doc, SLOT(itemSelection_SetItemFillTransparency(double)));
		connect(this, SIGNAL(NewTransS(double)), m_doc, SLOT(itemSelection_SetItemLineTransparency(double)));
		connect(this, SIGNAL(NewBlend(int)), m_doc, SLOT(itemSelection_SetItemFillBlend(int)));
		connect(this, SIGNAL(NewBlendS(int)), m_doc, SLOT(itemSelection_SetItemLineBlend(int)));
		connect(this, SIGNAL(NewGradient(int)), m_doc, SLOT(itemSelection_SetItemGradMask(int)));
		connect(this, SIGNAL(NewPattern(QString)), m_doc, SLOT(itemSelection_SetItemPatternMask(QString)));
		connect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), m_doc, SLOT(itemSelection_SetItemPatternMaskProps(double, double, double, double, double, double, double, bool, bool)));

		// Group
		connect(this, SIGNAL(NewTrans(double))   , this, SLOT(handleGroupTransparency(double)));
		connect(this, SIGNAL(NewBlend(int))      , this, SLOT(handleGroupBlending(int)));
		connect(this, SIGNAL(NewGradient(int))   , this, SLOT(handleGroupGradMask(int)));
		connect(this, SIGNAL(NewPattern(QString)), this, SLOT(handleGroupPatternMask(QString)));
		connect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), this, SLOT(handleGroupPatternMaskProps(double, double, double, double, double, double, double, bool, bool)));


		connect(m_doc->scMW(), SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
		connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));

		// Handle properties update when switching document
	handleSelectionChanged();
}

void PropertyWidgetFrame_Transparency::unsetDoc()
{

	if (m_doc)
	{
		disconnect(this, SIGNAL(NewTrans(double)), 0, 0);
		disconnect(this, SIGNAL(NewTransS(double)), 0, 0);
		disconnect(this, SIGNAL(NewGradient(int)), 0, 0);
		disconnect(this, SIGNAL(NewBlend(int)), 0, 0);
		disconnect(this, SIGNAL(NewBlendS(int)), 0, 0);
		disconnect(this, SIGNAL(NewPattern(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), 0, 0);

		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_haveDoc = false;
	m_haveItem = false;
	m_doc=NULL;
	m_item = NULL;

	setEnabled(false);

}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidgetFrame_Transparency::setCurrentItem(PageItem* i)
{

	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (!m_doc)
		setDoc(i->doc());

	m_haveItem = false;
	m_item = i;

	disconnectSignals();

	if (!m_item)
		return;



	setActTrans(m_item->fillTransparency(), m_item->lineTransparency());
	setActBlend(m_item->fillBlendmode(), m_item->lineBlendmode());
	gradEdit->setGradient(m_item->mask_gradient);

	if (!m_item->gradientMask().isEmpty())
	{
		setCurrentComboItem(namedGradient, m_item->gradientMask());
		gradEdit->setGradientEditable(false);
	}
	else
	{
		namedGradient->setCurrentIndex(0);
		gradEdit->setGradientEditable(true);
	}

	if (m_item->maskType() == 0)
		tabWidget->setCurrentIndex(0);
	else if ((m_item->maskType() == 1) || (m_item->maskType() == 2) || (m_item->maskType() == 4) || (m_item->maskType() == 5))
		tabWidget->setCurrentIndex(1);
	else
		tabWidget->setCurrentIndex(2);
	if (patternList->count() == 0)
		tabWidget->setTabEnabled(2, false);
	else
		tabWidget->setTabEnabled(2, true);

	transpCalcGradient->setChecked(false);
	transpCalcPattern->setChecked(false);
	usePatternInverted->setChecked(false);

	if ((m_item->maskType() == 4) || (m_item->maskType() == 5))
		transpCalcGradient->setChecked(true);
	if ((m_item->maskType() == 6) || (m_item->maskType() == 7))
		transpCalcPattern->setChecked(true);
	if ((m_item->maskType() == 7) || (m_item->maskType() == 8))
		usePatternInverted->setChecked(true);
	if ((m_item->maskType() == 1) || (m_item->maskType() == 4))
		gradientType->setCurrentIndex(0);
	else if ((m_item->maskType() == 2) || (m_item->maskType() == 5))
		gradientType->setCurrentIndex(1);

	if(TGradDia && gradEditButton->isChecked())
		TGradDia->setValues(m_item->GrMaskStartX, m_item->GrMaskStartY, m_item->GrMaskEndX, m_item->GrMaskEndY, m_item->GrMaskFocalX, m_item->GrMaskFocalY, m_item->GrMaskScale, m_item->GrMaskSkew, 0, 0);

	double patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY;
	bool mirrorX, mirrorY;
	m_item->maskTransform(patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY);
	m_item->maskFlip(mirrorX, mirrorY);
	setActPattern(m_item->patternMask(), patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, mirrorX, mirrorY);

	m_haveItem = true;

	connectSignals();
}


void PropertyWidgetFrame_Transparency::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;

	handleSelectionChanged();
}


/*********************************************************************
*
* Update helper
*
**********************************************************************/

void PropertyWidgetFrame_Transparency::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqColorsUpdate)
		updateColorList();
}

void  PropertyWidgetFrame_Transparency::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{

	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem   = (itemType != -1);

		switch (itemType)
		{
		case -1:
			m_haveItem = false;
			break;
		}
	}

	update();

	if (currItem)
	{
		setCurrentItem(currItem);
	}
}

void PropertyWidgetFrame_Transparency::updateColorList()
{
	if (!m_doc)
		return;

	if (m_item)
		disconnectSignals();

	this->setColors(m_doc->PageColors);
	this->setPatterns(&m_doc->docPatterns);
	this->setGradients(&m_doc->docGradients);

	if (m_item)
		setCurrentItem(m_item);
}

void PropertyWidgetFrame_Transparency::hideSelectionButtons()
{
	editLineSelector->hide();
	editLineSelector->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
	editFillSelector->hide();
	editFillSelector->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
}



void PropertyWidgetFrame_Transparency::updateCList()
{
	gradEdit->setColors(colorList);
}

void PropertyWidgetFrame_Transparency::updateGradientList()
{
	disconnect(namedGradient, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
	namedGradient->clear();
	namedGradient->setIconSize(QSize(48, 12));
	namedGradient->addItem( tr("Custom"));
	QStringList patK = gradientList->keys();
	qSort(patK);
	for (int a = 0; a < patK.count(); a++)
	{
		VGradient gr = gradientList->value(patK[a]);
		QImage pixm(48, 12, QImage::Format_ARGB32_Premultiplied);
		QPainter pb;
		QBrush b(QColor(205,205,205), IconManager::instance()->loadPixmap("testfill.png"));
		pb.begin(&pixm);
		pb.fillRect(0, 0, 48, 12, b);
		pb.end();
		ScPainter *p = new ScPainter(&pixm, 48, 12);
		p->setPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
		p->setFillMode(2);
		p->fill_gradient = gr;
		p->setGradient(VGradient::linear, FPoint(0,6), FPoint(48, 6), FPoint(0,0), 1, 0);
		p->drawRect(0, 0, 48, 12);
		p->end();
		delete p;
		QPixmap pm;
		pm = QPixmap::fromImage(pixm);
		namedGradient->addItem(pm, patK[a]);
	}
	connect(namedGradient, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
}

void PropertyWidgetFrame_Transparency::unitChange()
{
	if (!m_haveDoc || !m_doc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();

	if (TGradDia)
		TGradDia->unitChange(m_unitIndex);

	m_haveItem = tmp;
}


/*********************************************************************
*
* Feature Gradient
*
**********************************************************************/

void PropertyWidgetFrame_Transparency::setGradients(QHash<QString, VGradient> *docGradients)
{
	gradientList = docGradients;
	updateGradientList();
}

void PropertyWidgetFrame_Transparency::setColors(ColorList newColorList)
{
	colorList.clear();
	colorList = newColorList;
	updateCList();
}

void PropertyWidgetFrame_Transparency::slotGrad(int number)
{
	if(!m_item)
		return;

	if (number == 1)
	{
		disconnect(namedGradient, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
		if (!m_item->gradientMask().isEmpty())
		{
			setCurrentComboItem(namedGradient, m_item->gradientMask());
			gradEdit->setGradient(gradientList->value(m_item->gradientMask()));
			gradEdit->setGradientEditable(false);
		}
		else
		{
			namedGradient->setCurrentIndex(0);
			gradEdit->setGradient(m_item->mask_gradient);
			gradEdit->setGradientEditable(true);
		}
		if (gradientType->currentIndex() == 0)
		{
			if (transpCalcGradient->isChecked())
				emit NewGradient(4);
			else
				emit NewGradient(1);
		}
		else
		{
			if (transpCalcGradient->isChecked())
				emit NewGradient(5);
			else
				emit NewGradient(2);
		}
		connect(namedGradient, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
	}
	else if (number == 2)
	{
		if (transpCalcPattern->isChecked())
		{
			if (usePatternInverted->isChecked())
				emit NewGradient(7);
			else
				emit NewGradient(6);
		}
		else
		{
			if (usePatternInverted->isChecked())
				emit NewGradient(8);
			else
				emit NewGradient(3);
		}
	}
	else
		emit NewGradient(0);
}

void PropertyWidgetFrame_Transparency::slotGradType(int type)
{
	if (type == 0)
	{
		if (transpCalcGradient->isChecked())
			emit NewGradient(4);
		else
			emit NewGradient(1);
	}
	else
	{
		if (transpCalcGradient->isChecked())
			emit NewGradient(5);
		else
			emit NewGradient(2);
	}
}

void PropertyWidgetFrame_Transparency::setNamedGradient(const QString &name)
{
	if(!m_item)
		return;

	if (namedGradient->currentIndex() == 0)
	{
		gradEdit->setGradient(m_item->mask_gradient);
		m_item->setGradientMask("");
		gradEdit->setGradientEditable(true);
	}
	else
	{
		gradEdit->setGradient(gradientList->value(name));
		gradEdit->setGradientEditable(false);
		m_item->setGradientMask(name);
	}
	if (gradientType->currentIndex() == 0)
	{
		if (transpCalcGradient->isChecked())
			emit NewGradient(4);
		else
			emit NewGradient(1);
	}
	else
	{
		if (transpCalcGradient->isChecked())
			emit NewGradient(5);
		else
			emit NewGradient(2);
	}
}

void PropertyWidgetFrame_Transparency::switchGradientMode()
{
	if (gradientType->currentIndex() == 0)
	{
		if (transpCalcGradient->isChecked())
			emit NewGradient(4);
		else
			emit NewGradient(1);
	}
	else
	{
		if (transpCalcGradient->isChecked())
			emit NewGradient(5);
		else
			emit NewGradient(2);
	}
}

void PropertyWidgetFrame_Transparency::switchPatternMode()
{
	if (transpCalcPattern->isChecked())
	{
		if (usePatternInverted->isChecked())
			emit NewGradient(7);
		else
			emit NewGradient(6);
	}
	else
	{
		if (usePatternInverted->isChecked())
			emit NewGradient(8);
		else
			emit NewGradient(3);
	}
}

void PropertyWidgetFrame_Transparency::editGradientVector()
{
	if (!m_haveDoc || !m_doc)
		return;
	if(!m_item)
		return;

	if (gradEditButton->isChecked())
	{
		TGradDia->unitChange(m_doc->unitIndex());
		TGradDia->setValues(m_item->GrMaskStartX, m_item->GrMaskStartY, m_item->GrMaskEndX, m_item->GrMaskEndY, m_item->GrMaskFocalX, m_item->GrMaskFocalY, m_item->GrMaskScale, m_item->GrMaskSkew, 0, 0);
		if ((m_item->GrMask == 1) || (m_item->GrMask == 4))
			TGradDia->selectLinear();
		else
			TGradDia->selectRadial();
		TGradDia->show();
	}
	else
	{
		TGradDia->hide();
	}
	emit editGradient();
}

void PropertyWidgetFrame_Transparency::setActiveGradDia(bool active)
{
	if (!active)
	{
		gradEditButton->setChecked(false);
		emit editGradient();
	}
}


void PropertyWidgetFrame_Transparency::updateColorSpecialGradient()
{
	if (!m_haveDoc || !m_doc)
		return;
	if(m_doc->m_Selection->isEmpty())
		return;

	PageItem *currItem=m_doc->m_Selection->itemAt(0);
	if (currItem)
	{
		if (!currItem->isGroup()){
			if (TGradDia)
				TGradDia->setValues(currItem->GrMaskStartX, currItem->GrMaskStartY, currItem->GrMaskEndX, currItem->GrMaskEndY, currItem->GrMaskFocalX, currItem->GrMaskFocalY, currItem->GrMaskScale, currItem->GrMaskSkew,0,0);
		}
	}
}

void PropertyWidgetFrame_Transparency::handleGradientChanged()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem) && (m_doc))
	{
		VGradient vg(this->gradEdit->gradient());
		m_doc->itemSelection_SetMaskGradient(vg);
	}
}

void PropertyWidgetFrame_Transparency::handleGradientEdit()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		m_ScMW->view->editStrokeGradient = 2;
		if (this->gradEditButton->isChecked())
			m_ScMW->view->requestMode(modeEditGradientVectors);
		else
			m_ScMW->view->requestMode(modeNormal);
	}
}



void PropertyWidgetFrame_Transparency::handleSpecialGradient(double x1, double y1, double x2, double y2, double fx, double fy, double sg, double sk)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem) && (m_item) && (m_doc))
	{
		QRectF upRect;
		m_item->GrMaskStartX = x1 / m_unitRatio;
		m_item->GrMaskStartY = y1 / m_unitRatio;
		m_item->GrMaskEndX = x2 / m_unitRatio;
		m_item->GrMaskEndY = y2 / m_unitRatio;
		m_item->GrMaskFocalX = fx / m_unitRatio;
		m_item->GrMaskFocalY = fy / m_unitRatio;
		m_item->GrMaskScale = sg;
		m_item->GrMaskSkew = sk;
		if ((m_item->GrMask == 1) || (m_item->GrMask == 4))
		{
			m_item->GrMaskFocalX = m_item->GrMaskStartX;
			m_item->GrMaskFocalY = m_item->GrMaskStartY;
		}
		m_item->update();
		upRect = QRectF(QPointF(m_item->GrMaskStartX, m_item->GrMaskStartY), QPointF(m_item->GrMaskEndX, m_item->GrMaskEndY));
		double radEnd = distance(m_item->GrMaskEndX - m_item->GrMaskStartX, m_item->GrMaskEndY - m_item->GrMaskStartY);
		double rotEnd = xy2Deg(m_item->GrMaskEndX - m_item->GrMaskStartX, m_item->GrMaskEndY - m_item->GrMaskStartY);
		QTransform m;
		m.translate(m_item->GrMaskStartX, m_item->GrMaskStartY);
		m.rotate(rotEnd);
		m.rotate(-90);
		m.rotate(m_item->GrMaskSkew);
		m.translate(radEnd * m_item->GrMaskScale, 0);
		QPointF shP = m.map(QPointF(0,0));
		upRect |= QRectF(shP, QPointF(m_item->GrMaskEndX, m_item->GrMaskEndY)).normalized();
		upRect |= QRectF(shP, QPointF(m_item->GrMaskStartX, m_item->GrMaskStartY)).normalized();
		upRect |= QRectF(shP, QPointF(0, 0)).normalized();
		upRect |= QRectF(shP, QPointF(m_item->width(), m_item->height())).normalized();
		upRect.translate(m_item->xPos(), m_item->yPos());
		m_doc->regionsChanged()->update(upRect.adjusted(-10.0, -10.0, 10.0, 10.0));
		m_doc->changed();
	}
}


/*********************************************************************
*
* Feature Pattern
*
**********************************************************************/


void PropertyWidgetFrame_Transparency::hideEditedPatterns(QStringList names)
{
	for (int a = 0; a < names.count(); a++)
	{
		QList<QListWidgetItem*> items = patternBox->findItems(names[a], Qt::MatchExactly);
		if (items.count() > 0)
			items[0]->setHidden(true);
	}
}


void PropertyWidgetFrame_Transparency::updatePatternList()
{
	disconnect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	patternBox->clear();
	patternBox->setIconSize(QSize(48, 48));
	QStringList patK = patternList->keys();
	qSort(patK);
	for (int a = 0; a < patK.count(); a++)
	{
		ScPattern sp = patternList->value(patK[a]);
		QPixmap pm;
		if (sp.getPattern()->width() >= sp.getPattern()->height())
			pm=QPixmap::fromImage(sp.getPattern()->scaledToWidth(48, Qt::SmoothTransformation));
		else
			pm=QPixmap::fromImage(sp.getPattern()->scaledToHeight(48, Qt::SmoothTransformation));
		QPixmap pm2(48, 48);
		pm2.fill(palette().color(QPalette::Base));
		QPainter p;
		p.begin(&pm2);
		p.drawPixmap(24 - pm.width() / 2, 24 - pm.height() / 2, pm);
		p.end();
		QListWidgetItem *item = new QListWidgetItem(pm2, patK[a], patternBox);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
	patternBox->clearSelection();
	connect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
}

void PropertyWidgetFrame_Transparency::setPatterns(QHash<QString, ScPattern> *docPatterns)
{
	patternList = docPatterns;
	updatePatternList();
}

void PropertyWidgetFrame_Transparency::selectPattern(QListWidgetItem *c)
{
	if (c == NULL)
		return;
	emit NewPattern(c->text());
}

void PropertyWidgetFrame_Transparency::setActPattern(QString pattern, double scaleX, double scaleY, double offsetX, double offsetY, double rotation, double skewX, double skewY, bool mirrorX, bool mirrorY)
{
	disconnect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	QList<QListWidgetItem*> itl = patternBox->findItems(pattern, Qt::MatchExactly);
	if (itl.count() != 0)
	{
		QListWidgetItem *it = itl[0];
		patternBox->setCurrentItem(it);
	}
	else
		patternBox->clearSelection();
	m_Pattern_scaleX = scaleX;
	m_Pattern_scaleY = scaleX;
	m_Pattern_offsetX = offsetX;
	m_Pattern_offsetY = offsetY;
	m_Pattern_rotation = rotation;
	m_Pattern_skewX = skewX;
	m_Pattern_skewY = skewY;
	m_Pattern_mirrorX = mirrorX;
	m_Pattern_mirrorY = mirrorY;
	connect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
}

void PropertyWidgetFrame_Transparency::changePatternProps()
{
	PatternPropsDialog *dia = new PatternPropsDialog(this, m_unitIndex, false);
	dia->spinXscaling->setValue(m_Pattern_scaleX);
	dia->spinYscaling->setValue(m_Pattern_scaleY);
	if (m_Pattern_scaleX == m_Pattern_scaleY)
		dia->keepScaleRatio->setChecked(true);
	dia->spinXoffset->setValue(m_Pattern_offsetX);
	dia->spinYoffset->setValue(m_Pattern_offsetY);
	dia->spinAngle->setValue(m_Pattern_rotation);
	double asina = atan(m_Pattern_skewX);
	dia->spinXSkew->setValue(asina / (M_PI / 180.0));
	double asinb = atan(m_Pattern_skewY);
	dia->spinYSkew->setValue(asinb / (M_PI / 180.0));
	dia->FlipH->setChecked(m_Pattern_mirrorX);
	dia->FlipV->setChecked(m_Pattern_mirrorY);
	connect(dia, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)));
	dia->exec();
	m_Pattern_scaleX = dia->spinXscaling->value();
	m_Pattern_scaleY = dia->spinYscaling->value();
	m_Pattern_offsetX = dia->spinXoffset->value();
	m_Pattern_offsetY = dia->spinYoffset->value();
	m_Pattern_rotation = dia->spinAngle->value();
	double a    = M_PI / 180.0 * dia->spinXSkew->value();
	double b    = M_PI / 180.0 * dia->spinYSkew->value();
	double sina = tan(a);
	double sinb = tan(b);
	m_Pattern_skewX = sina;
	m_Pattern_skewY = sinb;
	m_Pattern_mirrorX = dia->FlipH->isChecked();
	m_Pattern_mirrorY = dia->FlipV->isChecked();
	delete dia;
}

/*********************************************************************
*
* Feature Fills
*
**********************************************************************/

void PropertyWidgetFrame_Transparency::setActTrans(double val, double val2)
{
	disconnect(strokeOpacity, SIGNAL(valueChanged(double)), this, SLOT(slotTransS(double)));
	disconnect(fillOpacity, SIGNAL(valueChanged(double)), this, SLOT(slotTransF(double)));
	strokeOpacity->setValue(qRound(100 - (val2 * 100)));
	fillOpacity->setValue(qRound(100 - (val * 100)));
	connect(strokeOpacity, SIGNAL(valueChanged(double)), this, SLOT(slotTransS(double)));
	connect(fillOpacity, SIGNAL(valueChanged(double)), this, SLOT(slotTransF(double)));
}

void PropertyWidgetFrame_Transparency::setActBlend(int val, int val2)
{
	disconnect(blendModeFill, SIGNAL(activated(int)), this, SIGNAL(NewBlend(int)));
	disconnect(blendModeStroke, SIGNAL(activated(int)), this, SIGNAL(NewBlendS(int)));
	blendModeFill->setCurrentIndex(val);
	blendModeStroke->setCurrentIndex(val2);
	connect(blendModeFill, SIGNAL(activated(int)), this, SIGNAL(NewBlend(int)));
	connect(blendModeStroke, SIGNAL(activated(int)), this, SIGNAL(NewBlendS(int)));
}

void PropertyWidgetFrame_Transparency::slotTransS(double val)
{
	emit NewTransS((100 - val) / 100.0);
}

void PropertyWidgetFrame_Transparency::slotTransF(double val)
{
	emit NewTrans((100 - val) / 100.0);
}

void PropertyWidgetFrame_Transparency::editLineSelectorButton()
{
	if (editLineSelector->isChecked())
	{
		stackedWidget->setCurrentIndex(0);
		editFillSelector->setChecked(false);
	}
	setCurrentItem(m_item);
}

void PropertyWidgetFrame_Transparency::editFillSelectorButton()
{
	if (editFillSelector->isChecked())
	{
		stackedWidget->setCurrentIndex(1);
		editLineSelector->setChecked(false);
	}
	setCurrentItem(m_item);
}

/*********************************************************************
*
* Feature Group Transparency
*
**********************************************************************/

void PropertyWidgetFrame_Transparency::handleGroupTransparency(double trans)
{
	if ((m_haveDoc) && (m_haveItem))
	{
		m_item->setFillTransparency(trans);
		m_item->update();
	}
}

void PropertyWidgetFrame_Transparency::handleGroupBlending(int blend)
{
	if ((m_haveDoc) && (m_haveItem))
	{
		m_item->setFillBlendmode(blend);
		m_item->update();
	}
}

void PropertyWidgetFrame_Transparency::handleGroupGradMask(int typ)
{
	if ((m_haveDoc) && (m_haveItem))
	{
		m_item->GrMask = typ;
		if ((typ > 0) && (typ < 7))
			m_item->updateGradientVectors();
		m_item->update();
	}
}

void PropertyWidgetFrame_Transparency::handleGroupPatternMask(QString pattern)
{
	if ((m_haveDoc) && (m_haveItem))
	{
		m_item->setPatternMask(pattern);
		m_item->update();
	}
}

void PropertyWidgetFrame_Transparency::handleGroupPatternMaskProps(double imageScaleX, double imageScaleY, double offsetX, double offsetY, double rotation, double skewX, double skewY, bool mirrorX, bool mirrorY)
{
	if ((m_haveDoc) && (m_haveItem))
	{
		m_item->setMaskTransform(imageScaleX, imageScaleY, offsetX, offsetY, rotation, skewX, skewY);
		m_item->setMaskFlip(mirrorX, mirrorY);
		m_item->update();
	}
}

