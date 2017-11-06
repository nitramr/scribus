/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
#ifndef PROPERTIESCONTENTPALETTE_TEXT_H
#define PROPERTIESCONTENTPALETTE_TEXT_H



#include "scribusapi.h"
#include "scrpalettebase.h"
#include "scrspinbox.h"

#include "scguardedptr.h"
#include "sctextstruct.h"

#include "sclayoutsection.h" // Indigo Section
#include "flowlayout.h"

class PageItem;
class PropertyWidgetText_Distance;
class PropertyWidgetText_ParEffect;
class PropertyWidgetText_FontFeatures;
class PropertyWidgetText_GlyphSpace;
class PropertyWidgetText_GlyphStretch;
class PropertyWidgetText_Hyphenate;
class PropertyWidgetText_Hyphenation;
class PropertyWidgetText_OptMargins;
class PropertyWidgetText_Orphans;
class PropertyWidgetText_PathText;
class PropertyWidgetText_TextAdvanced;
class PropertyWidgetText_TextAlignment;
class PropertyWidgetText_TextFont;
class PropertyWidgetText_TextColor;
class PropertyWidgetText_TextStyles;
class ScribusDoc;
class ScribusMainWindow;
class Selection;

class SCRIBUS_API PropertiesContentPalette_Text : public QWidget
{
	Q_OBJECT

public:
	PropertiesContentPalette_Text(QWidget* parent = 0);
	~PropertiesContentPalette_Text() {}

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

	PageItem *m_item;
	ScribusMainWindow*       m_ScMW;
	ScGuardedPtr<ScribusDoc> m_doc;

	ScLayoutSection *layoutSectionText;
	ScLayoutSection *layoutSectionParagraph;
	ScLayoutSection *layoutSectionCharacter;
	ScLayoutSection *layoutSectionLists;
	ScLayoutSection *layoutSectionTextPath;

	PropertyWidgetText_GlyphSpace* charSpaceWidgets;
	PropertyWidgetText_GlyphStretch* charStretchWidgets;
	PropertyWidgetText_Distance* distanceWidgets;
	PropertyWidgetText_FontFeatures* fontfeaturesWidget;
	PropertyWidgetText_Hyphenate* hyphenateWidget;
	PropertyWidgetText_Hyphenation* hyphenationWidget;
	PropertyWidgetText_OptMargins* optMargins;
	PropertyWidgetText_Orphans* orphanBox;
	PropertyWidgetText_ParEffect* parEffectWidgets;
	PropertyWidgetText_PathText* pathTextWidgets;
	PropertyWidgetText_TextAdvanced* textAdvancedWidgets;
	PropertyWidgetText_TextAlignment* textAlignmentWidgets;
	PropertyWidgetText_TextFont* textWidgets;
	PropertyWidgetText_TextColor* colorWidgets;
	PropertyWidgetText_TextStyles* textStylesWidgets;

private:
	PageItem* currentItemFromSelection();

	
public slots:
	void setMainWindow(ScribusMainWindow *mw);
	
	void setDoc(ScribusDoc *d);
	void unsetDoc();
	void unsetItem();

	void handleSelectionChanged();

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

};

#endif
