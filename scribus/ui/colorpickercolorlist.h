#ifndef COLORPICKERCOLORLIST_H
#define COLORPICKERCOLORLIST_H

#include <QWidget>

#include "ui_colorpickercolorlist.h"
#include "scribusapi.h"
#include "colorpickerstruct.h"
#include "sccolor.h"
#include "vgradient.h"
#include "scpattern.h"
#include "propertywidgetbase.h"

class ScribusMainWindow;

class SCRIBUS_API ColorPickerColorList : public QWidget, Ui::ColorPickerColorList,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	explicit ColorPickerColorList(QWidget *parent = 0);
	~ColorPickerColorList(){};

	void setMainWindow(ScribusMainWindow* mw);
	void setDoc(ScribusDoc *d);

	QHash<QString, VGradient> dialogGradients;
	QMap<QString,QString> origNames;
	QMap<QString,QString> replaceMap;
	QStringList origGradients;
	ColorList m_colorList;
	QMap<QString,QString> replaceColorMap;

	QHash<QString, ScPattern> dialogPatterns;
	QMap<QString,QString> replaceMapPatterns;
	QMap<QString,QString> origNamesPatterns;
	QStringList origPatterns;

	bool hasImportedColors;

private:

	ColorPaintMode colorPaintMode;
	ObjectPaintMode objectPaintMode;
	GradientTypes gradientTypes;

	int sortRule;
	bool modified;
	bool paletteLocked;

	ColorList inDocUsedColors;
	ColorList getGradientColors();
	QString getColorTooltip(const ScColor& color);
	bool isMandatoryColor(QString colorName);
	QStringList getUsedPatternsHelper(QString pattern, QStringList &results);

	void init();
	void updateGradientColors(QString newName, QString oldName);
	void updateDoc();

protected:
	ScribusMainWindow *m_ScMW;
	double    m_unitRatio;
	int		  m_unitIndex;
	bool      m_haveDoc;
	bool      m_haveItem;
	PageItem* m_item;

	ScColor Color;

	VGradient m_gradient;
	QString m_gradientName;


//	QTreeWidgetItem *colorItems;
//	QTreeWidgetItem *gradientItems;
//	QTreeWidgetItem *patternItems;

	QTreeWidgetItem* updatePatternList(QString addedName = "");
	QTreeWidgetItem* updateGradientList(QString addedName = "");
	QTreeWidgetItem* updateColorList(QString addedName = "");

	void loadScribusFormat(QString fileName);
	void loadGimpFormat(QString fileName);
	void addGimpColor(QString &colorName, double r, double g, double b);
	void loadVectors(QString data);

signals:
	void emitColor(ScColor);
	void emitGradient(QString, VGradient);

	void emitMainWindowUpdateColorList();
	void emitMainWindowSlotDocCh(bool);

private slots:
	void slotRightClick(QPoint p);
	void selEditColor(QTreeWidgetItem *it);
	void itemSelectionChanged();
	void itemSelected(QTreeWidgetItem* it);

	void setGradientData(const QString name, VGradient gradient);

	void createNew();
	void removeColorItem();
	void editColorItem();
	void importColorItems();
	void removeUnusedColorItem();

public slots:
	void setColorPaintMode(ColorPaintMode mode, GradientTypes gradient);
	void setObjectPaintMode(ObjectPaintMode mode);

	void setColor(ScColor color);
	void toggleColorList();
	void handleUpdateRequest(int);

};

#endif // COLORPICKERCOLORLIST_H
