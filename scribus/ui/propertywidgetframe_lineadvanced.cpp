/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgetframe_lineadvanced.h"

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <QSignalBlocker>

#include "arrowchooser.h"
#include "commonstrings.h"
#include "iconmanager.h"
#include "pageitem.h"
#include "pageitem_textframe.h"
#include "sccolorengine.h"
#include "sccombobox.h"
#include "scraction.h"
#include "scribuscore.h"
#include "selection.h"
#include "ui/propertiespalette_utils.h"
#include "undomanager.h"
#include "units.h"
#include "util.h"
#include "util_math.h"


//using namespace std;

PropertyWidgetFrame_LineAdvanced::PropertyWidgetFrame_LineAdvanced( QWidget* parent) : QWidget(parent)
{
	m_ScMW = 0;
	m_doc  = 0;
	m_haveDoc  = false;
	m_haveItem = false;
	m_item     = NULL;
	m_lineMode = false;
	m_unitRatio = 1.0;
	m_unitIndex = 0;

	setupUi(this);
	setSizePolicy( QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

	lineModeLabel->setBuddy(lineMode);

	startArrowLabel->setBuddy(startArrow);
	endArrowLabel->setBuddy(endArrow);

	startArrowScale->setMaximum( 1000 );
	startArrowScale->setMinimum( 1 );
	startArrowScale->setDecimals(0);

	endArrowScale->setMaximum( 1000 );
	endArrowScale->setMinimum( 1 );
	endArrowScale->setDecimals(0);

	lineJoinLabel->setBuddy(lineJoinStyle);
	lineEndLabel->setBuddy(lineEndStyle);

	languageChange();

	connect(lineJoinStyle, SIGNAL(activated(int))      , this, SLOT(handleLineJoin()));
	connect(lineEndStyle , SIGNAL(activated(int))      , this, SLOT(handleLineEnd()));
	connect(lineMode  , SIGNAL(activated(int))      , this, SLOT(handleLineMode()));
	connect(startArrow, SIGNAL(activated(int))      , this, SLOT(handleStartArrow(int )));
	connect(endArrow  , SIGNAL(activated(int))      , this, SLOT(handleEndArrow(int )));
	connect(startArrowScale, SIGNAL(valueChanged(double)), this, SLOT(handleStartArrowScale(double )));
	connect(endArrowScale  , SIGNAL(valueChanged(double)), this, SLOT(handleEndArrowScale(double )));
}

void PropertyWidgetFrame_LineAdvanced::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}

PageItem* PropertyWidgetFrame_LineAdvanced::currentItemFromSelection()
{
	PageItem *currentItem = NULL;

	if (m_doc)
	{
		if (m_doc->m_Selection->count() > 1)
		{
			currentItem = m_doc->m_Selection->itemAt(0);
		}
		else if (m_doc->m_Selection->count() == 1)
		{
			currentItem = m_doc->m_Selection->itemAt(0);
		}
	}

	return currentItem;
}

void PropertyWidgetFrame_LineAdvanced::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this  , SLOT(handleUpdateRequest(int)));
}

void PropertyWidgetFrame_LineAdvanced::setDoc(ScribusDoc *d)
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

	m_haveDoc  = true;
	m_haveItem = false;

	startArrow->rebuildList(&m_doc->arrowStyles());
	endArrow->rebuildList(&m_doc->arrowStyles());

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidgetFrame_LineAdvanced::unsetDoc()
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

void PropertyWidgetFrame_LineAdvanced::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	handleSelectionChanged();
}

void PropertyWidgetFrame_LineAdvanced::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{
		setEnabled(true);
	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem = (itemType != -1);

		lineMode->setEnabled(false);
		switch (itemType)
		{
		case -1:
			setEnabled(false);
			break;
		case PageItem::ImageFrame:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
			setEnabled(currItem->asOSGFrame() == NULL);
			break;
		case PageItem::Line:
			setEnabled(true);
			lineMode->setEnabled(true);
			break;
		case PageItem::Arc:
		case PageItem::ItemType1:
		case PageItem::ItemType3:
		case PageItem::Polygon:
		case PageItem::PolyLine:
		case PageItem::PathText:
		case PageItem::RegularPolygon:
		case PageItem::TextFrame:
			setEnabled(true);
			break;
		case PageItem::Symbol:
			setEnabled(false);
			break;
		}
	}
	if (currItem)
	{
		setCurrentItem(currItem);
	}

}

void PropertyWidgetFrame_LineAdvanced::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqArrowStylesUpdate)
		updateArrowStyles();
}

void PropertyWidgetFrame_LineAdvanced::setCurrentItem(PageItem *item)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	//CB We shouldn't really need to process this if our item is the same one
	//maybe we do if the item has been changed by scripter.. but that should probably
	//set some status if so.
	//FIXME: This won't work until when a canvas deselect happens, m_item must be NULL.
	//if (m_item == i)
	//	return;

	if (!m_doc)
		setDoc(item->doc());

	m_haveItem = false;
	m_item = item;

	startArrow->blockSignals(true);
	endArrow->blockSignals(true);
	startArrowScale->blockSignals(true);
	endArrowScale->blockSignals(true);
	lineMode->blockSignals(true);

	if ((m_item->asLine()) || (m_item->asPolyLine()) || (m_item->asSpiral()))
	{
		startArrow->setEnabled(true);
		endArrow->setEnabled(true);
		startArrow->setCurrentIndex(m_item->startArrowIndex());
		endArrow->setCurrentIndex(m_item->endArrowIndex());
		startArrowScale->setEnabled(true);
		endArrowScale->setEnabled(true);
		endArrowScale->setValue(m_item->endArrowScale());
		startArrowScale->setValue(m_item->startArrowScale());
	}
	else
	{
		startArrow->setEnabled(false);
		endArrow->setEnabled(false);
		startArrowScale->setEnabled(false);
		endArrowScale->setEnabled(false);
	}

	bool setter = false;
	if (m_item->NamedLStyle.isEmpty())
	{
		setter = true;
	}
	else
	{
		setter = false;
	}

	lineJoinStyle->setEnabled(setter);
	lineEndStyle->setEnabled(setter);

	if (m_lineMode)
		lineMode->setCurrentIndex(1);
	else
		lineMode->setCurrentIndex(0);

	startArrow->blockSignals(false);
	endArrow->blockSignals(false);
	startArrowScale->blockSignals(false);
	endArrowScale->blockSignals(false);
	lineMode->blockSignals(false);

	setter = false;

	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		setEnabled(false);
	}

	m_haveItem = true;

	showLineValues(/*m_item->lineStyle(), */m_item->lineEnd(), m_item->lineJoin());

	if (m_item->asOSGFrame())
	{
		setEnabled(false);
	}
	if (m_item->asSymbolFrame())
	{
		setEnabled(false);
	}
}

void PropertyWidgetFrame_LineAdvanced::updateArrowStyles()
{
	updateArrowStyles(m_doc);
}

void PropertyWidgetFrame_LineAdvanced::updateArrowStyles(ScribusDoc *doc)
{
	if (doc)
	{
		startArrow->rebuildList(&doc->arrowStyles());
		endArrow->rebuildList(&doc->arrowStyles());
	}
}


void PropertyWidgetFrame_LineAdvanced::showLineValues(Qt::PenCapStyle pc, Qt::PenJoinStyle pj)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	lineEndStyle->blockSignals(true);
	switch (pc)
	{
	case Qt::FlatCap:
		lineEndStyle->setCurrentIndex(0);
		break;
	case Qt::SquareCap:
		lineEndStyle->setCurrentIndex(1);
		break;
	case Qt::RoundCap:
		lineEndStyle->setCurrentIndex(2);
		break;
	default:
		lineEndStyle->setCurrentIndex(0);
		break;
	}
	lineEndStyle->blockSignals(false);

	lineJoinStyle->blockSignals(true);
	switch (pj)
	{
	case Qt::MiterJoin:
		lineJoinStyle->setCurrentIndex(0);
		break;
	case Qt::BevelJoin:
		lineJoinStyle->setCurrentIndex(1);
		break;
	case Qt::RoundJoin:
		lineJoinStyle->setCurrentIndex(2);
		break;
	default:
		lineJoinStyle->setCurrentIndex(0);
		break;
	}
	lineJoinStyle->blockSignals(false);
}


void PropertyWidgetFrame_LineAdvanced::handleLineJoin()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		Qt::PenJoinStyle c = Qt::MiterJoin;
		switch (lineJoinStyle->currentIndex())
		{
		case 0:
			c = Qt::MiterJoin;
			break;
		case 1:
			c = Qt::BevelJoin;
			break;
		case 2:
			c = Qt::RoundJoin;
			break;
		}
		m_doc->itemSelection_SetLineJoin(c);
	}
}

void PropertyWidgetFrame_LineAdvanced::handleLineEnd()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		Qt::PenCapStyle c = Qt::FlatCap;
		switch (lineEndStyle->currentIndex())
		{
		case 0:
			c = Qt::FlatCap;
			break;
		case 1:
			c = Qt::SquareCap;
			break;
		case 2:
			c = Qt::RoundCap;
			break;
		}
		m_doc->itemSelection_SetLineEnd(c);
	}
}

void PropertyWidgetFrame_LineAdvanced::handleLineMode()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_lineMode = (lineMode->currentIndex() == 1); 
	emit lineModeChanged(lineMode->currentIndex());
}

void PropertyWidgetFrame_LineAdvanced::handleStartArrow(int id)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_ApplyArrowHead(id,-1);
}

void PropertyWidgetFrame_LineAdvanced::handleEndArrow(int id)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_ApplyArrowHead(-1, id);
}

void PropertyWidgetFrame_LineAdvanced::handleStartArrowScale(double sc)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_ApplyArrowScale(static_cast<int>(sc), -1, NULL);
}

void PropertyWidgetFrame_LineAdvanced::handleEndArrowScale(double sc)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_ApplyArrowScale(-1, static_cast<int>(sc), NULL);
}

void PropertyWidgetFrame_LineAdvanced::handleLineStyle(QListWidgetItem *widgetItem)
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning() || !widgetItem)
		return;
	bool setter = (widgetItem->listWidget()->currentRow() == 0);
	m_doc->itemSelection_SetNamedLineStyle(setter ? QString("") : widgetItem->text());
	lineJoinStyle->setEnabled(setter);
	lineEndStyle->setEnabled(setter);
}

void PropertyWidgetFrame_LineAdvanced::languageChange()
{
	QSignalBlocker lineModeBlocker(lineMode);
	int oldLineMode = lineMode->currentIndex();
	lineMode->clear();
	lineMode->addItem( tr("Left Point"));
	lineMode->addItem( tr("End Points"));
	lineMode->setCurrentIndex(oldLineMode);

	lineModeLabel->setText( tr("&Basepoint:"));
	startArrowLabel->setText( tr("Start Arrow:"));
	endArrowLabel->setText( tr("End Arrow:"));
	if (m_haveDoc)
	{
		QSignalBlocker startArrowBlocker(startArrow);
		int arrowItem = startArrow->currentIndex();
		startArrow->rebuildList(&m_doc->arrowStyles());
		startArrow->setCurrentIndex(arrowItem);
		QSignalBlocker endArrowBlocker(endArrow);
		arrowItem = endArrow->currentIndex();
		endArrow->rebuildList(&m_doc->arrowStyles());
		endArrow->setCurrentIndex(arrowItem);
	}
	lineJoinLabel->setText( tr("Ed&ges:"));

	QSignalBlocker lineJoinStyleBlocker(lineJoinStyle);
	int oldLJoinStyle = lineJoinStyle->currentIndex();
	lineJoinStyle->clear();
	IconManager* im=IconManager::instance();
	lineJoinStyle->addItem(im->loadIcon("16/stroke-join-miter.png"), tr("Miter Join"));
	lineJoinStyle->addItem(im->loadIcon("16/stroke-join-bevel.png"), tr("Bevel Join"));
	lineJoinStyle->addItem(im->loadIcon("16/stroke-join-round.png"), tr("Round Join"));
	lineJoinStyle->setCurrentIndex(oldLJoinStyle);

	QSignalBlocker lineEndStyleBlocker(lineEndStyle);
	int oldLEndStyle = lineEndStyle->currentIndex();
	lineEndStyle->clear();
	lineEndStyle->addItem(im->loadIcon("16/stroke-cap-butt.png"), tr("Flat Cap"));
	lineEndStyle->addItem(im->loadIcon("16/stroke-cap-square.png"), tr("Square Cap"));
	lineEndStyle->addItem(im->loadIcon("16/stroke-cap-round.png"), tr("Round Cap"));
	lineEndStyle->setCurrentIndex(oldLEndStyle);
	lineEndLabel->setText( tr("&Endings:"));

	QString pctSuffix = tr(" %");
	startArrowScale->setSuffix(pctSuffix);
	endArrowScale->setSuffix(pctSuffix);

	lineMode->setToolTip( tr("Change settings for left or end points"));
	lineJoinStyle->setToolTip( tr("Type of line joins"));
	lineEndStyle->setToolTip( tr("Type of line end"));
	startArrow->setToolTip( tr("Arrow head style for start of line"));
	endArrow->setToolTip( tr("Arrow head style for end of line"));
	startArrowScale->setToolTip( tr("Arrow head scale for start of line"));
	endArrowScale->setToolTip( tr("Arrow head scale for end of line"));
}

void PropertyWidgetFrame_LineAdvanced::unitChange()
{
	if (!m_doc)
		return;

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();
}
