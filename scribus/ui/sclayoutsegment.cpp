/*
 *
 * Layoutblock are used as breaking blocks in a floating layout
 *
 */

#include "sclayoutsegment.h"


ScLayoutSegment::ScLayoutSegment(QWidget *parent) : QWidget(parent)
{

	m_layout = new QVBoxLayout;

	this->setLayout(m_layout);
	//this->setAutoFillBackground(true);

}


//void ScLayoutSegment::addWidget(QWidget * item, int row, int column){


//	m_layout->addWidget(item, row, column);

//}

//void ScLayoutSegment::addGridLayout(QGridLayout *gridLayout){

//	m_layout = gridLayout;

//}


//QGridLayout * ScLayoutSegment::getLayout(){
//	return m_layout;
//}
