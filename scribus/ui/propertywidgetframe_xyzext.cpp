/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgetframe_xyzext.h"

#include <QMessageBox>

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "appmodehelper.h"
#include "appmodes.h"
#include "commonstrings.h"
#include "pageitem.h"
#include "pageitem_group.h"
#include "pageitem_textframe.h"
#include "propertiespalette_utils.h"

#include "scribuscore.h"
#include "scraction.h"
#include "scribusdoc.h"
#include "scribusview.h"
#include "selection.h"
#include "tabmanager.h"
#include "units.h"
#include "undomanager.h"
#include "util.h"
#include "iconmanager.h"
#include "util_math.h"


PropertyWidgetFrame_XYZExt::PropertyWidgetFrame_XYZExt( QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_haveDoc  = false;
	m_haveItem = false;

	setupUi(this);
	nameEdit->setFocusPolicy(Qt::ClickFocus); //

	IconManager* im=IconManager::instance();

//	doGroup->setIcon(im->loadIcon("group.png"));
//	doUnGroup->setIcon(im->loadIcon("ungroup.png"));

	noPrint->setCheckable( true );
	QIcon a2 = QIcon();
	a2.addPixmap(im->loadPixmap("NoPrint.png"), QIcon::Normal, QIcon::On);
	a2.addPixmap(im->loadPixmap("16/document-print.png"), QIcon::Normal, QIcon::Off);
	noPrint->setIcon(a2);

	languageChange();


	connect(nameEdit , SIGNAL(Leaved()) , this, SLOT(handleNewName()));
	connect(noPrint  , SIGNAL(clicked()), this, SLOT(handlePrint()));
//	connect(doGroup  , SIGNAL(clicked()), this, SLOT(handleGrouping()) );
//	connect(doUnGroup, SIGNAL(clicked()), this, SLOT(handleUngrouping()) );

	m_haveItem = false;
}

void PropertyWidgetFrame_XYZExt::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	//connect(mw->appModeHelper, SIGNAL(AppModeChanged(int, int)), this, SLOT(handleAppModeChanged(int, int)));
}

void PropertyWidgetFrame_XYZExt::setDoc(ScribusDoc *d)
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

	m_haveDoc = true;
	m_haveItem = false;

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidgetFrame_XYZExt::unsetDoc()
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
	nameEdit->clear();

//	doGroup->setEnabled(false);
//	doUnGroup->setEnabled(false);

	setEnabled(false);
}

void PropertyWidgetFrame_XYZExt::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	handleSelectionChanged();
}


PageItem* PropertyWidgetFrame_XYZExt::currentItemFromSelection()
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

void PropertyWidgetFrame_XYZExt::setCurrentItem(PageItem *i)
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
		setDoc(i->doc());

	disconnect(nameEdit, SIGNAL(Leaved()), this, SLOT(handleNewName()));

	m_haveItem = false;
	m_item = i;

	nameEdit->setText(m_item->itemName());

	connect(nameEdit, SIGNAL(Leaved()), this, SLOT(handleNewName()));

//CB replaces old emits from PageItem::emitAllToGUI()
	disconnect(noPrint, SIGNAL(clicked()), this, SLOT(handlePrint()));

	noPrint->setChecked(!i->printEnabled());


//CB TODO reconnect PP signals from here
	connect(noPrint , SIGNAL(clicked()), this, SLOT(handlePrint()));

	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		setEnabled(true);
	}
	m_haveItem = true;

	
//	doGroup->setEnabled(false);
//	doUnGroup->setEnabled(false);
//	if ((m_doc->m_Selection->count() > 1) && (haveSameParent))
//		doGroup->setEnabled(true);
//	if (m_doc->m_Selection->count() == 1)
//		doUnGroup->setEnabled(m_item->isGroup());
//	if ((m_doc->appMode == modeEditClip) && (m_item->isGroup()))
//		doUnGroup->setEnabled(false);

	if (m_item->asOSGFrame())
	{
		setEnabled(true);
	}
	if (m_item->asSymbolFrame())
	{
		setEnabled(true);
	}
}

//void PropertyWidgetFrame_XYZExt::handleAppModeChanged(int oldMode, int mode)
//{
//	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
//		return;
//	doUnGroup->setEnabled(mode != modeEdit && mode != modeEditClip && m_item->isGroup());
//	doLock->setEnabled(mode != modeEditClip);
//}

void PropertyWidgetFrame_XYZExt::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	nameEdit->setEnabled(m_doc->m_Selection->count() == 1);

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{

		nameEdit->setEnabled(false);

		setEnabled(true);
	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		
		m_haveItem = (itemType!=-1);
//		if (itemType == -1)
//		{
//			doGroup->setEnabled(false);
//			doUnGroup->setEnabled(false);
//		}
		nameEdit->setEnabled(true);

		setEnabled(true);
		
		switch (itemType)
		{
		case -1:
			setEnabled(false);
			break;
		case PageItem::ImageFrame:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
#ifdef HAVE_OSG
			if (currItem->asOSGFrame())
			{
				setEnabled(true);
				//rotationSpin->setEnabled(false);
			}
#endif
			break;
		case PageItem::Line:
			break;
		}
	}
	if (currItem)
	{
		setCurrentItem(currItem);
	}
}

void PropertyWidgetFrame_XYZExt::unitChange()
{
	if (!m_haveDoc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;
	// unused
	m_haveItem = tmp;
}


void PropertyWidgetFrame_XYZExt::handlePrint()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->scrActions["itemPrintingEnabled"]->toggle();
}


void PropertyWidgetFrame_XYZExt::handleNewName()
{
	if (m_ScMW->scriptIsRunning() || !m_haveDoc || !m_haveItem)
		return;
	QString NameOld = m_item->itemName();
	QString NameNew = nameEdit->text();
	if (NameNew.isEmpty())
	{
		nameEdit->setText(NameOld);
		return;
	}
	bool found = false;
	QList<PageItem*> allItems;
	for (int a = 0; a < m_doc->Items->count(); ++a)
	{
		PageItem *currItem = m_doc->Items->at(a);
		if (currItem->isGroup())
			allItems = currItem->getItemList();
		else
			allItems.append(currItem);
		for (int ii = 0; ii < allItems.count(); ii++)
		{
			PageItem* item = allItems.at(ii);
			if ((NameNew == item->itemName()) && (item != m_item))
			{
				found = true;
				break;
			}
		}
		allItems.clear();
	}
	if (found)
	{
		ScMessageBox::warning(this, CommonStrings::trWarning, "<qt>"+ tr("Name \"%1\" isn't unique.<br/>Please choose another.").arg(NameNew)+"</qt>");
		nameEdit->setText(NameOld);
		nameEdit->setFocus();
	}
	else
	{
		if (m_item->itemName() != nameEdit->text())
		{
			m_item->setItemName(nameEdit->text());
			m_doc->changed();
		}
	}
}


void PropertyWidgetFrame_XYZExt::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
	}
	else
		QWidget::changeEvent(e);
}

void PropertyWidgetFrame_XYZExt::languageChange()
{
//	setWindowTitle( tr("Properties"));
	retranslateUi(this);

}


void PropertyWidgetFrame_XYZExt::showPrintingEnabled(bool isPrintingEnabled)
{
	noPrint->setChecked(!isPrintingEnabled);
}


//void PropertyWidgetFrame_XYZExt::handleGrouping()
//{
//	m_ScMW->GroupObj();
//	doGroup->setEnabled(false);//
//	doUnGroup->setEnabled(true);//
//	handleSelectionChanged();
//	//FIXME
//	//TabStack->setItemEnabled(idShapeItem, false);
//}

//void PropertyWidgetFrame_XYZExt::handleUngrouping()
//{
//	m_ScMW->UnGroupObj();
//	m_doc->invalidateAll();
//	m_doc->regionsChanged()->update(QRect());
//}
