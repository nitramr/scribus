#include <QPainter>
#include <QSignalBlocker>
#include <QSlider>

#include "colorpickercolormixer.h"
#include "sccolorengine.h"
#include "colorchart.h"
#include "util_color.h"
#include "scrspinbox.h"
#include "sccombobox.h"
#include "scmessagebox.h"
#include "iconmanager.h"


ColorPickerColorMixer::ColorPickerColorMixer(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);

	m_mixermode = MixerMode::Map;
	Wsave = false;
	dynamic = true;
	isHLC = false;

	widgetBlack->hide();
	widgetCyan->hide();
	widgetMagenta->hide();
	widgetYellow->hide();

	Color = ScColor(0,0,0,0);
	Color.setDisplayName(tr("New Color"));

	imageA = QPixmap(40,40);
	imageN = QPixmap(40,40);

	alertIcon = IconManager::instance()->loadPixmap("alert.png");

	CyanSp->setNewUnit(0);
	CyanSp->setMinimum(0);
	CyanSp->setMaximum(100);
	CyanSp->setSuffix( tr(" %"));

	CyanSL->setAutoFillBackground(true);
	CyanSL->setMinimum(0);
	CyanSL->setMaximum(100);
//	CyanSL->setMinimumSize(QSize(255, 16));

	MagentaSp->setNewUnit(0);
	MagentaSp->setMinimum(0);
	MagentaSp->setMaximum(100);
	MagentaSp->setSuffix( tr(" %"));

	MagentaSL->setAutoFillBackground(true);
	MagentaSL->setMinimum(0);
	MagentaSL->setMaximum(100);
//	MagentaSL->setMinimumSize(QSize(255, 16));

	YellowSp->setNewUnit(0);
	YellowSp->setMinimum(0);
	YellowSp->setMaximum(100);
	YellowSp->setSuffix( tr(" %"));

	YellowSL->setAutoFillBackground(true);
	YellowSL->setMinimum(0);
	YellowSL->setMaximum(100);
//	YellowSL->setMinimumSize(QSize(255, 16));

	BlackSp->setNewUnit(0);
	BlackSp->setMinimum(0);
	BlackSp->setMaximum(100);
	BlackSp->setSuffix( tr(" %"));

	BlackSL->setAutoFillBackground(true);
	BlackSL->setMinimum(0);
	BlackSL->setMaximum(100);
//	BlackSL->setMinimumSize(QSize(255, 16));

	ComboBox1->addItem( tr( "CMYK" ) );
	ComboBox1->addItem( tr( "RGB" ) );
	ComboBox1->addItem( tr( "Web Safe RGB" ) );
	ComboBox1->addItem( tr( "Lab" ) );
	ComboBox1->addItem( tr( "HLC" ) );

	ColorName->installEventFilter(this);

	connect(toggleColorMap, SIGNAL(clicked(bool)), this, SLOT(handleToggleColorMap()));
	connect(toggleColorList, SIGNAL(clicked(bool)), this, SLOT(handleToggleColorList()));
	connect(ColorName, SIGNAL(textChanged(QString)), this, SLOT(handleColorName()));

}

/*********************************************************************
*
* Setup
*
**********************************************************************/

bool ColorPickerColorMixer::eventFilter(QObject *object, QEvent *event){
	Q_UNUSED(object)
		if (event->type() == QEvent::FocusOut) {

			if (ColorName->text().isEmpty())
			{
				ScMessageBox::information(this, CommonStrings::trWarning, tr("You cannot create a color without a name.\nPlease give it a name"));
				setColorName(tr("New Color"));
				ColorName->setFocus();
				ColorName->selectAll();
			}else if (ColorName->text() == CommonStrings::None || ColorName->text() == CommonStrings::tr_NoneColor)
			{
				ScMessageBox::information(this, CommonStrings::trWarning, tr("You cannot create a color named \"%1\".\nIt is a reserved name for transparent color").arg(ColorName->text()));
				setColorName(tr("New Color"));
				ColorName->setFocus();
				ColorName->selectAll();
			}else prepColor();
		}
		return false;
}

/*********************************************************************
*
* Doc
*
**********************************************************************/
void ColorPickerColorMixer::setDoc(ScribusDoc *d)
{
	if((d == (ScribusDoc*) m_doc)/* || (m_ScMW && m_ScMW->scriptIsRunning())*/)
		return;

//	if (m_doc)
//	{
//		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
//		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
//	}

	m_doc = d;

	setObjectColor(Color);

	//updatePickerSettings();

}

/*********************************************************************
*
* Color Name
*
**********************************************************************/

void ColorPickerColorMixer::setColorName(QString name){

	ColorName->setText(name);
//	ColorName->selectAll();
//	ColorName->setFocus();

}

QString ColorPickerColorMixer::colorName()
{
	return ColorName->text();
}


void ColorPickerColorMixer::handleColorName(){

	prepColor();

}

/*********************************************************************
*
* Color
*
**********************************************************************/

void ColorPickerColorMixer::setObjectColor(ScColor orig){

	Color = orig;
	setColorName(Color.getDisplayName());

	updatePickerSettings();

}

void ColorPickerColorMixer::setSpot()
{
	disconnect(ComboBox1, SIGNAL(activated(const QString&)), this, SLOT(selModel(const QString&)));
	if (Separations->isChecked())
	{
		ComboBox1->setCurrentIndex( 0 );
//		Commented out to allow RGB Spot-Colors
//		selModel( tr("CMYK"));
	}
	connect(ComboBox1, SIGNAL(activated(const QString&)), this, SLOT(selModel(const QString&)));
}

bool ColorPickerColorMixer::isSpotColor()
{
	return Separations->isChecked();
}

/*********************************************************************
*
* Members
*
**********************************************************************/

void ColorPickerColorMixer::updatePickerSettings(){

	if(!m_doc)
		m_doc = 0; //return;


	imageA.fill( ScColorEngine::getDisplayColor(Color, m_doc) );
	if ( ScColorEngine::isOutOfGamut(Color, m_doc) )
		paintAlert(alertIcon,imageA, 2, 2, false);

	imageN.fill( ScColorEngine::getDisplayColor(Color, m_doc) );
	if ( ScColorEngine::isOutOfGamut(Color, m_doc) )
		paintAlert(alertIcon, imageN, 2, 2, false);

	OldC->setPixmap( imageA );
	NewC->setPixmap( imageN );

	Separations->setChecked(Color.isSpotColor());

	CyanSL->setPalette(sliderPix(180));
	MagentaSL->setPalette(sliderPix(300));
	YellowSL->setPalette(sliderPix(60));
	BlackSL->setPalette(sliderBlack());

	if (Color.getColorModel () == colorModelCMYK)
	{
		double ccd, cmd, cyd, ckd;
		CMYKColor cmyk;
		ScColorEngine::getCMYKValues(Color, m_doc, cmyk);
		ccd = cmyk.c / 2.55;
		cmd = cmyk.m / 2.55;
		cyd = cmyk.y / 2.55;
		ckd = cmyk.k / 2.55;
		CyanSp->setValue(ccd);
		CyanSL->setValue(qRound(ccd * 1000));
		MagentaSp->setValue(cmd);
		MagentaSL->setValue(qRound(cmd * 1000));
		YellowSp->setValue(cyd);
		YellowSL->setValue(qRound(cyd * 1000));
		BlackSp->setValue(ckd);
		BlackSL->setValue(qRound(ckd * 1000));
		BlackComp = cmyk.k;
	}

	int h, s, v;
	ScColorEngine::getRGBColor(Color, m_doc).getHsv(&h, &s, &v);
	ColorMap->setFixedWidth(180);
	ColorMap->drawPalette(v);
	ColorMap->setMark(h, s);

	if (Color.getColorModel () == colorModelRGB)
	{
		ComboBox1->setCurrentIndex(1);
		selModel ( tr( "RGB" ));
	}
	else if (Color.getColorModel() == colorModelCMYK)
	{
		ComboBox1->setCurrentIndex(0);
		selModel ( tr( "CMYK" ));
	}
	else if (Color.getColorModel() == colorModelLab)
	{
		ComboBox1->setCurrentIndex(3);
		selModel ( tr( "Lab" ));
	}
	isRegistration = Color.isRegistrationColor();
	if (Color.isRegistrationColor())
	{
		ComboBox1->setEnabled(false);
		Separations->setEnabled(false);
	}


	connect( CyanSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( MagentaSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( YellowSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( BlackSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );

	connect( CyanSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	connect( MagentaSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	connect( YellowSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	connect( BlackSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );

	connect( CyanSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	connect( MagentaSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	connect( YellowSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	connect( BlackSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );

	connect( ColorMap, SIGNAL( ColorVal(int, int, bool)), this, SLOT( setColor2(int, int, bool)));
	connect( ComboBox1, SIGNAL(activated(const QString&)), this, SLOT(selModel(const QString&)));
	connect( Separations, SIGNAL(clicked()), this, SLOT(setSpot()));

}


QPalette ColorPickerColorMixer::sliderPix(int color)
{
	RGBColor rgb;
	CMYKColor cmyk;
	QImage image0 = QImage(255, 10, QImage::Format_ARGB32);
	QPainter p;
	p.begin(&image0);
	p.setPen(Qt::NoPen);
	int r, g, b, c, m, y, k;
	QColor tmp;
	for (int x = 0; x < 255; x += 5)
	{
		if (Color.getColorModel() == colorModelCMYK)
		{
			ScColorEngine::getCMYKValues(Color, m_doc, cmyk);
			cmyk.getValues(c, m, y, k);
			if (dynamic)
			{
				switch (color)
				{
				case 180:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(x, m, y, k), m_doc);
					break;
				case 300:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(c, x, y, k), m_doc);
					break;
				case 60:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(c, m, x, k), m_doc);
					break;
				}
				p.setBrush(tmp);
			}
			else
			{
				switch (color)
				{
				case 180:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(x, 0, 0, 0), m_doc);
					break;
				case 300:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(0, x, 0, 0), m_doc);
					break;
				case 60:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(0, 0, x, 0), m_doc);
					break;
				}
				p.setBrush(tmp);
			}
		}
		else if (Color.getColorModel() == colorModelRGB)
		{
			ScColorEngine::getRGBValues(Color, m_doc, rgb);
			rgb.getValues(r, g, b);
			if (dynamic)
			{
				switch (color)
				{
				case 0:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(x, g, b), m_doc);
					break;
				case 120:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(r, x, b), m_doc);
					break;
				case 240:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(r, g, x), m_doc);
					break;
				}
				p.setBrush(tmp);
			}
			else
			{
				switch (color)
				{
				case 0:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(x, 0, 0), m_doc);
					break;
				case 120:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(0, x, 0), m_doc);
					break;
				case 240:
					tmp = ScColorEngine::getDisplayColorGC(ScColor(0, 0, x), m_doc);
					break;
				}
				p.setBrush(tmp);
			}
		}
		else if (Color.getColorModel() == colorModelLab)
		{
			double L, a, b;
			double val = static_cast<double>(x) / 255.0;
			Color.getLab(&L, &a, &b);
			if (isHLC)
			{
				QLineF lin;
				lin.setP1(QPointF(0.0, 0.0));
				lin.setP2(QPointF(a, b));
				double H = lin.angle();
				double C = lin.length();
				double tmpA, tmpB;
				if (dynamic)
				{
					switch (color)
					{
					case 0:
						lin = QLineF::fromPolar(C, -360 * val);
						tmpA = lin.p2().x();
						tmpB = lin.p2().y();
						tmp = ScColorEngine::getDisplayColorGC(ScColor(L, tmpA, tmpB), m_doc);
						break;
					case 120:
						tmp = ScColorEngine::getDisplayColorGC(ScColor(100 * val, a, b), m_doc);
						break;
					case 240:
						lin = QLineF::fromPolar(128 * val, H);
						tmpA = lin.p2().x();
						tmpB = lin.p2().y();
						tmp = ScColorEngine::getDisplayColorGC(ScColor(L, tmpA, tmpB), m_doc);
						break;
					}
					p.setBrush(tmp);
				}
				else
				{
					switch (color)
					{
					case 0:
						lin = QLineF::fromPolar(128, -360 * val);
						tmpA = lin.p2().x();
						tmpB = lin.p2().y();
						tmp = ScColorEngine::getDisplayColorGC(ScColor(100.0, tmpA, tmpB), m_doc);
						break;
					case 120:
						tmp = ScColorEngine::getDisplayColorGC(ScColor(100 * val, 0.0, 0.0), m_doc);
						break;
					case 240:
						lin = QLineF::fromPolar(128 * val, 0);
						tmpA = lin.p2().x();
						tmpB = lin.p2().y();
						tmp = ScColorEngine::getDisplayColorGC(ScColor(100.0, tmpA, tmpB), m_doc);
						break;
					}
					p.setBrush(tmp);
				}
			}
			else
			{
				if (dynamic)
				{
					switch (color)
					{
					case 0:
						tmp = ScColorEngine::getDisplayColorGC(ScColor(100 * val, a, b), m_doc);
						break;
					case 120:
						tmp = ScColorEngine::getDisplayColorGC(ScColor(L, 256 * val - 128.0, b), m_doc);
						break;
					case 240:
						tmp = ScColorEngine::getDisplayColorGC(ScColor(L, a, 256 * val - 128.0), m_doc);
						break;
					}
					p.setBrush(tmp);
				}
				else
				{
					switch (color)
					{
					case 0:
						tmp = ScColorEngine::getDisplayColorGC(ScColor(100 * val, 0.0, 0.0), m_doc);
						break;
					case 120:
						tmp = ScColorEngine::getDisplayColorGC(ScColor(100.0, 256 * val - 128.0, 0.0), m_doc);
						break;
					case 240:
						tmp = ScColorEngine::getDisplayColorGC(ScColor(100.0, 0.0, 256 * val - 128.0), m_doc);
						break;
					}
				}
				p.setBrush(tmp);
			}
		}
		p.drawRect(x, 0, 5, 10);
	}
	p.end();
	QPalette pal;
	pal.setBrush(QPalette::Window, QBrush(image0));
	return pal;
}

QPalette ColorPickerColorMixer::sliderBlack()
{
	QImage image0 = QImage(255, 10, QImage::Format_ARGB32);
	QPainter p;
	int val = 255;
	p.begin(&image0);
	p.setPen(Qt::NoPen);
	int c, m, y, k;
	CMYKColor cmyk;
	ScColorEngine::getCMYKValues(Color, m_doc, cmyk);
	cmyk.getValues(c, m, y, k);
	for (int x = 0; x < 255; x += 5)
	{
		if (dynamic)
			p.setBrush( ScColorEngine::getDisplayColorGC(ScColor(c, m, y, x), m_doc) );
		else
			p.setBrush( ScColorEngine::getDisplayColorGC(ScColor(0, 0, 0, x), m_doc) );
		p.drawRect(x, 0, 5, 10);
		val -= 5;
	}
	p.end();
	QPalette pal;
	pal.setBrush(QPalette::Window, QBrush(image0));
	return pal;
}

void ColorPickerColorMixer::setValues()
{
	QSignalBlocker cyanSpBlocker(CyanSp);
	QSignalBlocker cyanSLBlocker(CyanSL);
	QSignalBlocker magentaSpBlocker(MagentaSp);
	QSignalBlocker magentaSLBlocker(MagentaSL);
	QSignalBlocker yellowSpBlocker(YellowSp);
	QSignalBlocker yellowSLBlocker(YellowSL);
	QSignalBlocker blackSpBlocker(BlackSp);
	QSignalBlocker blackSLBlocker(BlackSL);

	if (Color.getColorModel() == colorModelCMYK)
	{
		CMYKColor cmyk;
		int cc, cm, cy, ck;
		ScColorEngine::getCMYKValues(Color, m_doc, cmyk);
		cmyk.getValues(cc, cm, cy, ck);
		CyanSp->setValue(cc / 2.55);
		CyanSL->setValue(qRound(cc / 2.55) * 1000.0);
		MagentaSp->setValue(cm / 2.55);
		MagentaSL->setValue(qRound(cm / 2.55) * 1000.0);
		YellowSp->setValue(cy / 2.55);
		YellowSL->setValue(qRound(cy / 2.55) * 1000.0);
		BlackSp->setValue(ck / 2.55);
		BlackSL->setValue(qRound(ck / 2.55) * 1000.0);
		if (dynamic)
		{
			CyanSL->setPalette(sliderPix(180));
			MagentaSL->setPalette(sliderPix(300));
			YellowSL->setPalette(sliderPix(60));
			BlackSL->setPalette(sliderBlack());
		}
	}
	else if (Color.getColorModel() == colorModelRGB)
	{
		RGBColor rgb;
		int r, g, b;
		ScColorEngine::getRGBValues(Color, m_doc, rgb);
		rgb.getValues(r, g, b);
		CyanSp->setValue(static_cast<double>(r));
		CyanSL->setValue(r * 1000.0);
		MagentaSp->setValue(static_cast<double>(g));
		MagentaSL->setValue(g * 1000.0);
		YellowSp->setValue(static_cast<double>(b));
		YellowSL->setValue(b * 1000.0);
		int h, s, v;
		ScColorEngine::getRGBColor(Color, m_doc).getHsv(&h, &s, &v);
		BlackComp = 255 - v;
		if (dynamic)
		{
			CyanSL->setPalette(sliderPix(0));
			MagentaSL->setPalette(sliderPix(120));
			YellowSL->setPalette(sliderPix(240));
		}
	}
	else if (Color.getColorModel() == colorModelLab)
	{
		double L, a, b;
		Color.getLab(&L, &a, &b);
		if (isHLC)
		{
			MagentaSp->setValue(L);
			MagentaSL->setValue(L * 1000.0);
			QLineF lin;
			lin.setP1(QPointF(0.0, 0.0));
			lin.setP2(QPointF(a, b));
			CyanSp->setValue(360 - lin.angle());
			CyanSL->setValue(360 - lin.angle() * 1000.0);
			YellowSp->setValue(lin.length());
			YellowSL->setValue(lin.length() * 1000.0);
		}
		else
		{
			CyanSp->setValue(L);
			CyanSL->setValue(L * 1000.0);
			MagentaSp->setValue(a);
			MagentaSL->setValue(a * 1000.0);
			YellowSp->setValue(b);
			YellowSL->setValue(b * 1000.0);
		}
		if (dynamic)
		{
			CyanSL->setPalette(sliderPix(0));
			MagentaSL->setPalette(sliderPix(120));
			YellowSL->setPalette(sliderPix(240));
		}
	}
}

void ColorPickerColorMixer::setColor()
{
	int c, m, y;
	int h, s, v;
	int k = 0;
	double L, a, b;
	ScColor tmp;
	if (Color.getColorModel() == colorModelCMYK)
	{
		c = qRound(CyanSp->value() * 2.55);
		m = qRound(MagentaSp->value() * 2.55);
		y = qRound(YellowSp->value() * 2.55);
		k = qRound(BlackSp->value() * 2.55);
		tmp.setColor(c, m, y, k);
		Color = tmp;
		if (dynamic)
		{
			CyanSL->setPalette(sliderPix(180));
			MagentaSL->setPalette(sliderPix(300));
			YellowSL->setPalette(sliderPix(60));
			BlackSL->setPalette(sliderBlack());
		}
		BlackComp = k;
		ScColorEngine::getRGBColor(tmp, m_doc).getHsv(&h, &s, &v);
		ColorMap->drawPalette(v);
		ColorMap->setMark(h, s);
	}
	else if (Color.getColorModel() == colorModelRGB)
	{
		c = qRound(CyanSp->value());
		m = qRound(MagentaSp->value());
		y = qRound(YellowSp->value());
		k = qRound(BlackSp->value());
		if (Wsave)
		{
			blockSignals(true);
			c = c / 51 * 51;
			m = m / 51 * 51;
			y = y / 51 * 51;
			CyanSp->setValue(c);
			MagentaSp->setValue(m);
			YellowSp->setValue(y);
			CyanSL->setValue(c * 1000.0);
			MagentaSL->setValue(m * 1000.0);
			YellowSL->setValue(y * 1000.0);
			blockSignals(false);
		}
		tmp.setColorRGB(c, m, y);
		QColor tmp2 = QColor(c, m, y);
		tmp2.getHsv(&h, &s, &v);
		BlackComp = 255 - v;
		Color = tmp;
		if (dynamic)
		{
			CyanSL->setPalette(sliderPix(0));
			MagentaSL->setPalette(sliderPix(120));
			YellowSL->setPalette(sliderPix(240));
		}
		BlackComp = k;
		ScColorEngine::getRGBColor(tmp, m_doc).getHsv(&h, &s, &v);
		ColorMap->drawPalette(v);
		ColorMap->setMark(h, s);
	}
	else if (Color.getColorModel() == colorModelLab)
	{
		double Lalt;
		Color.getLab(&Lalt, &a, &b);
		if (isHLC)
		{
			L = MagentaSp->value();
			double cv = 360 - CyanSp->value();
			double yv = YellowSp->value();
			QLineF lin = QLineF::fromPolar(yv, cv);
			a = lin.p2().x();
			b = lin.p2().y();
		}
		else
		{
			L = CyanSp->value();
			a = MagentaSp->value();
			b = YellowSp->value();
		}
		tmp.setColor(L, a, b);
		Color = tmp;
		if (dynamic)
		{
			CyanSL->setPalette(sliderPix(0));
			MagentaSL->setPalette(sliderPix(120));
			YellowSL->setPalette(sliderPix(240));
		}
		BlackComp = qRound(L * 2.55);
		if (L != Lalt)
			ColorMap->drawPalette(L * 2.55);
		ColorMap->setMark(a, b);
	}
	imageN.fill(ScColorEngine::getDisplayColor(tmp, m_doc) );
	if ( ScColorEngine::isOutOfGamut(tmp, m_doc) )
		paintAlert(alertIcon, imageN, 2, 2, false);
	NewC->setPixmap( imageN );

	prepColor();
}

void ColorPickerColorMixer::setColor2(int h, int s, bool ende)
{
	ScColor tmp;
	if (Color.getColorModel() == colorModelLab)
	{
		if (isHLC)
			tmp = ScColor(MagentaSp->value(), static_cast<double>(h), static_cast<double>(s));
		else
			tmp = ScColor(CyanSp->value(), static_cast<double>(h), static_cast<double>(s));
	}
	else
	{
		QColor tm = QColor::fromHsv(qMax(qMin(359,h),0), qMax(qMin(255,255-s),0), 255-BlackComp, QColor::Hsv);
		int r, g, b;
		tm.getRgb(&r, &g, &b);
		tmp.fromQColor(tm);
		if (Color.getColorModel() == colorModelCMYK)
		{
			CMYKColor cmyk;
			ScColorEngine::getCMYKValues(tmp, m_doc, cmyk);
			tmp.setColor(cmyk.c, cmyk.m, cmyk.y, cmyk.k);
		}
	}
	imageN.fill( ScColorEngine::getDisplayColor(tmp, m_doc) );
	if ( ScColorEngine::isOutOfGamut(tmp, m_doc) )
		paintAlert(alertIcon, imageN, 2, 2, false);
	NewC->setPixmap( imageN );
	Color = tmp;
	if (ende)
		setValues();

	prepColor();
}

void ColorPickerColorMixer::setValSLiders(double value)
{
	if (CyanSp == sender())
		CyanSL->setValue(value * 1000);
	if (MagentaSp == sender())
		MagentaSL->setValue(value * 1000);
	if (YellowSp == sender())
		YellowSL->setValue(value * 1000);
	if (BlackSp == sender())
		BlackSL->setValue(value * 1000);
}

void ColorPickerColorMixer::setValueS(int val)
{
	disconnect( CyanSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	disconnect( MagentaSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	disconnect( YellowSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	disconnect( BlackSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	if (CyanSL == sender())
		CyanSp->setValue(val / 1000.0);
	if (MagentaSL == sender())
		MagentaSp->setValue(val / 1000.0);
	if (YellowSL == sender())
		YellowSp->setValue(val / 1000.0);
	if (BlackSL == sender())
		BlackSp->setValue(val / 1000.0);
	setColor();
	connect( CyanSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( MagentaSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( YellowSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( BlackSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
}

void ColorPickerColorMixer::setColorPaintMode(ColorPaintMode mode, GradientTypes gradient)
{


	switch(mode){
	case ColorPaintMode::Pattern:
		this->setVisible(false);
		this->toggleColorList->setVisible(true);
		break;
	case ColorPaintMode::Gradient:
		switch(gradient){
		case GradientTypes::Linear:
		case GradientTypes::Radial:
		case GradientTypes::Diamond:
		case GradientTypes::Conical:
			this->setVisible(true);
			this->toggleColorList->setVisible(true);
			break;
		case GradientTypes::FourColors:
		case GradientTypes::Mesh:
		case GradientTypes::PatchMesh:
		default:
			this->setVisible(true);
			this->toggleColorList->setVisible(false);
			break;
		}
		break;

	case ColorPaintMode::Hatch:
		this->setVisible(true);
		this->toggleColorList->setVisible(false);
		break;
	case ColorPaintMode::Solid:
		this->setVisible(true);
		this->toggleColorList->setVisible(true);
		break;
	}

}


/*********************************************************************
*
* Event Handler
*
**********************************************************************/

void ColorPickerColorMixer::handleToggleColorMap(){

	switch(m_mixermode){
	case MixerMode::Map:

		ColorMap->hide();
		widgetBlack->show();
		widgetCyan->show();
		widgetMagenta->show();
		widgetYellow->show();
		m_mixermode = MixerMode::Slider;
		break;
	case MixerMode::Slider:
		ColorMap->show();
		widgetBlack->hide();
		widgetCyan->hide();
		widgetMagenta->hide();
		widgetYellow->hide();
		m_mixermode = MixerMode::Map;
		break;
	}

}

void ColorPickerColorMixer::handleToggleColorList(){

	emit sendToggleColorList();

}

void ColorPickerColorMixer::prepColor(){

	Color.setDisplayName(colorName());
	Color.setSpotColor(this->isSpotColor());

	emit emitColor(Color);
}



void ColorPickerColorMixer::selModel(const QString& mod)
{
	int h, s, v;

	disconnect( CyanSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	disconnect( MagentaSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	disconnect( YellowSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	disconnect( BlackSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	disconnect( CyanSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	disconnect( MagentaSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	disconnect( YellowSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	disconnect( BlackSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	disconnect( CyanSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	disconnect( MagentaSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	disconnect( YellowSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	disconnect( BlackSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	isHLC = false;
	if (mod == tr("CMYK"))
	{
		Wsave = false;
		CyanSL->setMaximum( 100 * 1000.0);
		CyanSL->setMinimum( 0 * 1000.0 );
		CyanSL->setSingleStep(1 * 1000.0);
		CyanSL->setPageStep(10 * 1000.0);

		MagentaSL->setMaximum( 100 * 1000.0 );
		MagentaSL->setMinimum( 0 * 1000.0 );
		MagentaSL->setSingleStep(1 * 1000.0);
		MagentaSL->setPageStep(10 * 1000.0);

		YellowSL->setMaximum( 100 * 1000.0 );
		YellowSL->setMinimum( 0 * 1000.0 );
		YellowSL->setSingleStep(1 * 1000.0);
		YellowSL->setPageStep(10 * 1000.0);

		BlackSL->setMaximum( 100 * 1000.0);
		BlackSL->setMinimum( 0 * 1000.0);
		BlackSL->setSingleStep(1 * 1000.0);
		BlackSL->setPageStep(10 * 1000.0);

		CyanSp->setMaximum( 100 );
		CyanSp->setMinimum( 0 );
		CyanSp->setDecimals(1);
		CyanSp->setSingleStep(1);
		CyanSp->setSuffix( tr(" %"));

		MagentaSp->setMaximum( 100);
		MagentaSp->setMinimum( 0 );
		MagentaSp->setDecimals(1);
		MagentaSp->setSingleStep(1);
		MagentaSp->setSuffix( tr(" %"));

		YellowSp->setMaximum( 100 );
		YellowSp->setMinimum( 0 );
		YellowSp->setDecimals(1);
		YellowSp->setSingleStep(1);
		YellowSp->setSuffix( tr(" %"));

		BlackSp->setDecimals(1);

		CyanT->setText( tr("C:"));
		MagentaT->setText( tr("M:"));
		YellowT->setText( tr("Y:"));
		BlackSL->show();
		BlackSp->show();
		BlackT->show();
		if (Color.getColorModel() != colorModelCMYK)
			Color = ScColorEngine::convertToModel(Color, m_doc, colorModelCMYK);
		CyanSL->setPalette(sliderPix(180));
		MagentaSL->setPalette(sliderPix(300));
		YellowSL->setPalette(sliderPix(60));
		BlackSL->setPalette(sliderBlack());
		ScColorEngine::getRGBColor(Color, m_doc).getHsv(&h, &s, &v);
		setValues();
		ColorMap->drawMode = 0;
		ColorMap->drawPalette(v);
		ColorMap->setMark(h, s);
	}
	else if ((mod == tr("Web Safe RGB")) || (mod == tr("RGB")))
	{
		Wsave = false;
		CyanSL->setMaximum( 255 * 1000.0 );
		CyanSL->setMinimum( 0 * 1000.0 );
		CyanSL->setSingleStep(1 * 1000.0);
		CyanSL->setPageStep(1 * 1000.0);

		MagentaSL->setMaximum( 255 * 1000.0 );
		MagentaSL->setMinimum( 0 * 1000.0 );
		MagentaSL->setSingleStep(1 * 1000.0);
		MagentaSL->setPageStep(1 * 1000.0);

		YellowSL->setMaximum( 255 * 1000.0 );
		YellowSL->setMinimum( 0 * 1000.0 );
		YellowSL->setSingleStep(1 * 1000.0);
		YellowSL->setPageStep(1 * 1000.0);

		CyanSp->setSingleStep(1);
		CyanSp->setMaximum( 255 );
		CyanSp->setMinimum( 0 );
		CyanSp->setDecimals(0);
		CyanSp->setSuffix("");

		MagentaSp->setSingleStep(1);
		MagentaSp->setMaximum( 255 );
		MagentaSp->setMinimum( 0 );
		MagentaSp->setDecimals(0);
		MagentaSp->setSuffix("");

		YellowSp->setSingleStep(1);
		YellowSp->setMaximum( 255 );
		YellowSp->setMinimum( 0 );
		YellowSp->setDecimals(0);
		YellowSp->setSuffix("");

		CyanT->setText( tr("R:"));
		MagentaT->setText( tr("G:"));
		YellowT->setText( tr("B:"));
		BlackSL->hide();
		BlackSp->hide();
		BlackT->hide();
		if (mod == tr("Web Safe RGB"))
		{
			Wsave = true;
			CyanSL->setSingleStep(51 * 1000.0);
			MagentaSL->setSingleStep(51 * 1000.0);
			YellowSL->setSingleStep(51 * 1000.0);
			CyanSL->setPageStep(51 * 1000.0);
			MagentaSL->setPageStep(51 * 1000.0);
			YellowSL->setPageStep(51 * 1000.0);
			CyanSp->setSingleStep(51);
			MagentaSp->setSingleStep(51);
			YellowSp->setSingleStep(51);
		}
		if (Color.getColorModel() != colorModelRGB)
			Color = ScColorEngine::convertToModel(Color, m_doc, colorModelRGB);
		CyanSL->setPalette(sliderPix(0));
		MagentaSL->setPalette(sliderPix(120));
		YellowSL->setPalette(sliderPix(240));
		ScColorEngine::getRGBColor(Color, m_doc).getHsv(&h, &s, &v);
		setValues();
		ColorMap->drawMode = 0;
		ColorMap->drawPalette(v);
		ColorMap->setMark(h, s);
	}
	else if (mod == tr("Lab"))
	{
		Wsave = false;
		CyanSL->setSingleStep(1 * 1000.0);
		CyanSL->setPageStep(10 * 1000.0);
		CyanSL->setMaximum( 100 * 1000.0 );
		CyanSL->setMinimum( 0 * 1000.0 );
		MagentaSL->setSingleStep(1 * 1000.0);
		MagentaSL->setPageStep(10 * 1000.0);
		MagentaSL->setMaximum( 128 * 1000.0 );
		MagentaSL->setMinimum( -128 * 1000.0 );
		YellowSL->setSingleStep(1 * 1000.0);
		YellowSL->setPageStep(10 * 1000.0);
		YellowSL->setMaximum( 128 * 1000.0 );
		YellowSL->setMinimum( -128 * 1000.0 );

		CyanSp->setDecimals(2);
		CyanSp->setSingleStep(1);
		CyanSp->setMaximum( 100 );
		CyanSp->setSuffix( tr(""));

		MagentaSp->setDecimals(2);
		MagentaSp->setSingleStep(1);
		MagentaSp->setMaximum( 128);
		MagentaSp->setMinimum( -128 );
		MagentaSp->setSuffix("");

		YellowSp->setDecimals(2);
		YellowSp->setMaximum( 128 );
		YellowSp->setMinimum( -128 );
		YellowSp->setSingleStep(1);
		YellowSp->setSuffix("");

		CyanT->setText( tr("L:"));
		MagentaT->setText( tr("a:"));
		YellowT->setText( tr("b:"));

		BlackSL->hide();
		BlackSp->hide();
		BlackT->hide();
		if (Color.getColorModel() != colorModelLab)
			Color = ScColorEngine::convertToModel(Color, m_doc, colorModelLab);
		CyanSL->setPalette(sliderPix(0));
		MagentaSL->setPalette(sliderPix(120));
		YellowSL->setPalette(sliderPix(240));
		setValues();
		ColorMap->drawMode = 1;
		ColorMap->drawPalette(CyanSp->value() * 2.55);
		ColorMap->setMark(MagentaSp->value(), YellowSp->value());
	}
	else if (mod == tr("HLC"))
	{
		Wsave = false;
		CyanSL->setSingleStep(1 * 1000.0);
		CyanSL->setPageStep(10 * 1000.0);
		CyanSL->setMaximum( 360 * 1000.0 );
		CyanSL->setMinimum( 0 * 1000.0 );
		MagentaSL->setSingleStep(1 * 1000.0);
		MagentaSL->setPageStep(10 * 1000.0);
		MagentaSL->setMaximum( 100 * 1000.0 );
		MagentaSL->setMinimum( 0 * 1000.0 );
		YellowSL->setSingleStep(1 * 1000.0);
		YellowSL->setPageStep(10 * 1000.0);
		YellowSL->setMaximum( 128 * 1000.0 );
		YellowSL->setMinimum( 0 * 1000.0 );

		CyanSp->setDecimals(2);
		CyanSp->setSingleStep(1);
		CyanSp->setMaximum( 360 );
		CyanSp->setSuffix( tr(""));

		MagentaSp->setDecimals(2);
		MagentaSp->setSingleStep(1);
		MagentaSp->setMaximum( 100);
		MagentaSp->setMinimum( 0 );
		MagentaSp->setSuffix("");

		YellowSp->setDecimals(2);
		YellowSp->setMaximum( 128 );
		YellowSp->setMinimum( 0 );
		YellowSp->setSingleStep(1);
		YellowSp->setSuffix("");

		CyanT->setText( tr("H:"));
		MagentaT->setText( tr("L:"));
		YellowT->setText( tr("C:"));

		BlackSL->hide();
		BlackSp->hide();
		BlackT->hide();
		if (Color.getColorModel() != colorModelLab)
			Color = ScColorEngine::convertToModel(Color, m_doc, colorModelLab);
		isHLC = true;
		CyanSL->setPalette(sliderPix(0));
		MagentaSL->setPalette(sliderPix(120));
		YellowSL->setPalette(sliderPix(240));
		setValues();
		ColorMap->drawMode = 2;
		double L, a, b;
		Color.getLab(&L, &a, &b);
		ColorMap->drawPalette(L * 2.55);
		ColorMap->setMark(a, b);
	}
	imageN.fill( ScColorEngine::getDisplayColor(Color, m_doc) );
	if (ScColorEngine::isOutOfGamut(Color, m_doc))
		paintAlert(alertIcon, imageN, 2, 2, false);
	NewC->setPixmap( imageN );
	NewC->setToolTip( "<qt>" + tr( "If color management is enabled, an exclamation mark indicates that the color may be outside of the color gamut of the current printer profile selected. What this means is the color may not print exactly as indicated on screen. More hints about gamut warnings are in the online help under Color Management." ) + "</qt>");
	OldC->setToolTip( "<qt>" + tr( "If color management is enabled, an exclamation mark indicates that the color may be outside of the color gamut of the current printer profile selected. What this means is the color may not print exactly as indicated on screen. More hints about gamut warnings are in the online help under Color Management." ) + "</qt>");

	prepColor();

	connect( CyanSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( MagentaSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( YellowSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( BlackSp, SIGNAL( valueChanged(double) ), this, SLOT( setValSLiders(double) ) );
	connect( CyanSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	connect( MagentaSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	connect( YellowSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	connect( BlackSL, SIGNAL( valueChanged(int) ), this, SLOT( setValueS(int) ) );
	connect( CyanSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	connect( MagentaSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	connect( YellowSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
	connect( BlackSL, SIGNAL( valueChanged(int) ), this, SLOT( setColor() ) );
}
