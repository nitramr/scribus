#ifndef COLORPICKERMODEEDITOR_H
#define COLORPICKERMODEEDITOR_H

#include <QWidget>
#include "ui_colorpickermodeeditor.h"

class ColorPickerModeEditor : public QWidget, Ui::ColorPickerModeEditor
{
	Q_OBJECT

public:
	explicit ColorPickerModeEditor(QWidget *parent = 0);
	~ColorPickerModeEditor(){};

private:

};

#endif // COLORPICKERMODEEDITOR_H
