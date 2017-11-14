/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#include "propertywidgetframe_xyz.h"

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

PropertyWidgetFrame_XYZ::PropertyWidgetFrame_XYZ( QWidget* parent) : QWidget(parent)
{
	m_ScMW=0;
	m_doc=0;
	m_haveDoc  = false;
	m_haveItem = false;
	m_lineMode = false;
	m_oldRotation = 0;
	m_unitRatio = 1.0;

	setupUi(this);
	setSizePolicy( QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

	_userActionOn = false;
	userActionSniffer = new UserActionSniffer(this);
	connect(userActionSniffer, SIGNAL(actionStart()), this, SLOT(spinboxStartUserAction()));
	connect(userActionSniffer, SIGNAL(actionEnd()), this, SLOT(spinboxFinishUserAction()));

	installSniffer(xposSpin);
	installSniffer(yposSpin);
	installSniffer(widthSpin);
	installSniffer(heightSpin);

	keepFrameWHRatioButton->setCheckable( true );
	keepFrameWHRatioButton->setAutoRaise( true );
	keepFrameWHRatioButton->setMaximumSize( QSize( 15, 32767 ) );
	keepFrameWHRatioButton->setChecked(false);
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
	
	doLock->setCheckable( true );
	QIcon a = QIcon();
	a.addPixmap(im->loadPixmap("16/lock.png"), QIcon::Normal, QIcon::On);
	a.addPixmap(im->loadPixmap("16/lock-unlocked.png"), QIcon::Normal, QIcon::Off);
	doLock->setIcon(a);

	noResize->setCheckable( true );
	QIcon a3 = QIcon();
	a3.addPixmap(im->loadPixmap("framenoresize.png"), QIcon::Normal, QIcon::On);
	a3.addPixmap(im->loadPixmap("frameresize.png"), QIcon::Normal, QIcon::Off);
	noResize->setIcon(a3);

	m_lineMode = false;

	languageChange();

	connect(xposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewX()));
	connect(yposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewY()));
	connect(widthSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewW()));
	connect(heightSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewH()));
	connect(rotationSpin, SIGNAL(valueChanged(double)), this, SLOT(handleRotation()));
	connect(flipH, SIGNAL(clicked()), this, SLOT(handleFlipH()));
	connect(flipV, SIGNAL(clicked()), this, SLOT(handleFlipV()));
	connect(levelUp, SIGNAL(clicked()), this, SLOT(handleRaise()));
	connect(levelDown, SIGNAL(clicked()), this, SLOT(handleLower()));
	connect(levelTop, SIGNAL(clicked()), this, SLOT(handleFront()));
	connect(levelBottom, SIGNAL(clicked()), this, SLOT(handleBack()));
	connect(basePointWidget, SIGNAL(buttonClicked(int)), this, SLOT(handleBasePoint(int)));

	connect(doLock   , SIGNAL(clicked()), this, SLOT(handleLock()));
	connect(noResize , SIGNAL(clicked()), this, SLOT(handleLockSize()));
	connect(doGroup  , SIGNAL(clicked()), this, SLOT(handleGrouping()) );
	connect(doUnGroup, SIGNAL(clicked()), this, SLOT(handleUngrouping()) );

	m_haveItem = false;
	xposSpin->showValue(0);
	yposSpin->showValue(0);
	widthSpin->showValue(0);
	heightSpin->showValue(0);
	rotationSpin->showValue(0);
}

void PropertyWidgetFrame_XYZ::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(mw->appModeHelper, SIGNAL(AppModeChanged(int, int)), this, SLOT(handleAppModeChanged(int, int)));
}

void PropertyWidgetFrame_XYZ::setDoc(ScribusDoc *d)
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
//qt4 FIXME here
	double maxXYWHVal =  16777215 * m_unitRatio;
	double minXYVal   = -16777215 * m_unitRatio;

	m_haveDoc = true;
	m_haveItem = false;

	QMap<QString, double>* docConstants = m_doc? &m_doc->constants()  : NULL;
	xposSpin->setValues( minXYVal, maxXYWHVal, precision, minXYVal);
	xposSpin->setConstants(docConstants);
	yposSpin->setValues( minXYVal, maxXYWHVal, precision, minXYVal);
	yposSpin->setConstants(docConstants);
	widthSpin->setValues( m_unitRatio, maxXYWHVal, precision, m_unitRatio);
	widthSpin->setConstants(docConstants);
	heightSpin->setValues( m_unitRatio, maxXYWHVal, precision, m_unitRatio);
	heightSpin->setConstants(docConstants);

	rotationSpin->setValues( 0, 359.99, 1, 0);

	updateSpinBoxConstants();

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
}

void PropertyWidgetFrame_XYZ::unsetDoc()
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
	xposSpin->setConstants(NULL);
	yposSpin->setConstants(NULL);
	widthSpin->setConstants(NULL);
	heightSpin->setConstants(NULL);
	doGroup->setEnabled(false); //
	doUnGroup->setEnabled(false); //
	flipH->setEnabled(false);
	flipV->setEnabled(false);
	xposLabel->setText( tr( "&X-Pos" ) );
	yposLabel->setText( tr( "&Y-Pos" ) );
	widthLabel->setText( tr( "&Width" ) );
	heightLabel->setText( tr( "&Height" ) );
	xposSpin->showValue(0);
	yposSpin->showValue(0);
	widthSpin->showValue(0);
	heightSpin->showValue(0);
	rotationSpin->showValue(0);
	setEnabled(false);
}

void PropertyWidgetFrame_XYZ::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	handleSelectionChanged();
}

void PropertyWidgetFrame_XYZ::setLineMode(int lineMode)
{
	if (lineMode == 0)
	{
		xposLabel->setText( tr( "&X-Pos:" ) );
		yposLabel->setText( tr( "&Y-Pos:" ) );
		widthLabel->setText( tr( "&Width:" ) );
		heightLabel->setText( tr( "&Height:" ) );
		rotationSpin->setEnabled(true);
		heightSpin->setEnabled(false);
		m_lineMode = false;
	}
	else
	{
		xposLabel->setText( tr( "&X1:" ) );
		yposLabel->setText( tr( "Y&1:" ) );
		widthLabel->setText( tr( "X&2:" ) );
		heightLabel->setText( tr( "&Y2:" ) );
		rotationSpin->setEnabled(false);
		heightSpin->setEnabled(true);
		m_lineMode = true;
	}
}

PageItem* PropertyWidgetFrame_XYZ::currentItemFromSelection()
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

void PropertyWidgetFrame_XYZ::setCurrentItem(PageItem *i)
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
	disconnect(xposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewX()));
	disconnect(yposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewY()));
	disconnect(widthSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewW()));
	disconnect(heightSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewH()));
	disconnect(doLock, SIGNAL(clicked()), this, SLOT(handleLock()));
	disconnect(noResize, SIGNAL(clicked()), this, SLOT(handleLockSize()));
	disconnect(flipH, SIGNAL(clicked()), this, SLOT(handleFlipH()));
	disconnect(flipV, SIGNAL(clicked()), this, SLOT(handleFlipV()));
	disconnect(rotationSpin, SIGNAL(valueChanged(double)), this, SLOT(handleRotation()));

	double selX = m_item->xPos();
	double selY = m_item->yPos();
	double selW = m_item->width();
	double selH = m_item->height();
	if (m_doc->m_Selection->count() > 1)
		m_doc->m_Selection->getGroupRect(&selX, &selY, &selW, &selH);
//	showXY(selX, selY);
//	showWH(selW, selH);
	
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
	connect(xposSpin    , SIGNAL(valueChanged(double)), this, SLOT(handleNewX()));
	connect(yposSpin    , SIGNAL(valueChanged(double)), this, SLOT(handleNewY()));
	connect(widthSpin   , SIGNAL(valueChanged(double)), this, SLOT(handleNewW()));
	connect(heightSpin  , SIGNAL(valueChanged(double)), this, SLOT(handleNewH()));
	connect(doLock  , SIGNAL(clicked()), this, SLOT(handleLock()));
	connect(noResize, SIGNAL(clicked()), this, SLOT(handleLockSize()));
	connect(flipH   , SIGNAL(clicked()), this, SLOT(handleFlipH()));
	connect(flipV   , SIGNAL(clicked()), this, SLOT(handleFlipV()));
	connect(rotationSpin, SIGNAL(valueChanged(double)), this, SLOT(handleRotation()));

	bool setter = false;
	xposSpin->setEnabled(!setter);
	yposSpin->setEnabled(!setter);
	bool haveSameParent = m_doc->m_Selection->objectsHaveSameParent();
	levelGroup->setEnabled(haveSameParent && !i->locked());
	if ((m_item->isGroup()) && (!m_item->isSingleSel))
	{
		setEnabled(true);
	}
	if ((m_item->itemType() == PageItem::Line) && m_lineMode)
	{
		xposLabel->setText( tr( "&X1:" ) );
		yposLabel->setText( tr( "Y&1:" ) );
		widthLabel->setText( tr( "X&2:" ) );
		heightLabel->setText( tr( "&Y2:" ) );
		rotationSpin->setEnabled(false);
	}
	else
	{
		xposLabel->setText( tr( "&X-Pos:" ) );
		yposLabel->setText( tr( "&Y-Pos:" ) );
		widthLabel->setText( tr( "&Width:" ) );
		heightLabel->setText( tr( "&Height:" ) );
		rotationSpin->setEnabled(true);
	}
	m_haveItem = true;
	if (m_item->asLine())
	{
		keepFrameWHRatioButton->setEnabled(false);
		heightSpin->setEnabled(m_lineMode && !m_item->locked());
	}
	else
	{
		heightSpin->setEnabled(true);
		keepFrameWHRatioButton->setEnabled(true);
	}
	showXY(selX, selY);
	showWH(selW, selH);
	showLocked(i->locked());
	showSizeLocked(i->sizeLocked());
	double rrR = i->imageRotation();
	if (i->imageRotation() > 0)
		rrR = 360 - rrR;
	noResize->setEnabled(!m_item->isArc());
	
	doGroup->setEnabled(false); //
	doUnGroup->setEnabled(false);
	if ((m_doc->m_Selection->count() > 1) && (haveSameParent))
		doGroup->setEnabled(true);
	if (m_doc->m_Selection->count() == 1)
		doUnGroup->setEnabled(m_item->isGroup());
	if ((m_doc->appMode == modeEditClip) && (m_item->isGroup()))
		doUnGroup->setEnabled(false);//

	if (m_item->asOSGFrame())
	{
		setEnabled(true);
		rotationSpin->setEnabled(false);
	}
	if (m_item->asSymbolFrame())
	{
		setEnabled(true);
	}
	updateSpinBoxConstants();
}

void PropertyWidgetFrame_XYZ::handleAppModeChanged(int oldMode, int mode)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	doUnGroup->setEnabled(mode != modeEdit && mode != modeEditClip && m_item->isGroup()); //
	doLock->setEnabled(mode != modeEditClip);
}

void PropertyWidgetFrame_XYZ::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{
		m_oldRotation = 0;
		double gx, gy, gh, gw;
		m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
		int bp = basePointWidget->checkedId();
		if (bp == 0)
			m_ScMW->view->RCenter = FPoint(gx, gy);
		else if (bp == 1)
			m_ScMW->view->RCenter = FPoint(gx + gw, gy);
		else if (bp == 2)
			m_ScMW->view->RCenter = FPoint(gx + gw / 2.0, gy + gh / 2.0);
		else if (bp == 3)
			m_ScMW->view->RCenter = FPoint(gx, gy + gh);
		else if (bp == 0)
			m_ScMW->view->RCenter = FPoint(gx + gw, gy + gh);
		xposLabel->setText( tr( "&X-Pos:" ) );
		yposLabel->setText( tr( "&Y-Pos:" ) );
		widthLabel->setText( tr( "&Width:" ) );
		heightLabel->setText( tr( "&Height:" ) );

		xposSpin->showValue(gx);
		yposSpin->showValue(gy);
		widthSpin->showValue(gw);
		heightSpin->showValue(gh);
		rotationSpin->showValue(0);

		xposSpin->setEnabled(true);
		yposSpin->setEnabled(true);
		widthSpin->setEnabled(true);
		heightSpin->setEnabled(true);
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

		basePointWidget->setEnabled(true);

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
			xposLabel->setText( tr( "&X-Pos:" ) );
			yposLabel->setText( tr( "&Y-Pos:" ) );
			widthLabel->setText( tr( "&Width:" ) );
			heightLabel->setText( tr( "&Height:" ) );

			xposSpin->showValue(0);
			yposSpin->showValue(0);
			widthSpin->showValue(0);
			heightSpin->showValue(0);
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
			basePointWidget->setEnabled(false);
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

void PropertyWidgetFrame_XYZ::unitChange()
{
	if (!m_haveDoc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;
	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();
	xposSpin->setNewUnit( m_unitIndex );
	yposSpin->setNewUnit( m_unitIndex );
	widthSpin->setNewUnit( m_unitIndex );
	heightSpin->setNewUnit( m_unitIndex );
	m_haveItem = tmp;
}

void PropertyWidgetFrame_XYZ::showXY(double x, double y)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	disconnect(xposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewX()));
	disconnect(yposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewY()));
	bool useLineMode = false;
	bool tmp = m_haveItem;
	double inX, inY, b, h, r, dummy1, dummy2;
	QTransform ma;
	FPoint n;
	if (m_haveItem)
	{
		if (m_doc->m_Selection->isMultipleSelection())
		{
			m_doc->m_Selection->getGroupRect(&dummy1, &dummy2, &b, &h);
			r = 0.0;
			ma.translate(dummy1, dummy2);
		}
		else
		{
			b = m_item->width();
			h = m_item->height();
			r = m_item->rotation();
			ma.translate(x, y);
			useLineMode = (m_lineMode && m_item->isLine());
		}
	}
	else
	{
		b = 0.0;
		h = 0.0;
		r = 0.0;
		ma.translate(x, y);
	}
	m_haveItem = false;
	ma.rotate(r);
	int bp = basePointWidget->checkedId();
	// #8890 : basepoint is meaningless when lines use "end points" mode
	if (bp == 0 || useLineMode)
		n = FPoint(0.0, 0.0);
	else if (bp == 1)
		n = FPoint(b, 0.0);
	else if (bp == 2)
		n = FPoint(b / 2.0, h / 2.0);
	else if (bp == 3)
		n = FPoint(0.0, h);
	else if (bp == 4)
		n = FPoint(b, h);
	inX = ma.m11() * n.x() + ma.m21() * n.y() + ma.dx();
	inY = ma.m22() * n.y() + ma.m12() * n.x() + ma.dy();
	if (tmp)
	{
		inX -= m_doc->rulerXoffset;
		inY -= m_doc->rulerYoffset;
		if (m_doc->guidesPrefs().rulerMode)
		{
			inX -= m_doc->currentPage()->xOffset();
			inY -= m_doc->currentPage()->yOffset();
		}
	}
	xposSpin->setValue(inX*m_unitRatio);
	yposSpin->setValue(inY*m_unitRatio);
	if (useLineMode)
		showWH(m_item->width(), m_item->height());
	m_haveItem = tmp;
	connect(xposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewX()));
	connect(yposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewY()));
}

void PropertyWidgetFrame_XYZ::showWH(double x, double y)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	QTransform ma;
	QPoint dp;
	bool sigBlocked1 = widthSpin->blockSignals(true);
	bool sigBlocked2 = heightSpin->blockSignals(true);
	if ((m_lineMode) && (m_item->asLine()))
	{
		ma.translate(static_cast<double>(xposSpin->value()) / m_unitRatio, static_cast<double>(yposSpin->value()) / m_unitRatio);
		ma.rotate(static_cast<double>(rotationSpin->value())*(-1));
		dp = QPoint(static_cast<int>(x), static_cast<int>(y)) * ma;
		widthSpin->setValue(dp.x()*m_unitRatio);
		heightSpin->setValue(dp.y()*m_unitRatio);
	}
	else
	{
		widthSpin->setValue(x*m_unitRatio);
		heightSpin->setValue(y*m_unitRatio);
	}
	widthSpin->blockSignals(sigBlocked1);
	heightSpin->blockSignals(sigBlocked2);
}

void PropertyWidgetFrame_XYZ::showRotation(double r)
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

void PropertyWidgetFrame_XYZ::handleNewX()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		double x,y,w,h, gx, gy, gh, gw, base;
		QTransform ma;
		x = xposSpin->value() / m_unitRatio;
		y = yposSpin->value() / m_unitRatio;
		w = widthSpin->value() / m_unitRatio;
		h = heightSpin->value() / m_unitRatio;
		base = 0;
		x += m_doc->rulerXoffset;
		y += m_doc->rulerYoffset;
		if (m_doc->guidesPrefs().rulerMode)
		{
			x += m_doc->currentPage()->xOffset();
			y += m_doc->currentPage()->yOffset();
		}
		if (m_doc->m_Selection->isMultipleSelection())
		{
			m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
			int bp = basePointWidget->checkedId();
			if ((bp == 0) || (bp == 3))
				base = gx;
			else if (bp == 2)
				base = gx + gw / 2.0;
			else if ((bp == 1) || (bp == 4))
				base = gx + gw;
			if (!_userActionOn)
				m_ScMW->view->startGroupTransaction();
			m_doc->moveGroup(x - base, 0);
			if (!_userActionOn)
			{
				m_ScMW->view->endGroupTransaction();
			}
			m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
			showXY(gx, gy);
		}
		else
		{
			if ((m_item->asLine()) && (m_lineMode))
			{
				w += m_doc->rulerXoffset;
				h += m_doc->rulerYoffset;
				if (m_doc->guidesPrefs().rulerMode)
				{
					w += m_doc->currentPage()->xOffset();
					h += m_doc->currentPage()->yOffset();
				}
				double r = atan2(h-y,w-x)*(180.0/M_PI);
				w = sqrt(pow(w-x,2)+pow(h-y,2));
				m_item->setXYPos(x, m_item->yPos(), true);
				m_item->setRotation(r, true);
				m_doc->sizeItem(w, m_item->height(), m_item, true);
			}
			else
			{
				ma.translate(m_item->xPos(), m_item->yPos());
				ma.rotate(m_item->rotation());
				int bp = basePointWidget->checkedId();
				if (bp == 0)
					base = m_item->xPos();
				else if (bp == 2)
					base = ma.m11() * (m_item->width() / 2.0) + ma.m21() * (m_item->height() / 2.0) + ma.dx();
				else if (bp == 1)
					base = ma.m11() * m_item->width() + ma.m21() * 0.0 + ma.dx();
				else if (bp == 4)
					base = ma.m11() * m_item->width() + ma.m21() * m_item->height() + ma.dx();
				else if (bp == 3)
					base = ma.m11() * 0.0 + ma.m21() * m_item->height() + ma.dx();
				m_doc->moveItem(x - base, 0, m_item);
			}
		}
		m_doc->regionsChanged()->update(QRect());
		m_doc->changed();
	}
}

void PropertyWidgetFrame_XYZ::handleNewY()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	double x,y,w,h, gx, gy, gh, gw, base;
	QTransform ma;
	x = xposSpin->value() / m_unitRatio;
	y = yposSpin->value() / m_unitRatio;
	w = widthSpin->value() / m_unitRatio;
	h = heightSpin->value() / m_unitRatio;
	base = 0;
	x += m_doc->rulerXoffset;
	y += m_doc->rulerYoffset;
	if (m_doc->guidesPrefs().rulerMode)
	{
		x += m_doc->currentPage()->xOffset();
		y += m_doc->currentPage()->yOffset();
	}
	if (m_doc->m_Selection->isMultipleSelection())
	{
		m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
		int bp = basePointWidget->checkedId();
		if ((bp == 0) || (bp == 1))
			base = gy;
		else if (bp == 2)
			base = gy + gh / 2.0;
		else if ((bp == 3) || (bp == 4))
			base = gy + gh;
		if (!_userActionOn)
			m_ScMW->view->startGroupTransaction();
		m_doc->moveGroup(0, y - base);
		if (!_userActionOn)
		{
			m_ScMW->view->endGroupTransaction();
		}
		m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
		showXY(gx, gy);
	}
	else
	{
		if ((m_item->asLine()) && (m_lineMode))
		{
			w += m_doc->rulerXoffset;
			h += m_doc->rulerYoffset;
			if (m_doc->guidesPrefs().rulerMode)
			{
				w += m_doc->currentPage()->xOffset();
				h += m_doc->currentPage()->yOffset();
			}
			double r = atan2(h-y,w-x)*(180.0/M_PI);
			w = sqrt(pow(w-x,2)+pow(h-y,2));
			m_doc->moveItem(0, y - m_item->yPos(), m_item);
			m_item->setXYPos(m_item->xPos(), y, true);
			m_item->setRotation(r, true);
			m_doc->sizeItem(w, m_item->height(), m_item, true);
			m_doc->rotateItem(r, m_item);
		}
		else
		{
			ma.translate(m_item->xPos(), m_item->yPos());
			ma.rotate(m_item->rotation());
			int bp = basePointWidget->checkedId();
			if (bp == 0)
				base = m_item->yPos();
			else if (bp == 2)
				base = ma.m22() * (m_item->height() / 2.0) + ma.m12() * (m_item->width() / 2.0) + ma.dy();
			else if (bp == 1)
				base = ma.m22() * 0.0 + ma.m12() * m_item->width() + ma.dy();
			else if (bp == 4)
				base = ma.m22() * m_item->height() + ma.m12() * m_item->width() + ma.dy();
			else if (bp == 3)
				base = ma.m22() * m_item->height() + ma.m12() * 0.0 + ma.dy();
			m_doc->moveItem(0, y - base, m_item);
		}
	}
	m_doc->regionsChanged()->update(QRect());
	m_doc->changed();
}

void PropertyWidgetFrame_XYZ::handleNewW()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	
	double x,y,w,h, gx, gy, gh, gw;
	x = xposSpin->value() / m_unitRatio;
	y = yposSpin->value() / m_unitRatio;
	w = widthSpin->value() / m_unitRatio;
	h = heightSpin->value() / m_unitRatio;
	double oldW = (m_item->width()  != 0.0) ? m_item->width()  : 1.0;
	double oldH = (m_item->height() != 0.0) ? m_item->height() : 1.0;
	if (m_doc->m_Selection->isMultipleSelection())
	{
		if (!_userActionOn)
			m_ScMW->view->startGroupTransaction();
		m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
		if (keepFrameWHRatioButton->isChecked())
		{
			m_doc->scaleGroup(w / gw, w / gw, false);
			showWH(w, (w / gw) * gh);
		}
		else
		{
			m_doc->scaleGroup(w / gw, 1.0, false);
			m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
			showWH(gw, gh);
		}
		if (!_userActionOn)
		{
			m_ScMW->view->endGroupTransaction();
		}
	}
	else
	{
		bool oldS = m_item->Sizing;
		m_item->Sizing = false;
		m_item->OldB2 = m_item->width();
		m_item->OldH2 = m_item->height();
		if (m_item->asLine())
		{
			if (m_lineMode)
			{
				double r = atan2(h-y,w-x)*(180.0/M_PI);
				m_item->setRotation(r, true);
				w = sqrt(pow(w-x,2)+pow(h-y,2));
			}
			m_doc->sizeItem(w, m_item->height(), m_item, true, true, false);
		}
		else
		{
			if (keepFrameWHRatioButton->isChecked())
			{
				showWH(w, (w / oldW) * m_item->height());
				m_doc->sizeItem(w, (w / oldW) * m_item->height(), m_item, true, true, false);
			}
			else
				m_doc->sizeItem(w, m_item->height(), m_item, true, true, false);
		}
		if (m_item->isArc())
		{
			double dw = w - oldW;
			double dh = h - oldH;
			PageItem_Arc* item = m_item->asArc();
			double dsch = item->arcHeight / oldH;
			double dscw = item->arcWidth / oldW;
			item->arcWidth += dw * dscw;
			item->arcHeight += dh * dsch;
			item->recalcPath();
		}
		if (m_item->isSpiral())
		{
			PageItem_Spiral* item = m_item->asSpiral();
			item->recalcPath();
		}
		m_item->Sizing = oldS;
	}
	m_doc->changed();
	m_doc->regionsChanged()->update(QRect());
	m_ScMW->setStatusBarTextSelectedItemInfo();
}

void PropertyWidgetFrame_XYZ::handleNewH()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	double x,y,w,h, gx, gy, gh, gw;
	x = xposSpin->value() / m_unitRatio;
	y = yposSpin->value() / m_unitRatio;
	w = widthSpin->value() / m_unitRatio;
	h = heightSpin->value() / m_unitRatio;
	double oldW = (m_item->width()  != 0.0) ? m_item->width()  : 1.0;
	double oldH = (m_item->height() != 0.0) ? m_item->height() : 1.0;
	if (m_doc->m_Selection->isMultipleSelection())
	{
		if (!_userActionOn)
			m_ScMW->view->startGroupTransaction();
		m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
		if (keepFrameWHRatioButton->isChecked())
		{
			m_doc->scaleGroup(h / gh, h / gh, false);
			showWH((h / gh) * gw, h);
		}
		else
		{
			m_doc->scaleGroup(1.0, h / gh, false);
			m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
			showWH(gw, gh);
		}
		if (!_userActionOn)
		{
			m_ScMW->view->endGroupTransaction();
		}
	}
	else
	{
		bool oldS = m_item->Sizing;
		m_item->Sizing = false;
		m_item->OldB2 = m_item->width();
		m_item->OldH2 = m_item->height();
		if (m_item->asLine())
		{
			if (m_lineMode)
			{
				double r = atan2(h-y,w-x)*(180.0/M_PI);
				m_item->setRotation(r, true);
				w = sqrt(pow(w-x,2)+pow(h-y,2));
			}
			m_doc->sizeItem(w, m_item->height(), m_item, true, true, false);
		}
		else
		{
			if (keepFrameWHRatioButton->isChecked())
			{
				showWH((h / oldH) * m_item->width(), h);
				m_doc->sizeItem((h / oldH) * m_item->width(), h, m_item, true, true, false);
			}
			else
				m_doc->sizeItem(m_item->width(), h, m_item, true, true, false);
		}
		if (m_item->isArc())
		{
			double dw = w - oldW;
			double dh = h - oldH;
			PageItem_Arc* item = m_item->asArc();
			double dsch = item->arcHeight / oldH;
			double dscw = item->arcWidth / oldW;
			item->arcWidth += dw * dscw;
			item->arcHeight += dh * dsch;
			item->recalcPath();
		}
		if (m_item->isSpiral())
		{
			PageItem_Spiral* item = m_item->asSpiral();
			item->recalcPath();
		}
		m_item->Sizing = oldS;
	}
	m_doc->changed();
	m_doc->regionsChanged()->update(QRect());
	m_ScMW->setStatusBarTextSelectedItemInfo();
}

void PropertyWidgetFrame_XYZ::handleRotation()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	double gx, gy, gh, gw;
	if ((m_haveDoc) && (m_haveItem))
	{
		if (!_userActionOn)
			m_ScMW->view->startGroupTransaction(Um::Rotate, "", Um::IRotate);
		if (m_doc->m_Selection->isMultipleSelection())
		{
			m_doc->rotateGroup((rotationSpin->value() - m_oldRotation)*(-1), m_ScMW->view->RCenter);
			m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
			showXY(gx, gy);
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

void PropertyWidgetFrame_XYZ::handleLower()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_LowerItem();
	levelLabel->setText( QString::number(m_item->level()) );
}

void PropertyWidgetFrame_XYZ::handleRaise()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->itemSelection_RaiseItem();
	levelLabel->setText( QString::number(m_item->level()) );
}

void PropertyWidgetFrame_XYZ::handleFront()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->bringItemSelectionToFront();
	levelLabel->setText( QString::number(m_item->level()) );
}

void PropertyWidgetFrame_XYZ::handleBack()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_doc->sendItemSelectionToBack();
	levelLabel->setText( QString::number(m_item->level()) );
}

void PropertyWidgetFrame_XYZ::handleBasePoint(int m)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	double inX, inY, gx, gy, gh, gw;
	inX = 0;
	inY = 0;
	if ((m_haveDoc) && (m_haveItem))
	{
		m_haveItem = false;
		m_doc->setRotationMode(m);
		if (m_doc->m_Selection->isMultipleSelection())
		{
			m_doc->m_Selection->setGroupRect();
			m_doc->m_Selection->getGroupRect(&gx, &gy, &gw, &gh);
			if (m == 0)
			{
				m_ScMW->view->RCenter = FPoint(gx, gy);
				inX = gx;
				inY = gy;
			}
			if (m == 1)
			{
				m_ScMW->view->RCenter = FPoint(gx+gw, gy);
				inX = gx+gw;
				inY = gy;
			}
			if (m == 2)
			{
				m_ScMW->view->RCenter = FPoint(gx + gw / 2.0, gy + gh / 2.0);
				inX = gx + gw / 2.0;
				inY = gy + gh / 2.0;
			}
			if (m == 3)
			{
				m_ScMW->view->RCenter = FPoint(gx, gy+gh);
				inX = gx;
				inY = gy+gh;
			}
			if (m == 4)
			{
				m_ScMW->view->RCenter = FPoint(gx+gw, gy+gh);
				inX = gx+gw;
				inY = gy+gh;
			}
			inX -= m_doc->rulerXoffset;
			inY -= m_doc->rulerYoffset;
			if (m_doc->guidesPrefs().rulerMode)
			{
				inX -= m_doc->currentPage()->xOffset();
				inY -= m_doc->currentPage()->yOffset();
			}
			xposSpin->setValue(inX*m_unitRatio);
			yposSpin->setValue(inY*m_unitRatio);
		}
		else
		{
			double b, h, r;
			QTransform ma;
			FPoint n;
			b = m_item->width();
			h = m_item->height();
			r = m_item->rotation();
			ma.translate(m_item->xPos(), m_item->yPos());
			ma.rotate(r);
			int bp = basePointWidget->checkedId();
			if (bp == 0)
				n = FPoint(0.0, 0.0);
			else if (bp == 1)
				n = FPoint(b, 0.0);
			else if (bp == 2)
				n = FPoint(b / 2.0, h / 2.0);
			else if (bp == 3)
				n = FPoint(0.0, h);
			else if (bp == 4)
				n = FPoint(b, h);
			inX = ma.m11() * n.x() + ma.m21() * n.y() + ma.dx();
			inY = ma.m22() * n.y() + ma.m12() * n.x() + ma.dy();
			inX -= m_doc->rulerXoffset;
			inY -= m_doc->rulerYoffset;
			if (m_doc->guidesPrefs().rulerMode)
			{
				inX -= m_doc->currentPage()->xOffset();
				inY -= m_doc->currentPage()->yOffset();
			}
			xposSpin->setValue(inX*m_unitRatio);
			yposSpin->setValue(inY*m_unitRatio);
		}
		if (m_item->itemType() == PageItem::ImageFrame)
		{
			// FIXME
			if (false /*!FreeScale->isChecked()*/)
			{
				m_item->AdjustPictScale();
				m_item->update();
			}
		}
		m_haveItem = true;
	}
}

void PropertyWidgetFrame_XYZ::handleLock()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->scrActions["itemLock"]->toggle();
}

void PropertyWidgetFrame_XYZ::handleLockSize()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->scrActions["itemLockSize"]->toggle();
}

void PropertyWidgetFrame_XYZ::handleFlipH()
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->scrActions["itemFlipH"]->toggle();
}

void PropertyWidgetFrame_XYZ::handleFlipV()
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	m_ScMW->scrActions["itemFlipV"]->toggle();
}


void PropertyWidgetFrame_XYZ::installSniffer(ScrSpinBox *spinBox)
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

bool PropertyWidgetFrame_XYZ::userActionOn()
{
	return _userActionOn;
}

void PropertyWidgetFrame_XYZ::spinboxStartUserAction()
{
	_userActionOn = true;
}

void PropertyWidgetFrame_XYZ::spinboxFinishUserAction()
{
	_userActionOn = false;

	for (int i = 0; i < m_doc->m_Selection->count(); ++i)
		m_doc->m_Selection->itemAt(i)->checkChanges(true);
	if (m_ScMW->view->groupTransactionStarted())
	{
		m_ScMW->view->endGroupTransaction();
	}
}

void PropertyWidgetFrame_XYZ::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
	}
	else
		QWidget::changeEvent(e);
}

void PropertyWidgetFrame_XYZ::languageChange()
{
	setWindowTitle( tr("Properties"));
	retranslateUi(this);

	QString ptSuffix = tr(" pt");

	QString ein = (m_haveDoc) ? unitGetSuffixFromIndex(m_doc->unitIndex()) : ptSuffix;

	xposSpin->setSuffix(ein);
	yposSpin->setSuffix(ein);
	widthSpin->setSuffix(ein);
	heightSpin->setSuffix(ein);
}

void PropertyWidgetFrame_XYZ::updateSpinBoxConstants()
{
	if (!m_haveDoc)
		return;
	if(m_doc->m_Selection->count()==0)
		return;
	widthSpin->setConstants(&m_doc->constants());
	heightSpin->setConstants(&m_doc->constants());
	xposSpin->setConstants(&m_doc->constants());
	yposSpin->setConstants(&m_doc->constants());

}

void PropertyWidgetFrame_XYZ::showLocked(bool isLocked)
{
	xposSpin->setReadOnly(isLocked);
	yposSpin->setReadOnly(isLocked);
	widthSpin->setReadOnly(isLocked);
	heightSpin->setReadOnly(isLocked);
	rotationSpin->setReadOnly(isLocked);
	QPalette pal(qApp->palette());
	if (isLocked)
		pal.setCurrentColorGroup(QPalette::Disabled);

	xposSpin->setPalette(pal);
	yposSpin->setPalette(pal);
	widthSpin->setPalette(pal);
	heightSpin->setPalette(pal);
	rotationSpin->setPalette(pal);

	doLock->setChecked(isLocked);
}

void PropertyWidgetFrame_XYZ::showSizeLocked(bool isSizeLocked)
{
	bool b=isSizeLocked;
	if (m_haveItem && m_item->locked())
		b=true;
	widthSpin->setReadOnly(b);
	heightSpin->setReadOnly(b);
	QPalette pal(qApp->palette());
	
	if (b)
		pal.setCurrentColorGroup(QPalette::Disabled);

	widthSpin->setPalette(pal);
	heightSpin->setPalette(pal);
	noResize->setChecked(isSizeLocked);
}


void PropertyWidgetFrame_XYZ::showFlippedH(bool isFlippedH)
{
	flipH->setChecked(isFlippedH);
}

void PropertyWidgetFrame_XYZ::showFlippedV(bool isFlippedV)
{
	flipV->setChecked(isFlippedV);
}

void PropertyWidgetFrame_XYZ::handleGrouping()
{
	m_ScMW->GroupObj();
	doGroup->setEnabled(false);//
	doUnGroup->setEnabled(true);//
	handleSelectionChanged();
	//FIXME
	//TabStack->setItemEnabled(idShapeItem, false);
}

void PropertyWidgetFrame_XYZ::handleUngrouping()
{
	m_ScMW->UnGroupObj();
	m_doc->invalidateAll();
	m_doc->regionsChanged()->update(QRect());
}
