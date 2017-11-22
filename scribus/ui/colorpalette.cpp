/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/***************************************************************************
                          cpalette.cpp  -  description
                             -------------------
    begin                : Wed Apr 25 2001
    copyright            : (C) 2001 by Franz Schmid
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

#include "colorpalette.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QRect>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStandardItem>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDebug>

#include "appmodes.h"
#include "colorlistbox.h"
#include "commonstrings.h"
#include "gradienteditor.h"
#include "iconmanager.h"
#include "insertTable.h"
#include "pageitem.h"
#include "sccolorengine.h"
#include "sccombobox.h"
#include "sclistwidgetdelegate.h"
#include "scpage.h"
#include "scpainter.h"
#include "scpattern.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "scribusview.h"
#include "scrspinbox.h"
#include "selection.h"
#include "undomanager.h"
#include "units.h"
#include "util.h"
#include "util_math.h"

ColorPalette::ColorPalette(QWidget* parent) : QWidget(parent)
{
	undoManager = UndoManager::instance();
	m_ScMW = 0;
	m_item = 0;
	m_haveDoc   = false;
	m_haveItem  = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;


	m_blockUpdates = 0;
	patternList = NULL;
	CGradDia = NULL;
	CGradDia = new GradientVectorDialog(this->parentWidget());
	CGradDia->hide();

	setupUi(this);

	setFixedWidth(380);

	fillModeCombo->addItem( tr("Solid") );
	fillModeCombo->addItem( tr("Gradient") );
	fillModeCombo->addItem( tr("Hatch") );
	fillShade->setDecimals(0);
	strokeShade->setDecimals(0);
	color1Alpha->setDecimals(0);
	color2Alpha->setDecimals(0);
	color3Alpha->setDecimals(0);
	color4Alpha->setDecimals(0);
	color1Shade->setDecimals(0);
	color2Shade->setDecimals(0);
	color3Shade->setDecimals(0);
	color4Shade->setDecimals(0);
	shadeMeshPoint->setDecimals(0);
	strokeModeCombo->addItem( tr("Solid") );
	strokeModeCombo->addItem( tr("Gradient") );

	colorPoint1->setPixmapType(ColorCombo::fancyPixmaps);
	colorPoint2->setPixmapType(ColorCombo::fancyPixmaps);
	colorPoint3->setPixmapType(ColorCombo::fancyPixmaps);
	colorPoint4->setPixmapType(ColorCombo::fancyPixmaps);
	colorMeshPoint->setPixmapType(ColorCombo::fancyPixmaps);
	colorListFill->setPixmapType(ColorListBox::fancyPixmap);
	colorListStroke->setPixmapType(ColorListBox::fancyPixmap);
	hatchLineColor->setPixmapType(ColorCombo::fancyPixmaps);
	hatchBackground->setPixmapType(ColorCombo::fancyPixmaps);

/*  Setting a delegate to dispaly only icons for the patterns */
/*	ScListWidgetDelegate* delegateF = new ScListWidgetDelegate(patternBox, patternBox);
	delegateF->setIconOnly(true);
	patternBox->setItemDelegate(delegateF);
	ScListWidgetDelegate* delegateS = new ScListWidgetDelegate(patternBoxStroke, patternBoxStroke);
	delegateS->setIconOnly(true);
	patternBoxStroke->setItemDelegate(delegateS);
*/
	gradEdit->layout()->setAlignment(Qt::AlignTop);
	gradEditStroke->layout()->setAlignment(Qt::AlignTop);
	gradientTypeStroke->setCurrentIndex(0);
	
	tabFillStroke->setCurrentIndex(0);
	setCurrentItem(0);
	hatchAngle->setDecimals(0);
	hatchAngle->setNewUnit(6);
	hatchDist->setDecimals(0);
	hatchDist->setNewUnit(0);
	hatchDist->setValues(1, 1000, 0, 1);
	/*editFillColorSelector->setChecked(true);
	editFillColorSelectorButton();*/

	connect(this, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)), this, SLOT(NewSpGradient(double, double, double, double, double, double, double, double, double, double )));
	connect(this, SIGNAL(editGradient(int)), this, SLOT(toggleGradientEdit(int)));

}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void ColorPalette::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;
	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
}

void ColorPalette::connectSignals()
{
	connect(CGradDia, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)), this, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)));
	connect(CGradDia, SIGNAL(paletteShown(bool)), this, SLOT(setActiveGradDia(bool)));
	connect(CGradDia, SIGNAL(editGradient(int)), this, SIGNAL(editGradient(int)));
	connect(CGradDia, SIGNAL(createNewMesh()), this, SLOT(createNewMeshGradient()));
	connect(CGradDia, SIGNAL(resetMesh()), this, SLOT(resetMeshGradient()));
	connect(CGradDia, SIGNAL(meshToShape()), this, SLOT(meshGradientToShape()));
	connect(CGradDia, SIGNAL(reset1Control()), this, SLOT(resetOneControlPoint()));
	connect(CGradDia, SIGNAL(resetAllControl()), this, SLOT(resetAllControlPoints()));
	connect(CGradDia, SIGNAL(removePatch()), this, SLOT(handleRemovePatch()));
	connect(CGradDia, SIGNAL(snapToMGrid(bool)), this, SLOT(snapToPatchGrid(bool)));

	connect(colorPoint1    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	connect(colorPoint2    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	connect(colorPoint3    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	connect(colorPoint4    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	connect(color1Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color2Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color3Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color4Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color1Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color2Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color3Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color4Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(colorListFill  , SIGNAL(currentRowChanged(int)), this, SLOT(selectColorF(int)));
	connect(colorListStroke, SIGNAL(currentRowChanged(int)), this, SLOT(selectColorS(int)));
	connect(colorMeshPoint , SIGNAL(activated(int)), this, SLOT(updateMeshPoint()));
	connect(editMeshColors , SIGNAL(clicked()), this, SLOT(editMeshPointColor()));
	connect(editPatternProps      , SIGNAL(clicked()) , this, SLOT(changePatternProps()));
	connect(editPatternPropsStroke, SIGNAL(clicked()), this, SLOT(changePatternPropsStroke()));
	connect(fillModeCombo , SIGNAL(currentIndexChanged(int)), this, SLOT(slotGrad(int)));
	connect(fillShade     , SIGNAL(valueChanged(double)), this, SIGNAL(NewBrushShade(double)));
	connect(followsPath   , SIGNAL(clicked()), this, SLOT(toggleStrokePattern()));
	connect(gradientTypeStroke , SIGNAL(activated(int)), this, SLOT(slotGradTypeStroke(int)));
	connect(gradEdit      , SIGNAL(gradientChanged()), this, SLOT(handleFillGradient()));
	connect(gradEditButton, SIGNAL(clicked()), this, SLOT(editGradientVector()));
	connect(gradEditButtonStroke, SIGNAL(clicked()), this, SLOT(editGradientVectorStroke()));
	connect(gradEditStroke, SIGNAL(gradientChanged()), this, SLOT(handleStrokeGradient()));
	connect(gradientType  , SIGNAL(activated(int)), this, SLOT(slotGradType(int)));
	connect(gradientTypeStroke , SIGNAL(activated(int)), this, SLOT(slotGradTypeStroke(int)));
	connect(namedGradient , SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
	connect(namedGradientStroke, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradientStroke(const QString &)));
	connect(overPrintCombo     , SIGNAL(activated(int)), this, SIGNAL(NewOverprint(int)));
	connect(patternBox         , SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	connect(patternBoxStroke   , SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPatternS(QListWidgetItem*)));
	connect(shadeMeshPoint     , SIGNAL(valueChanged(double)), this, SLOT(updateMeshPoint()));
	connect(strokeShade        , SIGNAL(valueChanged(double)), this, SIGNAL(NewPenShade(double)));
	connect(strokeModeCombo    , SIGNAL(currentIndexChanged(int)), this, SLOT(slotGradStroke(int)));
	connect(tabFillStroke      , SIGNAL(currentChanged(int)), this, SLOT(fillStrokeSelector(int)));
	connect(transparencyMeshPoint, SIGNAL(valueChanged(double)), this, SLOT(updateMeshPoint()));
	connect(hatchAngle, SIGNAL(valueChanged(double)), this, SLOT(changeHatchProps()));
	connect(hatchDist, SIGNAL(valueChanged(double)), this, SLOT(changeHatchProps()));
	connect(hatchType, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	connect(hatchBackground, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	connect(hatchLineColor, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	connect(gradientExtend  , SIGNAL(activated(int)), this, SLOT(handleGradientExtend(int)));
	connect(GradientExtendS  , SIGNAL(activated(int)), this, SLOT(handleStrokeGradientExtend(int)));
}

void ColorPalette::disconnectSignals()
{
	disconnect(CGradDia, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)), this, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)));
	disconnect(CGradDia, SIGNAL(paletteShown(bool)), this, SLOT(setActiveGradDia(bool)));
	disconnect(CGradDia, SIGNAL(editGradient(int)), this, SIGNAL(editGradient(int)));
	disconnect(CGradDia, SIGNAL(createNewMesh()), this, SLOT(createNewMeshGradient()));
	disconnect(CGradDia, SIGNAL(resetMesh()), this, SLOT(resetMeshGradient()));
	disconnect(CGradDia, SIGNAL(meshToShape()), this, SLOT(meshGradientToShape()));
	disconnect(CGradDia, SIGNAL(reset1Control()), this, SLOT(resetOneControlPoint()));
	disconnect(CGradDia, SIGNAL(resetAllControl()), this, SLOT(resetAllControlPoints()));
	disconnect(CGradDia, SIGNAL(removePatch()), this, SLOT(handleRemovePatch()));
	disconnect(CGradDia, SIGNAL(snapToMGrid(bool)), this, SLOT(snapToPatchGrid(bool)));

	disconnect(colorPoint1    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	disconnect(colorPoint2    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	disconnect(colorPoint3    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	disconnect(colorPoint4    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	disconnect(color1Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color2Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color3Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color4Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color1Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color2Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color3Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color4Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(colorListFill  , SIGNAL(currentRowChanged(int)), this, SLOT(selectColorF(int)));
	disconnect(colorListStroke, SIGNAL(currentRowChanged(int)), this, SLOT(selectColorS(int)));
	disconnect(colorMeshPoint , SIGNAL(activated(int)), this, SLOT(updateMeshPoint()));
	disconnect(editMeshColors , SIGNAL(clicked()), this, SLOT(editMeshPointColor()));
	disconnect(editPatternProps      , SIGNAL(clicked()) , this, SLOT(changePatternProps()));
	disconnect(editPatternPropsStroke, SIGNAL(clicked()), this, SLOT(changePatternPropsStroke()));
	disconnect(fillModeCombo , SIGNAL(currentIndexChanged(int)), this, SLOT(slotGrad(int)));
	disconnect(fillShade     , SIGNAL(valueChanged(double)), this, SIGNAL(NewBrushShade(double)));
	disconnect(followsPath   , SIGNAL(clicked()), this, SLOT(toggleStrokePattern()));
	disconnect(gradientTypeStroke , SIGNAL(activated(int)), this, SLOT(slotGradTypeStroke(int)));
	disconnect(gradEdit      , SIGNAL(gradientChanged()), this, SLOT(handleFillGradient()));
	disconnect(gradEditButton, SIGNAL(clicked()), this, SLOT(editGradientVector()));
	disconnect(gradEditButtonStroke, SIGNAL(clicked()), this, SLOT(editGradientVectorStroke()));
	disconnect(gradEditStroke, SIGNAL(gradientChanged()), this, SLOT(handleStrokeGradient()));
	disconnect(gradientType  , SIGNAL(activated(int)), this, SLOT(slotGradType(int)));
	disconnect(gradientTypeStroke , SIGNAL(activated(int)), this, SLOT(slotGradTypeStroke(int)));
	disconnect(namedGradient , SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
	disconnect(namedGradientStroke, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradientStroke(const QString &)));
	disconnect(overPrintCombo     , SIGNAL(activated(int)), this, SIGNAL(NewOverprint(int)));
	disconnect(patternBox         , SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	disconnect(patternBoxStroke   , SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPatternS(QListWidgetItem*)));
	disconnect(shadeMeshPoint, SIGNAL(valueChanged(double)), this, SLOT(updateMeshPoint()));
	disconnect(strokeModeCombo    , SIGNAL(currentIndexChanged(int)), this, SLOT(slotGradStroke(int)));
	disconnect(strokeShade    , SIGNAL(valueChanged(double)), this, SIGNAL(NewPenShade(double)));
	disconnect(tabFillStroke      , SIGNAL(currentChanged(int)), this, SLOT(fillStrokeSelector(int)));
	disconnect(transparencyMeshPoint, SIGNAL(valueChanged(double)), this, SLOT(updateMeshPoint()));
	disconnect(hatchAngle, SIGNAL(valueChanged(double)), this, SLOT(changeHatchProps()));
	disconnect(hatchDist, SIGNAL(valueChanged(double)), this, SLOT(changeHatchProps()));
	disconnect(hatchType, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	disconnect(hatchBackground, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	disconnect(hatchLineColor, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	disconnect(gradientExtend  , SIGNAL(activated(int)), this, SLOT(handleGradientExtend(int)));
	disconnect(GradientExtendS  , SIGNAL(activated(int)), this, SLOT(handleStrokeGradientExtend(int)));
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void ColorPalette::setDoc(ScribusDoc* d)
{
//	disconnect(this, SIGNAL(NewPen(QString)), 0, 0);
//	disconnect(this, SIGNAL(NewBrush(QString)), 0, 0);
//	disconnect(this, SIGNAL(NewPenShade(double)), 0, 0);
//	disconnect(this, SIGNAL(NewBrushShade(double)), 0, 0);
//	disconnect(this, SIGNAL(NewGradient(int)), 0, 0);
//	disconnect(this, SIGNAL(NewGradientS(int)), 0, 0);
//	disconnect(this, SIGNAL(NewPattern(QString)), 0, 0);
//	disconnect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), 0, 0);
//	disconnect(this, SIGNAL(NewOverprint(int)), 0, 0);
//	disconnect(this, SIGNAL(NewPatternS(QString)), 0, 0);
//	disconnect(this, SIGNAL(NewPatternTypeS(bool)), 0, 0);
//	disconnect(this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), 0, 0);
//	disconnect(displayAllColors, SIGNAL(clicked()), this, SLOT(toggleColorDisplay()));

//	if (m_doc)
//	{
//		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
//		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
//		disconnect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
//	}

//	ScribusDoc* oldDoc = m_doc;
//	m_doc = d;

//	if (d == NULL)
//	{
//		colorListStroke->cList = NULL;
//		colorListFill->cList = NULL;
//		disconnectSignals();
//	}
//	else
//	{
//		colorListStroke->cList = &d->PageColors;
//		colorListFill->cList = &d->PageColors;
//		gradEdit->setColors(d->PageColors);
//		m_unitIndex = d->unitIndex();

//		updateColorList();
//		updateCList();

//		connect(this, SIGNAL(NewPen(QString))      , d, SLOT(itemSelection_SetItemPen(QString)));
//		connect(this, SIGNAL(NewBrush(QString))    , d, SLOT(itemSelection_SetItemBrush(QString)));
//		connect(this, SIGNAL(NewPenShade(double))     , this, SLOT(handleStrokeShade(double)));
//		connect(this, SIGNAL(NewBrushShade(double))   , this, SLOT(handleFillShade(double)));
//		connect(this, SIGNAL(NewGradient(int))     , d, SLOT(itemSelection_SetItemGradFill(int)));
//		connect(this, SIGNAL(NewGradientS(int))    , d, SLOT(itemSelection_SetItemGradStroke(int)));
//		connect(this, SIGNAL(NewPattern(QString))  , d, SLOT(itemSelection_SetItemPatternFill(QString)));
//		connect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), d, SLOT(itemSelection_SetItemPatternProps(double, double, double, double, double, double, double, bool, bool)));
//		connect(this, SIGNAL(NewOverprint(int))    , this, SLOT(handleOverprint(int)));
//		connect(this, SIGNAL(NewPatternS(QString)) , d, SLOT(itemSelection_SetItemStrokePattern(QString)));
//		connect(this, SIGNAL(NewPatternTypeS(bool)), d, SLOT(itemSelection_SetItemStrokePatternType(bool)));
//		connect(this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), d, SLOT(itemSelection_SetItemStrokePatternProps(double, double, double, double, double, double, double, double, bool, bool)));
//		connect(displayAllColors, SIGNAL(clicked()), this, SLOT(toggleColorDisplay()));

//		connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
//		connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
//		connect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
//	}

//	if (oldDoc != m_doc)
//	{
//		showGradient(0);
//	}

	// -------------------

	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		disconnect(this, SIGNAL(NewPen(QString)), 0, 0);
		disconnect(this, SIGNAL(NewBrush(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPenShade(double)), 0, 0);
		disconnect(this, SIGNAL(NewBrushShade(double)), 0, 0);
		disconnect(this, SIGNAL(NewGradient(int)), 0, 0);
		disconnect(this, SIGNAL(NewGradientS(int)), 0, 0);
		disconnect(this, SIGNAL(NewPattern(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), 0, 0);
		disconnect(this, SIGNAL(NewOverprint(int)), 0, 0);
		disconnect(this, SIGNAL(NewPatternS(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPatternTypeS(bool)), 0, 0);
		disconnect(this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), 0, 0);
		disconnect(displayAllColors, SIGNAL(clicked()), this, SLOT(toggleColorDisplay()));


		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
		disconnect(m_doc->scMW(), SIGNAL(UpdateRequest(int)), this, 0);
	}

	ScribusDoc* oldDoc = m_doc;
	m_doc = d;
	m_item = NULL;

	if (!m_doc)
	{
		colorListStroke->cList = NULL;
		colorListFill->cList = NULL;
		disconnectSignals();
		return;
	}

	m_unitRatio   = m_doc->unitRatio();
	m_unitIndex   = m_doc->unitIndex();

	colorListStroke->cList = &d->PageColors;
	colorListFill->cList = &d->PageColors;
	gradEdit->setColors(d->PageColors);
	m_unitIndex = d->unitIndex();

	updateColorList();
	updateCList();

	connect(this, SIGNAL(NewPen(QString))      , d, SLOT(itemSelection_SetItemPen(QString)));
	connect(this, SIGNAL(NewBrush(QString))    , d, SLOT(itemSelection_SetItemBrush(QString)));
	connect(this, SIGNAL(NewPenShade(double))     , this, SLOT(handleStrokeShade(double)));
	connect(this, SIGNAL(NewBrushShade(double))   , this, SLOT(handleFillShade(double)));
	connect(this, SIGNAL(NewGradient(int))     , d, SLOT(itemSelection_SetItemGradFill(int)));
	connect(this, SIGNAL(NewGradientS(int))    , d, SLOT(itemSelection_SetItemGradStroke(int)));
	connect(this, SIGNAL(NewPattern(QString))  , d, SLOT(itemSelection_SetItemPatternFill(QString)));
	connect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), d, SLOT(itemSelection_SetItemPatternProps(double, double, double, double, double, double, double, bool, bool)));
	connect(this, SIGNAL(NewOverprint(int))    , this, SLOT(handleOverprint(int)));
	connect(this, SIGNAL(NewPatternS(QString)) , d, SLOT(itemSelection_SetItemStrokePattern(QString)));
	connect(this, SIGNAL(NewPatternTypeS(bool)), d, SLOT(itemSelection_SetItemStrokePatternType(bool)));
	connect(this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), d, SLOT(itemSelection_SetItemStrokePatternProps(double, double, double, double, double, double, double, double, bool, bool)));
	connect(displayAllColors, SIGNAL(clicked()), this, SLOT(toggleColorDisplay()));

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	connect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));

	if (oldDoc != m_doc)
	{
		showGradient(0);
	}

	// Handle properties update when switching document
	handleSelectionChanged();
}

void ColorPalette::unsetDoc()
{

	if (m_doc)
	{
		disconnect(this, SIGNAL(NewPen(QString)), 0, 0);
		disconnect(this, SIGNAL(NewBrush(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPenShade(double)), 0, 0);
		disconnect(this, SIGNAL(NewBrushShade(double)), 0, 0);
		disconnect(this, SIGNAL(NewGradient(int)), 0, 0);
		disconnect(this, SIGNAL(NewGradientS(int)), 0, 0);
		disconnect(this, SIGNAL(NewPattern(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), 0, 0);
		disconnect(this, SIGNAL(NewOverprint(int)), 0, 0);
		disconnect(this, SIGNAL(NewPatternS(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPatternTypeS(bool)), 0, 0);
		disconnect(this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), 0, 0);
		disconnect(displayAllColors, SIGNAL(clicked()), this, SLOT(toggleColorDisplay()));

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

void ColorPalette::setCurrentItem(PageItem* i)
{
//	if ((i == NULL) || (m_item != i))
//	{
//		editStrokeGradient = 0;
//		CGradDia->hide();
//		editMeshColors->setEnabled(true);
//		gradEditButton->setEnabled(true);
//	}

//	m_item = i;
//	disconnectSignals();
	
//	if (!m_item)
//		return;

//	showOverprint(m_item->doOverprint ? 1 : 0);
//	showColorValues(m_item->lineColor(), m_item->fillColor(), m_item->lineShade(), m_item->fillShade());
//	showGradient(m_item->gradientType());
//	showGradientStroke(m_item->strokeGradientType());
//	gradEdit->setGradient(m_item->fill_gradient);
//	gradEditStroke->setGradient(m_item->stroke_gradient);
//	if (!m_item->gradient().isEmpty())
//	{
//		setCurrentComboItem(namedGradient, m_item->gradient());
//		gradEdit->setGradientEditable(false);
//	}
//	else
//	{
//		namedGradient->setCurrentIndex(0);
//		gradEdit->setGradientEditable(true);
//	}
//	if (!m_item->strokeGradient().isEmpty())
//	{
//		setCurrentComboItem(namedGradientStroke, m_item->strokeGradient());
//		gradEditStroke->setGradientEditable(false);
//	}
//	else
//	{
//		namedGradientStroke->setCurrentIndex(0);
//		gradEditStroke->setGradientEditable(true);
//	}
//	if (m_item->GrTypeStroke > 0)
//	{
//		if (m_item->GrTypeStroke == 6)
//			gradientTypeStroke->setCurrentIndex(0);
//		else
//			gradientTypeStroke->setCurrentIndex(1);
//	}
//	/*if (patternList->count() == 0)
//	{
//		tabWidgetStroke->setTabEnabled(2, false);
//		tabWidget->setTabEnabled(2, false);
//	}
//	else
//	{
//		tabWidgetStroke->setTabEnabled(2, true);
//		tabWidget->setTabEnabled(2, true);
//	}*/
//	enablePatterns(patternList->count() != 0);
//	/*if (!currentItem->strokePattern().isEmpty())
//		strokeModeCombo->setCurrentIndex(2);
//	else if (currentItem->GrTypeStroke > 0)
//		strokeModeCombo->setCurrentIndex(1);
//	else
//		strokeModeCombo->setCurrentIndex(0);*/
//	if (m_item->gradientType() == 12)
//		setMeshPatchPoint();
//	else
//		setMeshPoint();
//	if(CGradDia && gradEditButton->isChecked())
//	{
//		if(tabFillStroke->currentIndex() == 0)
//			setGradientVectorValues();
//		else
//			setGradientVectorStrokeValues();
//	}
//	editMeshColors->setEnabled(!CGradDia->isVisible());
//	gradEditButton->setEnabled(!editMeshColors->isChecked());
//	gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
//	fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));

//	double patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, patternSpace;
//	bool mirrorX, mirrorY;
//	m_item->patternTransform(patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY);
//	m_item->patternFlip(mirrorX, mirrorY);
//	setActPattern(m_item->pattern(), patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, mirrorX, mirrorY);
//	m_item->strokePatternTransform(patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, patternSpace);
//	m_item->strokePatternFlip(mirrorX, mirrorY);
//	setActPatternStroke(m_item->strokePattern(), patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, mirrorX, mirrorY, patternSpace, m_item->isStrokePatternToPath());

//	connectSignals();

	// -----------------

	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (!m_doc)
		setDoc(i->doc());

	m_haveItem = false;
	m_item = i;

	if ((i == NULL) || (m_item != i))
	{
		editStrokeGradient = 0;
		CGradDia->hide();
		editMeshColors->setEnabled(true);
		gradEditButton->setEnabled(true);
	}

	disconnectSignals();

	if (!m_item)
		return;

	showOverprint(m_item->doOverprint ? 1 : 0);
	showColorValues(m_item->lineColor(), m_item->fillColor(), m_item->lineShade(), m_item->fillShade());
	showGradient(m_item->gradientType());
	showGradientStroke(m_item->strokeGradientType());
	gradEdit->setGradient(m_item->fill_gradient);
	gradEditStroke->setGradient(m_item->stroke_gradient);
	if (!m_item->gradient().isEmpty())
	{
		setCurrentComboItem(namedGradient, m_item->gradient());
		gradEdit->setGradientEditable(false);
	}
	else
	{
		namedGradient->setCurrentIndex(0);
		gradEdit->setGradientEditable(true);
	}
	if (!m_item->strokeGradient().isEmpty())
	{
		setCurrentComboItem(namedGradientStroke, m_item->strokeGradient());
		gradEditStroke->setGradientEditable(false);
	}
	else
	{
		namedGradientStroke->setCurrentIndex(0);
		gradEditStroke->setGradientEditable(true);
	}
	if (m_item->GrTypeStroke > 0)
	{
		if (m_item->GrTypeStroke == 6)
			gradientTypeStroke->setCurrentIndex(0);
		else
			gradientTypeStroke->setCurrentIndex(1);
	}
	/*if (patternList->count() == 0)
	{
		tabWidgetStroke->setTabEnabled(2, false);
		tabWidget->setTabEnabled(2, false);
	}
	else
	{
		tabWidgetStroke->setTabEnabled(2, true);
		tabWidget->setTabEnabled(2, true);
	}*/
	enablePatterns(patternList->count() != 0);
	/*if (!currentItem->strokePattern().isEmpty())
		strokeModeCombo->setCurrentIndex(2);
	else if (currentItem->GrTypeStroke > 0)
		strokeModeCombo->setCurrentIndex(1);
	else
		strokeModeCombo->setCurrentIndex(0);*/
	if (m_item->gradientType() == 12)
		setMeshPatchPoint();
	else
		setMeshPoint();
	if(CGradDia && gradEditButton->isChecked())
	{
		if(tabFillStroke->currentIndex() == 0)
			setGradientVectorValues();
		else
			setGradientVectorStrokeValues();
	}
	editMeshColors->setEnabled(!CGradDia->isVisible());
	gradEditButton->setEnabled(!editMeshColors->isChecked());
	gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));

	double patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, patternSpace;
	bool mirrorX, mirrorY;
	m_item->patternTransform(patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY);
	m_item->patternFlip(mirrorX, mirrorY);
	setActPattern(m_item->pattern(), patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, mirrorX, mirrorY);
	m_item->strokePatternTransform(patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, patternSpace);
	m_item->strokePatternFlip(mirrorX, mirrorY);
	setActPatternStroke(m_item->strokePattern(), patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, mirrorX, mirrorY, patternSpace, m_item->isStrokePatternToPath());

	m_haveItem = true;

	connectSignals();

}

void ColorPalette::updateFromItem()
{
	setCurrentItem(m_item);
}



PageItem* ColorPalette::currentItemFromSelection()
{
	PageItem *currentItem = NULL;
	if (m_doc)
		currentItem = m_doc->m_Selection->itemAt(0);
	return currentItem;
}


void ColorPalette::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;

	handleSelectionChanged();
}


/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void ColorPalette::handleSelectionChanged()
{
	if (m_doc && !updatesBlocked())
	{
		PageItem* currItem = currentItemFromSelection();
		setCurrentItem(currItem);

		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem   = (itemType != -1);

		switch (itemType)
		{
		case -1:
			m_haveItem = false;

			this->showGradient(0);
		}
	}
}

void ColorPalette::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqColorsUpdate)
		updateColorList();
}

void ColorPalette::unitChange()
{
	if (CGradDia)
		CGradDia->unitChange(m_unitIndex);
	hatchDist->setNewUnit(m_unitIndex);
//	m_unitIndex = m_unitIndex;
}

void ColorPalette::languageChange()
{
	// Needed to avoid issues if an item is selected and patterns are available
	if (m_item)
		disconnectSignals();

	// Save fill tab state
	int oldFillModeComboIndex = fillModeCombo->currentIndex();
	int oldGradientTypeIndex = gradientType->currentIndex();
	int oldGradientExtendIndex = gradientExtend->currentIndex();
	int oldHatchTypeIndex = hatchType->currentIndex();

	// Save stroke tab state
	int oldStrokeModeComboIndex = strokeModeCombo->currentIndex();
	int oldGradientTypeStrokeIndex = gradientTypeStroke->currentIndex();
	int oldGradientExtendSIndex = GradientExtendS->currentIndex();

	// Save properties outside of tabs
	int oldOverPrintComboIndex = overPrintCombo->currentIndex();

	// Retranslate UI
	retranslateUi(this);

	fillModeCombo->clear();
	fillModeCombo->addItem( tr("Solid"));
	fillModeCombo->addItem( tr("Gradient"));
	fillModeCombo->addItem( tr("Hatch"));

	strokeModeCombo->clear();
	strokeModeCombo->addItem( tr("Solid"));
	strokeModeCombo->addItem( tr("Gradient"));

	if (m_doc)
		enablePatterns(patternList->count() != 0);

	// Restore properties
	fillModeCombo->setCurrentIndex(oldFillModeComboIndex);
	gradientType->setCurrentIndex(oldGradientTypeIndex);
	gradientExtend->setCurrentIndex(oldGradientExtendIndex);
	hatchType->setCurrentIndex(oldHatchTypeIndex);

	strokeModeCombo->setCurrentIndex(oldStrokeModeComboIndex);
	gradientTypeStroke->setCurrentIndex(oldGradientTypeStrokeIndex);
	GradientExtendS->setCurrentIndex(oldGradientExtendSIndex);

	overPrintCombo->setCurrentIndex(oldOverPrintComboIndex);

	// Reconnect signals if necessary
	if (m_item)
		connectSignals();
}


/*********************************************************************
*
* General
*
**********************************************************************/

void ColorPalette::toggleColorDisplay()
{
	if (m_doc)
	{
		colorListStroke->cList = &m_doc->PageColors;
		colorListFill->cList = &m_doc->PageColors;
		colorList = m_doc->PageColors;
		updateColorList();
	}
}

void ColorPalette::showOverprint(int val)
{
	bool sigBlocked = overPrintCombo->blockSignals(true);
	overPrintCombo->setCurrentIndex(val);
	overPrintCombo->blockSignals(sigBlocked);
}

// Gradients

void ColorPalette::updateGradientList()
{
	bool sigBlocked1 = namedGradient->blockSignals(true);
	bool sigBlocked2 = namedGradientStroke->blockSignals(true);

	namedGradient->clear();
	namedGradient->setIconSize(QSize(48, 12));
	namedGradient->addItem( tr("Custom"));
	namedGradientStroke->clear();
	namedGradientStroke->setIconSize(QSize(48, 12));
	namedGradientStroke->addItem( tr("Custom"));
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
		namedGradientStroke->addItem(pm, patK[a]);
	}
	namedGradient->blockSignals(sigBlocked1);
	namedGradientStroke->blockSignals(sigBlocked2);
}

void ColorPalette::setGradients(QHash<QString, VGradient> *docGradients)
{
	gradientList = docGradients;
	updateGradientList();
}


/*********************************************************************
*
* Color
*
**********************************************************************/


void ColorPalette::showColorValues(QString stroke, QString fill, int sShade, int fShade)
{
	bool sigBlocked1 = fillShade->blockSignals(true);
	bool sigBlocked2 = strokeShade->blockSignals(true);
	bool sigBlocked3 = colorListStroke->blockSignals(true);
	bool sigBlocked4 = colorListFill->blockSignals(true);

	strokeShade->setValue(sShade);
	fillShade->setValue(fShade);
	if ((stroke != CommonStrings::None) && (!stroke.isEmpty()))
		colorListStroke->setCurrentColor(stroke);
	else
		colorListStroke->setCurrentRow(0);
	if ((fill != CommonStrings::None) && (!fill.isEmpty()))
		colorListFill->setCurrentColor(fill);
	else
		colorListFill->setCurrentRow(0);

	fillShade->blockSignals(sigBlocked1);
	strokeShade->blockSignals(sigBlocked2);
	colorListStroke->blockSignals(sigBlocked3);
	colorListFill->blockSignals(sigBlocked4);
}

void ColorPalette::selectColorS(int row)
{
	QString colorName;
	QVariant varValue = colorListStroke->data(row, Qt::UserRole);
	if (varValue.canConvert<ColorPixmapValue>())
	{
		ColorPixmapValue value(varValue.value<ColorPixmapValue>());
		colorName = value.m_name;
	}
	else
	{
		colorName = varValue.toString();
	}
	if (colorName.isEmpty())
		return;
	emit NewPen(colorName);
}

void ColorPalette::selectColorF(int row)
{
	QString colorName;
	QVariant varValue = colorListFill->data(row, Qt::UserRole);
	if (varValue.canConvert<ColorPixmapValue>())
	{
		ColorPixmapValue value(varValue.value<ColorPixmapValue>());
		colorName = value.m_name;
	}
	else
	{
		colorName = varValue.toString();
	}
	if (colorName.isEmpty())
		return;
	emit NewBrush(colorName);
}

void ColorPalette::setColors(ColorList newColorList)
{
	colorList.clear();
	colorList = newColorList;
	updateCList();
}

// Color

void ColorPalette::updateColorList()
{
	if (!m_doc)
		return;
	
	if (m_item)
		disconnectSignals();

	this->setColors(m_doc->PageColors);
	this->setGradients(&m_doc->docGradients);
	this->setPatterns(&m_doc->docPatterns);

	if (m_item)
		setCurrentItem(m_item);
}

void ColorPalette::updateCList()
{
	bool sigBlocked1 = colorListStroke->blockSignals(true);
	bool sigBlocked2 = colorListFill->blockSignals(true);

	if (displayAllColors->isChecked())
	{
		if (m_doc)
			m_doc->getUsedColors(colorList);
	}
	colorListFill->setColors(colorList, true);
	colorListStroke->setColors(colorList, true);
	gradEditStroke->setColors(colorList);
	gradEdit->setColors(colorList);
	colorPoint1->setColors(colorList, true);
	colorPoint2->setColors(colorList, true);
	colorPoint3->setColors(colorList, true);
	colorPoint4->setColors(colorList, true);
	colorMeshPoint->setColors(colorList, true);
	colorListFill->clearSelection();
	colorListStroke->clearSelection();
	hatchLineColor->setColors(colorList, false);
	hatchBackground->setColors(colorList, true);

	colorListStroke->blockSignals(sigBlocked1);
	colorListFill->blockSignals(sigBlocked2);
}



/*********************************************************************
*
* Feature Fill
*
**********************************************************************/

void ColorPalette::handleFillShade(double val)
{
	if (m_doc)
	{
		blockUpdates(true);
		m_doc->itemSelection_SetItemBrushShade(static_cast<int>(val));
		blockUpdates(false);
	}
}

/*********************************************************************
*
* Feature Fill Gradient
*
**********************************************************************/

void ColorPalette::handleFillGradient()
{
	if (m_doc)
	{
		VGradient gradient(gradEdit->gradient());
		blockUpdates(true);
		m_doc->updateManager()->setUpdatesDisabled();
		m_doc->itemSelection_SetFillGradient(gradient);
		m_doc->updateManager()->setUpdatesEnabled();
		blockUpdates(false);
	}
}

void ColorPalette::handleOverprint(int val)
{
	if (m_doc)
	{
		bool setter = true;
		if (val == 0)
			setter = false;
		m_doc->itemSelection_SetOverprint(setter);
	}
}

void ColorPalette::handleGradientExtend(int val)
{
	if (m_doc)
	{
		if (val == 0)
			m_item->setGradientExtend(VGradient::none);
		else
			m_item->setGradientExtend(VGradient::pad);
		m_item->update();
		m_doc->regionsChanged()->update(QRect());
	}
}

void ColorPalette::setNamedGradient(const QString &name)
{
	if (namedGradient->currentIndex() == 0)
	{
		gradEdit->setGradient(m_item->fill_gradient);
		m_item->setGradient("");
		gradEdit->setGradientEditable(true);
	}
	else
	{
		gradEdit->setGradient(gradientList->value(name));
		gradEdit->setGradientEditable(false);
		m_item->setGradient(name);
	}
	if (gradientType->currentIndex() == 0)
		emit NewGradient(6);
	else if (gradientType->currentIndex() == 1)
		emit NewGradient(7);
	else if (gradientType->currentIndex() == 2)
		emit NewGradient(13);
	else if (gradientType->currentIndex() == 3)
	{
		setGradientColors();
		emit NewGradient(9);
	}
	else if (gradientType->currentIndex() == 4)
		emit NewGradient(10);
	else if (gradientType->currentIndex() == 5)
		emit NewGradient(11);
}


void ColorPalette::setGradientColors()
{
	QString color1 = colorPoint1->currentText();
	if (color1 == CommonStrings::tr_NoneColor)
		color1 = CommonStrings::None;
	QString color2 = colorPoint2->currentText();
	if (color2 == CommonStrings::tr_NoneColor)
		color2 = CommonStrings::None;
	QString color3 = colorPoint3->currentText();
	if (color3 == CommonStrings::tr_NoneColor)
		color3 = CommonStrings::None;
	QString color4 = colorPoint4->currentText();
	if (color4 == CommonStrings::tr_NoneColor)
		color4 = CommonStrings::None;
	double t1 = color1Alpha->value() / 100.0;
	double t2 = color2Alpha->value() / 100.0;
	double t3 = color3Alpha->value() / 100.0;
	double t4 = color4Alpha->value() / 100.0;
	UndoTransaction trans;
	if (UndoManager::undoEnabled())
		trans = undoManager->beginTransaction(Um::Selection,Um::IFill,Um::GradVal,"",Um::IFill);
	m_item->set4ColorShade(static_cast<int>(color1Shade->value()), static_cast<int>(color2Shade->value()), static_cast<int>(color3Shade->value()), static_cast<int>(color4Shade->value()));
	m_item->set4ColorTransparency(t1, t2, t3, t4);
	m_item->set4ColorColors(color1, color2, color3, color4);
	if (trans)
		trans.commit();
	m_item->update();
}

void ColorPalette::showGradient(int number)
{
	bool sigBlocked = fillModeCombo->blockSignals(true);
	if (number==-1)
	{
		fillModeCombo->setCurrentIndex(0);
	}
	if (number == 0)
		fillModeCombo->setCurrentIndex(0);
	else if (((number > 0) && (number < 8)) || ((number >= 9) && (number <= 13)))
	{
		if ((number == 5) || (number == 7) || (number == 13))
		{
			stackedWidget_2->setCurrentIndex(0);
			if ((number == 5) || (number == 7))
				gradientType->setCurrentIndex(1);
			if (number == 13)
				gradientType->setCurrentIndex(2);
		}
		else if (number == 9)
		{
			stackedWidget_2->setCurrentIndex(1);
			gradientType->setCurrentIndex(3);
			if ((m_item->GrColorP1 != CommonStrings::None) && (!m_item->GrColorP1.isEmpty()))
				setCurrentComboItem(colorPoint1, m_item->GrColorP1);
			else
				colorPoint1->setCurrentIndex(0);
			if ((m_item->GrColorP2 != CommonStrings::None) && (!m_item->GrColorP2.isEmpty()))
				setCurrentComboItem(colorPoint2, m_item->GrColorP2);
			else
				colorPoint2->setCurrentIndex(0);
			if ((m_item->GrColorP3 != CommonStrings::None) && (!m_item->GrColorP3.isEmpty()))
				setCurrentComboItem(colorPoint3, m_item->GrColorP3);
			else
				colorPoint3->setCurrentIndex(0);
			if ((m_item->GrColorP4 != CommonStrings::None) && (!m_item->GrColorP4.isEmpty()))
				setCurrentComboItem(colorPoint4, m_item->GrColorP4);
			else
				colorPoint4->setCurrentIndex(0);
			color1Alpha->setValue(qRound(m_item->GrCol1transp * 100));
			color2Alpha->setValue(qRound(m_item->GrCol2transp * 100));
			color3Alpha->setValue(qRound(m_item->GrCol3transp * 100));
			color4Alpha->setValue(qRound(m_item->GrCol4transp * 100));
			color1Shade->setValue(m_item->GrCol1Shade);
			color2Shade->setValue(m_item->GrCol2Shade);
			color3Shade->setValue(m_item->GrCol3Shade);
			color4Shade->setValue(m_item->GrCol4Shade);
		}
		else if (number == 10)
		{
			stackedWidget_2->setCurrentIndex(0);
			gradientType->setCurrentIndex(4);
		}
		else if (number == 11)
		{
			stackedWidget_2->setCurrentIndex(2);
			if ((m_item->selectedMeshPointX > -1) && (m_item->selectedMeshPointY > -1l))
			{
				meshPoint mp = m_item->meshGradientArray[m_item->selectedMeshPointX][m_item->selectedMeshPointY];
				setCurrentComboItem(colorMeshPoint, mp.colorName);
				shadeMeshPoint->setValue(mp.shade);
				transparencyMeshPoint->setValue(mp.transparency * 100);
			}
			gradientType->setCurrentIndex(5);
		}
		else if (number == 12)
		{
			stackedWidget_2->setCurrentIndex(2);
			gradientType->setCurrentIndex(6);
		}
		else
		{
			stackedWidget_2->setCurrentIndex(0);
			gradientType->setCurrentIndex(0);
		}
		fillModeCombo->setCurrentIndex(1);
		if (m_item->getGradientExtend() == VGradient::none)
			gradientExtend->setCurrentIndex(0);
		else
			gradientExtend->setCurrentIndex(1);
	}
	else if (number == 14)
	{
		fillModeCombo->setCurrentIndex(2);
		hatchAngle->setValue(m_item->hatchAngle);
		hatchDist->setValue(m_item->hatchDistance * unitGetRatioFromIndex(m_unitIndex));
		hatchType->setCurrentIndex(m_item->hatchType);
		if ((m_item->hatchForeground != CommonStrings::None) && (!m_item->hatchForeground.isEmpty()))
			setCurrentComboItem(hatchLineColor, m_item->hatchForeground);
		if (m_item->hatchUseBackground)
		{
			if (!m_item->hatchBackground.isEmpty())
				setCurrentComboItem(hatchBackground, m_item->hatchBackground);
		}
		else
			setCurrentComboItem(hatchBackground, CommonStrings::None);
	}
	else
	{
		if (patternList->count() == 0)
		{
			fillModeCombo->setCurrentIndex(0);
			emit NewGradient(0);
		}
		else
			fillModeCombo->setCurrentIndex(3);
	}
	fillModeStack->setCurrentIndex( fillModeCombo->currentIndex() );
	fillModeCombo->blockSignals(sigBlocked);
}

void ColorPalette::slotGrad(int number)
{
	if (number == 1)
	{
		bool sigBlocked1 = gradEdit->blockSignals(true);
		bool sigBlocked2 = gradientType->blockSignals(true);
		bool sigBlocked3 = namedGradient->blockSignals(true);
		if (!m_item->gradient().isEmpty())
		{
			setCurrentComboItem(namedGradient, m_item->gradient());
			gradEdit->setGradient(gradientList->value(m_item->gradient()));
			gradEdit->setGradientEditable(false);
		}
		else
		{
			namedGradient->setCurrentIndex(0);
			gradEdit->setGradient(m_item->fill_gradient);
			gradEdit->setGradientEditable(true);
		}
		if (gradientType->currentIndex() == 0)
		{
			stackedWidget_2->setCurrentIndex(0);
			emit NewGradient(6);
		}
		else if (gradientType->currentIndex() == 1)
		{
			stackedWidget_2->setCurrentIndex(0);
			emit NewGradient(7);
		}
		else if (gradientType->currentIndex() == 2)
		{
			stackedWidget_2->setCurrentIndex(0);
			emit NewGradient(13);
		}
		else if (gradientType->currentIndex() == 3)
		{
			stackedWidget_2->setCurrentIndex(1);
			if ((m_item->GrColorP1 != CommonStrings::None) && (!m_item->GrColorP1.isEmpty()))
				setCurrentComboItem(colorPoint1, m_item->GrColorP1);
			else
				colorPoint1->setCurrentIndex(0);
			if ((m_item->GrColorP2 != CommonStrings::None) && (!m_item->GrColorP2.isEmpty()))
				setCurrentComboItem(colorPoint2, m_item->GrColorP2);
			else
				colorPoint2->setCurrentIndex(0);
			if ((m_item->GrColorP3 != CommonStrings::None) && (!m_item->GrColorP3.isEmpty()))
				setCurrentComboItem(colorPoint3, m_item->GrColorP3);
			else
				colorPoint3->setCurrentIndex(0);
			if ((m_item->GrColorP4 != CommonStrings::None) && (!m_item->GrColorP4.isEmpty()))
				setCurrentComboItem(colorPoint4, m_item->GrColorP4);
			else
				colorPoint4->setCurrentIndex(0);
			color1Alpha->setValue(qRound(m_item->GrCol1transp * 100));
			color2Alpha->setValue(qRound(m_item->GrCol2transp * 100));
			color3Alpha->setValue(qRound(m_item->GrCol3transp * 100));
			color4Alpha->setValue(qRound(m_item->GrCol4transp * 100));
			color1Shade->setValue(m_item->GrCol1Shade);
			color2Shade->setValue(m_item->GrCol2Shade);
			color3Shade->setValue(m_item->GrCol3Shade);
			color4Shade->setValue(m_item->GrCol4Shade);
			emit NewGradient(9);
		}
		else if (gradientType->currentIndex() == 4)
		{
			stackedWidget_2->setCurrentIndex(0);
			emit NewGradient(10);
		}
		else if (gradientType->currentIndex() == 5)
		{
			stackedWidget_2->setCurrentIndex(2);
			if ((m_item->selectedMeshPointX > -1) && (m_item->selectedMeshPointY > -1l))
			{
				meshPoint mp = m_item->meshGradientArray[m_item->selectedMeshPointX][m_item->selectedMeshPointY];
				setCurrentComboItem(colorMeshPoint, mp.colorName);
				shadeMeshPoint->setValue(mp.shade);
				transparencyMeshPoint->setValue(mp.transparency * 100);
			}
			emit NewGradient(11);
		}
		else if (gradientType->currentIndex() == 6)
		{
			stackedWidget_2->setCurrentIndex(2);
			emit NewGradient(12);
		}
		gradEdit->blockSignals(sigBlocked1);
		gradientType->blockSignals(sigBlocked2);
		namedGradient->blockSignals(sigBlocked3);
	}
	else if (number == 2)
	{
		hatchAngle->setValue(m_item->hatchAngle);
		hatchDist->setValue(m_item->hatchDistance * unitGetRatioFromIndex(m_unitIndex));
		hatchType->setCurrentIndex(m_item->hatchType);
		if ((m_item->hatchForeground != CommonStrings::None) && (!m_item->hatchForeground.isEmpty()))
			setCurrentComboItem(hatchLineColor, m_item->hatchForeground);
		if (m_item->hatchUseBackground)
		{
			if (!m_item->hatchBackground.isEmpty())
				setCurrentComboItem(hatchBackground, m_item->hatchBackground);
		}
		else
			setCurrentComboItem(hatchBackground, CommonStrings::None);
		emit NewGradient(14);
	}
	else if (number == 3)
		emit NewGradient(8);
	else
		emit NewGradient(0);
	fillModeStack->setCurrentIndex(number);
}

void ColorPalette::slotGradType(int type)
{
	if (type == 0)
	{
		stackedWidget_2->setCurrentIndex(0);
		emit NewGradient(6);
	}
	else if (type == 1)
	{
		stackedWidget_2->setCurrentIndex(0);
		emit NewGradient(7);
	}
	else if (type == 2)
	{
		stackedWidget_2->setCurrentIndex(0);
		emit NewGradient(13);
	}
	else if (type == 3)
	{
		stackedWidget_2->setCurrentIndex(1);
		if ((m_item->GrColorP1 != CommonStrings::None) && (!m_item->GrColorP1.isEmpty()))
			setCurrentComboItem(colorPoint1, m_item->GrColorP1);
		else
			colorPoint1->setCurrentIndex(0);
		if ((m_item->GrColorP2 != CommonStrings::None) && (!m_item->GrColorP2.isEmpty()))
			setCurrentComboItem(colorPoint2, m_item->GrColorP2);
		else
			colorPoint2->setCurrentIndex(0);
		if ((m_item->GrColorP3 != CommonStrings::None) && (!m_item->GrColorP3.isEmpty()))
			setCurrentComboItem(colorPoint3, m_item->GrColorP3);
		else
			colorPoint3->setCurrentIndex(0);
		if ((m_item->GrColorP4 != CommonStrings::None) && (!m_item->GrColorP4.isEmpty()))
			setCurrentComboItem(colorPoint4, m_item->GrColorP4);
		else
			colorPoint4->setCurrentIndex(0);
		color1Alpha->setValue(qRound(m_item->GrCol1transp * 100));
		color2Alpha->setValue(qRound(m_item->GrCol2transp * 100));
		color3Alpha->setValue(qRound(m_item->GrCol3transp * 100));
		color4Alpha->setValue(qRound(m_item->GrCol4transp * 100));
		color1Shade->setValue(m_item->GrCol1Shade);
		color2Shade->setValue(m_item->GrCol2Shade);
		color3Shade->setValue(m_item->GrCol3Shade);
		color4Shade->setValue(m_item->GrCol4Shade);
		emit NewGradient(9);
	}
	else if (type == 4)
	{
		stackedWidget_2->setCurrentIndex(0);
		emit NewGradient(10);
	}
	else if (type == 5)
	{
		stackedWidget_2->setCurrentIndex(2);
		if ((m_item->selectedMeshPointX > -1) && (m_item->selectedMeshPointY > -1l))
		{
			meshPoint mp = m_item->meshGradientArray[m_item->selectedMeshPointX][m_item->selectedMeshPointY];
			setCurrentComboItem(colorMeshPoint, mp.colorName);
			shadeMeshPoint->setValue(mp.shade);
			transparencyMeshPoint->setValue(mp.transparency * 100);
		}
		emit NewGradient(11);
	}
	else if (type == 6)
	{
		stackedWidget_2->setCurrentIndex(2);
		emit NewGradient(12);
	}
}

void ColorPalette::updateColorSpecialGradient()
{
//	if (!m_haveDoc)
//		return;
	if(m_doc->m_Selection->isEmpty())
		return;

	PageItem *currItem=m_doc->m_Selection->itemAt(0);
	if (currItem)
	{
		if (m_ScMW->view->editStrokeGradient == 0)
			this->setSpecialGradient(currItem->GrStartX, currItem->GrStartY, currItem->GrEndX, currItem->GrEndY, currItem->GrFocalX, currItem->GrFocalY, currItem->GrScale, currItem->GrSkew, 0, 0);
		else if (m_ScMW->view->editStrokeGradient == 1)
			this->setSpecialGradient(currItem->GrStrokeStartX, currItem->GrStrokeStartY, currItem->GrStrokeEndX, currItem->GrStrokeEndY, currItem->GrStrokeFocalX, currItem->GrStrokeFocalY, currItem->GrStrokeScale, currItem->GrStrokeSkew, 0, 0);
		else if (m_ScMW->view->editStrokeGradient == 3)
			this->setSpecialGradient(currItem->GrControl1.x(), currItem->GrControl1.y(), currItem->GrControl2.x(), currItem->GrControl2.y(), currItem->GrControl3.x(), currItem->GrControl3.y(), currItem->GrControl4.x(), currItem->GrControl4.y(), 0, 0);
		else if (m_ScMW->view->editStrokeGradient == 4)
			this->setSpecialGradient(currItem->GrControl1.x(), currItem->GrControl1.y(), currItem->GrControl2.x(), currItem->GrControl2.y(), currItem->GrControl3.x(), currItem->GrControl3.y(), currItem->GrControl4.x(), currItem->GrControl4.y(), currItem->GrControl5.x(), currItem->GrControl5.y());
		else if ((m_ScMW->view->editStrokeGradient == 5) || (m_ScMW->view->editStrokeGradient == 6))
			this->setMeshPoint();
		else if (m_ScMW->view->editStrokeGradient == 8)
			this->setMeshPatchPoint();
		else if (m_ScMW->view->editStrokeGradient == 9)
			this->setMeshPatch();
	}
}

void ColorPalette::NewSpGradient(double x1, double y1, double x2, double y2, double fx, double fy, double sg, double sk, double cx, double cy)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		QRectF upRect;
		UndoTransaction trans;
		if (UndoManager::undoEnabled())
			trans = undoManager->beginTransaction(Um::Selection, Um::ILine, Um::GradPos + "p", "", Um::ILine);
		if (m_ScMW->view->editStrokeGradient == 1)
		{
			m_item->setGradientStrokeStartX(x1 / m_unitRatio);
			m_item->setGradientStrokeStartY(y1 / m_unitRatio);
			m_item->setGradientStrokeEndX(x2 / m_unitRatio);
			m_item->setGradientStrokeEndY(y2 / m_unitRatio);
			m_item->setGradientStrokeFocalX(fx / m_unitRatio);
			m_item->setGradientStrokeFocalY(fy / m_unitRatio);
			m_item->setGradientStrokeScale(sg);
			m_item->setGradientStrokeSkew(sk);
			if (m_item->strokeGradientType() == 6)
			{
				m_item->setGradientStrokeFocalX(m_item->gradientStrokeStartX());
				m_item->setGradientStrokeFocalY(m_item->gradientStrokeStartY());
			}
			m_item->update();
			upRect = QRectF(QPointF(m_item->gradientStrokeStartX(), m_item->gradientStrokeStartY()), QPointF(m_item->gradientStrokeEndX(), m_item->gradientStrokeEndY()));
			double radEnd = distance(m_item->gradientStrokeEndX() - m_item->gradientStrokeStartX(), m_item->gradientStrokeEndY() - m_item->gradientStrokeStartY());
			double rotEnd = xy2Deg(m_item->gradientStrokeEndX() - m_item->gradientStrokeStartX(), m_item->gradientStrokeEndY() - m_item->gradientStrokeStartY());
			QTransform m;
			m.translate(m_item->gradientStrokeStartX(), m_item->gradientStrokeStartY());
			m.rotate(rotEnd);
			m.rotate(-90);
			m.rotate(m_item->gradientStrokeSkew());
			m.translate(radEnd * m_item->gradientStrokeScale(), 0);
			QPointF shP = m.map(QPointF(0,0));
			upRect = upRect.united(QRectF(shP, QPointF(m_item->gradientStrokeEndX(), m_item->gradientStrokeEndY())).normalized());
			upRect = upRect.united(QRectF(shP, QPointF(m_item->gradientStrokeStartX(), m_item->gradientStrokeStartY())).normalized());
			upRect |= QRectF(shP, QPointF(0, 0)).normalized();
			upRect |= QRectF(shP, QPointF(m_item->width(), m_item->height())).normalized();
		}
		else if (m_ScMW->view->editStrokeGradient == 3)
		{
			m_item->setGradientControl1(FPoint(x1 / m_unitRatio, y1 / m_unitRatio));
			m_item->setGradientControl2(FPoint(x2 / m_unitRatio, y2 / m_unitRatio));
			m_item->setGradientControl3(FPoint(fx / m_unitRatio, fy / m_unitRatio));
			m_item->setGradientControl4(FPoint(sg / m_unitRatio, sk / m_unitRatio));
			m_item->update();
			upRect = QRectF(QPointF(-m_item->width(), -m_item->height()), QPointF(m_item->width() * 2, m_item->height() * 2)).normalized();
		}
		else if (m_ScMW->view->editStrokeGradient == 4)
		{
			m_item->setGradientControl1(FPoint(x1 / m_unitRatio, y1 / m_unitRatio));
			m_item->setGradientControl2(FPoint(x2 / m_unitRatio, y2 / m_unitRatio));
			m_item->setGradientControl3(FPoint(fx / m_unitRatio, fy / m_unitRatio));
			m_item->setGradientControl4(FPoint(sg / m_unitRatio, sk / m_unitRatio));
			m_item->setGradientControl5(FPoint(cx / m_unitRatio, cy / m_unitRatio));
			m_item->update();
			upRect = QRectF(QPointF(-m_item->width(), -m_item->height()), QPointF(m_item->width() * 2, m_item->height() * 2)).normalized();
		}
		else
		{
			if (m_item->gradientType() == 13 && UndoManager::undoEnabled())
			{
				SimpleState *ss= new SimpleState("Refresh");
				ss->set("UNDO_UPDATE_CONICAL");
				undoManager->action(m_item,ss);
			}
			m_item->setGradientStartX(x1 / m_unitRatio);
			m_item->setGradientStartY(y1 / m_unitRatio);
			m_item->setGradientEndX(x2 / m_unitRatio);
			m_item->setGradientEndY(y2 / m_unitRatio);
			m_item->setGradientFocalX(fx / m_unitRatio);
			m_item->setGradientFocalY(fy / m_unitRatio);
			m_item->setGradientScale(sg);
			m_item->setGradientSkew(sk);
			if (m_item->strokeGradientType() == 6)
			{
				m_item->setGradientFocalX(m_item->gradientStartX());
				m_item->setGradientFocalY(m_item->gradientStartY());
			}
			if (m_item->gradientType() == 13 && UndoManager::undoEnabled())
			{
				m_item->createConicalMesh();
				SimpleState *ss= new SimpleState("Refresh");
				ss->set("REDO_UPDATE_CONICAL");
				undoManager->action(m_item,ss);
			}
			m_item->update();
			upRect = QRectF(QPointF(m_item->gradientStartX(), m_item->gradientStartY()), QPointF(m_item->gradientEndX(), m_item->gradientEndY()));
			double radEnd = distance(m_item->gradientEndX() - m_item->gradientStartX(), m_item->gradientEndY() - m_item->gradientStartY());
			double rotEnd = xy2Deg(m_item->gradientEndX() - m_item->gradientStartX(), m_item->gradientEndY() - m_item->gradientStartY());
			QTransform m;
			m.translate(m_item->gradientStartX(), m_item->gradientStartY());
			m.rotate(rotEnd);
			m.rotate(-90);
			m.rotate(m_item->gradientSkew());
			m.translate(radEnd * m_item->gradientScale(), 0);
			QPointF shP = m.map(QPointF(0,0));
			upRect |= QRectF(shP, QPointF(m_item->gradientEndX(), m_item->gradientEndY())).normalized();
			upRect |= QRectF(shP, QPointF(m_item->gradientStartX(), m_item->gradientStartY())).normalized();
			upRect |= QRectF(shP, QPointF(0, 0)).normalized();
			upRect |= QRectF(shP, QPointF(m_item->width(), m_item->height())).normalized();
		}
		if (trans)
			trans.commit();
		upRect.translate(m_item->xPos(), m_item->yPos());
		m_doc->regionsChanged()->update(upRect.adjusted(-10.0, -10.0, 10.0, 10.0));
		m_doc->changed();
	}
}

void ColorPalette::toggleGradientEdit(int stroke)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		m_ScMW->view->editStrokeGradient = stroke;
		if (stroke == 1)
		{
			if (this->gradEditButtonStroke->isChecked())
				m_ScMW->view->requestMode(modeEditGradientVectors);
			else
				m_ScMW->view->requestMode(modeNormal);
		}
		else
		{
			if ((this->gradEditButton->isChecked()) || (this->editMeshColors->isChecked()))
			{
				if ((stroke == 5) || (stroke == 6) || (stroke == 7))
					m_ScMW->view->requestMode(modeEditMeshGradient);
				else if ((stroke == 8) || (stroke == 9) || (stroke == 10) || (stroke == 11))
					m_ScMW->view->requestMode(modeEditMeshPatch);
				else
					m_ScMW->view->requestMode(modeEditGradientVectors);
			}
			else
				m_ScMW->view->requestMode(modeNormal);
		}
		m_ScMW->view->RefreshGradient(m_item);
	}
}

/*********************************************************************
*
* Feature Stroke
*
**********************************************************************/

void ColorPalette::handleStrokeShade(double val)
{
	if (m_doc)
	{
		blockUpdates(true);
		m_doc->itemSelection_SetItemPenShade(static_cast<int>(val));
		blockUpdates(false);
	}
}



/*********************************************************************
*
* Feature Stroke Gradient
*
**********************************************************************/

void ColorPalette::handleStrokeGradient()
{
	if (m_doc)
	{
		VGradient gradient(gradEditStroke->gradient());
		blockUpdates(true);
		m_doc->updateManager()->setUpdatesDisabled();
		m_doc->itemSelection_SetLineGradient(gradient);
		m_doc->updateManager()->setUpdatesEnabled();
		blockUpdates(false);
	}
}

void ColorPalette::handleStrokeGradientExtend(int val)
{
	if (m_doc)
	{
		if (val == 0)
			m_item->setStrokeGradientExtend(VGradient::none);
		else
			m_item->setStrokeGradientExtend(VGradient::pad);
		m_item->update();
		m_doc->regionsChanged()->update(QRect());
	}
}

void ColorPalette::setNamedGradientStroke(const QString &name)
{
	if (namedGradientStroke->currentIndex() == 0)
	{
		gradEditStroke->setGradient(m_item->stroke_gradient);
		m_item->setStrokeGradient("");
		gradEditStroke->setGradientEditable(true);
	}
	else
	{
		gradEditStroke->setGradient(gradientList->value(name));
		gradEditStroke->setGradientEditable(false);
		m_item->setStrokeGradient(name);
	}
	if (gradientTypeStroke->currentIndex() == 0)
		emit NewGradientS(6);
	else
		emit NewGradientS(7);
}

void ColorPalette::slotGradStroke(int number)
{
	if (number == 1)
	{
		bool sigBlocked = namedGradientStroke->blockSignals(true);
		if (!m_item->strokeGradient().isEmpty())
		{
			setCurrentComboItem(namedGradientStroke, m_item->strokeGradient());
			gradEditStroke->setGradient(gradientList->value(m_item->strokeGradient()));
			gradEditStroke->setGradientEditable(false);
		}
		else
		{
			namedGradientStroke->setCurrentIndex(0);
			gradEditStroke->setGradient(m_item->stroke_gradient);
			gradEditStroke->setGradientEditable(true);
		}
		emit NewPatternS("");
		if (gradientTypeStroke->currentIndex() == 0)
			emit NewGradientS(6);
		else
			emit NewGradientS(7);
		namedGradientStroke->blockSignals(sigBlocked);
	}
	else if (number == 2)
	{
		emit NewGradientS(8);
		if (patternBoxStroke->currentItem())
			emit NewPatternS(patternBoxStroke->currentItem()->text());
	}
	else
	{
		emit NewGradientS(0);
		emit NewPatternS("");
	}
	strokeModeStack->setCurrentIndex(number);
}

void ColorPalette::showGradientStroke(int number)
{
	bool sigBlocked = strokeModeCombo->blockSignals(true);
	if (number==-1 || number == 0)
		strokeModeCombo->setCurrentIndex(0);
	else if (number > 0 && number < 8)
	{
		if (number == 7)
			gradientTypeStroke->setCurrentIndex(1);
		else
			gradientTypeStroke->setCurrentIndex(0);
		strokeModeCombo->setCurrentIndex(1);
		if (m_item->getStrokeGradientExtend() == VGradient::none)
			GradientExtendS->setCurrentIndex(0);
		else
			GradientExtendS->setCurrentIndex(1);
	}
	else
	{
		if (patternList->count() == 0)
		{
			strokeModeCombo->setCurrentIndex(0);
			emit NewGradient(0);
		}
		else
			strokeModeCombo->setCurrentIndex(2);
	}
	strokeModeStack->setCurrentIndex( strokeModeCombo->currentIndex() );
	strokeModeCombo->blockSignals(sigBlocked);
}

void ColorPalette::slotGradTypeStroke(int type)
{
	if (type == 0)
		emit NewGradientS(6);
	else
		emit NewGradientS(7);
}

/*********************************************************************
*
* Feature Pattern
*
**********************************************************************/

void ColorPalette::updatePatternList()
{
	disconnect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
	patternBox->clear();
	patternBox->setIconSize(QSize(48, 48));
	patternBoxStroke->clear();
	patternBoxStroke->setIconSize(QSize(48, 48));
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
		QListWidgetItem *itemS = new QListWidgetItem(pm2, patK[a], patternBoxStroke);
		itemS->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
	patternBox->clearSelection();
	patternBoxStroke->clearSelection();
	connect(patternBox, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
}


void ColorPalette::hideEditedPatterns(QStringList names)
{
	for (int a = 0; a < names.count(); a++)
	{
		QList<QListWidgetItem*> items = patternBox->findItems(names[a], Qt::MatchExactly);
		if (items.count() > 0)
			items[0]->setHidden(true);
		items = patternBoxStroke->findItems(names[a], Qt::MatchExactly);
		if (items.count() > 0)
			items[0]->setHidden(true);
	}
}

void ColorPalette::setPatterns(QHash<QString, ScPattern> *docPatterns)
{
	patternList = docPatterns;
	updatePatternList();
}

void ColorPalette::selectPattern(QListWidgetItem *c)
{
	if (c == NULL)
		return;
	emit NewPattern(c->text());
}

void ColorPalette::selectPatternS(QListWidgetItem *c)
{
	if (c == NULL)
		return;
	emit NewPatternS(c->text());
}

void ColorPalette::setActPatternStroke(QString pattern, double scaleX, double scaleY, double offsetX, double offsetY, double rotation, double skewX, double skewY, bool mirrorX, bool mirrorY, double space, bool pathF)
{
	bool sigBlocked = patternBoxStroke->blockSignals(true);
	QList<QListWidgetItem*> itl = patternBoxStroke->findItems(pattern, Qt::MatchExactly);
	if (itl.count() != 0)
	{
		QListWidgetItem *it = itl[0];
		patternBoxStroke->setCurrentItem(it);
	}
	else
		patternBoxStroke->clearSelection();
	m_Pattern_scaleXS = scaleX;
	m_Pattern_scaleYS = scaleX;
	m_Pattern_offsetXS = offsetX;
	m_Pattern_offsetYS = offsetY;
	m_Pattern_rotationS = rotation;
	m_Pattern_skewXS = skewX;
	m_Pattern_skewYS = skewY;
	m_Pattern_mirrorXS = mirrorX;
	m_Pattern_mirrorYS = mirrorY;
	m_Pattern_spaceS = space;
	followsPath->setChecked(pathF);
	patternBoxStroke->blockSignals(sigBlocked);
}

void ColorPalette::setActPattern(QString pattern, double scaleX, double scaleY, double offsetX, double offsetY, double rotation, double skewX, double skewY, bool mirrorX, bool mirrorY)
{
	bool sigBlocked = patternBox->blockSignals(true);
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
	patternBox->blockSignals(sigBlocked);
}

void ColorPalette::changePatternProps()
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
	double skewX = dia->spinXSkew->value();
	double a;
	if (skewX == 90)
		a = 1;
	else if (skewX == 180)
		a = 0;
	else if (skewX == 270)
		a = -1;
	else if (skewX == 360)
		a = 0;
	else
		a = tan(M_PI / 180.0 * skewX);
	m_Pattern_skewX = tan(a);
	skewX = dia->spinYSkew->value();
	if (skewX == 90)
		a = 1;
	else if (skewX == 180)
		a = 0;
	else if (skewX == 270)
		a = -1;
	else if (skewX == 360)
		a = 0;
	else
		a = tan(M_PI / 180.0 * skewX);
	m_Pattern_skewY = tan(a);
	m_Pattern_mirrorX = dia->FlipH->isChecked();
	m_Pattern_mirrorY = dia->FlipV->isChecked();
	delete dia;
	fillModeCombo->setCurrentIndex(3);
	emit NewGradient(8);
}

void ColorPalette::changePatternPropsStroke()
{
	PatternPropsDialog *dia = new PatternPropsDialog(this, m_unitIndex, true);
	dia->spinXscaling->setValue(m_Pattern_scaleXS);
	dia->spinYscaling->setValue(m_Pattern_scaleYS);
	if (m_Pattern_scaleXS == m_Pattern_scaleYS)
		dia->keepScaleRatio->setChecked(true);
	dia->spinXoffset->setValue(m_Pattern_offsetXS);
	dia->spinYoffset->setValue(m_Pattern_offsetYS);
	dia->spinAngle->setValue(m_Pattern_rotationS);
	dia->spinSpacing->setValue(m_Pattern_spaceS * 100.0);
	double asina = atan(m_Pattern_skewXS);
	dia->spinXSkew->setValue(asina / (M_PI / 180.0));
	double asinb = atan(m_Pattern_skewYS);
	dia->spinYSkew->setValue(asinb / (M_PI / 180.0));
	dia->FlipH->setChecked(m_Pattern_mirrorXS);
	dia->FlipV->setChecked(m_Pattern_mirrorYS);
	connect(dia, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)));
	dia->exec();
	m_Pattern_scaleXS = dia->spinXscaling->value();
	m_Pattern_scaleYS = dia->spinYscaling->value();
	m_Pattern_offsetXS = dia->spinXoffset->value();
	m_Pattern_offsetYS = dia->spinYoffset->value();
	m_Pattern_rotationS = dia->spinAngle->value();
	double skewX = dia->spinXSkew->value();
	double a;
	if (skewX == 90)
		a = 1;
	else if (skewX == 180)
		a = 0;
	else if (skewX == 270)
		a = -1;
	else if (skewX == 360)
		a = 0;
	else
		a = tan(M_PI / 180.0 * skewX);
	m_Pattern_skewXS = tan(a);
	skewX = dia->spinYSkew->value();
	if (skewX == 90)
		a = 1;
	else if (skewX == 180)
		a = 0;
	else if (skewX == 270)
		a = -1;
	else if (skewX == 360)
		a = 0;
	else
		a = tan(M_PI / 180.0 * skewX);
	m_Pattern_skewYS = tan(a);
	m_Pattern_spaceS = dia->spinSpacing->value() / 100.0;
	m_Pattern_mirrorXS = dia->FlipH->isChecked();
	m_Pattern_mirrorYS = dia->FlipV->isChecked();
	delete dia;
}

void ColorPalette::changeHatchProps()
{
	QString color1 = hatchLineColor->currentText();
	if (color1 == CommonStrings::tr_NoneColor)
		color1 = CommonStrings::None;
	QString color2 = hatchBackground->currentText();
	if (color2 == CommonStrings::tr_NoneColor)
		color2 = CommonStrings::None;
	bool useB = (color2 != CommonStrings::None);
	double angle = hatchAngle->value();
	double dist = hatchDist->value() / unitGetRatioFromIndex(m_unitIndex);
	m_item->setHatchParameters(hatchType->currentIndex(), dist, angle, useB, color2, color1);
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void ColorPalette::toggleStrokePattern()
{
	emit NewPatternTypeS(followsPath->isChecked());
}




void ColorPalette::fillStrokeSelector(int /*index*/)
{
	if (gradEditButton->isChecked() || editMeshColors->isChecked())
	{
		editStrokeGradient = 0;
		CGradDia->hide();
		editMeshColors->setEnabled(true);
		editMeshColors->setChecked(false);
		gradEditButton->setEnabled(true);
		gradEditButton->setChecked(false);
		emit editGradient(editStrokeGradient);
	}
	updateFromItem();
}

void ColorPalette::enablePatterns(bool enable)
{
	if (enable)
	{
		if (fillModeCombo->count() < 4)
			fillModeCombo->addItem( tr("Pattern") );
		if (strokeModeCombo->count() < 3)
			strokeModeCombo->addItem( tr("Pattern") );
	}
	else
	{
		if (fillModeCombo->count() == 4)
			fillModeCombo->removeItem(3);
		if (strokeModeCombo->count() == 3)
			strokeModeCombo->removeItem(2);
	}
}

/*void Cpalette::editLineColorSelectorButton()
{
	if (editLineColorSelector->isChecked())
	{
		stackedWidget->setCurrentIndex(0);
		editFillColorSelector->setChecked(false);
	}
	updateFromItem();
}

void Cpalette::editFillColorSelectorButton()
{
	if (editFillColorSelector->isChecked())
	{
		stackedWidget->setCurrentIndex(1);
		editLineColorSelector->setChecked(false);
	}
	updateFromItem();
}*/



/*********************************************************************
*
* Feature Mesh Gradient
*
**********************************************************************/


void ColorPalette::editMeshPointColor()
{
	if (editMeshColors->isChecked())
	{
		if (m_item->gradientType() == 11)
			editStrokeGradient = 6;
		else if (m_item->gradientType() == 12)
			editStrokeGradient = 8;
		else
			editStrokeGradient = 0;
		gradEditButton->setEnabled(false);
	}
	else
	{
		editStrokeGradient = 0;
		gradEditButton->setEnabled(true);
	}
	gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	emit editGradient(editStrokeGradient);
}

void ColorPalette::createNewMeshGradient()
{
	InsertTable* dia = new InsertTable(this, 255, 255);
	dia->setWindowTitle( tr( "Create Mesh" ) );
	if (dia->exec())
	{
		m_item->createGradientMesh(dia->Rows->value(), dia->Cols->value());
		m_item->update();
		m_doc->regionsChanged()->update(QRect());
	}
	delete dia;
}

void ColorPalette::resetMeshGradient()
{
	m_item->resetGradientMesh();
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void ColorPalette::meshGradientToShape()
{
	m_item->meshToShape();
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void ColorPalette::updateMeshPoint()
{
	QString color = colorMeshPoint->currentText();
	if (color == CommonStrings::tr_NoneColor)
		color = CommonStrings::None;
	double t = transparencyMeshPoint->value() / 100.0;
	m_item->setMeshPointColor(m_item->selectedMeshPointX, m_item->selectedMeshPointY, color, static_cast<int>(shadeMeshPoint->value()), t, m_doc->view()->editStrokeGradient == 8);
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void ColorPalette::setMeshPatchPoint()
{
	if ((m_item->selectedMeshPointX > -1) && (m_item->selectedMeshPointY > 0))
	{
		colorMeshPoint->setEnabled(true);
		shadeMeshPoint->setEnabled(true);
		transparencyMeshPoint->setEnabled(true);
		meshGradientPatch patch = m_item->meshGradientPatches[m_item->selectedMeshPointX];
		meshPoint mp;
		switch (m_item->selectedMeshPointY)
		{
			case 1:
				mp = patch.TL;
				break;
			case 2:
				mp = patch.TR;
				break;
			case 3:
				mp = patch.BR;
				break;
			case 4:
				mp = patch.BL;
				break;
		}
		setCurrentComboItem(colorMeshPoint, mp.colorName);
		shadeMeshPoint->setValue(mp.shade);
		transparencyMeshPoint->setValue(mp.transparency * 100);
	}
	else
	{
		colorMeshPoint->setEnabled(false);
		shadeMeshPoint->setEnabled(false);
		transparencyMeshPoint->setEnabled(false);
	}
}

void ColorPalette::setMeshPatch()
{
	CGradDia->changebuttonRemovePatch((m_item->selectedMeshPointX > -1) && (m_item->meshGradientPatches.count() > 1));
}


void ColorPalette::resetOneControlPoint()
{
	int grow = m_item->selectedMeshPointX;
	int gcol = m_item->selectedMeshPointY;
	int cont = m_item->selectedMeshControlPoint;
	meshPoint tmp;
	if (m_item->gradientType() == 12)
	{
		if ((grow == -1) || (gcol == 0))
			return;
		ScItemState<QPair<meshPoint,meshPoint> > *ss = NULL;
		if(UndoManager::undoEnabled())
		{
			ss = new ScItemState<QPair<meshPoint,meshPoint> >(Um::GradPos);
			ss->set("MOVE_MESH_PATCH");
			ss->set("ARRAY", false);
			ss->set("X", grow);
			ss->set("Y", gcol);
		}
		switch (gcol)
		{
			case 1:
				tmp = m_item->meshGradientPatches[grow].TL;
				if (cont == 2)
					tmp.controlBottom = tmp.gridPoint;
				if (cont == 4)
					tmp.controlRight  = tmp.gridPoint;
				if(UndoManager::undoEnabled())
					ss->setItem(qMakePair(m_item->meshGradientPatches[grow].TL,tmp));
				m_item->meshGradientPatches[grow].TL = tmp;
				break;
			case 2:
				tmp = m_item->meshGradientPatches[grow].TR;
				if (cont == 2)
					tmp.controlBottom = tmp.gridPoint;
				if (cont == 3)
					tmp.controlLeft   = tmp.gridPoint;
				if(UndoManager::undoEnabled())
					ss->setItem(qMakePair(m_item->meshGradientPatches[grow].TR,tmp));
				m_item->meshGradientPatches[grow].TR = tmp;
				break;
			case 3:
				tmp = m_item->meshGradientPatches[grow].BR;
				if (cont == 1)
					tmp.controlTop  = tmp.gridPoint;
				if (cont == 3)
					tmp.controlLeft = tmp.gridPoint;
				if(UndoManager::undoEnabled())
					ss->setItem(qMakePair(m_item->meshGradientPatches[grow].BR,tmp));
				m_item->meshGradientPatches[grow].BR = tmp;
				break;
			case 4:
				tmp = m_item->meshGradientPatches[grow].BL;
				if (cont == 1)
					tmp.controlTop   = tmp.gridPoint;
				if (cont == 4)
					tmp.controlRight = tmp.gridPoint;
				if(UndoManager::undoEnabled())
					ss->setItem(qMakePair(m_item->meshGradientPatches[grow].BL,tmp));
				m_item->meshGradientPatches[grow].BL = tmp;
				break;
		}
		if(UndoManager::undoEnabled())
			undoManager->action(m_item,ss);
	}
	else
	{
		if ((grow == -1) || (gcol == -1))
			return;

		tmp = m_item->meshGradientArray[grow][gcol];
		if (cont == 1)
			tmp.controlTop    = tmp.gridPoint;
		else if (cont == 2)
			tmp.controlBottom = tmp.gridPoint;
		else if (cont == 3)
			tmp.controlLeft   = tmp.gridPoint;
		else if (cont == 4)
			tmp.controlRight  = tmp.gridPoint;

		if(UndoManager::undoEnabled())
		{
			ScItemState<QPair<meshPoint,meshPoint> > *ss = new ScItemState<QPair<meshPoint,meshPoint> >(Um::GradPos);
			ss->set("MOVE_MESH_PATCH");
			ss->set("ARRAY", true);
			ss->set("X", grow);
			ss->set("Y", gcol);
			ss->setItem(qMakePair(m_item->meshGradientArray[grow][gcol],tmp));
			undoManager->action(m_item,ss);
		}
		m_item->meshGradientArray[grow][gcol] = tmp;
	}
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void ColorPalette::resetAllControlPoints()
{
	int grow = m_item->selectedMeshPointX;
	int gcol = m_item->selectedMeshPointY;
	meshPoint tmp;
	if (m_item->gradientType() == 12)
	{
		if ((grow == -1) || (gcol == 0))
			return;
		ScItemState<QPair<meshPoint,meshPoint> > *ss = NULL;
		if(UndoManager::undoEnabled())
		{
			ss = new ScItemState<QPair<meshPoint,meshPoint> >(Um::GradPos);
			ss->set("MOVE_MESH_PATCH");
			ss->set("ARRAY", false);
			ss->set("X", grow);
			ss->set("Y", gcol);
		}
		switch (gcol)
		{
			case 1:
				tmp = m_item->meshGradientPatches[grow].TL;
				tmp.controlTop    = tmp.gridPoint;
				tmp.controlLeft   = tmp.gridPoint;
				tmp.controlRight  = tmp.gridPoint;
				tmp.controlBottom = tmp.gridPoint;
				if(UndoManager::undoEnabled())
					ss->setItem(qMakePair(m_item->meshGradientPatches[grow].TL,tmp));
				m_item->meshGradientPatches[grow].TL = tmp;
		break;
			case 2:
				tmp = m_item->meshGradientPatches[grow].TR;
				tmp.controlTop    = tmp.gridPoint;
				tmp.controlLeft   = tmp.gridPoint;
				tmp.controlRight  = tmp.gridPoint;
				tmp.controlBottom = tmp.gridPoint;
				if(UndoManager::undoEnabled())
					ss->setItem(qMakePair(m_item->meshGradientPatches[grow].TR,tmp));
				m_item->meshGradientPatches[grow].TR = tmp;
		break;
			case 3:
				tmp = m_item->meshGradientPatches[grow].BR;
				tmp.controlTop    = tmp.gridPoint;
				tmp.controlLeft   = tmp.gridPoint;
				tmp.controlRight  = tmp.gridPoint;
				tmp.controlBottom = tmp.gridPoint;
				if(UndoManager::undoEnabled())
					ss->setItem(qMakePair(m_item->meshGradientPatches[grow].BR,tmp));
				m_item->meshGradientPatches[grow].BR = tmp;
		break;
			case 4:
				tmp = m_item->meshGradientPatches[grow].BL;
				tmp.controlTop    = tmp.gridPoint;
				tmp.controlLeft   = tmp.gridPoint;
				tmp.controlRight  = tmp.gridPoint;
				tmp.controlBottom = tmp.gridPoint;
				if(UndoManager::undoEnabled())
					ss->setItem(qMakePair(m_item->meshGradientPatches[grow].BL,tmp));
				m_item->meshGradientPatches[grow].BL = tmp;
		break;
		}
		if(UndoManager::undoEnabled())
			undoManager->action(m_item,ss);
	}
	else
	{
		if ((grow == -1) || (gcol == -1))
			return;
		tmp = m_item->meshGradientArray[grow][gcol];
		tmp.controlTop    = tmp.gridPoint;
		tmp.controlLeft   = tmp.gridPoint;
		tmp.controlRight  = tmp.gridPoint;
		tmp.controlBottom = tmp.gridPoint;
		if(UndoManager::undoEnabled())
		{
			ScItemState<QPair<meshPoint,meshPoint> > *ss = new ScItemState<QPair<meshPoint,meshPoint> >(Um::GradPos);
			ss->set("MOVE_MESH_PATCH");
			ss->set("ARRAY", true);
			ss->set("X", grow);
			ss->set("Y", gcol);
			ss->setItem(qMakePair(m_item->meshGradientArray[grow][gcol],tmp));
			undoManager->action(m_item,ss);
		}
		m_item->meshGradientArray[grow][gcol] = tmp;
	}
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}


/*********************************************************************
*
* Feature Vector Gradient
*
**********************************************************************/

void ColorPalette::editGradientVector()
{
	if (gradEditButton->isChecked())
	{
		setGradientVectorValues();
		CGradDia->show();
	}
	else
	{
		CGradDia->hide();
		editMeshColors->setEnabled(true);
	}
	if (m_item->gradientType() == 9)
		editStrokeGradient = 3;
	else if (m_item->gradientType() == 10)
		editStrokeGradient = 4;
	else if (m_item->gradientType() == 11)
		editStrokeGradient = 5;
	else if (m_item->gradientType() == 12)
		editStrokeGradient = 9;
	else
		editStrokeGradient = 0;
	gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	emit editGradient(editStrokeGradient);
}

void ColorPalette::editGradientVectorStroke()
{
	if (gradEditButtonStroke->isChecked())
	{
		setGradientVectorStrokeValues();
		CGradDia->show();
	}
	else
	{
		CGradDia->hide();
	}
	editStrokeGradient = 1;
	gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	emit editGradient(editStrokeGradient);
}

void ColorPalette::setGradientVectorValues()
{
	if (gradEditButton->isChecked())
	{
		CGradDia->unitChange(m_doc->unitIndex());
		CGradDia->setValues(m_item->gradientStartX(), m_item->gradientStartY(), m_item->gradientEndX(), m_item->gradientEndY(), m_item->GrFocalX, m_item->GrFocalY, m_item->GrScale, m_item->GrSkew, 0, 0);
		if (m_item->gradientType() == 6)
			CGradDia->selectLinear();
		else if (m_item->gradientType() == 7)
			CGradDia->selectRadial();
		else if (m_item->gradientType() == 9)
		{
			CGradDia->setValues(m_item->GrControl1.x(), m_item->GrControl1.y(), m_item->GrControl2.x(), m_item->GrControl2.y(), m_item->GrControl3.x(), m_item->GrControl3.y(), m_item->GrControl4.x(), m_item->GrControl4.y(), 0, 0);
			CGradDia->selectFourColor();
		}
		else if (m_item->gradientType() == 10)
		{
			CGradDia->setValues(m_item->GrControl1.x(), m_item->GrControl1.y(), m_item->GrControl2.x(), m_item->GrControl2.y(), m_item->GrControl3.x(), m_item->GrControl3.y(), m_item->GrControl4.x(), m_item->GrControl4.y(), m_item->GrControl5.x(), m_item->GrControl5.y());
			CGradDia->selectDiamond();
		}
		else if (m_item->gradientType() == 11)
		{
			CGradDia->selectMesh();
			editMeshColors->setEnabled(false);
		}
		else if (m_item->gradientType() == 12)
		{
			CGradDia->selectPatchMesh();
			editMeshColors->setEnabled(false);
		}
		else if (m_item->gradientType() == 13)
			CGradDia->selectConical();
	}
}

void ColorPalette::setGradientVectorStrokeValues()
{
	if (gradEditButtonStroke->isChecked())
	{
		CGradDia->unitChange(m_doc->unitIndex());
		CGradDia->setValues(m_item->GrStrokeStartX, m_item->GrStrokeStartY, m_item->GrStrokeEndX, m_item->GrStrokeEndY, m_item->GrStrokeFocalX, m_item->GrStrokeFocalY, m_item->GrStrokeScale, m_item->GrStrokeSkew, 0, 0);
		if (m_item->strokeGradientType() == 6)
			CGradDia->selectLinear();
		else
			CGradDia->selectRadial();
	}
}

void ColorPalette::setActiveGradDia(bool active)
{
	if (!active)
	{
		if (editStrokeGradient == 1)
			gradEditButtonStroke->setChecked(false);
		else
			gradEditButton->setChecked(false);
		emit editGradient(editStrokeGradient);
		editMeshColors->setEnabled(true);
	}
}

void ColorPalette::setSpecialGradient(double x1, double y1, double x2, double y2, double fx, double fy, double sg, double sk, double cx, double cy)
{
	if (CGradDia)
		CGradDia->setValues(x1, y1, x2, y2, fx, fy, sg, sk, cx, cy);
}

void ColorPalette::setMeshPoint()
{
	if ((m_item->selectedMeshPointX > -1) && (m_item->selectedMeshPointY > -1))
	{
		colorMeshPoint->setEnabled(true);
		shadeMeshPoint->setEnabled(true);
		transparencyMeshPoint->setEnabled(true);
		meshPoint mp = m_item->meshGradientArray[m_item->selectedMeshPointX][m_item->selectedMeshPointY];
		setCurrentComboItem(colorMeshPoint, mp.colorName);
		shadeMeshPoint->setValue(mp.shade);
		transparencyMeshPoint->setValue(mp.transparency * 100);
	}
	else
	{
		colorMeshPoint->setEnabled(false);
		shadeMeshPoint->setEnabled(false);
		transparencyMeshPoint->setEnabled(false);
	}
}

void ColorPalette::endPatchAdd()
{
	CGradDia->endPAddButton();
}

void ColorPalette::snapToPatchGrid(bool val)
{
	m_item->setSnapToPatchGrid(val);
}

void ColorPalette::handleRemovePatch()
{
	if ((m_item->selectedMeshPointX > -1) && (m_item->meshGradientPatches.count() > 1))
	{
		if(UndoManager::undoEnabled())
		{
			ScItemState<meshGradientPatch> *ss = new ScItemState<meshGradientPatch>(Um::RemoveMeshPatch,"",Um::ILine);
			ss->set("REMOVE_MESH_PATCH");
			ss->setItem(m_item->meshGradientPatches.takeAt(m_item->selectedMeshPointX));
			ss->set("POS", m_item->selectedMeshPointX);
			undoManager->action(m_item,ss);
		}
		m_item->selectedMeshPointX = -1;
		CGradDia->changebuttonRemovePatch((m_item->selectedMeshPointX > -1) && (m_item->meshGradientPatches.count() > 1));
		m_item->update();
		m_doc->regionsChanged()->update(QRect());
		editStrokeGradient = 9;
		emit editGradient(editStrokeGradient);
	}
}

