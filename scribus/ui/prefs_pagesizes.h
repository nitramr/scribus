/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/

#ifndef PREFS_PAGESIZES_H
#define PREFS_PAGESIZES_H

#include "ui_prefs_pagesizesbase.h"
#include "prefs_pane.h"
#include "scribusapi.h"

class ScribusDoc;

class SCRIBUS_API Prefs_PageSizes : public Prefs_Pane, Ui::Prefs_PageSizes
{
	Q_OBJECT

	public:
		Prefs_PageSizes(QWidget* parent, ScribusDoc* doc = nullptr);
		~Prefs_PageSizes();

		void restoreDefaults(struct ApplicationPrefs *prefsData) override;
		void saveGuiToPrefs(struct ApplicationPrefs *prefsData) const override;

	public slots:
		void languageChange();

	protected slots:
		void moveToActive();
		void moveFromActive();

private:
		void moveItems(QTreeWidget* fromList, QTreeWidget* toList);
};

#endif // PREFS_PAGESIZES_H
