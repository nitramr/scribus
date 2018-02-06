#ifndef COLORPICKERCOLORMIXER_H
#define COLORPICKERCOLORMIXER_H

#include <QWidget>
#include <QPixmap>

#include "ui_colorpickercolormixer.h"
#include "sccolor.h"
#include "scribusapi.h"
#include "propertywidgetbase.h"
#include "colorpickerstruct.h"

class ColorPickerColorMixer : public QWidget, public Ui::ColorPickerColorMixer,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	explicit ColorPickerColorMixer(QWidget *parent = 0);
	~ColorPickerColorMixer(){};

	enum MixerMode { Map, Slider };

	void setDoc(ScribusDoc *d);	
	void setColorName(QString name);


	QString colorName();
	bool isSpotColor();

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

	bool eventFilter(QObject *object, QEvent *event);

	void updatePickerSettings();
	void prepColor();

protected:

	bool isHLC;
	QPalette sliderPix(int color);
	QPalette sliderBlack();

private slots:

	void handleToggleColorMap();
	void handleToggleColorList();
	void handleColorName();

	void setValSLiders(double value);
	void setValueS(int val);
	void setColor();
	void setColor2(int h, int s, bool ende);
	void setValues();
	void setSpot();
	void selModel(const QString& mod);

public slots:
	void setColorPaintMode(ColorPaintMode mode, GradientTypes gradient);
	void setObjectColor(ScColor orig);


signals:
	void sendToggleColorList();
	void emitColor(ScColor);

};

#endif // COLORPICKERCOLORMIXER_H
