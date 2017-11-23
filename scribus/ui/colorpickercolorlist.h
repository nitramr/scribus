#ifndef COLORPICKERCOLORLIST_H
#define COLORPICKERCOLORLIST_H

#include <QWidget>

#include "ui_colorpickercolorlist.h"


class ColorPickerColorList : public QWidget, Ui::ColorPickerColorList
{
	Q_OBJECT

public:
	explicit ColorPickerColorList(QWidget *parent = 0);
	~ColorPickerColorList(){};

private:

};

#endif // COLORPICKERCOLORLIST_H
