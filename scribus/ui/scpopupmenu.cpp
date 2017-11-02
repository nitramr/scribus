#include "scpopupmenu.h"
#include <QEvent>
#include <QLayout>


ScPopupMenu::ScPopupMenu(QWidget *widget)
{


	m_layout = new QVBoxLayout();
	m_layout->setSizeConstraint(QLayout::SetMinimumSize);
	m_layout->setMargin(1);

	if(widget){
		addWidget(widget);
	}

	QWidget *panel = new QWidget();
	//panel->setFixedSize(widget->size());
	panel->setLayout(m_layout);
	panel->adjustSize();
	panel->installEventFilter(this);

	QWidgetAction * action = new QWidgetAction(this);
	action->setDefaultWidget(panel);

	this->addAction(action);

}

void ScPopupMenu::addWidget(QWidget *widget)
{
	widget->adjustSize();

	m_layout->addWidget(widget);

}



bool ScPopupMenu::eventFilter(QObject *obj, QEvent *event)
{

	// prevent that menu is close by interact with the background
	if (event->type() == QEvent::MouseButtonPress ||
			event->type() == QEvent::MouseButtonDblClick ||
			event->type() == QEvent::MouseMove) {

		return true;
	}

		return QObject::eventFilter(obj, event);

}
