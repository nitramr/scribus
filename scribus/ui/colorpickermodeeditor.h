#ifndef COLORPICKERMODEEDITOR_H
#define COLORPICKERMODEEDITOR_H

#include <QWidget>
#include "ui_colorpickermodeeditor.h"
#include "propertywidgetbase.h"
#include "colorpickerstruct.h"
#include "ui/gradientvectordialog.h"
#include "util.h"
#include "units.h"

class PageItem;
class ScribusMainWindow;
class UndoManager;

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
	UndoManager * undoManager;

	ObjectPaintMode objectPaintMode;
	ColorPaintMode colorPaintMode;

	bool eventFilter(QObject *object, QEvent *event);

protected:
	GradientVectorDialog* CGradDia;
	ScribusMainWindow *m_ScMW;
	double    m_unitRatio;
	int		  m_unitIndex;
	bool      m_haveDoc;
	bool      m_haveItem;
	PageItem* m_item;
	int       editStrokeGradient;

	void   connectSignals();
	void   disconnectSignals();

	void   setCurrentItem(PageItem* i);
	void   updateFromItem();

	int    m_blockUpdates;
	void   blockUpdates(bool block) { if (block) ++m_blockUpdates; else --m_blockUpdates; }
	bool   updatesBlocked() { return (m_blockUpdates > 0); }

	void   setGradientVectorValues();

signals:
	void NewGradient(int);
	void NewSpecial(double, double, double, double, double, double, double, double, double, double);

	void emitGradientType(GradientTypes);
	void emitGradientUpdate( QString, VGradient);
	void editGradient(int);

private slots:

	void updateSizes(int index);
	void updateSizesGradient(int index);

	void NewSpGradient(double x1, double y1, double x2, double y2, double fx, double fy, double sg, double sk, double cx, double cy);
	void toggleGradientEdit(int);


public slots:
	void handleSelectionChanged();
	void handleUpdateRequest(int);
	void unitChange();
	void languageChange();

	void setColorPaintMode(ColorPaintMode number);
	void setObjectPaintMode(ObjectPaintMode mode);

	void slotGradType(int type);
	void setGradientColors();
	void handleFillGradient();
	void setFillGradient(const QString name, VGradient gradient);

	// Mesh Gradient
	void editMeshPointColor();
	void editGradientVector();

	// Gradient Vector Dialog
	void setActiveGradDia(bool active);
	void createNewMeshGradient();
	void resetMeshGradient();
	void meshGradientToShape();
//	void resetOneControlPoint();
//	void resetAllControlPoints();
	void handleRemovePatch();
	void snapToPatchGrid(bool val);

};

#endif // COLORPICKERMODEEDITOR_H
