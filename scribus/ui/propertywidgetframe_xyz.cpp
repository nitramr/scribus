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
	m_rotation = 0;
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

	IconManager* im=IconManager::instance();
	
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
	connect(basePointWidget, SIGNAL(buttonClicked(int)), this, SLOT(handleBasePoint(int)));

	connect(doLock   , SIGNAL(clicked()), this, SLOT(handleLock()));
	connect(noResize , SIGNAL(clicked()), this, SLOT(handleLockSize()));

	m_haveItem = false;
	xposSpin->showValue(0);
	yposSpin->showValue(0);
	widthSpin->showValue(0);
	heightSpin->showValue(0);
}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void PropertyWidgetFrame_XYZ::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(mw->appModeHelper, SIGNAL(AppModeChanged(int, int)), this, SLOT(handleAppModeChanged(int, int)));
}

/*********************************************************************
*
* Doc
*
**********************************************************************/

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
	xposLabel->setText( tr( "&X-Pos" ) );
	yposLabel->setText( tr( "&Y-Pos" ) );
	widthLabel->setText( tr( "&Width" ) );
	heightLabel->setText( tr( "&Height" ) );
	xposSpin->showValue(0);
	yposSpin->showValue(0);
	widthSpin->showValue(0);
	heightSpin->showValue(0);

	setEnabled(false);
}

/*********************************************************************
*
* Item
*
**********************************************************************/

void PropertyWidgetFrame_XYZ::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;
	handleSelectionChanged();
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


//CB replaces old emits from PageItem::emitAllToGUI()
	disconnect(xposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewX()));
	disconnect(yposSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewY()));
	disconnect(widthSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewW()));
	disconnect(heightSpin, SIGNAL(valueChanged(double)), this, SLOT(handleNewH()));
	disconnect(doLock, SIGNAL(clicked()), this, SLOT(handleLock()));
	disconnect(noResize, SIGNAL(clicked()), this, SLOT(handleLockSize()));

	double selX = m_item->xPos();
	double selY = m_item->yPos();
	double selW = m_item->width();
	double selH = m_item->height();
	if (m_doc->m_Selection->count() > 1)
		m_doc->m_Selection->getGroupRect(&selX, &selY, &selW, &selH);

//CB TODO reconnect PP signals from here
	connect(xposSpin    , SIGNAL(valueChanged(double)), this, SLOT(handleNewX()));
	connect(yposSpin    , SIGNAL(valueChanged(double)), this, SLOT(handleNewY()));
	connect(widthSpin   , SIGNAL(valueChanged(double)), this, SLOT(handleNewW()));
	connect(heightSpin  , SIGNAL(valueChanged(double)), this, SLOT(handleNewH()));
	connect(doLock  , SIGNAL(clicked()), this, SLOT(handleLock()));
	connect(noResize, SIGNAL(clicked()), this, SLOT(handleLockSize()));

	bool setter = false;
	xposSpin->setEnabled(!setter);
	yposSpin->setEnabled(!setter);

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
	}
	else
	{
		xposLabel->setText( tr( "&X-Pos:" ) );
		yposLabel->setText( tr( "&Y-Pos:" ) );
		widthLabel->setText( tr( "&Width:" ) );
		heightLabel->setText( tr( "&Height:" ) );
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

	noResize->setEnabled(!m_item->isArc());
	
	if (m_item->asOSGFrame())
	{
		setEnabled(true);
	}
	if (m_item->asSymbolFrame())
	{
		setEnabled(true);
	}
	updateSpinBoxConstants();
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void PropertyWidgetFrame_XYZ::setLineMode(int lineMode)
{
	if (lineMode == 0)
	{
		xposLabel->setText( tr( "&X-Pos:" ) );
		yposLabel->setText( tr( "&Y-Pos:" ) );
		widthLabel->setText( tr( "&Width:" ) );
		heightLabel->setText( tr( "&Height:" ) );
		heightSpin->setEnabled(false);
		m_lineMode = false;
	}
	else
	{
		xposLabel->setText( tr( "&X1:" ) );
		yposLabel->setText( tr( "Y&1:" ) );
		widthLabel->setText( tr( "X&2:" ) );
		heightLabel->setText( tr( "&Y2:" ) );
		heightSpin->setEnabled(true);
		m_lineMode = true;
	}
}

void PropertyWidgetFrame_XYZ::handleAppModeChanged(int oldMode, int mode)
{
	if (!m_haveDoc || !m_haveItem || !m_ScMW || m_ScMW->scriptIsRunning())
		return;
	doLock->setEnabled(mode != modeEditClip);
}

void PropertyWidgetFrame_XYZ::handleSelectionChanged()
{
	if (!m_haveDoc || !m_ScMW || m_ScMW->scriptIsRunning())
		return;

	PageItem* currItem = currentItemFromSelection();
	if (m_doc->m_Selection->count() > 1)
	{
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

		xposSpin->setEnabled(true);
		yposSpin->setEnabled(true);
		widthSpin->setEnabled(true);
		heightSpin->setEnabled(true);

		setEnabled(true);
	}
	else
	{
		int itemType = currItem ? (int) currItem->itemType() : -1;
		
		m_haveItem = (itemType!=-1);

		basePointWidget->setEnabled(true);

		setEnabled(true);

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
			setEnabled(false);
			break;
		case PageItem::ImageFrame:
		case PageItem::LatexFrame:
		case PageItem::OSGFrame:
#ifdef HAVE_OSG
			if (currItem->asOSGFrame())
			{
				setEnabled(true);
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


/*********************************************************************
*
* Feature Position
*
**********************************************************************/

void PropertyWidgetFrame_XYZ::setRotation(double rotation){
	m_rotation = rotation;
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
		ma.rotate(m_rotation); //static_cast<double>(rotationSpin->value())*(-1));
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

/*********************************************************************
*
* Feature Lock
*
**********************************************************************/

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

void PropertyWidgetFrame_XYZ::showLocked(bool isLocked)
{
	xposSpin->setEnabled(!isLocked);
	yposSpin->setEnabled(!isLocked);
	widthSpin->setEnabled(!isLocked);
	heightSpin->setEnabled(!isLocked);

	doLock->setChecked(isLocked);
}

void PropertyWidgetFrame_XYZ::showSizeLocked(bool isSizeLocked)
{
	bool b=isSizeLocked;
	if (m_haveItem && m_item->locked())
		b=true;

	widthSpin->setEnabled(!b);
	heightSpin->setEnabled(!b);

	noResize->setChecked(isSizeLocked);
}

/*********************************************************************
*
* Undo
*
**********************************************************************/

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

/*********************************************************************
*
* Events
*
**********************************************************************/

void PropertyWidgetFrame_XYZ::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::LanguageChange)
	{
		languageChange();
	}
	else
		QWidget::changeEvent(e);
}
