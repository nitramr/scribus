/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/


#include "propertiescontentpalette.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QTransform>
#include <QObject>
#include <QPoint>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QTimer>
#include <QToolBox>
#include <QToolTip>
#include <QVBoxLayout>
#include <QValidator>
#include <QWidget>

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>


#include "appmodehelper.h"
#include "appmodes.h"
#include "pageitem_table.h"
#include "pageitem_textframe.h"
#include "propertiescontentpalette_image.h"
#include "propertiescontentpalette_table.h"
#include "propertiescontentpalette_text.h"
#include "scribus.h"
#include "scribusdoc.h"
#include "selection.h"
#include "undomanager.h"


PropertiesContentPalette::PropertiesContentPalette( QWidget* parent) : ScDockPalette( parent, "PropertiesContentPalette", 0)
{
	undoManager = UndoManager::instance();
	m_ScMW=0;
	m_doc=0;
	m_haveDoc = false;
	m_haveItem = false;
	m_unitRatio = 1.0;

	setObjectName(QString::fromLocal8Bit("PropertiesContentPalette"));
///	setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

	QFont f(font());
	f.setPointSize(f.pointSize()-1);
	setFont(f);

	imagePal = new PropertiesContentPalette_Image( this );
	imagePal->setVisible(false);
	tablePal = new PropertiesContentPalette_Table( this );
	tablePal->setVisible(false);
	textPal = new PropertiesContentPalette_Text( this );
	textPal->setVisible(false);

	emptyPaletteLabel = new QLabel(tr("Select a content object to see properties here."));
	emptyPaletteLabel->setAlignment(Qt::AlignCenter);

	QVBoxLayout *paletteHolderLayout = new QVBoxLayout();
	paletteHolderLayout->setMargin(0);
	paletteHolderLayout->setSpacing(0);
	paletteHolderLayout->setContentsMargins(0,0,0,0);
	paletteHolderLayout->addWidget(imagePal);
	paletteHolderLayout->addWidget(tablePal);
	paletteHolderLayout->addWidget(textPal);
	paletteHolderLayout->addWidget(emptyPaletteLabel);

	QWidget * paletteHolder = new QWidget();
	paletteHolder->setLayout(paletteHolderLayout);

	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidget(paletteHolder);
	m_scrollArea->setMinimumWidth(this->width());
	m_scrollArea->setWidgetResizable(true);

	setWidget( m_scrollArea );

	languageChange();

	m_haveItem = false;
}

void PropertiesContentPalette::closeEvent(QCloseEvent *closeEvent)
{
	ScDockPalette::closeEvent(closeEvent);
}

void PropertiesContentPalette::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	// For some reason, the mapToGlobal() / mapFromGlobal() roundtrip
	// performed below does not give always good results, causing TP to
	// not display in some situations. Moreover the reparenting is useless 
	// as TP is already created with ScribusMainWindow as parent.
	/*QPoint p1 = mapToGlobal(pos());
	QPoint p2 = m_ScMW->mapFromGlobal(p1);
	setParent(m_ScMW);
	move(p2);*/

	this->tablePal->setMainWindow(mw);
	this->textPal->setMainWindow(mw);
	this->imagePal->setMainWindow(mw);

	connect(m_ScMW->appModeHelper, SIGNAL(AppModeChanged(int,int)), this, SLOT(AppModeChanged()));
}


void PropertiesContentPalette::setDoc(ScribusDoc *d)
{
	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_doc = d;
	m_item = NULL;
	setEnabled(!m_doc->drawAsPreview);

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();
//qt4 FIXME here
	m_haveDoc = true;
	m_haveItem = false;

	tablePal->setDoc(m_doc);
	textPal->setDoc(m_doc);
	imagePal->setDoc(m_doc);

	updateColorList();

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));

	// Handle properties update when switching document
	handleSelectionChanged();
}

void PropertiesContentPalette::unsetDoc()
{
	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}
	setEnabled(true);
	m_haveDoc = false;
	m_haveItem = false;
	m_doc=NULL;
	m_item = NULL;

	tablePal->unsetItem();
	tablePal->unsetDoc();
//	textPal->unsetItem();
	textPal->unsetDoc();
	imagePal->unsetItem();
	imagePal->unsetDoc();

	m_haveItem = false;
}

void PropertiesContentPalette::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;

	tablePal->unsetItem();
//	textPal->unsetItem();
	imagePal->unsetItem();

	handleSelectionChanged();
}

PageItem* PropertiesContentPalette::currentItemFromSelection()
{
	PageItem *currentItem = NULL;

	if (m_doc)
	{
		if (m_doc->m_Selection->count() > 0)
			currentItem = m_doc->m_Selection->itemAt(0);
	/*	if (m_doc->m_Selection->count() > 1)
		{
			int lowestItem = 999999;
			for (int a=0; a<m_doc->m_Selection->count(); ++a)
			{
				currentItem = m_doc->m_Selection->itemAt(a);
				lowestItem = qMin(lowestItem, m_doc->Items->indexOf(currentItem));
			}
			currentItem = m_doc->Items->at(lowestItem);
		}
		else if (m_doc->m_Selection->count() == 1)
		{
			currentItem = m_doc->m_Selection->itemAt(0);
		} */
	}

	return currentItem;
}

void PropertiesContentPalette::AppModeChanged()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		if (m_item->isTable())
		{
			textPal->setEnabled(m_doc->appMode == modeEditTable);
			if (m_doc->appMode == modeEditTable)
				connect(m_item->asTable(), SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
			else
				disconnect(m_item->asTable(), SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		}

//		tablePal->handleSelectionChanged();
//		textPal->handleSelectionChanged();
		imagePal->handleSelectionChanged();
	}
}

void PropertiesContentPalette::setCurrentItem(PageItem *i)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	//CB We shouldn't really need to process this if our item is the same one
	//maybe we do if the item has been changed by scripter.. but that should probably
	//set some status if so.
	//FIXME: This won't work until when a canvas deselect happens, m_item must be NULL.
	//if (m_item == i)
	//	return;

	if (!i)
	{
		unsetItem();
		return;
	}

	if (!m_doc)
		setDoc(i->doc());

	m_haveItem = false;
	m_item = i;

	tablePal->setCurrentItem(m_item);
	imagePal->setCurrentItem(m_item);


	if ((m_item->isGroup()) && (!m_item->isSingleSel)){
		tablePal->setEnabled(false);
		tablePal->setVisible(false);
		textPal->setEnabled(false);
		textPal->setVisible(false);
		imagePal->setEnabled(false);
		imagePal->setVisible(false);
		emptyPaletteLabel->setVisible(true);
	}

	m_haveItem = true;

	if (!sender() || (m_doc->appMode == modeEditTable))
	{
	//	tablePal->handleSelectionChanged();
		textPal->handleSelectionChanged();
		imagePal->handleSelectionChanged();
	}

	if (m_item->asOSGFrame())
	{
		tablePal->setEnabled(false);
		tablePal->setVisible(false);
		textPal->setEnabled(false);
		textPal->setVisible(false);
		imagePal->setEnabled(false);
		imagePal->setVisible(false);
		emptyPaletteLabel->setVisible(true);
	}
	if (m_item->asSymbolFrame())
	{
		tablePal->setEnabled(false);
		tablePal->setVisible(false);
		textPal->setEnabled(false);
		textPal->setVisible(false);
		imagePal->setEnabled(false);
		imagePal->setVisible(false);
		emptyPaletteLabel->setVisible(true);
	}
}

void  PropertiesContentPalette::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{
		tablePal->setEnabled(false);
		tablePal->setVisible(false);
		textPal->setEnabled(false);
		textPal->setVisible(false);
		imagePal->setEnabled(false);
		imagePal->setVisible(false);
		emptyPaletteLabel->setVisible(true);
	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem   = (itemType != -1);

		switch (itemType)
		{
		case -1:
		case PageItem::Line:
		case PageItem::ItemType1:
		case PageItem::ItemType3:
		case PageItem::Polygon:
		case PageItem::RegularPolygon:
		case PageItem::Arc:
		case PageItem::PolyLine:
		case PageItem::Spiral:
		case PageItem::Symbol:
		case PageItem::Group:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
			m_haveItem = false;
			textPal->setEnabled(false);
			textPal->setVisible(false);
			tablePal->setEnabled(false);
			tablePal->setVisible(false);
			imagePal->setEnabled(false);
			imagePal->setVisible(false);
			emptyPaletteLabel->setVisible(true);
			break;
		case PageItem::ImageFrame:
			textPal->setEnabled(false);
			textPal->setVisible(false);
			tablePal->setEnabled(false);
			tablePal->setVisible(false);
			imagePal->setEnabled(true);
			imagePal->setVisible(true);
			emptyPaletteLabel->setVisible(false);
			break;
		case PageItem::TextFrame:
		case PageItem::PathText:
			textPal->setEnabled(true);
			textPal->setVisible(true);
			tablePal->setEnabled(false);
			tablePal->setVisible(false);
			imagePal->setEnabled(false);
			imagePal->setVisible(false);
			emptyPaletteLabel->setVisible(false);
			break;
		case PageItem::Table:
			textPal->setEnabled(m_doc->appMode == modeEditTable);
			textPal->setVisible(m_doc->appMode == modeEditTable);
			tablePal->setEnabled(true);
			tablePal->setVisible(true);
			imagePal->setEnabled(false);
			imagePal->setVisible(false);
			emptyPaletteLabel->setVisible(false);
			break;
		}
	}

	update();

	if (currItem)
		setCurrentItem(currItem);
}

void PropertiesContentPalette::unitChange()
{
//	if (!m_haveDoc)
//		return;
//	bool tmp = m_haveItem;
//	m_haveItem = false;
//	m_unitRatio = m_doc->unitRatio();
//	m_unitIndex = m_doc->unitIndex();

	tablePal->unitChange();
	textPal->unitChange();
	imagePal->unitChange();
//	m_haveItem = tmp;
}


void PropertiesContentPalette::updateColorList()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	tablePal->updateColorList();
	textPal->updateColorList();

	assert (m_doc->PageColors.document());
}


void PropertiesContentPalette::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
		return;
	}
	if(e->type() == QEvent::Resize){
		m_scrollArea->setMinimumWidth(this->width());
		return;
	}
	ScDockPalette::changeEvent(e);
}

void PropertiesContentPalette::languageChange()
{
	setWindowTitle( tr("Content Properties"));
	tablePal->languageChange();
	textPal->languageChange();
	imagePal->languageChange();
}


