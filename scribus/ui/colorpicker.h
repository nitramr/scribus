#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>

#include "ui_colorpicker.h"
#include "scribusapi.h"
#include "sccolor.h"
#include "colorpickerstruct.h"
#include "propertywidgetbase.h"

class ScribusMainWindow;

class SCRIBUS_API ColorPicker : public QWidget, Ui::ColorPicker,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	explicit ColorPicker(QWidget *parent = 0);
	~ColorPicker(){};

	void setMainWindow(ScribusMainWindow* mw);
	void setDoc(ScribusDoc *d);
	void unsetDoc();
	void unsetItem();

	void setColor(ScColor color, QString name = "");

	void updateColorList();

	void setColors(ColorList newColorList);
	void setGradients(QHash<QString, VGradient> *docGradients);
	void setPatterns(QHash<QString, ScPattern> *docPatterns);

private:

	ScColor Color;
	ColorPaintMode colorPaintMode;
	ObjectPaintMode objectPaintMode;
	GradientTypes gradientTypes;

	PageItem* currentItemFromSelection();

protected:
	ScribusMainWindow *m_ScMW;
	double    m_unitRatio;
	int		  m_unitIndex;
	bool      m_haveDoc;
	bool      m_haveItem;
	PageItem* m_item;

	ColorList colorList;
	QHash<QString, ScPattern> *patternList;
	QHash<QString, VGradient> *gradientList;

	void   connectSignals();
	void   disconnectSignals();

	void   setCurrentItem(PageItem* i);
	void   updateFromItem();

	void   enablePatterns(bool enable);

	int    m_blockUpdates;
	void   blockUpdates(bool block) { if (block) ++m_blockUpdates; else --m_blockUpdates; }
	bool   updatesBlocked() { return (m_blockUpdates > 0); }

private slots:
	void setColorPaintMode(int index);
	void setObjectPaintMode(int index);
	void setGradientTypes(GradientTypes type);


public slots:
	void handleSelectionChanged();
	void handleUpdateRequest(int);
	void unitChange();
	void languageChange();

};

#endif // COLORPICKER_H
