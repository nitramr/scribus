#ifndef PROPERTYWIDGETTEXT_PAREFFECT_H
#define PROPERTYWIDGETTEXT_PAREFFECT_H

#include "ui_propertywidgettext_pareffectbase.h"
#include "numeration.h"
#include "propertywidgetbase.h"
#include "ui/charselectenhanced.h"



class PageItem;
class ParagraphStyle;
class ScribusMainWindow;

class PropertyWidgetText_ParEffect : public QWidget, private Ui::PropertyWidget_ParEffectBase, public PropertyWidgetBase
{
	Q_OBJECT

public:
	PropertyWidgetText_ParEffect(QWidget *parent = 0);
	~PropertyWidgetText_ParEffect() {}

	void updateStyle(const ParagraphStyle& newPStyle);
	void updateCharStyles();

	void showCharStyle(const QString& name);
	void connectSignals();
	void disconnectSignals();
	CharSelectEnhanced * m_enhanced;
	
	void fillNumerationsCombo();

protected:
	double m_unitRatio;
	int    m_unitIndex;

	PageItem *         m_item;
	ScribusMainWindow* m_ScMW;

	void configureWidgets();
	void setCurrentItem(PageItem *item);

	virtual void changeEvent(QEvent *e);
	void handleChanges(PageItem* item, ParagraphStyle& newStyle);

public slots:
	void setMainWindow(ScribusMainWindow *mw);
	void setDoc(ScribusDoc *doc);

	void handleAppModeChanged(int oldMode, int mode);
	void handleSelectionChanged();
	void handleUpdateRequest(int);

	void languageChange();
	void unitChange();

	void handleParEffectUse();
	void handleBulletStr(QString);
	void handleDropCapLines(int);
	void handleNumName(QString);
	void handleNumFormat(int);
	void handleNumLevel(int);
	void handleNumPrefix(QString);
	void handleNumSuffix(QString);
	void handleNumStart(int);
	void handlePEOffset(double);
	void handlePEIndent(bool);
	void handlePECharStyle(QString);

private slots:
	void on_bulletCharTableButton_toggled(bool checked);
	void insertSpecialChars(const QString &chars);

private:
	void openEnhanced();
	void closeEnhanced(bool show = false);
	void enableDropCap(bool);
	void enableBullet(bool);
	void enableNum(bool);
	void enableParEffect(bool);
	void fillBulletStrEditCombo();

	void fillNumFormatCombo();

	void fillPECombo();

signals:
	void needsRelayout();
};

#endif // PROPERTYWIDGET_PAREFFECT_H
