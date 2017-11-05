/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTIESPALETTE_TEXT_H
#define PROPERTIESPALETTE_TEXT_H



#include "scribusapi.h"
#include "scrpalettebase.h"
#include "scrspinbox.h"

#include "scguardedptr.h"
#include "sctextstruct.h"

#include "sclayoutsection.h" // Indigo Section
#include "flowlayout.h"

class PageItem;
class PropertyWidget_Advanced;
class PropertyWidget_Distance;
class PropertyWidget_ParEffect;
class PropertyWidget_Flop;
class PropertyWidget_FontFeatures;
class PropertyWidget_Hyphenation;
class PropertyWidget_OptMargins;
class PropertyWidget_Orphans;
class PropertyWidget_PathText;
class PropertyWidget_TextAdvanced;
class PropertyWidget_TextAlignment;
class PropertyWidget_TextFont;
class PropertyWidget_TextColor;
class PropertyWidget_TextStyles;
//class ScComboBox;
class ScribusDoc;
class ScribusMainWindow;
class Selection;

class SCRIBUS_API PropertiesPalette_Text : public QWidget
{
	Q_OBJECT

public:
	PropertiesPalette_Text(QWidget* parent = 0);
	~PropertiesPalette_Text() {}

	virtual void changeEvent(QEvent *e);
	
	void updateColorList();

	/** @brief Returns true if there is a user action going on at the moment of call. */
	bool userActionOn(); // not yet implemented!!! This is needed badly.
                         // When user releases the mouse button or arrow key, changes must be checked
                         // and if in ScribusView a groupTransaction has been started it must be also
                         // commmited
protected:

	bool   m_haveDoc;
	bool   m_haveItem;
//	double m_unitRatio;
//	int    m_unitIndex;

	PageItem *m_item;
	ScribusMainWindow*       m_ScMW;
	ScGuardedPtr<ScribusDoc> m_doc;

	ScLayoutSection *layoutSectionText;
	ScLayoutSection *layoutSectionParagraph;
	ScLayoutSection *layoutSectionCharacter;
	ScLayoutSection *layoutSectionLists;
	ScLayoutSection *layoutSectionTextPath;

	PropertyWidget_Advanced* advancedWidgets;
	PropertyWidget_Distance* distanceWidgets;
//	PropertyWidget_Flop* flopBox;
	PropertyWidget_FontFeatures* fontfeaturesWidget;
	PropertyWidget_Hyphenation* hyphenationWidget;
	PropertyWidget_OptMargins* optMargins;
	PropertyWidget_Orphans* orphanBox;
	PropertyWidget_ParEffect* parEffectWidgets;
	PropertyWidget_PathText* pathTextWidgets;
	PropertyWidget_TextAdvanced* textAdvancedWidgets;
	PropertyWidget_TextAlignment* textAlignmentWidgets;
	PropertyWidget_TextFont* textWidgets;
	PropertyWidget_TextColor* colorWidgets;
	PropertyWidget_TextStyles* textStylesWidgets;

private:
	PageItem* currentItemFromSelection();

	
public slots:
	void setMainWindow(ScribusMainWindow *mw);
	
	void setDoc(ScribusDoc *d);
	void setCurrentItem(PageItem *i);
	void unsetDoc();
	void unsetItem();

	void handleSelectionChanged();
	void handleUpdateRequest(int);

	void languageChange();
	void unitChange();

	void showAlignment(int e);
	void showDirection(int e);
	void showCharStyle(const QString& name);
	void showFontSize(double s);
	void showLanguage(QString w);
	void showParStyle(const QString& name);
	
	/// update TB values:
	void updateCharStyle(const CharStyle& charStyle);
	void updateStyle(const ParagraphStyle& newCurrent);	
	void updateParagraphStyles();

	
private slots:
	void handleAlignment();
//	void handleFirstLinePolicy(int);

};

#endif
