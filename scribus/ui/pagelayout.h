/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PAGELAYOUT_H
#define PAGELAYOUT_H

#include <QGroupBox>
#include <QListWidget>
#include <QList>

class QToolButton;
class QLabel;
class QHBoxLayout;
class FormWidget;
class QMenu;

#include "scribusapi.h"
#include "scribusstructs.h"
#include "scribusdoc.h"

/*! \brief A widget for changing pages layout.
User can change layouts on-the-fly changing widget's
comboboxes. */
class SCRIBUS_API PageLayouts : public QWidget
{
	Q_OBJECT

public:
	PageLayouts(QWidget* parent);
	~PageLayouts() {};

	void updateSchemeSelector(const ScribusDoc* doc);
	void selectFirstPage(int nr);
	void selectScheme(int nr);

	void setHideLabelsPermanently(bool hide);
	bool hideLabelsPermanently() { return m_hideLabels; };

public slots:
	void schemeSelected(int ic);
	void toggleLabelVisibility(bool visibility);

signals:
	void selectedScheme(int);
	void selectedFirstPage(int);

private:

	int m_scheme {0};
	int m_firstPage {0};
	bool m_hideLabels {false};
	int docPagePositioning {0};

	FormWidget* labelScheme;
	FormWidget* labelPages;
	QList<PageSet> pageSets;
	QToolButton* buttonScheme;
	QToolButton* buttonFirstPage;
	QMenu* menuScheme;
	QMenu* menuFirstPage;
	QHBoxLayout* layoutGroupLayout;

	void reloadScheme();
	void reloadFirstPage(int scheme);

protected slots:
	void languageChange();
	void schemeChanged(QAction* action);
	void firstPageChanged(QAction* action);

};

#endif // PAGELAYOUT_H
