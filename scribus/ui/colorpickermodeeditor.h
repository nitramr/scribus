#ifndef COLORPICKERMODEEDITOR_H
#define COLORPICKERMODEEDITOR_H

#include <QWidget>
#include "ui_colorpickermodeeditor.h"
#include "propertywidgetbase.h"
#include "colorpickerstruct.h"
#include "util.h"
#include "units.h"

class PageItem;
class ScribusMainWindow;

class ColorPickerModeEditor : public QWidget, Ui::ColorPickerModeEditor,
		public PropertyWidgetBase
{


	Q_OBJECT

public:
	explicit ColorPickerModeEditor(QWidget *parent = 0);
	~ColorPickerModeEditor(){};

	void setMainWindow(ScribusMainWindow *mw);
	void setDoc(ScribusDoc* d);
	void unsetDoc();
	void unsetItem();

private:
	PageItem* currentItemFromSelection();

protected:
	ScribusMainWindow *m_ScMW;
	double    m_unitRatio;
	int		  m_unitIndex;
	bool      m_haveDoc;
	bool      m_haveItem;
	PageItem* m_item;

	void   connectSignals();
	void   disconnectSignals();

	void   setCurrentItem(PageItem* i);
	void   updateFromItem();

	int    m_blockUpdates;
	void   blockUpdates(bool block) { if (block) ++m_blockUpdates; else --m_blockUpdates; }
	bool   updatesBlocked() { return (m_blockUpdates > 0); }

signals:
	void NewGradient(int);
	void emitGradientType(GradientTypes);

private slots:
	void updateSizes(int index);
	void updateSizesGradient(int index);


public slots:
	void handleSelectionChanged();
	void handleUpdateRequest(int);
	void unitChange();
	void languageChange();

	void setColorPaintMode(ColorPaintMode number);
	void slotGradType(int type);

};

#endif // COLORPICKERMODEEDITOR_H
