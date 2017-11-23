#ifndef COLORPICKERCOLORMIXER_H
#define COLORPICKERCOLORMIXER_H

#include <QWidget>
#include <QPixmap>

#include "ui_colorpickercolormixer.h"
#include "sccolor.h"
#include "scribusapi.h"
#include "propertywidgetbase.h"

class ColorPickerColorMixer : public QWidget, public Ui::ColorPickerColorMixer,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	explicit ColorPickerColorMixer(QWidget *parent = 0);
	~ColorPickerColorMixer(){};

	enum MixerMode { Map, Slider };

	void setDoc(ScribusDoc *d);

private:
	MixerMode m_mixermode;

	ScColor Color;
	bool Wsave;
	bool dynamic;
	bool isRegistration;
	int BlackComp;
	QPixmap imageA;
	QPixmap imageN;
	QPixmap alertIcon;

protected:

	bool isHLC;
	QPalette sliderPix(int color);
	QPalette sliderBlack();

private slots:

	void handleToggleColorMap();
	void handleToggleColorList();

	void setValSLiders(double value);
	void setValueS(int val);
	void setColor();
	void setColor2(int h, int s, bool ende);
	void setValues();
	void setSpot();
	void selModel(const QString& mod);


signals:
	void sendToggleColorList();

};

#endif // COLORPICKERCOLORMIXER_H
