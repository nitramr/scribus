#include "colorpicker.h"
#include "scribus.h"
#include "selection.h"
#include "colorpickercolorlist.h"
#include "colorpickercolormixer.h"
#include "colorpickermodeeditor.h"


ColorPicker::ColorPicker(QWidget *parent) :
	QWidget(parent)
{
	m_ScMW = 0;
	m_item = 0;
	m_haveDoc   = false;
	m_haveItem  = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	patternList = NULL;

	setupUi(this);

	colorPaintMode = ColorPaintMode::Solid;
	objectPaintMode = ObjectPaintMode::Fill;
	gradientTypes = GradientTypes::Linear;

	fillModeCombo->addItem( tr("Solid") );
	fillModeCombo->addItem( tr("Gradient") );
	fillModeCombo->addItem( tr("Hatch") );

}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void ColorPicker::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

	colorPickerModeEditor->setMainWindow(mw);
	colorPickerColorList->setMainWindow(mw);

	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
}

void ColorPicker::connectSignals()
{

	connect(fillModeCombo , SIGNAL(currentIndexChanged(int)), this, SLOT(setColorPaintMode(int)));
	connect(colorPickerColorMixer->toggleColorList, SIGNAL(clicked(bool)), colorPickerColorList, SLOT(toggleColorList()) );
	connect(colorPickerColorMixer, SIGNAL(emitColor(ScColor)), colorPickerColorList, SLOT(setColor(ScColor)));
	connect(colorPickerModeEditor, SIGNAL(emitGradientType(GradientTypes)), this, SLOT(setGradientTypes(GradientTypes)));
	connect(colorPickerColorList, SIGNAL(emitColor(ScColor)), colorPickerColorMixer, SLOT(setObjectColor(ScColor)));


}

void ColorPicker::disconnectSignals()
{

	disconnect(fillModeCombo , SIGNAL(currentIndexChanged(int)), this, SLOT(setColorPaintMode(int)));
	disconnect(colorPickerColorMixer->toggleColorList, SIGNAL(clicked(bool)), colorPickerColorList, SLOT(toggleColorList()) );
	disconnect(colorPickerColorMixer, SIGNAL(emitColor(ScColor)), colorPickerColorList, SLOT(setColor(ScColor)));
	disconnect(colorPickerModeEditor, SIGNAL(emitGradientType(GradientTypes)), this, SLOT(setGradientTypes(GradientTypes)));
	disconnect(colorPickerColorList, SIGNAL(emitColor(ScColor)), colorPickerColorMixer, SLOT(setObjectColor(ScColor)));

}

/*********************************************************************
*
* Doc
*
**********************************************************************/
void ColorPicker::setDoc(ScribusDoc *d)
{
	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
		disconnect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
	}

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
//	gradEdit->setColors(m_doc->PageColors);
	m_unitIndex = m_doc->unitIndex();

	m_haveDoc = true;
	m_haveItem = false;


	colorPickerColorMixer->setDoc(m_doc);
	colorPickerModeEditor->setDoc(m_doc);
	colorPickerColorList->setDoc(m_doc);

	updateColorList();

	//setColor(ScColor(128,23,86));

	connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
	connect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));

	// Handle properties update when switching document
	handleSelectionChanged();

}

void ColorPicker::unsetDoc()
{

	if (m_doc)
	{

		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
		disconnect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
	}

	m_haveDoc = false;
	m_haveItem = false;
	m_doc=NULL;
	m_item = NULL;

	colorPickerModeEditor->unsetDoc();

	setEnabled(false);

}

/*********************************************************************
*
* Item
*
**********************************************************************/

void ColorPicker::setCurrentItem(PageItem* i)
{

	if (!m_ScMW || m_ScMW->scriptIsRunning())
		return;

	if (!m_doc)
		setDoc(i->doc());

	m_item = i;
	disconnectSignals();

	if (!m_item)
		return;

	m_haveItem = true;


//	fillModeCombo->setEnabled(!(gradEditButton->isChecked() || editMeshColors->isChecked()));
	enablePatterns(patternList->count() != 0);

	connectSignals();

}

void ColorPicker::updateFromItem()
{
	setCurrentItem(m_item);
}



PageItem* ColorPicker::currentItemFromSelection()
{
	PageItem *currentItem = NULL;
	if (m_doc)
		currentItem = m_doc->m_Selection->itemAt(0);
	return currentItem;
}


void ColorPicker::unsetItem()
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

void ColorPicker::handleSelectionChanged()
{
	if (m_doc/* && !updatesBlocked()*/)
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

void ColorPicker::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqColorsUpdate)
		updateColorList();
}

void ColorPicker::updateColorList()
{
	if (!m_doc)
		return;

	if (m_item)
		disconnectSignals();

	this->setColors(m_doc->PageColors);
	this->setGradients(&m_doc->docGradients);
	this->setPatterns(&m_doc->docPatterns);

	if (m_item)
		setCurrentItem(m_item);
}

void ColorPicker::unitChange()
{
	if (!m_haveDoc || !m_doc)
		return;
	bool tmp = m_haveItem;
	m_haveItem = false;

	m_unitRatio = m_doc->unitRatio();
	m_unitIndex = m_doc->unitIndex();





	m_haveItem = tmp;
}


void ColorPicker::languageChange()
{
	// Needed to avoid issues if an item is selected and patterns are available
	if (m_item)
		disconnectSignals();

	// Save fill tab state
	int oldFillModeComboIndex = fillModeCombo->currentIndex();

	// Retranslate UI
	retranslateUi(this);

	fillModeCombo->clear();
	fillModeCombo->addItem( tr("Solid"));
	fillModeCombo->addItem( tr("Gradient"));
	fillModeCombo->addItem( tr("Hatch"));

	if (m_doc)
		enablePatterns(patternList->count() != 0);

	// Restore properties
	fillModeCombo->setCurrentIndex(oldFillModeComboIndex);

	// Reconnect signals if necessary
	if (m_item)
		connectSignals();
}



/*********************************************************************
*
* Members
*
**********************************************************************/

void ColorPicker::setColor(ScColor color, QString name){

	Color = color;

	colorPickerColorMixer->setObjectColor(Color);
	colorPickerColorMixer->setColorName(name);

}


void ColorPicker::setPatterns(QHash<QString, ScPattern> *docPatterns)
{
	patternList = docPatterns;
	//updatePatternList();
}

void ColorPicker::setGradients(QHash<QString, VGradient> *docGradients)
{
	gradientList = docGradients;
	//updateGradientList();
}

void ColorPicker::setColors(ColorList newColorList)
{
	colorList.clear();
	colorList = newColorList;
	//updateCList();
}

/*********************************************************************
*
* Event handler
*
**********************************************************************/


void ColorPicker::setColorPaintMode(int index)
{

	switch(index){
	case 0:
	default:
		colorPaintMode = ColorPaintMode::Solid;
		break;
	case 1:
		colorPaintMode = ColorPaintMode::Gradient;
		break;
	case 2:
		colorPaintMode = ColorPaintMode::Hatch;
		break;
	case 3:
		colorPaintMode = ColorPaintMode::Pattern;
		break;
	}

	colorPickerModeEditor->setColorPaintMode(colorPaintMode);
	colorPickerColorMixer->setColorPaintMode(colorPaintMode, gradientTypes);
	colorPickerColorList->setColorPaintMode(colorPaintMode, gradientTypes);

}

void ColorPicker::setObjectPaintMode(int index){

	switch(index){
	case 0:
	default:
		objectPaintMode = ObjectPaintMode::Fill;
		break;
	case 1:
		objectPaintMode = ObjectPaintMode::Stroke;
		break;
	}

	colorPickerColorList->setObjectPaintMode(objectPaintMode);

}

void ColorPicker::setGradientTypes(GradientTypes type){

	gradientTypes = type;

	colorPickerColorMixer->setColorPaintMode(colorPaintMode, gradientTypes);
	colorPickerColorList->setColorPaintMode(colorPaintMode, gradientTypes);

}

void ColorPicker::enablePatterns(bool enable)
{
	if (enable)
	{
		if (fillModeCombo->count() < 4)
			fillModeCombo->addItem( tr("Pattern") );
//		if (strokeModeCombo->count() < 3)
//			strokeModeCombo->addItem( tr("Pattern") );
	}
	else
	{
		if (fillModeCombo->count() == 4)
			fillModeCombo->removeItem(3);
//		if (strokeModeCombo->count() == 3)
//			strokeModeCombo->removeItem(2);
	}
}
