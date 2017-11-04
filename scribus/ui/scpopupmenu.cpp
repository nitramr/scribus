#include "scpopupmenu.h"
#include <QEvent>
#include <QLayout>
#include <QActionEvent>
#include <QApplication>

ScPopupMenu::ScPopupMenu(QWidget *widget)
{


	m_layout = new QVBoxLayout();
	m_layout->setSizeConstraint(QLayout::SetMinimumSize);
	m_layout->setMargin(1);

	if(widget){
		addWidget(widget);
	}

	panel = new QWidget();
	panel->installEventFilter(this);
	panel->setLayout(m_layout);
	panel->adjustSize();



	action = new QWidgetAction(this);
	action->setDefaultWidget(panel);

	this->addAction(action);

	//updateSize();

}

ScPopupMenu::~ScPopupMenu() {

	this->clear();
}

void ScPopupMenu::addWidget(QWidget *widget)
{
	widget->updateGeometry();
	widget->adjustSize();

	m_layout->addWidget(widget);


}


void ScPopupMenu::updateSize(){

//	panel->adjustSize();
//	this->adjustSize();

	panel->adjustSize();
	panel->resize(panel->sizeHint());
//	this->resize(panel->sizeHint());

//	QWidget * pan = action->defaultWidget();

//	this->clear();

	//QWidgetAction * action = new QWidgetAction(this);
	//action->setDefaultWidget(panel);
//	action->setDefaultWidget(pan);
//	this->addAction(action);

	//action->setDefaultWidget(NULL);
//	action->setDefaultWidget(panel);

	//this->resize(panel->size());
}


bool ScPopupMenu::eventFilter(QObject *obj, QEvent *event)
{

	// prevent that menu is close by interact with the background
	if (event->type() == QEvent::MouseButtonPress ||
			event->type() == QEvent::MouseButtonDblClick ||
			event->type() == QEvent::MouseMove) {

		return true;
	}
	if(event->type() == QEvent::Resize){

//		QWidget *widget = qobject_cast<QWidget *>(obj);
//		this->resize(widget->size());
//		return true;
	}

		return QObject::eventFilter(obj, event);

}
