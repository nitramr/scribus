/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgetframe_xyztransform.h"

#include <QMessageBox>

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "appmodehelper.h"
#include "appmodes.h"
#include "autoform.h"
#include "basepointwidget.h"
#include "commonstrings.h"
#include "colorlistbox.h"
#include "sccolorengine.h"
#include "pageitem.h"
#include "pageitem_arc.h"
#include "pageitem_group.h"
#include "pageitem_spiral.h"
#include "pageitem_textframe.h"
#include "propertiespalette_utils.h"
#include "sccombobox.h"

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

//using namespace std;

PropertyWidgetFrame_XYZTransform::PropertyWidgetFrame_XYZTransform( QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_haveDoc  = false;
	m_haveItem = false;
	m_lineMode = false;
	m_oldRotation = 0;
	m_unitRatio = 1.0;

	setupUi(this);

	_userActionOn = false;
	userActionSniffer = new UserActionSniffer(this);
	connect(userActionSniffer, SIGNAL(actionStart()), this, SLOT(spinboxStartUserAction()));
	connect(userActionSniffer, SIGNAL(actionEnd()), this, SLOT(spinboxFinishUserAction()));

	rotationSpin->setWrapping( true );
	installSniffer(rotationSpin);

	rotationSpin->setNewUnit(6);
	rotationLabel->setBuddy(rotationSpin);
	IconManager* im=IconManager::instance();
	levelUp->setIcon(im->loadIcon("16/go-up.png"));
	levelDown->setIcon(im->loadIcon("16/go-down.png"));
	levelTop->setIcon(im->loadIcon("16/go-top.png"));
	levelBottom->setIcon(im->loadIcon("16/go-bottom.png"));
	levelLabel->setAlignment( Qt::AlignCenter );

	doGroup->setIcon(im->loadIcon("group.png"));
	doUnGroup->setIcon(im->loadIcon("ungroup.png"));

	flipH->setIcon(im->loadIcon("16/flip-object-horizontal.png"));
	flipH->setCheckable( true );
	flipV->setIcon(im->loadIcon("16/flip-object-vertical.png"));
	flipV->setCheckable( true );

	m_lineMode = false;

	languageChange();

	connect(rotationSpin, SIGNAL(valueChanged(double)), this, SLOT(handleRotation()));
	connect(flipH, SIGNAL(clicked()), this, SLOT(handleFlipH()));
	connect(flipV, SIGNAL(clicked()), this, SLOT(handleFlipV()));
	connect(levelUp, SIGNAL(clicked()), this, SLOT(handleRaise()));
	connect(levelDown, SIGNAL(clicked()), this, SLOT(handleLower()));
	connect(levelTop, SIGNAL(clicked()), this, SLOT(handleFront()));
	connect(levelBottom, SIGNAL(clicked()), this, SLOT(handleBack()));
	connect(doGroup  , SIGNAL(clicked()), this, SLOT(handleGrouping()) );
	connect(doUnGroup, SIGNAL(clicked()), this, SLOT(handleUngrouping()) );

	m_haveItem = false;

	rotationSpin->showValue(0);
}

void PropertyWidgetFrame_XYZTransform::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(mw->appModeHelper, SIGNAL(AppModeChanged(int, int)), this, SLOT(handleAppModeChanged(int, int)));
}

void PropertyWidgetFrame_XYZTransform::setDoc(ScribusDoc *d)
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

	rotationSpin->setValues( 0, 359.99, 1, 0);

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidgetFrame_XYZTransform::unsetDoc()
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

	doGroup->setEnabled(false);
	doUnGroup->setEnabled(false);
	flipH->setEnabled(false);
	flipV->setEnabled(false);
	rotationSpin->showValue(0);

	setEnabled(false);
}

void PropertyWidgetFrame_XYZTransform::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	handleSelectionChanged();
}

void PropertyWidgetFrame_XYZTransform::setLineMode(int lineMode)
{
	if (lineMode == 0)
	{
		rotationSpin->setEnabled(true);

		m_lineMode = false;
	}
	else
	{
		rotationSpin->setEnabled(false);

		m_lineMode = true;
	}
}

PageItem* PropertyWidgetFrame_XYZTransform::currentItemFromSelection()
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

void PropertyWidgetFrame_XYZTransform::setCurrentItem(PageItem *i)
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

	m_haveItem = false;
	m_item = i;

	levelLabel->setText( QString::number(m_item->level()) );

//CB replaces old emits from PageItem::emitAllToGUI()
	disconnect(flipH, SIGNAL(clicked()), this, SLOT(handleFlipH()));
	disconnect(flipV, SIGNAL(clicked()), this, SLOT(handleFlipV()));
	disconnect(rotationSpin, SIGNAL(valueChanged(double)), this, SLOT(handleRotation()));

	double selX = m_item->xPos();
	double selY = m_item->yPos();
	double selW = m_item->width();
	double selH = m_item->height();
	if (m_doc->m_Selection->count() > 1)
		m_doc->m_Selection->getGroupRect(&selX, &selY, &selW, &selH);
	
	bool checkableFlip = (i->isImageFrame() || i->isTextFrame() || i->isLatexFrame() || i->isOSGFrame() || i->isSymbol() || i->isGroup() || i->isSpiral());
	flipH->setCheckable(checkableFlip);
	flipV->setCheckable(checkableFlip);

	showFlippedH(i->imageFlippedH());
	showFlippedV(i->imageFlippedV());
	double rr = i->rotation();
	if (i->rotation() > 0)
		rr = 360 - rr;
	m_oldRotation = fabs(rr);
	rotationSpin->setValue(fabs(rr));

//CB TODO reconnect PP signals from here
	connect(flipH   , SIGNAL(clicked()), this, SLOT(handleFlipH()));
	connect(flipV   , SIGNAL(clicked()), this, SLOT(handleFlipV()));
	connect(rotationSpin, SIGNAL(valueChanged(double)), this, SLOT(handleRotation()));

	bool haveSameParent = m_doc->m_Selection->objectsHaveSameParent();
	levelGroup->setEnabled(haveSameParent && !i->locked());
	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		setEnabled(true);
	}
	if ((m_item->itemType() == PageItem::Line) && m_lineMode)
	{
		rotationSpin->setEnabled(false);
	}
	else
	{
		rotationSpin->setEnabled(true);
	}
	m_haveItem = true;

	showLocked(i->locked());

//	double rrR = i->imageRotation();
//	if (i->imageRotation() > 0)
//		rrR = 360 - rrR;
	
	doGroup->setEnabled(false);
	doUnGroup->setEnabled(false);
	if ((m_doc->m_Selection->count() > 1) && (haveSameParent))
		doGroup->setEnabled(true);
	if (m_doc->m_Selection->count() == 1)
		doUnGroup->setEnabled(m_item->isGroup());
	if ((m_doc->appMode == modeEditClip) && (m_item->isGroup()))
		doUnGroup->setEnabled(false);

	if (m_item->asOSGFrame())
	{
		setEnabled(true);
		rotationSpin->setEnabled(false);
	}
	if (m_item->asSymbolFrame())
	{
		setEnabled(true);
	}
}

void PropertyWidgetFrame_XYZTransform::handleAppModeChanged(int oldMode, int mode)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	doUnGroup->setEnabled(mode != modeEdit && mode != modeEditClip && m_item->isGroup()); //
}

void PropertyWidgetFrame_XYZTransform::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{
		m_oldRotation = 0;

		rotationSpin->showValue(0);
		rotationSpin->setEnabled(true);

		flipH->setCheckable( false );
		flipV->setCheckable( false );
		flipH->setChecked(false);
		flipV->setChecked(false);

		flipH->setEnabled(true);
		flipV->setEnabled(true);

		setEnabled(true);
	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		
		m_haveItem = (itemType!=-1);
		if (itemType == -1) //
		{
			doGroup->setEnabled(false);
			doUnGroup->setEnabled(false);
		}

		setEnabled(true);

		//CB If Toggle is not possible, then we need to enable it so we can turn it off
		//It then gets reset below for items where its valid
		flipH->setCheckable(true);
		flipV->setCheckable(true);
		if ((itemType == 2) || (itemType == 4) || ((itemType >= 9) && (itemType <= 12)) || (itemType == 15))
		{
			flipH->setCheckable(true);
			flipV->setCheckable(true);
		}
		else
		{
			flipH->setCheckable(false);
			flipV->setCheckable(false);
			flipH->setChecked(false);
			flipV->setChecked(false);
		}
		
		//CB Why can't we do this for lines?
//		flipH->setEnabled((itemType !=-1) && (itemType !=5));
//		flipV->setEnabled((itemType !=-1) && (itemType !=5));
		flipH->setEnabled(itemType !=-1);
		flipV->setEnabled(itemType !=-1);
		switch (itemType)
		{
		case -1:
			rotationSpin->showValue(0);
			levelLabel->setText("  ");
			setEnabled(false);
			break;
		case PageItem::ImageFrame:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
#ifdef HAVE_OSG
			if (currItem->asOSGFrame())
			{
				setEnabled(true);
				rotationSpin->setEnabled(false);
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
	updateGeometry();
	//repaint();
}

void PropertyWidgetFrame_XYZTransform::unitChange()
{
	if (!m_haveDoc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;
	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();
	m_haveItem = tmp;
}


void PropertyWidgetFrame_XYZTransform::showRotation(double r)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	double rr = r;
	if (r > 0)
		rr = 360 - rr;
	bool sigBlocked = rotationSpin->blockSignals(true);
	rotationSpin->setValue(fabs(rr));
	rotationSpin->blockSignals(sigBlocked);
}

void PropertyWidgetFrame_XYZTransform::sendRotation(){
	emit setRotation(rotationSpin->value()*(-1));
}


void PropertyWidgetFrame_XYZTransform::handleRotation()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	double gx, gy, gh, gw;
	if ((m_haveDoc) && (m_haveItem))
	{
		sendRotation();

		if (!_userActionOn)
			m_ScMW->view->startGroupTransaction(Um::Rotate, "", Um::IRotate);
		if (m_doc->m_Selection->isMultipleSelection())
		{
			m_doc->rotateGroup((rotationSpin->value() - m_oldRotation)*(-1), m_ScMW->view->RCenter);
			m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);

			emit updateXY(gx,gy);
		}
		else
			m_doc->rotateItem(rotationSpin->value()*(-1), m_item);
		if (!_userActionOn)
		{
			for (int i = 0; i < m_doc->m_Selection->count(); ++i)
				m_doc->m_Selection->itemAt(i)->checkChanges(true);
			m_ScMW->view->endGroupTransaction();
		}
		m_doc->changed();
		m_doc->regionsChanged()->update(QRect());
		m_oldRotation = rotationSpin->value();
	}
}

void PropertyWidgetFrame_XYZTransform::handleLower()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_LowerItem();
	levelLabel->setText( QString::number(m_item->level()) );
}

void PropertyWidgetFrame_XYZTransform::handleRaise()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_RaiseItem();
	levelLabel->setText( QString::number(m_item->level()) );
}

void PropertyWidgetFrame_XYZTransform::handleFront()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->bringItemSelectionToFront();
	levelLabel->setText( QString::number(m_item->level()) );
}

void PropertyWidgetFrame_XYZTransform::handleBack()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->sendItemSelectionToBack();
	levelLabel->setText( QString::number(m_item->level()) );
}


void PropertyWidgetFrame_XYZTransform::handleFlipH()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->scrActions["itemFlipH"]->toggle();
}

void PropertyWidgetFrame_XYZTransform::handleFlipV()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->scrActions["itemFlipV"]->toggle();
}


void PropertyWidgetFrame_XYZTransform::installSniffer(ScrSpinBox *spinBox)
{
	const QList<QObject*> list = spinBox->children();
	if (!list.isEmpty())
	{
		QListIterator<QObject*> it(list);
		QObject *obj;
		while (it.hasNext())
		{
			obj = it.next();
			obj->installEventFilter(userActionSniffer);
		}
	}
}

bool PropertyWidgetFrame_XYZTransform::userActionOn()
{
	return _userActionOn;
}

void PropertyWidgetFrame_XYZTransform::spinboxStartUserAction()
{
	_userActionOn = true;
}

void PropertyWidgetFrame_XYZTransform::spinboxFinishUserAction()
{
	_userActionOn = false;

	for (int i = 0; i < m_doc->m_Selection->count(); ++i)
		m_doc->m_Selection->itemAt(i)->checkChanges(true);
	if (m_ScMW->view->groupTransactionStarted())
	{
		m_ScMW->view->endGroupTransaction();
	}
}

void PropertyWidgetFrame_XYZTransform::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
	}
	else
		QWidget::changeEvent(e);
}

void PropertyWidgetFrame_XYZTransform::languageChange()
{
	retranslateUi(this);

}


void PropertyWidgetFrame_XYZTransform::showLocked(bool isLocked)
{

	rotationSpin->setReadOnly(isLocked);
	QPalette pal(qApp->palette());
	if (isLocked)
		pal.setCurrentColorGroup(QPalette::Disabled);

	rotationSpin->setPalette(pal);

}

void PropertyWidgetFrame_XYZTransform::showFlippedH(bool isFlippedH)
{
	flipH->setChecked(isFlippedH);
}

void PropertyWidgetFrame_XYZTransform::showFlippedV(bool isFlippedV)
{
	flipV->setChecked(isFlippedV);
}

void PropertyWidgetFrame_XYZTransform::handleGrouping()
{
	m_ScMW->GroupObj();
	doGroup->setEnabled(false);
	doUnGroup->setEnabled(true);
	handleSelectionChanged();

}

void PropertyWidgetFrame_XYZTransform::handleUngrouping()
{
	m_ScMW->UnGroupObj();
	m_doc->invalidateAll();
	m_doc->regionsChanged()->update(QRect());
}
