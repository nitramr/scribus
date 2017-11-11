/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTIESPALETTE_SHADOWOPTIONS_H
#define PROPERTIESPALETTE_SHADOWOPTIONS_H
#include "ui_propertiespalette_shadowoptions.h"

#include "scribusapi.h"
#include "scguardedptr.h"

//#include "sccolorfillsbox.h"

class PageItem;
class ScribusDoc;
class Selection;
class ScribusMainWindow;

class SCRIBUS_API PropertiesPalette_ShadowOptions : public QWidget, Ui::PropertiesPalette_ShadowOptions
{
	Q_OBJECT

public:
	PropertiesPalette_ShadowOptions(QWidget* parent);
	~PropertiesPalette_ShadowOptions() {};

	virtual void changeEvent(QEvent *e);
	void updateColorList();

private:

	PageItem* currentItemFromSelection();
	void connectSignals();
	void disconnectSignals();

public slots:
	void setMainWindow(ScribusMainWindow *mw);

	void setDoc(ScribusDoc *d);
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void languageChange();
	void unitChange(){};
	void handleSelectionChanged();
	//void handleUpdateRequest(int);

//	void setShadowOn(bool isOn);
//	void setShadowOptions(QString, double, double, double, int, double);

private slots:
	void handleNewValues();
	//void handleFillColorBox();
	//void showColor(QString b, double sb);

protected slots:
	//void updateSpinBoxConstants();

signals:
	void sendShadowOptions(int, bool, bool);

protected:

	ScribusMainWindow *m_ScMW;

	bool      m_haveDoc;
	bool      m_haveItem;
	double    m_unitRatio;
	int       m_unitIndex;

	PageItem *m_item;
	ScGuardedPtr<ScribusDoc> m_doc;
//	ScColorFillsBox * shadowColor;


//	bool shadow;
//	QString shadowC;
//	double shadowX;
//	double shadowY;
//	double shadowR;
//	int shadowS;
//	double shadowO;
};

#endif
