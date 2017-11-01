#ifndef SCCOLORPICKER_H
#define SCCOLORPICKER_H

#include <QWidget>

namespace Ui {
class ScColorPicker;
}

class ScColorPicker : public QWidget
{
	Q_OBJECT

public:
	explicit ScColorPicker(QWidget *parent = 0);
	~ScColorPicker();

private:
	Ui::ScColorPicker *ui;

signals:
	void setPreview(QPixmap);
	void resetColor();

private slots:
	void submitButtonPress();
	void resetButtonPress();

};

#endif // SCCOLORPICKER_H
