/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGETIMAGE_IMAGESETTINGS_H
#define PROPERTYWIDGETIMAGE_IMAGESETTINGS_H

#include "ui_propertywidgetimage_imagesettings.h"

#include "scribusapi.h"
#include "scrpalettebase.h"
#include "scrspinbox.h"
#include "sclistboxpixmap.h"

#include "propertywidgetbase.h"

class PageItem;
class ScComboBox;
class ScribusDoc;
class ScribusMainWindow;
class Selection;

class SCRIBUS_API PropertyWidgetImage_ImageSettings : public QWidget, Ui::PropertyWidgetImage_ImageSettings,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetImage_ImageSettings(QWidget* parent = 0);
	~PropertyWidgetImage_ImageSettings() {};

	virtual void changeEvent(QEvent *e);

	void updateProfileList();
	void showCMSOptions();

protected:

	bool      m_haveDoc;
	bool      m_haveItem;

	PageItem *m_item;
	ScribusMainWindow*       m_ScMW;

private:

	PageItem* currentItemFromSelection();

public slots:
	void setMainWindow(ScribusMainWindow *mw);

	void setDoc(ScribusDoc *d);
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void handleSelectionChanged();
	void handleUpdateRequest(int updateFlags);

	void languageChange();


private slots:


	void handleProfile(const QString& prn);
	void handleIntent();
	void handleCompressionMethod();
	void handleCompressionQuality();

};

#endif
