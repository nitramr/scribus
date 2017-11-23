#include "colorpicker.h"


ColorPicker::ColorPicker(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);

	connect(colorPickerColorMixer->toggleColorList, SIGNAL(clicked(bool)), this, SLOT(toggleColorList()) );
}

/*********************************************************************
*
* Setup
*
**********************************************************************/



/*********************************************************************
*
* Doc
*
**********************************************************************/
void ColorPicker::setDoc(ScribusDoc *d)
{
	if((d == (ScribusDoc*) m_doc)/* || (m_ScMW && m_ScMW->scriptIsRunning())*/)
		return;

//	if (m_doc)
//	{
//		disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
//		disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
//	}

	m_doc = d;

	colorPickerColorMixer->setDoc(m_doc);

}





/*********************************************************************
*
* Event handler
*
**********************************************************************/

void ColorPicker::toggleColorList(){

	if(colorPickerColorList->isHidden()){
		colorPickerColorList->show();
	}else{
		colorPickerColorList->hide();
	}

}
