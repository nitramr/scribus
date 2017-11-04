/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTYWIDGET_TEXTBASE_H
#define PROPERTYWIDGET_TEXTBASE_H

#include "ui_propertywidget_textbase.h"
#include "propertywidgetbase.h"
//#include "scribus.h"
#include "shadebutton.h"
#include "colorcombo.h"
#include "sccolorfillsbox.h"

class PageItem;
class CharStyle;
class ParagraphStyle;
class ScribusDoc;
class ScribusMainWindow;

class PropertyWidget_TextBase : public QWidget, Ui::PropertyWidget_TextBase,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidget_TextBase(QWidget* parent = 0);
	~PropertyWidget_TextBase() {}

	void updateColorList();
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
	void disconnectSignals();
	void connectSignals();

private:
	PageItem* currentItemFromSelection();
	ColorCombo *fillColor;
	ShadeButton *fillShade;
	ScColorFillsBox * fillsColorBox;


public slots:
	void setMainWindow(ScribusMainWindow *mw);

	void setDoc(ScribusDoc *d);
	void unsetDoc();

	void languageChange();

	void showFontFace(const QString&);
	void showFontSize(double s);
	void showLineSpacing(double r);
	void showTextColors(QString b, double shb);

	void setCurrentItem(PageItem *i);
	void setupLineSpacingSpinbox(int mode, double value);

	/// update TB values:
	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& newCurrent);

	void handleSelectionChanged();

	void handleLineSpacing();
	void handleLineSpacingMode(int id);

	void handleFontSize();
	void handleTextFont(QString);

	void handleUpdateRequest(int updateFlags);

private slots:
	void handleFillColorBox();
	void handleTextFill();
	void handleTextShade();
};

#endif
