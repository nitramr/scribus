#ifndef SCPOPUPMENU_H
#define SCPOPUPMENU_H

#include "scribusapi.h"
#include "scribusstructs.h"

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QWidgetAction>

class SCRIBUS_API ScPopupMenu : public QMenu
{
	Q_OBJECT

public:
	ScPopupMenu(QWidget*widget);

	~ScPopupMenu(){};

	void addWidget(QWidget *widget);
	void setBuddy(QPushButton *button);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
private:
	QVBoxLayout *m_layout;
	QWidget *panel;
	QPushButton *m_buddy;
	QLabel * label;
	QWidgetAction * action;

private slots:
	void updateContent();

	void showPopup(bool state);
	void movePopup(QPoint pos);
	void hidePopup();

};

#endif // SCPOPUPMENU_H
