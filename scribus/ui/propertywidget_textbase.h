/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_TEXTBASE_H
#define PROPERTYWIDGET_TEXTBASE_H

#include "ui_propertywidget_textbase.h"

//#include "scribusapi.h"
//#include "scrpalettebase.h"
#include "scrspinbox.h"

#include "scguardedptr.h"
//#include "sctextstruct.h"

//#include "propertywidgetbase.h"


class PageItem;
class CharStyle;
class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;
class ScComboBox;
//class ScribusDoc;
//class Selection;

class PropertyWidget_TextBase : public QWidget, public Ui::PropertyWidget_TextBase
{
	Q_OBJECT

public:
	PropertyWidget_TextBase(QWidget* parent = 0);
	~PropertyWidget_TextBase() {}

	virtual void changeEvent(QEvent *e);


	/** @brief Returns true if there is a user action going on at the moment of call. */
	bool userActionOn(); // not yet implemented!!! This is needed badly.
						 // When user releases the mouse button or arrow key, changes must be checked
						 // and if in ScribusView a groupTransaction has been started it must be also
						 // commmited
protected:

	bool   m_haveDoc;
	bool   m_haveItem;
	double m_unitRatio;
	int    m_unitIndex;

	PageItem *m_item;
	ScribusMainWindow*       m_ScMW;
	ScGuardedPtr<ScribusDoc> m_doc;

private:
	PageItem* currentItemFromSelection();


public slots:
	void setMainWindow(ScribusMainWindow *mw);

	void setDoc(ScribusDoc *d);
	void unsetDoc();

	void languageChange();
	void changeLang(int id);

	void showAlignment(int e);
	void showDirection(int e);
	void showCharStyle(const QString& name);
	void showFontFace(const QString&);
	void showFontSize(double s);
	void showLanguage(QString w);
	void showLineSpacing(double r);
	void showParStyle(const QString& name);
	void setCurrentItem(PageItem *i);

	void setupLineSpacingSpinbox(int mode, double value);

	/// update TB values:
	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& newCurrent);

	void updateCharStyles();
	void updateParagraphStyles();
	void updateTextStyles();

	void handleLineSpacing();
	void handleLineSpacingMode(int id);

	void handleFontSize();
	void handleTextFont(QString);

	void handleAlignment(int a);
	void handleDirection(int d);

	void handleUpdateRequest(int updateFlags);

private slots:

	void handleVAlign();
	void doClearCStyle();
	void doClearPStyle();

};

#endif
