/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgetframe_line.h"

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <QSignalBlocker>

#include "commonstrings.h"
#include "dasheditor.h"
#include "iconmanager.h"
#include "pageitem.h"
#include "pageitem_textframe.h"
#include "sccolorengine.h"
#include "sccombobox.h"
#include "scraction.h"
#include "scribuscore.h"
#include "selection.h"
#include "ui/propertiespalette_utils.h"
#include "ui/stylemanager.h"
#include "ui/smlinestyle.h"
#include "undomanager.h"
#include "units.h"
#include "util.h"
#include "util_math.h"


//using namespace std;

PropertyWidgetFrame_Line::PropertyWidgetFrame_Line( QWidget* parent) : QWidget(parent)
{
	m_ScMW = 0;
	m_doc  = 0;
	m_haveDoc  = false;
	m_haveItem = false;
	m_item     = NULL;
	m_unitRatio = 1.0;
	m_unitIndex = 0;
//	m_styleManager = 0;


	setupUi(this);

	lineType->addItem( tr("Custom"));

	lineTypeLabel->setBuddy(lineType);

	lineWidthLabel->setBuddy(lineWidth);

	lineStyles->setItemDelegate(new LineFormatItemDelegate);
	lineStyles->addItem( "No Style" );

	languageChange();

	connect(lineWidth     , SIGNAL(valueChanged(double)), this, SLOT(handleLineWidth()));
	connect(lineType    , SIGNAL(activated(int))      , this, SLOT(handleLineStyle()));
	connect(dashEditor, SIGNAL(dashChanged())       , this, SLOT(handleDashChange()));
	connect(lineStyles, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(handleLineStyle(QListWidgetItem*)));
//	connect(lineStyleEdit, SIGNAL(clicked(bool)), this, SLOT(handleLineEdit()));
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetFrame_Line::setMainWindow(ScribusMainWindow *mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this  , SLOT(handleUpdateRequest(int)));
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

void PropertyWidgetFrame_Line::setDoc(ScribusDoc *d)
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

//	m_styleManager = m_ScMW->styleMgr();

	lineWidth->setMaximum( 300 );
	lineWidth->setMinimum( 0 );

	updateLineStyles(m_doc);

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidgetFrame_Line::unsetDoc()
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

//	m_styleManager = 0;

	updateLineStyles(0);

	setEnabled(false);
}

/*********************************************************************
*
* Item
*
**********************************************************************/

PageItem* PropertyWidgetFrame_Line::currentItemFromSelection()
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

void PropertyWidgetFrame_Line::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	dashEditor->hide();
	handleSelectionChanged();
}

void PropertyWidgetFrame_Line::setCurrentItem(PageItem *item)
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

	lineStyles->blockSignals(true);

	if (lineStyles->currentItem())
		lineStyles->currentItem()->setSelected(false);

	bool setter = false;
	if (m_item->NamedLStyle.isEmpty())
	{
		setter = true;
		QListWidgetItem *itemStl = NULL;
		itemStl = lineStyles->item(0);
		if (itemStl != NULL)
			itemStl->setSelected(true);
	}
	else
	{
		QList<QListWidgetItem*> results (lineStyles->findItems(m_item->NamedLStyle, Qt::MatchFixedString|Qt::MatchCaseSensitive));
		if (results.count() > 0)
			results[0]->setSelected(true);
		setter = false;
	}

	lineType->setEnabled(setter);
	lineWidth->setEnabled(setter);

	if (m_item->dashes().count() == 0)
		dashEditor->hide();
	else
	{
		lineType->setCurrentIndex(37);
		dashEditor->setDashValues(m_item->dashes(), qMax(m_item->lineWidth(), 0.001), m_item->dashOffset());
		dashEditor->show();
	}

	lineStyles->blockSignals(false);

	setter = false;

	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		setEnabled(false);
	}

	m_haveItem = true;

	showLineWidth(m_item->lineWidth());
	showLineValues(m_item->lineStyle()/*, m_item->lineEnd(), m_item->lineJoin()*/);

	if (m_item->asOSGFrame())
	{
		setEnabled(false);
	}
	if (m_item->asSymbolFrame())
	{
		setEnabled(false);
	}
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidgetFrame_Line::handleSelectionChanged()
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

void PropertyWidgetFrame_Line::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqLineStylesUpdate)
		updateLineStyles();
}

void PropertyWidgetFrame_Line::languageChange()
{
	QSignalBlocker lineTypeBlocker(lineType);
	int oldLineStyle = lineType->currentIndex();
	lineType->clear();
	lineType->updateList();
	lineType->addItem( tr("Custom"));
	lineType->setCurrentIndex(oldLineStyle);

	lineTypeLabel->setText( tr("T&ype of Line:"));
	lineWidthLabel->setText( tr("Line &Width:"));

	QString ptSuffix = tr(" pt");
	QString suffix = (m_doc) ? unitGetSuffixFromIndex(m_doc->unitIndex()) : ptSuffix;

	lineWidth->setSuffix(suffix);
	lineWidth->setSpecialValueText( tr("Hairline"));

	if(lineStyles->count() > 0)
		lineStyles->item(0)->setText( tr("No Style") );

	lineType->setToolTip( tr("Pattern of line"));
	lineWidth->setToolTip( tr("Thickness of line"));
	lineStyles->setToolTip( tr("Line style of current object"));
}

void PropertyWidgetFrame_Line::unitChange()
{
	if (!m_doc)
		return;

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();

	lineWidth->blockSignals(true);
	lineWidth->setNewUnit( m_unitIndex );
	lineWidth->blockSignals(false);
}

/*********************************************************************
*
* Feature Line
*
**********************************************************************/

void PropertyWidgetFrame_Line::updateLineStyles()
{
	updateLineStyles(m_doc);
}

void PropertyWidgetFrame_Line::updateLineStyles(ScribusDoc *dd)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	
	lineStyles->blockSignals(true);
	lineStyles->clear();
	if (dd != 0)
	{
		QHash<QString,multiLine>::Iterator it;
		for (it = dd->MLineStyles.begin(); it != dd->MLineStyles.end(); ++it)
			lineStyles->addItem( new LineFormatItem(dd, it.value(), it.key()) );
		lineStyles->sortItems();
		lineStyles->insertItem( 0, tr("No Style"));
		if (lineStyles->currentItem())
			lineStyles->currentItem()->setSelected(false);
	}
	lineStyles->blockSignals(false);

}

void PropertyWidgetFrame_Line::showLineWidth(double s)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	lineWidth->showValue(s * m_unitRatio);
	if (m_haveItem)
	{
		if (m_item->dashes().count() != 0)
		{
			dashEditor->blockSignals(true);
			if (m_item->lineWidth() != 0.0)
			{
				dashEditor->setDashValues(m_item->dashes(), m_item->lineWidth(), m_item->dashOffset());
				dashEditor->setEnabled(true);
			}
			else
				dashEditor->setEnabled(false);
			dashEditor->blockSignals(false);
		}
	}
}

void PropertyWidgetFrame_Line::showLineValues(Qt::PenStyle p)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	lineType->blockSignals(true);
	dashEditor->blockSignals(true);
	if (m_haveItem)
	{
		if (m_item->dashes().count() != 0)
		{
			lineType->setCurrentIndex(37);
			dashEditor->setDashValues(m_item->dashes(), qMax(m_item->lineWidth(), 0.001), m_item->dashOffset());
		}
		else
			lineType->setCurrentIndex(static_cast<int>(p) - 1);
	}
	else
		lineType->setCurrentIndex(static_cast<int>(p) - 1);
	dashEditor->blockSignals(false);
	lineType->blockSignals(false);

}

void PropertyWidgetFrame_Line::handleLineWidth()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		double oldL = m_item->lineWidth();
		m_doc->itemSelection_SetLineWidth(lineWidth->value() / m_unitRatio);
		if (m_item->dashes().count() != 0)
		{
			if ((oldL != 0.0) && (m_item->lineWidth() != 0.0))
			{
				for (int a = 0; a < m_item->DashValues.count(); a++)
				{
					m_item->DashValues[a] = m_item->DashValues[a] / oldL * m_item->lineWidth();
				}
				m_item->setDashOffset(m_item->dashOffset() / oldL * m_item->lineWidth());
			}
			if (m_item->lineWidth() != 0.0)
			{
				dashEditor->setDashValues(m_item->dashes(), m_item->lineWidth(), m_item->dashOffset());
				dashEditor->setEnabled((m_item->lineWidth() != 0.0));
			}
			else
				dashEditor->setEnabled(false);
		}
		m_doc->invalidateAll();
		m_doc->regionsChanged()->update(QRect());
	}
}

void PropertyWidgetFrame_Line::handleLineStyle()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		if (lineType->currentIndex() == 37)
		{
			if (m_item->dashes().count() == 0)
			{
				if ((m_item->lineStyle() == 0) || (m_item->lineStyle() == 1))
				{
					m_item->DashValues.append(4.0 * qMax(m_item->lineWidth(), 1.0));
					m_item->DashValues.append(2.0 * qMax(m_item->lineWidth(), 1.0));
				}
				else
					getDashArray(m_item->lineStyle(), qMax(m_item->lineWidth(), 1.0), m_item->DashValues);
			}
			if (m_item->lineWidth() != 0.0)
				dashEditor->setDashValues(m_item->dashes(), m_item->lineWidth(), m_item->dashOffset());
			else
			{
				dashEditor->setEnabled(false);
				dashEditor->setDashValues(m_item->dashes(), 1.0, m_item->dashOffset());
			}
			dashEditor->show();
			m_item->update();
		}
		else
		{
			m_item->DashValues.clear();
			dashEditor->hide();
			m_doc->itemSelection_SetLineArt(static_cast<Qt::PenStyle>(lineType->currentIndex()+1));
		}
	}
}


void PropertyWidgetFrame_Line::handleDashChange()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		if (m_item->lineWidth() != 0.0)
		{
			m_item->setDashes(dashEditor->getDashValues(m_item->lineWidth()));
			m_item->setDashOffset(dashEditor->Offset->value() * m_item->lineWidth());
		}
		m_item->update();
	}
}

void PropertyWidgetFrame_Line::handleLineStyle(QListWidgetItem *widgetItem)
{
	if (!m_doc || !m_ScMW || m_ScMW->scriptIsRunning() || !widgetItem)
		return;
	bool setter = (widgetItem->listWidget()->currentRow() == 0);
	m_doc->itemSelection_SetNamedLineStyle(setter ? QString("") : widgetItem->text());
	lineType->setEnabled(setter);
	lineWidth->setEnabled(setter);
}

//void PropertyWidgetFrame_Line::handleLineEdit(){

//	if(!m_styleManager)
//		m_styleManager->setPaletteShown(true);
//}


/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetFrame_Line::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	QWidget::changeEvent(e);
}
