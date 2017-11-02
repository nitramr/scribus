#ifndef SCPOPUPMENU_H
#define SCPOPUPMENU_H

#include <QMenu>
#include <QWidgetAction>
#include <QBoxLayout>

class ScPopupMenu : public QMenu
{
public:
	ScPopupMenu(QWidget*widget);
	~ScPopupMenu() {}

	void addWidget(QWidget *widget);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
private:
	QVBoxLayout *m_layout;
};

#endif // SCPOPUPMENU_H
