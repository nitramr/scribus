#include "scpopupmenu.h"
#include <QEvent>
#include <QLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QScrollArea>

ScPopupMenu::ScPopupMenu(QWidget *widget)
{

	m_buddy = NULL;

	//setWindowFlags(Qt::Tool| Qt::CustomizeWindowHint);


	m_layout = new QVBoxLayout();
	m_layout->setSizeConstraint(QLayout::SetMinimumSize);
	m_layout->setMargin(1);
	m_layout->setAlignment( Qt::AlignTop );

	label = new QLabel();

	addWidget(label);

	if(widget){
		addWidget(widget);
	}


	panel = new QWidget();
	panel->setLayout(m_layout);
//	panel->installEventFilter(this);

//	QScrollArea * scrollArea = new QScrollArea();
////	scrollArea->setWidget(panel);
//	scrollArea->setLayout(m_layout);

	action = new QWidgetAction(this);
	action->setDefaultWidget(panel);
	action->setCheckable(true);

	this->addAction(action);

	//connect(this,SIGNAL(aboutToShow()),this,SLOT(updateContent()));
}


void ScPopupMenu::addWidget(QWidget *widget)
{
	m_layout->addWidget(widget);


}

void ScPopupMenu::updateContent()
{
	QMenu * menu = dynamic_cast<QMenu *>(sender());
	if(menu != 0)
	{
//		menu->blockSignals(true);
//		// Add/update widgets

//		menu->show();
//		qApp->processEvents();
//		menu->hide();
//		qApp->processEvents();
//		menu->show();
//		menu->blockSignals(false);

//		QAction *action = menu->;

//		menu->setDefaultAction(action);

		//panel->adjustSize();


		this->removeAction(action);

//		action = new QWidgetAction(this);
//		action->setDefaultWidget(panel);
//		action->setCheckable(true);

		this->addAction(action);

		//QPixmap::grabWidget(this);
	}
}

void ScPopupMenu::setBuddy(QPushButton *button){
//	m_buddy = button;
//	m_buddy->setCheckable(true);


	button->setMenu(this);
	button->setContextMenuPolicy(Qt::ActionsContextMenu);
	//button->setStyleSheet("QPushButton::menu-indicator { image: none; }");


//	m_buddy->installEventFilter(this);

//	connect(m_buddy, SIGNAL(toggled(bool)), this, SLOT(showPopup(bool)));
//	connect(m_buddy, SIGNAL(hide()), this, SLOT(hidePopup()));
}

void ScPopupMenu::showPopup(bool state){


	QPoint localPoint(m_buddy->pos());
	QPoint windowPoint = m_buddy->mapTo(m_buddy->window(), localPoint);

	QPoint ptnGlobalBuddy(m_buddy->mapToGlobal(m_buddy->pos()));
	QRect screen = QApplication::desktop()->screenGeometry();

	label->setText("Buddy local: " + QString::number(m_buddy->x()) + "+" + QString::number(m_buddy->y()) + '\n' +
					"WindowPoint: " + QString::number(windowPoint.x()) + "+" + QString::number(windowPoint.y()) + '\n' +
					"Buddy Global: " + QString::number(ptnGlobalBuddy.x()) + "+" + QString::number(ptnGlobalBuddy.y())
																								 );
	movePopup(ptnGlobalBuddy);

	this->setVisible(state);
}

void ScPopupMenu::hidePopup(){

	m_buddy->setChecked(false);
	this->setVisible(false);

}

void ScPopupMenu::movePopup(QPoint pos){

	QPoint p = pos;
	p += QPoint(2, m_buddy->height()
//#ifdef Q_WS_WIN
//				21
//#else
//				16
//#endif
		);

//	// if out of screen (right)
//	if (p.x() + this->width() > screen.x() + screen.width())
//		p.rx() -= 4 + this->width();

//	// if out of screen (bottom)
//	if (p.y() + this->height() > screen.y() + screen.height())
//		p.ry() -= 24 + this->height();

//	// if out of screen (top)
//	if (p.y() < screen.y())
//		p.setY(screen.y());

//	// if out of screen (right)
//	if (p.x() + this->width() > screen.x() + screen.width())
//		p.setX(screen.x() + screen.width() - this->width());

//	// if out of screen (left)
//	if (p.x() < screen.x())
//		p.setX(screen.x());

//	// if out of screen (bottom)
//	if (p.y() + this->height() > screen.y() + screen.height())
//		p.setY(screen.y() + screen.height() - this->height());

	this->move(p);

}


////Windows and KDE allows menus to cover the taskbar, while GNOME and Mac don't
//QRect ScPopupMenu::popupGeometry(int screen) const
//{
//#ifdef Q_WS_WIN
//	return QApplication::desktop()->screenGeometry(screen);
//#elif defined Q_WS_X11
//	if (X11->desktopEnvironment == DE_KDE)
//		return QApplication::desktop()->screenGeometry(screen);
//	else
//		return QApplication::desktop()->availableGeometry(screen);
//#else
//		return QApplication::desktop()->availableGeometry(screen);
//#endif
//}

bool ScPopupMenu::eventFilter(QObject *obj, QEvent *event)
{

	// prevent that menu is close by interact with the background
	if (event->type() == QEvent::MouseButtonPress ||
			event->type() == QEvent::MouseButtonDblClick ||
			event->type() == QEvent::MouseMove) {
	//	return true;
	}
	if(event->type() == QEvent::Resize){

//		this->adjustSize();

//		return true;
	}

	if(event->type() == QEvent::Move){


//		QPoint ptnGlobalBuddy(m_buddy->mapToGlobal(m_buddy->pos()));

//		movePopup(ptnGlobalBuddy);
//		return true;

	}

	return QObject::eventFilter(obj, event);

}
