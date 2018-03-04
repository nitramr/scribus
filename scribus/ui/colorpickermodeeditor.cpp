
#include "colorpickermodeeditor.h"
#include "appmodes.h"
#include "insertTable.h"
#include "scribus.h"
#include "scribusview.h"
#include "selection.h"
#include "undomanager.h"
#include "util_math.h"


ColorPickerModeEditor::ColorPickerModeEditor(QWidget *parent) :
	QWidget(parent)
{
	undoManager = UndoManager::instance();
	m_ScMW = 0;
	m_item = 0;
	m_haveDoc   = false;
	m_haveItem  = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	setupUi(this);

	this->setVisible(false);


//	fillShade->setDecimals(0);
//	strokeShade->setDecimals(0);
	color1Alpha->setDecimals(0);
	color2Alpha->setDecimals(0);
	color3Alpha->setDecimals(0);
	color4Alpha->setDecimals(0);
	color1Shade->setDecimals(0);
	color2Shade->setDecimals(0);
	color3Shade->setDecimals(0);
	color4Shade->setDecimals(0);
	shadeMeshPoint->setDecimals(0);
//	strokeModeCombo->addItem( tr("Solid") );
//	strokeModeCombo->addItem( tr("Gradient") );

	colorPoint1->setPixmapType(ColorCombo::fancyPixmaps);
	colorPoint2->setPixmapType(ColorCombo::fancyPixmaps);
	colorPoint3->setPixmapType(ColorCombo::fancyPixmaps);
	colorPoint4->setPixmapType(ColorCombo::fancyPixmaps);
	colorMeshPoint->setPixmapType(ColorCombo::fancyPixmaps);
//	colorListFill->setPixmapType(ColorListBox::fancyPixmap);
//	colorListStroke->setPixmapType(ColorListBox::fancyPixmap);
	hatchLineColor->setPixmapType(ColorCombo::fancyPixmaps);
	hatchBackground->setPixmapType(ColorCombo::fancyPixmaps);

	// Gradient Dummy
	VGradient fill_gradient = VGradient(VGradient::linear);
	fill_gradient.clearStops();
	fill_gradient.addStop(QColor(Qt::black), 0.0, 0.5, 1.0, "Black", 100);
	fill_gradient.addStop(QColor(Qt::white), 1.0, 0.5, 1.0, "White", 100);
	gradEdit->setGradient(fill_gradient);
	gradientName->setText(tr("New Gradient"));
	gradientName->installEventFilter(this);

	// Gradient Vector Editor Dialog
	CGradDia = NULL;
	CGradDia = new GradientVectorDialog(this->parentWidget());
	CGradDia->hide();

	connect(fillModeStack,SIGNAL(currentChanged(int)),this, SLOT(updateSizes(int)));
	connect(stackedWidget_2,SIGNAL(currentChanged(int)),this, SLOT(updateSizesGradient(int)));


	// update tab stack size policy
	updateSizes(0);
	updateSizesGradient(0);


	connect(this, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)), this, SLOT(NewSpGradient(double, double, double, double, double, double, double, double, double, double )));
	connect(this, SIGNAL(editGradient(int)), this, SLOT(toggleGradientEdit(int)));


}


/*********************************************************************
*
* Setup
*
**********************************************************************/

void ColorPickerModeEditor::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
}

void ColorPickerModeEditor::connectSignals()
{
	connect(CGradDia, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)), this, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)));
	connect(CGradDia, SIGNAL(paletteShown(bool)), this, SLOT(setActiveGradDia(bool)));
	connect(CGradDia, SIGNAL(editGradient(int)), this, SIGNAL(editGradient(int)));
	connect(CGradDia, SIGNAL(createNewMesh()), this, SLOT(createNewMeshGradient()));
	connect(CGradDia, SIGNAL(resetMesh()), this, SLOT(resetMeshGradient()));
	connect(CGradDia, SIGNAL(meshToShape()), this, SLOT(meshGradientToShape()));
//	connect(CGradDia, SIGNAL(reset1Control()), this, SLOT(resetOneControlPoint()));
//	connect(CGradDia, SIGNAL(resetAllControl()), this, SLOT(resetAllControlPoints()));
	connect(CGradDia, SIGNAL(removePatch()), this, SLOT(handleRemovePatch()));
	connect(CGradDia, SIGNAL(snapToMGrid(bool)), this, SLOT(snapToPatchGrid(bool)));

	connect(colorPoint1    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	connect(colorPoint2    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	connect(colorPoint3    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	connect(colorPoint4    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	connect(color1Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color2Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color3Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color4Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color1Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color2Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color3Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	connect(color4Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
//	connect(colorListFill  , SIGNAL(currentRowChanged(int)), this, SLOT(selectColorF(int)));
//	connect(colorListStroke, SIGNAL(currentRowChanged(int)), this, SLOT(selectColorS(int)));
	connect(editMeshColors , SIGNAL(clicked()), this, SLOT(editMeshPointColor()));
//	connect(editPatternProps      , SIGNAL(clicked()) , this, SLOT(changePatternProps()));
//	connect(editPatternPropsStroke, SIGNAL(clicked()), this, SLOT(changePatternPropsStroke()));
//	connect(fillShade     , SIGNAL(valueChanged(double)), this, SIGNAL(NewBrushShade(double)));
//	connect(followsPath   , SIGNAL(clicked()), this, SLOT(toggleStrokePattern()));
//	connect(gradientTypeStroke , SIGNAL(activated(int)), this, SLOT(slotGradTypeStroke(int)));
	connect(gradEdit      , SIGNAL(gradientChanged()), this, SLOT(handleFillGradient()));
	connect(gradientName  , SIGNAL(textChanged(QString)), this, SLOT(handleFillGradient()));
	connect(gradEditButton, SIGNAL(clicked()), this, SLOT(editGradientVector()));
//	connect(gradEditButtonStroke, SIGNAL(clicked()), this, SLOT(editGradientVectorStroke()));
//	connect(gradEditStroke, SIGNAL(gradientChanged()), this, SLOT(handleStrokeGradient()));
	connect(gradientType  , SIGNAL(activated(int)), this, SLOT(slotGradType(int)));
//	connect(gradientTypeStroke , SIGNAL(activated(int)), this, SLOT(slotGradTypeStroke(int)));
//	connect(namedGradient , SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
//	connect(namedGradientStroke, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradientStroke(const QString &)));
//	connect(overPrintCombo     , SIGNAL(activated(int)), this, SIGNAL(NewOverprint(int)));
//	connect(patternBox         , SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
//	connect(patternBoxStroke   , SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPatternS(QListWidgetItem*)));
//	connect(strokeShade        , SIGNAL(valueChanged(double)), this, SIGNAL(NewPenShade(double)));
//	connect(strokeModeCombo    , SIGNAL(currentIndexChanged(int)), this, SLOT(slotGradStroke(int)));
//	connect(tabFillStroke      , SIGNAL(currentChanged(int)), this, SLOT(fillStrokeSelector(int)));
	connect(colorMeshPoint , SIGNAL(activated(int)), this, SLOT(updateMeshPoint()));
	connect(shadeMeshPoint     , SIGNAL(valueChanged(double)), this, SLOT(updateMeshPoint()));
	connect(transparencyMeshPoint, SIGNAL(valueChanged(double)), this, SLOT(updateMeshPoint()));
	connect(hatchAngle		, SIGNAL(valueChanged(double)), this, SLOT(changeHatchProps()));
	connect(hatchDist		, SIGNAL(valueChanged(double)), this, SLOT(changeHatchProps()));
	connect(hatchType		, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	connect(hatchBackground	, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	connect(hatchLineColor	, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));

	connect(gradEdit  , SIGNAL(emitGradientExtend(int)), this, SLOT(handleGradientExtend(int)));
//	connect(gradientExtend  , SIGNAL(activated(int)), this, SLOT(handleGradientExtend(int))); // replaced by gradEdit Signal for Gradient Extend Setting
//	connect(GradientExtendS  , SIGNAL(activated(int)), this, SLOT(handleStrokeGradientExtend(int))); // replaced by gradEdit Signal for Gradient Extend Setting
}

void ColorPickerModeEditor::disconnectSignals()
{
	disconnect(CGradDia, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)), this, SIGNAL(NewSpecial(double, double, double, double, double, double, double, double, double, double)));
	disconnect(CGradDia, SIGNAL(paletteShown(bool)), this, SLOT(setActiveGradDia(bool)));
	disconnect(CGradDia, SIGNAL(editGradient(int)), this, SIGNAL(editGradient(int)));
	disconnect(CGradDia, SIGNAL(createNewMesh()), this, SLOT(createNewMeshGradient()));
	disconnect(CGradDia, SIGNAL(resetMesh()), this, SLOT(resetMeshGradient()));
	disconnect(CGradDia, SIGNAL(meshToShape()), this, SLOT(meshGradientToShape()));
//	disconnect(CGradDia, SIGNAL(reset1Control()), this, SLOT(resetOneControlPoint()));
//	disconnect(CGradDia, SIGNAL(resetAllControl()), this, SLOT(resetAllControlPoints()));
	disconnect(CGradDia, SIGNAL(removePatch()), this, SLOT(handleRemovePatch()));
	disconnect(CGradDia, SIGNAL(snapToMGrid(bool)), this, SLOT(snapToPatchGrid(bool)));
	disconnect(colorPoint1    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	disconnect(colorPoint2    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	disconnect(colorPoint3    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	disconnect(colorPoint4    , SIGNAL(activated(int)), this, SLOT(setGradientColors()));
	disconnect(color1Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color2Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color3Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color4Alpha    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color1Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color2Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color3Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
	disconnect(color4Shade    , SIGNAL(valueChanged(double)), this, SLOT(setGradientColors()));
//	disconnect(colorListFill  , SIGNAL(currentRowChanged(int)), this, SLOT(selectColorF(int)));
//	disconnect(colorListStroke, SIGNAL(currentRowChanged(int)), this, SLOT(selectColorS(int)));
	disconnect(editMeshColors , SIGNAL(clicked()), this, SLOT(editMeshPointColor()));
//	disconnect(editPatternProps      , SIGNAL(clicked()) , this, SLOT(changePatternProps()));
//	disconnect(editPatternPropsStroke, SIGNAL(clicked()), this, SLOT(changePatternPropsStroke()));
//	disconnect(fillShade     , SIGNAL(valueChanged(double)), this, SIGNAL(NewBrushShade(double)));
//	disconnect(followsPath   , SIGNAL(clicked()), this, SLOT(toggleStrokePattern()));
//	disconnect(gradientTypeStroke , SIGNAL(activated(int)), this, SLOT(slotGradTypeStroke(int)));
	disconnect(gradEdit      , SIGNAL(gradientChanged()), this, SLOT(handleFillGradient()));
	disconnect(gradientName  , SIGNAL(textChanged(QString)), this, SLOT(handleFillGradient()));
	disconnect(gradEditButton, SIGNAL(clicked()), this, SLOT(editGradientVector()));
//	disconnect(gradEditButtonStroke, SIGNAL(clicked()), this, SLOT(editGradientVectorStroke()));
//	disconnect(gradEditStroke, SIGNAL(gradientChanged()), this, SLOT(handleStrokeGradient()));
	disconnect(gradientType  , SIGNAL(activated(int)), this, SLOT(slotGradType(int)));
//	disconnect(gradientTypeStroke , SIGNAL(activated(int)), this, SLOT(slotGradTypeStroke(int)));
//	disconnect(namedGradient , SIGNAL(activated(const QString &)), this, SLOT(setNamedGradient(const QString &)));
//	disconnect(namedGradientStroke, SIGNAL(activated(const QString &)), this, SLOT(setNamedGradientStroke(const QString &)));
//	disconnect(overPrintCombo     , SIGNAL(activated(int)), this, SIGNAL(NewOverprint(int)));
//	disconnect(patternBox         , SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPattern(QListWidgetItem*)));
//	disconnect(patternBoxStroke   , SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectPatternS(QListWidgetItem*)));

//	disconnect(strokeModeCombo    , SIGNAL(currentIndexChanged(int)), this, SLOT(slotGradStroke(int)));
//	disconnect(strokeShade    , SIGNAL(valueChanged(double)), this, SIGNAL(NewPenShade(double)));
//	disconnect(tabFillStroke      , SIGNAL(currentChanged(int)), this, SLOT(fillStrokeSelector(int)));
	disconnect(colorMeshPoint , SIGNAL(activated(int)), this, SLOT(updateMeshPoint()));
	disconnect(shadeMeshPoint, SIGNAL(valueChanged(double)), this, SLOT(updateMeshPoint()));
	disconnect(transparencyMeshPoint, SIGNAL(valueChanged(double)), this, SLOT(updateMeshPoint()));
	disconnect(hatchAngle, SIGNAL(valueChanged(double)), this, SLOT(changeHatchProps()));
	disconnect(hatchDist, SIGNAL(valueChanged(double)), this, SLOT(changeHatchProps()));
	disconnect(hatchType, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	disconnect(hatchBackground, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));
	disconnect(hatchLineColor, SIGNAL(activated(int)), this, SLOT(changeHatchProps()));


	disconnect(gradEdit  , SIGNAL(emitGradientExtend(int)), this, SLOT(handleGradientExtend(int)));
//	disconnect(gradientExtend  , SIGNAL(activated(int)), this, SLOT(handleGradientExtend(int)));
//	disconnect(GradientExtendS  , SIGNAL(activated(int)), this, SLOT(handleStrokeGradientExtend(int)));
}

bool ColorPickerModeEditor::eventFilter(QObject *object, QEvent *event){
	Q_UNUSED(object)

	switch(colorPaintMode){
	case ColorPaintMode::Gradient:{
		if (event->type() == QEvent::FocusOut) {

			if (gradientName->text().isEmpty())
			{
				ScMessageBox::information(this, CommonStrings::trWarning, tr("You cannot create a gradient without a name.\nPlease give it a name"));
				gradientName->setText(tr("New Gradient"));
				gradientName->setFocus();
				gradientName->selectAll();
			}else handleFillGradient();
		}
		return false;
	break;
	}
	default:
		return false;
	break;
	}


}

/*********************************************************************
*
* Doc
*
**********************************************************************/
void ColorPickerModeEditor::setDoc(ScribusDoc* d)
{

	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		disconnect(this, SIGNAL(NewPen(QString)), 0, 0);
		disconnect(this, SIGNAL(NewBrush(QString)), 0, 0);
//		disconnect(this, SIGNAL(NewPenShade(double)), 0, 0);
//		disconnect(this, SIGNAL(NewBrushShade(double)), 0, 0);
		disconnect(this, SIGNAL(NewGradient(int)), 0, 0);
//		disconnect(this, SIGNAL(NewGradientS(int)), 0, 0);
		disconnect(this, SIGNAL(NewPattern(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), 0, 0);
		disconnect(this, SIGNAL(NewOverprint(int)), 0, 0);
//		disconnect(this, SIGNAL(NewPatternS(QString)), 0, 0);
//		disconnect(this, SIGNAL(NewPatternTypeS(bool)), 0, 0);
//		disconnect(this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), 0, 0);
//		disconnect(displayAllColors, SIGNAL(clicked()), this, SLOT(toggleColorDisplay()));


		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
		disconnect(m_doc->scMW(), SIGNAL(UpdateRequest(int)), this, 0);
	}

	ScribusDoc* oldDoc = m_doc;
	m_doc = d;
	m_item = NULL;

	if (!m_doc)
	{
//		colorListStroke->cList = NULL;
//		colorListFill->cList = NULL;
		disconnectSignals();
		return;
	}

	m_unitRatio   = m_doc->unitRatio();
	m_unitIndex   = m_doc->unitIndex();

//	colorListStroke->cList = &m_doc->PageColors;
//	colorListFill->cList = &m_doc->PageColors;
	gradEdit->setColors(m_doc->PageColors);

	m_haveDoc = true;
	m_haveItem = false;

//	updateColorList();
//	updateCList();

	connect(this, SIGNAL(NewPen(QString))      , m_doc, SLOT(itemSelection_SetItemPen(QString)));
	connect(this, SIGNAL(NewBrush(QString))    , m_doc, SLOT(itemSelection_SetItemBrush(QString)));
//	connect(this, SIGNAL(NewPenShade(double))     , this, SLOT(handleStrokeShade(double)));
//	connect(this, SIGNAL(NewBrushShade(double))   , this, SLOT(handleFillShade(double)));
	connect(this, SIGNAL(NewGradient(int))     , m_doc, SLOT(itemSelection_SetItemGradFill(int)));
//	connect(this, SIGNAL(NewGradientS(int))    , m_doc, SLOT(itemSelection_SetItemGradStroke(int)));
	connect(this, SIGNAL(NewPattern(QString))  , m_doc, SLOT(itemSelection_SetItemPatternFill(QString)));
	connect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), m_doc, SLOT(itemSelection_SetItemPatternProps(double, double, double, double, double, double, double, bool, bool)));
	connect(this, SIGNAL(NewOverprint(int))    , this, SLOT(handleOverprint(int)));
//	connect(this, SIGNAL(NewPatternS(QString)) , m_doc, SLOT(itemSelection_SetItemStrokePattern(QString)));
//	connect(this, SIGNAL(NewPatternTypeS(bool)), m_doc, SLOT(itemSelection_SetItemStrokePatternType(bool)));
//	connect(this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), m_doc, SLOT(itemSelection_SetItemStrokePatternProps(double, double, double, double, double, double, double, double, bool, bool)));
//	connect(displayAllColors, SIGNAL(clicked()), this, SLOT(toggleColorDisplay()));

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	connect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));

	if (oldDoc != m_doc)
	{
		//showGradient(0);
	}

	// Handle properties update when switching document
	handleSelectionChanged();
}

void ColorPickerModeEditor::unsetDoc()
{

	if (m_doc)
	{
		disconnect(this, SIGNAL(NewPen(QString)), 0, 0);
		disconnect(this, SIGNAL(NewBrush(QString)), 0, 0);
//		disconnect(this, SIGNAL(NewPenShade(double)), 0, 0);
//		disconnect(this, SIGNAL(NewBrushShade(double)), 0, 0);
		disconnect(this, SIGNAL(NewGradient(int)), 0, 0);
//		disconnect(this, SIGNAL(NewGradientS(int)), 0, 0);
		disconnect(this, SIGNAL(NewPattern(QString)), 0, 0);
		disconnect(this, SIGNAL(NewPatternProps(double, double, double, double, double, double, double, bool, bool)), 0, 0);
		disconnect(this, SIGNAL(NewOverprint(int)), 0, 0);
//		disconnect(this, SIGNAL(NewPatternS(QString)), 0, 0);
//		disconnect(this, SIGNAL(NewPatternTypeS(bool)), 0, 0);
//		disconnect(this, SIGNAL(NewPatternPropsS(double, double, double, double, double, double, double, double, bool, bool)), 0, 0);
//		disconnect(displayAllColors, SIGNAL(clicked()), this, SLOT(toggleColorDisplay()));

		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	}

	m_haveDoc = false;
	m_haveItem = false;
	m_doc=NULL;
	m_item = NULL;

	setEnabled(false);

}


/*********************************************************************
*
* Item
*
**********************************************************************/

void ColorPickerModeEditor::setCurrentItem(PageItem* i)
{

	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (!m_doc)
		setDoc(i->doc());

	if ((i == NULL) || (m_item != i))
	{
		editStrokeGradient = 0;
		CGradDia->hide();
		editMeshColors->setEnabled(true);
		gradEditButton->setEnabled(true);
	}

	m_item = i;
	disconnectSignals();

	if (!m_item)
		return;

	m_haveItem = true;

//	showOverprint(m_item->doOverprint ? 1 : 0);
//	showColorValues(m_item->lineColor(), m_item->fillColor(), m_item->lineShade(), m_item->fillShade());
//	showGradient(m_item->gradientType());
//	showGradientStroke(m_item->strokeGradientType());
	gradEdit->setGradient(m_item->fill_gradient);
//	gradEditStroke->setGradient(m_item->stroke_gradient);
	if (!m_item->gradient().isEmpty())
	{
//		setCurrentComboItem(namedGradient, m_item->gradient());
		gradEdit->setGradientEditable(false);
	}
	else
	{
		namedGradient->setCurrentIndex(0);
		gradEdit->setGradientEditable(true);
	}
	if (!m_item->strokeGradient().isEmpty())
	{
//		setCurrentComboItem(namedGradientStroke, m_item->strokeGradient());
//		gradEditStroke->setGradientEditable(false);
	}
	else
	{
//		namedGradientStroke->setCurrentIndex(0);
//		gradEditStroke->setGradientEditable(true);
	}
	if (m_item->GrTypeStroke > 0)
	{
//		if (m_item->GrTypeStroke == 6)
//			gradientTypeStroke->setCurrentIndex(0);
//		else
//			gradientTypeStroke->setCurrentIndex(1);
	}

//	enablePatterns(patternList->count() != 0);

//	if (m_item->gradientType() == 12)
//		setMeshPatchPoint();
//	else
//		setMeshPoint();
	if(CGradDia && gradEditButton->isChecked())
	{

		setGradientVectorValues(); // For Fills and Strokes
//		if(tabFillStroke->currentIndex() == 0)
//			setGradientVectorValues();
//		else
//			setGradientVectorStrokeValues();
	}
	editMeshColors->setEnabled(!CGradDia->isVisible());
	gradEditButton->setEnabled(!editMeshColors->isChecked());
	gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
//	fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));

	double patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, patternSpace;
	bool mirrorX, mirrorY;
	m_item->patternTransform(patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY);
	m_item->patternFlip(mirrorX, mirrorY);
//	setActPattern(m_item->pattern(), patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, mirrorX, mirrorY);
	m_item->strokePatternTransform(patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, patternSpace);
	m_item->strokePatternFlip(mirrorX, mirrorY);
//	setActPatternStroke(m_item->strokePattern(), patternScaleX, patternScaleY, patternOffsetX, patternOffsetY, patternRotation, patternSkewX, patternSkewY, mirrorX, mirrorY, patternSpace, m_item->isStrokePatternToPath());



	connectSignals();

}

void ColorPickerModeEditor::updateFromItem()
{
	setCurrentItem(m_item);
}



PageItem* ColorPickerModeEditor::currentItemFromSelection()
{
	PageItem *currentItem = NULL;
	if (m_doc)
		currentItem = m_doc->m_Selection->itemAt(0);
	return currentItem;
}


void ColorPickerModeEditor::unsetItem()
{
	m_haveItem = false;
	m_item     = NULL;

	handleSelectionChanged();
}

/*********************************************************************
*
* Update Helper
*
**********************************************************************/

void ColorPickerModeEditor::handleSelectionChanged()
{
	if (m_doc && !updatesBlocked())
	{
		PageItem* currItem = currentItemFromSelection();
		setCurrentItem(currItem);

		int itemType = currItem ? (int) currItem->itemType() : -1;
		m_haveItem   = (itemType != -1);

		switch (itemType)
		{
		case -1:
			m_haveItem = false;

			//this->showGradient(0);
			break;
		}
	}
}

void ColorPickerModeEditor::handleUpdateRequest(int updateFlags)
{
//	if (updateFlags & reqColorsUpdate)
//		updateColorList();
}

void ColorPickerModeEditor::unitChange()
{
	if (!m_haveDoc || !m_doc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();

	if (CGradDia)
		CGradDia->unitChange(m_unitIndex);
	hatchDist->setNewUnit(m_unitIndex);

	m_haveItem = tmp;
}


void ColorPickerModeEditor::languageChange()
{
	// Needed to avoid issues if an item is selected and patterns are available
	if (m_item)
		disconnectSignals();

	// Save fill tab state
	int oldGradientTypeIndex = gradientType->currentIndex();
//	int oldGradientExtendIndex = gradientExtend->currentIndex();
	int oldHatchTypeIndex = hatchType->currentIndex();

//	// Save stroke tab state
//	int oldStrokeModeComboIndex = strokeModeCombo->currentIndex();
//	int oldGradientTypeStrokeIndex = gradientTypeStroke->currentIndex();
//	int oldGradientExtendSIndex = GradientExtendS->currentIndex();

//	// Save properties outside of tabs
//	int oldOverPrintComboIndex = overPrintCombo->currentIndex();

	// Retranslate UI
	retranslateUi(this);


//	strokeModeCombo->clear();
//	strokeModeCombo->addItem( tr("Solid"));
//	strokeModeCombo->addItem( tr("Gradient"));

//	if (m_doc)
//		enablePatterns(patternList->count() != 0);

	// Restore properties
	gradientType->setCurrentIndex(oldGradientTypeIndex);
//	gradientExtend->setCurrentIndex(oldGradientExtendIndex);
	hatchType->setCurrentIndex(oldHatchTypeIndex);

//	strokeModeCombo->setCurrentIndex(oldStrokeModeComboIndex);
//	gradientTypeStroke->setCurrentIndex(oldGradientTypeStrokeIndex);
//	GradientExtendS->setCurrentIndex(oldGradientExtendSIndex);

//	overPrintCombo->setCurrentIndex(oldOverPrintComboIndex);

	// Reconnect signals if necessary
	if (m_item)
		connectSignals();
}

/*********************************************************************
*
* Slots
*
**********************************************************************/

void ColorPickerModeEditor::updateSizes(int index)
{
	for(int i=0;i<this->fillModeStack->count();i++)
		if(i!=index)
			this->fillModeStack->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	this->fillModeStack->widget(index)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	this->fillModeStack->widget(index)->resize(this->fillModeStack->widget(index)->minimumSizeHint());
	this->fillModeStack->widget(index)->adjustSize();
	resize(QSize(minimumSizeHint()));
	adjustSize();
}

void ColorPickerModeEditor::updateSizesGradient(int index)
{
	for(int i=0;i<this->stackedWidget_2->count();i++)
		if(i!=index)
			this->stackedWidget_2->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	this->stackedWidget_2->widget(index)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	this->stackedWidget_2->widget(index)->resize(this->stackedWidget_2->widget(index)->minimumSizeHint());
	this->stackedWidget_2->widget(index)->adjustSize();
	resize(QSize(minimumSizeHint()));
	adjustSize();
}

void ColorPickerModeEditor::NewSpGradient(double x1, double y1, double x2, double y2, double fx, double fy, double sg, double sk, double cx, double cy)
{
	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		QRectF upRect;
		UndoTransaction trans;
		if (UndoManager::undoEnabled())
			trans = undoManager->beginTransaction(Um::Selection, Um::ILine, Um::GradPos + "p", "", Um::ILine);

		switch(m_ScMW->view->editStrokeGradient){
		case 1:{

			m_item->setGradientStrokeStartX(x1 / m_unitRatio);
			m_item->setGradientStrokeStartY(y1 / m_unitRatio);
			m_item->setGradientStrokeEndX(x2 / m_unitRatio);
			m_item->setGradientStrokeEndY(y2 / m_unitRatio);
			m_item->setGradientStrokeFocalX(fx / m_unitRatio);
			m_item->setGradientStrokeFocalY(fy / m_unitRatio);
			m_item->setGradientStrokeScale(sg);
			m_item->setGradientStrokeSkew(sk);
			if (m_item->strokeGradientType() == 6)
			{
				m_item->setGradientStrokeFocalX(m_item->gradientStrokeStartX());
				m_item->setGradientStrokeFocalY(m_item->gradientStrokeStartY());
			}
			m_item->update();
			upRect = QRectF(QPointF(m_item->gradientStrokeStartX(), m_item->gradientStrokeStartY()), QPointF(m_item->gradientStrokeEndX(), m_item->gradientStrokeEndY()));
			double radEnd = distance(m_item->gradientStrokeEndX() - m_item->gradientStrokeStartX(), m_item->gradientStrokeEndY() - m_item->gradientStrokeStartY());
			double rotEnd = xy2Deg(m_item->gradientStrokeEndX() - m_item->gradientStrokeStartX(), m_item->gradientStrokeEndY() - m_item->gradientStrokeStartY());
			QTransform m;
			m.translate(m_item->gradientStrokeStartX(), m_item->gradientStrokeStartY());
			m.rotate(rotEnd);
			m.rotate(-90);
			m.rotate(m_item->gradientStrokeSkew());
			m.translate(radEnd * m_item->gradientStrokeScale(), 0);
			QPointF shP = m.map(QPointF(0,0));
			upRect = upRect.united(QRectF(shP, QPointF(m_item->gradientStrokeEndX(), m_item->gradientStrokeEndY())).normalized());
			upRect = upRect.united(QRectF(shP, QPointF(m_item->gradientStrokeStartX(), m_item->gradientStrokeStartY())).normalized());
			upRect |= QRectF(shP, QPointF(0, 0)).normalized();
			upRect |= QRectF(shP, QPointF(m_item->width(), m_item->height())).normalized();

			break;
		}
		case 3:
			m_item->setGradientControl1(FPoint(x1 / m_unitRatio, y1 / m_unitRatio));
			m_item->setGradientControl2(FPoint(x2 / m_unitRatio, y2 / m_unitRatio));
			m_item->setGradientControl3(FPoint(fx / m_unitRatio, fy / m_unitRatio));
			m_item->setGradientControl4(FPoint(sg / m_unitRatio, sk / m_unitRatio));
			m_item->update();
			upRect = QRectF(QPointF(-m_item->width(), -m_item->height()), QPointF(m_item->width() * 2, m_item->height() * 2)).normalized();

			break;
		case 4:
			m_item->setGradientControl1(FPoint(x1 / m_unitRatio, y1 / m_unitRatio));
			m_item->setGradientControl2(FPoint(x2 / m_unitRatio, y2 / m_unitRatio));
			m_item->setGradientControl3(FPoint(fx / m_unitRatio, fy / m_unitRatio));
			m_item->setGradientControl4(FPoint(sg / m_unitRatio, sk / m_unitRatio));
			m_item->setGradientControl5(FPoint(cx / m_unitRatio, cy / m_unitRatio));
			m_item->update();
			upRect = QRectF(QPointF(-m_item->width(), -m_item->height()), QPointF(m_item->width() * 2, m_item->height() * 2)).normalized();
			break;
		default:{
			if (m_item->gradientType() == 13 && UndoManager::undoEnabled())
			{
				SimpleState *ss= new SimpleState("Refresh");
				ss->set("UNDO_UPDATE_CONICAL");
				undoManager->action(m_item,ss);
			}
			m_item->setGradientStartX(x1 / m_unitRatio);
			m_item->setGradientStartY(y1 / m_unitRatio);
			m_item->setGradientEndX(x2 / m_unitRatio);
			m_item->setGradientEndY(y2 / m_unitRatio);
			m_item->setGradientFocalX(fx / m_unitRatio);
			m_item->setGradientFocalY(fy / m_unitRatio);
			m_item->setGradientScale(sg);
			m_item->setGradientSkew(sk);
			if (m_item->strokeGradientType() == 6)
			{
				m_item->setGradientFocalX(m_item->gradientStartX());
				m_item->setGradientFocalY(m_item->gradientStartY());
			}
			if (m_item->gradientType() == 13 && UndoManager::undoEnabled())
			{
				m_item->createConicalMesh();
				SimpleState *ss= new SimpleState("Refresh");
				ss->set("REDO_UPDATE_CONICAL");
				undoManager->action(m_item,ss);
			}
			m_item->update();
			upRect = QRectF(QPointF(m_item->gradientStartX(), m_item->gradientStartY()), QPointF(m_item->gradientEndX(), m_item->gradientEndY()));
			double radEnd = distance(m_item->gradientEndX() - m_item->gradientStartX(), m_item->gradientEndY() - m_item->gradientStartY());
			double rotEnd = xy2Deg(m_item->gradientEndX() - m_item->gradientStartX(), m_item->gradientEndY() - m_item->gradientStartY());
			QTransform m;
			m.translate(m_item->gradientStartX(), m_item->gradientStartY());
			m.rotate(rotEnd);
			m.rotate(-90);
			m.rotate(m_item->gradientSkew());
			m.translate(radEnd * m_item->gradientScale(), 0);
			QPointF shP = m.map(QPointF(0,0));
			upRect |= QRectF(shP, QPointF(m_item->gradientEndX(), m_item->gradientEndY())).normalized();
			upRect |= QRectF(shP, QPointF(m_item->gradientStartX(), m_item->gradientStartY())).normalized();
			upRect |= QRectF(shP, QPointF(0, 0)).normalized();
			upRect |= QRectF(shP, QPointF(m_item->width(), m_item->height())).normalized();

			break;
			}
		}

		if (trans)
			trans.commit();
		upRect.translate(m_item->xPos(), m_item->yPos());
		m_doc->regionsChanged()->update(upRect.adjusted(-10.0, -10.0, 10.0, 10.0));
		m_doc->changed();
	}
}


/*********************************************************************
*
* Features
*
**********************************************************************/

void ColorPickerModeEditor::setColorPaintMode(ColorPaintMode number)
{
	this->setVisible(false);

	colorPaintMode = number;

	switch(colorPaintMode){
	case ColorPaintMode::Gradient:{

		fillModeStack->setCurrentIndex( 0 );
		this->setVisible(true);

		bool sigBlocked1 = gradEdit->blockSignals(true);
		bool sigBlocked2 = gradientType->blockSignals(true);
		bool sigBlocked3 = namedGradient->blockSignals(true);
		if (!m_item->gradient().isEmpty())
		{
			setCurrentComboItem(namedGradient, m_item->gradient());
			//gradEdit->setGradient(gradientList->value(m_item->gradient()));
			gradEdit->setGradientEditable(false);
		}
		else
		{
			namedGradient->setCurrentIndex(0);
			gradEdit->setGradient(m_item->fill_gradient);
			gradEdit->setGradientEditable(true);
		}

		switch(gradientType->currentIndex()){
		case 0:
			stackedWidget_2->setCurrentIndex(0);
			emit NewGradient(6);
			break;
		case 1:
			stackedWidget_2->setCurrentIndex(0);
			emit NewGradient(7);
			break;
		case 2:
			stackedWidget_2->setCurrentIndex(0);
			emit NewGradient(13);
			break;
		case 3:{
			stackedWidget_2->setCurrentIndex(1);
			if ((m_item->GrColorP1 != CommonStrings::None) && (!m_item->GrColorP1.isEmpty()))
				setCurrentComboItem(colorPoint1, m_item->GrColorP1);
			else
				colorPoint1->setCurrentIndex(0);
			if ((m_item->GrColorP2 != CommonStrings::None) && (!m_item->GrColorP2.isEmpty()))
				setCurrentComboItem(colorPoint2, m_item->GrColorP2);
			else
				colorPoint2->setCurrentIndex(0);
			if ((m_item->GrColorP3 != CommonStrings::None) && (!m_item->GrColorP3.isEmpty()))
				setCurrentComboItem(colorPoint3, m_item->GrColorP3);
			else
				colorPoint3->setCurrentIndex(0);
			if ((m_item->GrColorP4 != CommonStrings::None) && (!m_item->GrColorP4.isEmpty()))
				setCurrentComboItem(colorPoint4, m_item->GrColorP4);
			else
				colorPoint4->setCurrentIndex(0);
			color1Alpha->setValue(qRound(m_item->GrCol1transp * 100));
			color2Alpha->setValue(qRound(m_item->GrCol2transp * 100));
			color3Alpha->setValue(qRound(m_item->GrCol3transp * 100));
			color4Alpha->setValue(qRound(m_item->GrCol4transp * 100));
			color1Shade->setValue(m_item->GrCol1Shade);
			color2Shade->setValue(m_item->GrCol2Shade);
			color3Shade->setValue(m_item->GrCol3Shade);
			color4Shade->setValue(m_item->GrCol4Shade);
			emit NewGradient(9);
			break;
		}
		case 4:
			stackedWidget_2->setCurrentIndex(0);
			emit NewGradient(10);
			break;
		case 5:{
			stackedWidget_2->setCurrentIndex(2);
			if ((m_item->selectedMeshPointX > -1) && (m_item->selectedMeshPointY > -1l))
			{
				meshPoint mp = m_item->meshGradientArray[m_item->selectedMeshPointX][m_item->selectedMeshPointY];
				setCurrentComboItem(colorMeshPoint, mp.colorName);
				shadeMeshPoint->setValue(mp.shade);
				transparencyMeshPoint->setValue(mp.transparency * 100);
			}
			emit NewGradient(11);
			break;
		}
		case 6:
			stackedWidget_2->setCurrentIndex(2);
			emit NewGradient(12);
			break;

		}

		gradEdit->blockSignals(sigBlocked1);
		gradientType->blockSignals(sigBlocked2);
		namedGradient->blockSignals(sigBlocked3);

		break;
	}
	case ColorPaintMode::Hatch:{
		fillModeStack->setCurrentIndex( 1 );
		this->setVisible(true);

		hatchAngle->setValue(m_item->hatchAngle);
		hatchDist->setValue(m_item->hatchDistance * unitGetRatioFromIndex(m_unitIndex));
		hatchType->setCurrentIndex(m_item->hatchType);
		if ((m_item->hatchForeground != CommonStrings::None) && (!m_item->hatchForeground.isEmpty()))
			setCurrentComboItem(hatchLineColor, m_item->hatchForeground);
		if (m_item->hatchUseBackground)
		{
			if (!m_item->hatchBackground.isEmpty())
				setCurrentComboItem(hatchBackground, m_item->hatchBackground);
		}
		else
			setCurrentComboItem(hatchBackground, CommonStrings::None);
		emit NewGradient(14);
		break;
	}
	case ColorPaintMode::Pattern:
		fillModeStack->setCurrentIndex( 2 );
		this->setVisible(true);

		emit NewGradient(8);
		break;
	case ColorPaintMode::Solid:
	default:
		fillModeStack->setCurrentIndex( -1 );
		emit NewGradient(0);
		break;

	}

}

void ColorPickerModeEditor::setObjectPaintMode(ObjectPaintMode mode)
{
	objectPaintMode = mode;

}


void ColorPickerModeEditor::slotGradType(int type)
{

	GradientTypes types = static_cast<GradientTypes>(type);

	emit emitGradientType(types);

	switch(types){
	case GradientTypes::Linear:
		stackedWidget_2->setCurrentIndex(0);
		emit NewGradient(6);
		break;
	case GradientTypes::Radial:
		stackedWidget_2->setCurrentIndex(0);
		emit NewGradient(7);
		break;
	case GradientTypes::Conical:
		stackedWidget_2->setCurrentIndex(0);
		emit NewGradient(13);
		break;
	case GradientTypes::FourColors:{
		stackedWidget_2->setCurrentIndex(1);
		if ((m_item->GrColorP1 != CommonStrings::None) && (!m_item->GrColorP1.isEmpty()))
			setCurrentComboItem(colorPoint1, m_item->GrColorP1);
		else
			colorPoint1->setCurrentIndex(0);
		if ((m_item->GrColorP2 != CommonStrings::None) && (!m_item->GrColorP2.isEmpty()))
			setCurrentComboItem(colorPoint2, m_item->GrColorP2);
		else
			colorPoint2->setCurrentIndex(0);
		if ((m_item->GrColorP3 != CommonStrings::None) && (!m_item->GrColorP3.isEmpty()))
			setCurrentComboItem(colorPoint3, m_item->GrColorP3);
		else
			colorPoint3->setCurrentIndex(0);
		if ((m_item->GrColorP4 != CommonStrings::None) && (!m_item->GrColorP4.isEmpty()))
			setCurrentComboItem(colorPoint4, m_item->GrColorP4);
		else
			colorPoint4->setCurrentIndex(0);
		color1Alpha->setValue(qRound(m_item->GrCol1transp * 100));
		color2Alpha->setValue(qRound(m_item->GrCol2transp * 100));
		color3Alpha->setValue(qRound(m_item->GrCol3transp * 100));
		color4Alpha->setValue(qRound(m_item->GrCol4transp * 100));
		color1Shade->setValue(m_item->GrCol1Shade);
		color2Shade->setValue(m_item->GrCol2Shade);
		color3Shade->setValue(m_item->GrCol3Shade);
		color4Shade->setValue(m_item->GrCol4Shade);
		emit NewGradient(9);
		break;
	}
	case GradientTypes::Diamond:
		stackedWidget_2->setCurrentIndex(0);
		emit NewGradient(10);
		break;
	case GradientTypes::Mesh:{
		stackedWidget_2->setCurrentIndex(2);
		if ((m_item->selectedMeshPointX > -1) && (m_item->selectedMeshPointY > -1l))
		{
			meshPoint mp = m_item->meshGradientArray[m_item->selectedMeshPointX][m_item->selectedMeshPointY];
			setCurrentComboItem(colorMeshPoint, mp.colorName);
			shadeMeshPoint->setValue(mp.shade);
			transparencyMeshPoint->setValue(mp.transparency * 100);
		}
		emit NewGradient(11);
		break;
	}
	case GradientTypes::PatchMesh:
		stackedWidget_2->setCurrentIndex(2);
		emit NewGradient(12);
		break;

	}


}


void ColorPickerModeEditor::setGradientColors()
{
	QString color1 = colorPoint1->currentText();
	if (color1 == CommonStrings::tr_NoneColor)
		color1 = CommonStrings::None;
	QString color2 = colorPoint2->currentText();
	if (color2 == CommonStrings::tr_NoneColor)
		color2 = CommonStrings::None;
	QString color3 = colorPoint3->currentText();
	if (color3 == CommonStrings::tr_NoneColor)
		color3 = CommonStrings::None;
	QString color4 = colorPoint4->currentText();
	if (color4 == CommonStrings::tr_NoneColor)
		color4 = CommonStrings::None;
	double t1 = color1Alpha->value() / 100.0;
	double t2 = color2Alpha->value() / 100.0;
	double t3 = color3Alpha->value() / 100.0;
	double t4 = color4Alpha->value() / 100.0;
	UndoTransaction trans;
	if (UndoManager::undoEnabled())
		trans = undoManager->beginTransaction(Um::Selection,Um::IFill,Um::GradVal,"",Um::IFill);
	m_item->set4ColorShade(static_cast<int>(color1Shade->value()), static_cast<int>(color2Shade->value()), static_cast<int>(color3Shade->value()), static_cast<int>(color4Shade->value()));
	m_item->set4ColorTransparency(t1, t2, t3, t4);
	m_item->set4ColorColors(color1, color2, color3, color4);
	if (trans)
		trans.commit();
	m_item->update();
}


void ColorPickerModeEditor::toggleGradientEdit(int stroke)
{

//	enum GradientTypes {
//		Linear = 0,
//		Radial = 1,
//		Conical = 2,
//		FourColors = 3,
//		Diamond = 4,
//		Mesh = 5,
//		PatchMesh = 6
//	};

	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;
	if ((m_haveDoc) && (m_haveItem))
	{
		m_ScMW->view->editStrokeGradient = stroke;
		if (stroke == 1)
		{
			if (this->gradEditButton->isChecked()) // gradEditButtonStroke
				m_ScMW->view->requestMode(modeEditGradientVectors);
			else
				m_ScMW->view->requestMode(modeNormal);
		}
		else
		{
			if ((this->gradEditButton->isChecked()) || (this->editMeshColors->isChecked()))
			{
				if ((stroke == 5) || (stroke == 6) || (stroke == 7))
					m_ScMW->view->requestMode(modeEditMeshGradient);
				else if ((stroke == 8) || (stroke == 9) || (stroke == 10) || (stroke == 11))
					m_ScMW->view->requestMode(modeEditMeshPatch);
				else
					m_ScMW->view->requestMode(modeEditGradientVectors);
			}
			else
				m_ScMW->view->requestMode(modeNormal);
		}

		m_ScMW->view->RefreshGradient(m_item);
	}
}

/*********************************************************************
*
* Feature Mesh Gradient
*
**********************************************************************/

void ColorPickerModeEditor::editMeshPointColor()
{
	if (editMeshColors->isChecked())
	{
		if (m_item->gradientType() == 11)
			editStrokeGradient = 6;
		else if (m_item->gradientType() == 12)
			editStrokeGradient = 8;
		else
			editStrokeGradient = 0;
		gradEditButton->setEnabled(false);
	}
	else
	{
		editStrokeGradient = 0;
		gradEditButton->setEnabled(true);
	}
	gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
//	fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	emit editGradient(editStrokeGradient);
}


void ColorPickerModeEditor::editGradientVector()
{
	switch(objectPaintMode){
	case ObjectPaintMode::Fill:{

		if (gradEditButton->isChecked())
		{
			setGradientVectorValues();
			CGradDia->show();
		}
		else
		{
			CGradDia->hide();
			editMeshColors->setEnabled(true);
		}
		if (m_item->gradientType() == 9)
			editStrokeGradient = 3;
		else if (m_item->gradientType() == 10)
			editStrokeGradient = 4;
		else if (m_item->gradientType() == 11)
			editStrokeGradient = 5;
		else if (m_item->gradientType() == 12)
			editStrokeGradient = 9;
		else
			editStrokeGradient = 0;
		gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
		//fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
		emit editGradient(editStrokeGradient);

	}
		break;
	case ObjectPaintMode::Stroke:{

		if (gradEditButton->isChecked())
		{
			setGradientVectorValues(); // setGradientVectorStrokeValues();
			CGradDia->show();
		}
		else
		{
			CGradDia->hide();
		}
		editStrokeGradient = 1;
		gradientType->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
		//fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
		emit editGradient(editStrokeGradient);

	}
		break;
	}


}


void ColorPickerModeEditor::setGradientVectorValues()
{
	switch(objectPaintMode){
	case ObjectPaintMode::Fill:{

		if (gradEditButton->isChecked())
		{
			CGradDia->unitChange(m_doc->unitIndex());
			CGradDia->setValues(m_item->gradientStartX(), m_item->gradientStartY(), m_item->gradientEndX(), m_item->gradientEndY(), m_item->GrFocalX, m_item->GrFocalY, m_item->GrScale, m_item->GrSkew, 0, 0);
			if (m_item->gradientType() == 6)
				CGradDia->selectLinear();
			else if (m_item->gradientType() == 7)
				CGradDia->selectRadial();
			else if (m_item->gradientType() == 9)
			{
				CGradDia->setValues(m_item->GrControl1.x(), m_item->GrControl1.y(), m_item->GrControl2.x(), m_item->GrControl2.y(), m_item->GrControl3.x(), m_item->GrControl3.y(), m_item->GrControl4.x(), m_item->GrControl4.y(), 0, 0);
				CGradDia->selectFourColor();
			}
			else if (m_item->gradientType() == 10)
			{
				CGradDia->setValues(m_item->GrControl1.x(), m_item->GrControl1.y(), m_item->GrControl2.x(), m_item->GrControl2.y(), m_item->GrControl3.x(), m_item->GrControl3.y(), m_item->GrControl4.x(), m_item->GrControl4.y(), m_item->GrControl5.x(), m_item->GrControl5.y());
				CGradDia->selectDiamond();
			}
			else if (m_item->gradientType() == 11)
			{
				CGradDia->selectMesh();
				editMeshColors->setEnabled(false);
			}
			else if (m_item->gradientType() == 12)
			{
				CGradDia->selectPatchMesh();
				editMeshColors->setEnabled(false);
			}
			else if (m_item->gradientType() == 13)
				CGradDia->selectConical();
		}
	}
		break;
	case ObjectPaintMode::Stroke:{
		if (gradEditButton->isChecked()) // gradEditButtonStroke
		{
			CGradDia->unitChange(m_doc->unitIndex());
			CGradDia->setValues(m_item->GrStrokeStartX, m_item->GrStrokeStartY, m_item->GrStrokeEndX, m_item->GrStrokeEndY, m_item->GrStrokeFocalX, m_item->GrStrokeFocalY, m_item->GrStrokeScale, m_item->GrStrokeSkew, 0, 0);
			if (m_item->strokeGradientType() == 6)
				CGradDia->selectLinear();
			else
				CGradDia->selectRadial();
		}
	}
		break;
	}
}

void ColorPickerModeEditor::updateMeshPoint()
{
	QString color = colorMeshPoint->currentText();
	if (color == CommonStrings::tr_NoneColor)
		color = CommonStrings::None;
	double t = transparencyMeshPoint->value() / 100.0;
	m_item->setMeshPointColor(m_item->selectedMeshPointX, m_item->selectedMeshPointY, color, static_cast<int>(shadeMeshPoint->value()), t, m_doc->view()->editStrokeGradient == 8);
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}


/*********************************************************************
*
* Gradient Vector Dialog
*
**********************************************************************/

void ColorPickerModeEditor::setActiveGradDia(bool active)
{
	if (!active)
	{
//		if (editStrokeGradient == 1)
//			gradEditButtonStroke->setChecked(false);
//		else
//			gradEditButton->setChecked(false);

		gradEditButton->setChecked(false);
		emit editGradient(editStrokeGradient);
//		emit editGradient(objectPaintMode); // shall be controlled via objectPaintMode (Fill/Stroke)
		editMeshColors->setEnabled(true);
	}
}

void ColorPickerModeEditor::createNewMeshGradient()
{
	InsertTable* dia = new InsertTable(this, 255, 255);
	dia->setWindowTitle( tr( "Create Mesh" ) );
	if (dia->exec())
	{
		m_item->createGradientMesh(dia->Rows->value(), dia->Cols->value());
		m_item->update();
		m_doc->regionsChanged()->update(QRect());
	}
	delete dia;
}

void ColorPickerModeEditor::resetMeshGradient()
{
	m_item->resetGradientMesh();
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void ColorPickerModeEditor::meshGradientToShape()
{
	m_item->meshToShape();
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}

void ColorPickerModeEditor::handleRemovePatch()
{
	if ((m_item->selectedMeshPointX > -1) && (m_item->meshGradientPatches.count() > 1))
	{
		if(UndoManager::undoEnabled())
		{
			ScItemState<meshGradientPatch> *ss = new ScItemState<meshGradientPatch>(Um::RemoveMeshPatch,"",Um::ILine);
			ss->set("REMOVE_MESH_PATCH");
			ss->setItem(m_item->meshGradientPatches.takeAt(m_item->selectedMeshPointX));
			ss->set("POS", m_item->selectedMeshPointX);
			undoManager->action(m_item,ss);
		}
		m_item->selectedMeshPointX = -1;
		CGradDia->changebuttonRemovePatch((m_item->selectedMeshPointX > -1) && (m_item->meshGradientPatches.count() > 1));
		m_item->update();
		m_doc->regionsChanged()->update(QRect());
		editStrokeGradient = 9;
		emit editGradient(editStrokeGradient);
	}
}

void ColorPickerModeEditor::snapToPatchGrid(bool val)
{
	m_item->setSnapToPatchGrid(val);
}

/*********************************************************************
*
* Gradient
*
**********************************************************************/

void ColorPickerModeEditor::handleFillGradient()
{
	VGradient gradient(gradEdit->gradient());
	emit emitGradientUpdate(gradientName->text(), gradient);

	if (m_doc)
	{
		blockUpdates(true);
		m_doc->updateManager()->setUpdatesDisabled();
		m_doc->itemSelection_SetFillGradient(gradient);
		m_doc->updateManager()->setUpdatesEnabled();
		blockUpdates(false);
	}

}

void ColorPickerModeEditor::setFillGradient(const QString name, VGradient gradient)
{
	gradientName->setText(name);
	gradEdit->setGradient(gradient);

	handleFillGradient();

}

void ColorPickerModeEditor::handleGradientExtend(int val)
{
	if (m_doc)
	{
		if (val == 0)
			m_item->setGradientExtend(VGradient::none);
		else
			m_item->setGradientExtend(VGradient::pad);
		m_item->update();
		m_doc->regionsChanged()->update(QRect());
	}
}

/*********************************************************************
*
* Hatch
*
**********************************************************************/

void ColorPickerModeEditor::changeHatchProps()
{
	QString color1 = hatchLineColor->currentText();
	if (color1 == CommonStrings::tr_NoneColor)
		color1 = CommonStrings::None;
	QString color2 = hatchBackground->currentText();
	if (color2 == CommonStrings::tr_NoneColor)
		color2 = CommonStrings::None;
	bool useB = (color2 != CommonStrings::None);
	double angle = hatchAngle->value();
	double dist = hatchDist->value() / unitGetRatioFromIndex(m_unitIndex);
	m_item->setHatchParameters(hatchType->currentIndex(), dist, angle, useB, color2, color1);
	m_item->update();
	m_doc->regionsChanged()->update(QRect());
}
