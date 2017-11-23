#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>

#include "ui_colorpicker.h"
#include "scribusapi.h"
#include "propertywidgetbase.h"

namespace Ui {
class ColorPicker;
}

class ColorPicker : public QWidget, Ui::ColorPicker,
		public PropertyWidgetBase
{
	Q_OBJECT

public:
	explicit ColorPicker(QWidget *parent = 0);
	~ColorPicker(){};

	void setDoc(ScribusDoc *d);

private:


private slots:
	void toggleColorList();

};

#endif // COLORPICKER_H
