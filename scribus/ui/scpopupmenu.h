#ifndef SCPOPUPMENU_H
#define SCPOPUPMENU_H

#include "scribusapi.h"
#include "scribusstructs.h"

#include <QMenu>
#include <QWidgetAction>
#include <QBoxLayout>
#include <QLabel>

class SCRIBUS_API ScPopupMenu : public QMenu
{
	Q_OBJECT

public:
	ScPopupMenu(QWidget*widget);
	~ScPopupMenu();

	void addWidget(QWidget *widget);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
private:
	QVBoxLayout *m_layout;
	QWidget *panel;
	QWidgetAction * action;


public slots:
	void updateSize();
};

#endif // SCPOPUPMENU_H
