#include "colorpickercolorlist.h"

#include <QDomDocument>
#include <QImageReader>

#include "dcolor.h"
#include "fileloader.h"
#include "gradientaddedit.h"
#include "iconmanager.h"
#include "loadsaveplugin.h"
#include "plugins/formatidlist.h"
#include "prefsfile.h"
#include "prefsmanager.h"
#include "query.h"
#include "scclocale.h"
#include "scribus.h"
#include "scribusview.h"
#include "scribusXml.h"
#include "sctextstream.h"
#include "selection.h"
#include "symbolpalette.h"
#include "util_color.h"
#include "util_formats.h"


ColorPickerColorList::ColorPickerColorList(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);

	// setup
	m_ScMW = 0;
	m_item = 0;
	m_haveDoc   = false;
	m_haveItem  = false;
	m_unitIndex = 0;
	m_unitRatio = 1.0;

	sortRule = 0;
	modified = false;
	paletteLocked = false;
	colorPaintMode = ColorPaintMode::Solid;
	objectPaintMode = ObjectPaintMode::Fill;
	gradientTypes = GradientTypes::Linear;

	// Buttons
	newButton->setEnabled(false); // Add
	deleteButton->setEnabled(false); // Delete
	editButton->setEnabled(false); // Rename
	duplicateButton->setEnabled(false); // Duplicate
	importButton->setEnabled(false); // Import
	deleteUnusedButton->setEnabled(false); // Remove Unused

	// Lists
	dataTree->setContextMenuPolicy(Qt::CustomContextMenu);
	dataTree->setIconSize(QSize(60, 48));
	dataTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// Color Dummy
	Color = ScColor(0,0,0,0);
	Color.setDisplayName(tr("New Color"));

	// Gradient Dummy
	VGradient fill_gradient = VGradient(VGradient::linear);
	fill_gradient.clearStops();
	fill_gradient.addStop(QColor(Qt::black), 0.0, 0.5, 1.0, "Black", 100);
	fill_gradient.addStop(QColor(Qt::white), 1.0, 0.5, 1.0, "White", 100);
	m_gradient = fill_gradient;
	m_gradientName = tr("New Gradient");

	// connections
	connect(dataTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(itemSelected(QTreeWidgetItem*)));
	connect(dataTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(selEditColor(QTreeWidgetItem*)));
	connect(dataTree, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
	connect(dataTree, SIGNAL(customContextMenuRequested (const QPoint &)), this, SLOT(slotRightClick(QPoint)));

	connect(newButton, SIGNAL(clicked()), this, SLOT(createNew()));
	connect(editButton, SIGNAL(clicked()), this, SLOT(editColorItem()));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(removeColorItem()));
	connect(importButton, SIGNAL(clicked()), this, SLOT(importColorItems()));
	connect(deleteUnusedButton, SIGNAL(clicked()), this, SLOT(removeUnusedColorItem()));

}

/*********************************************************************
*
* Setup
*
**********************************************************************/

void ColorPickerColorList::setMainWindow(ScribusMainWindow* mw)
{
	m_ScMW = mw;

//	connect(this, SIGNAL(emitMainWindowUpdateColorList()), m_ScMW, SLOT(updateColorLists()));
//	connect(this, SIGNAL(emitMainWindowSlotDocCh(bool)), m_ScMW, SLOT(slotDocCh(bool)));
	connect(m_ScMW, SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
}

void ColorPickerColorList::init(){

	ColorList edc;
	QHash<QString, VGradient> *docGradients;
	QHash<QString, ScPattern> *docPatterns;
	ScribusDoc* tmpDoc;
	if (m_haveDoc)
	{
		docGradients = &m_doc->docGradients;
		edc = m_doc->PageColors;
		docPatterns = &m_doc->docPatterns;
		tmpDoc = m_doc;
	}else return;

	// reset
	dataTree->clear();

	newButton->setEnabled(true);
	deleteButton->setEnabled(false);
	editButton->setEnabled(false);
	duplicateButton->setEnabled(false);
	importButton->setEnabled(false);
	deleteUnusedButton->setEnabled(false);


	//	if (m_ScMW->HaveDoc)
	//	{
	//		//label->setText( tr("Merge Color Set"));
	//		m_doc->getUsedColors(inDocUsedColors);
	//		paletteLocked = false;
	//	}

	// Init Solid Colors
	m_colorList = edc;
	updateColorList();


	// Init Gradients
	for (QHash<QString, VGradient>::Iterator it = docGradients->begin(); it != docGradients->end(); ++it)
	{
		dialogGradients.insert(it.key(), it.value());
		origNames.insert(it.key(), it.key());
	}
	origGradients = docGradients->keys();
	updateGradientList();


	// Init Patterns
	for (QHash<QString, ScPattern>::Iterator it = docPatterns->begin(); it != docPatterns->end(); ++it)
	{
		dialogPatterns.insert(it.key(), it.value());
		origNamesPatterns.insert(it.key(), it.key());
	}
	origPatterns = docPatterns->keys();
	updatePatternList();



	//m_undoManager->setUndoEnabled(false);


}


/*********************************************************************
*
* Doc
*
**********************************************************************/
void ColorPickerColorList::setDoc(ScribusDoc *d)
{
	if((d == (ScribusDoc*) m_doc) || (m_ScMW && m_ScMW->scriptIsRunning()))
		return;

	if (m_doc)
	{
		//			disconnect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
		//			disconnect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
					disconnect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));
	}

	m_doc = d;
	m_item = NULL;

	if (!m_doc)
	{
		//disconnectSignals();
		return;
	}

	m_unitRatio   = m_doc->unitRatio();
	m_unitIndex   = m_doc->unitIndex();

	m_unitIndex = m_doc->unitIndex();

	m_haveDoc = true;
	m_haveItem = false;

	this->setColorPaintMode(colorPaintMode, gradientTypes);


	//		connect(m_doc->m_Selection, SIGNAL(selectionChanged()), this, SLOT(handleSelectionChanged()));
	//		connect(m_doc             , SIGNAL(docChanged())      , this, SLOT(handleSelectionChanged()));
			connect(m_doc->scMW()     , SIGNAL(UpdateRequest(int)), this, SLOT(handleUpdateRequest(int)));

	// Handle properties update when switching document
	//		handleSelectionChanged();

}




/*********************************************************************
*
* Content List
*
**********************************************************************/

QTreeWidgetItem* ColorPickerColorList::updatePatternList(QString addedName)
{
	QTreeWidgetItem* ret = 0;

	switch(colorPaintMode){
	case ColorPaintMode::Pattern:{

//		QList<QTreeWidgetItem*> lg = patternItems->takeChildren();
//		for (int a = 0; a < lg.count(); a++)
//		{
//			delete lg[a];
//		}

		dataTree->clear();

		QStringList patK = dialogPatterns.keys();
		qSort(patK);
		for (int a = 0; a < patK.count(); a++)
		{
			ScPattern sp = dialogPatterns.value(patK[a]);
			QPixmap pm;
			if (sp.getPattern()->width() >= sp.getPattern()->height())
				pm = QPixmap::fromImage(sp.getPattern()->scaledToWidth(48, Qt::SmoothTransformation));
			else
				pm = QPixmap::fromImage(sp.getPattern()->scaledToHeight(48, Qt::SmoothTransformation));
			QPixmap pm2(48, 48);
			pm2.fill(palette().color(QPalette::Base));
			QPainter p;
			p.begin(&pm2);
			p.drawPixmap(24 - pm.width() / 2, 24 - pm.height() / 2, pm);
			p.end();
			QTreeWidgetItem *item = new QTreeWidgetItem(dataTree /*patternItems*/);
			item->setText(0, patK[a]);
			if (patK[a] == addedName)
				ret = item;
			item->setIcon(0, pm2);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}

		break;
	}
	}


	return ret;
}

QTreeWidgetItem* ColorPickerColorList::updateGradientList(QString addedName)
{
	QTreeWidgetItem* ret = 0;

	switch(colorPaintMode){
	case ColorPaintMode::Gradient:{

//		QList<QTreeWidgetItem*> lg = gradientItems->takeChildren();
//		for (int a = 0; a < lg.count(); a++)
//		{
//			delete lg[a];
//		}

		dataTree->clear();

		QStringList patK = dialogGradients.keys();
		qSort(patK);
		for (int a = 0; a < patK.count(); a++)
		{
			VGradient gr = dialogGradients.value(patK[a]);
			QImage pixm(48, 12, QImage::Format_ARGB32);
			QPainter pb;
			QBrush b(QColor(205,205,205), IconManager::instance()->loadPixmap("testfill.png"));
			pb.begin(&pixm);
			pb.fillRect(0, 0, 48, 12, b);
			pb.end();
			ScPainter *p = new ScPainter(&pixm, 48, 12);
			p->setPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
			p->setFillMode(2);
			p->fill_gradient = gr;
			p->setGradient(VGradient::linear, FPoint(0,6), FPoint(48, 6), FPoint(0, 0), 1.0, 0.0);
			p->drawRect(0, 0, 48, 12);
			p->end();
			delete p;
			QPixmap pm;
			pm = QPixmap::fromImage(pixm);
			QTreeWidgetItem *item = new QTreeWidgetItem(dataTree/*gradientItems*/);
			item->setText(0, patK[a]);
			if (patK[a] == addedName)
				ret = item;
			item->setIcon(0, pm);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}

		break;
	}
	}


	return ret;
}

QTreeWidgetItem* ColorPickerColorList::updateColorList(QString addedName)
{
	QTreeWidgetItem* ret = 0;

	switch(colorPaintMode){
	case ColorPaintMode::Solid:{

//		QList<QTreeWidgetItem*> lg = colorItems->takeChildren();
//		for (int a = 0; a < lg.count(); a++)
//		{
//			delete lg[a];
//		}

		dataTree->clear();

		if (sortRule > 0)
		{
			QMap<QString, QString> sortMap;
			ColorList::Iterator it;
			for (it = m_colorList.begin(); it != m_colorList.end(); ++it)
			{
				if (sortRule == 1)
				{
					QColor c = it.value().getRawRGBColor();
					QString sortString = QString("%1-%2-%3-%4").arg(c.hue(), 3, 10, QChar('0')).arg(c.saturation(), 3, 10, QChar('0')).arg(c.value(), 3, 10, QChar('0')).arg(it.key());
					sortMap.insert(sortString, it.key());
				}
				else if (sortRule == 2)
				{
					QString sortString = QString("%1-%2");
					if (it.value().isRegistrationColor())
						sortMap.insert(sortString.arg("A", it.key()), it.key());
					else if (it.value().isSpotColor())
						sortMap.insert(sortString.arg("B", it.key()), it.key());
					else if (it.value().getColorModel() == colorModelCMYK)
						sortMap.insert(sortString.arg("C", it.key()), it.key());
					else
						sortMap.insert(sortString.arg("D", it.key()), it.key());
				}
			}
			QMap<QString, QString>::Iterator itc;
			for (itc = sortMap.begin(); itc != sortMap.end(); ++itc)
			{
				const ScColor& color = m_colorList[itc.value()];
				QTreeWidgetItem *item = new QTreeWidgetItem(dataTree /*colorItems*/);
				item->setText(0, itc.value());
				if (itc.value() == addedName)
					ret = item;
				QPixmap* pPixmap = getFancyPixmap(color, m_doc);
				item->setIcon(0, *pPixmap);
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				item->setData(0, Qt::ToolTipRole, getColorTooltip(color));
			}
		}
		else
		{
			ColorList::Iterator it;
			for (it = m_colorList.begin(); it != m_colorList.end(); ++it)
			{
				const ScColor& color = it.value();
				QTreeWidgetItem *item = new QTreeWidgetItem(dataTree /*colorItems*/);
				item->setText(0, it.key());
				if (it.key() == addedName)
					ret = item;
				QPixmap* pPixmap = getFancyPixmap(color, m_doc);
				item->setIcon(0, *pPixmap);
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				item->setData(0, Qt::ToolTipRole, getColorTooltip(color));
			}
		}

		break;
	}
	}


	return ret;
}

void ColorPickerColorList::updateDoc(){

// scribus.cpp -> manageColorsAndFills()


//	m_undoManager->setUndoEnabled(false);

		if (m_haveDoc)
		{
			//emit emitMainWindowSlotDocCh(false);
			m_ScMW->slotDocCh();

			m_doc->PageColors = m_colorList;
			if (replaceColorMap.isEmpty())
			{
				// invalidate all charstyles, as replaceNamedResources() won't do it if all maps are empty
				const StyleSet<CharStyle> dummy;
				m_doc->redefineCharStyles(dummy, false);
			}
			else
			{
				ResourceCollection colorrsc;
				colorrsc.mapColors(replaceColorMap);
				// Update tools colors
				PrefsManager::replaceToolColors(m_doc->itemToolPrefs(), colorrsc.colors());
				// Update objects and styles colors
				m_doc->replaceNamedResources(colorrsc);
				// Temporary code until LineStyle is effectively used
				m_doc->replaceLineStyleColors(replaceColorMap);
			}
			m_doc->recalculateColors();
			m_doc->recalcPicturesRes();
			m_doc->setGradients(dialogGradients);
			if (!replaceMap.isEmpty())
			{
				ResourceCollection gradrsc;
				gradrsc.mapPatterns(replaceMap);
				m_doc->replaceNamedResources(gradrsc);
			}
			m_doc->setPatterns(dialogPatterns);
			if (!replaceMapPatterns.isEmpty())
			{
				ResourceCollection colorrsc;
				colorrsc.mapPatterns(replaceMapPatterns);
				m_doc->replaceNamedResources(colorrsc);
			}
		//	symbolPalette->updateSymbolList();
			m_ScMW->symbolPalette->updateSymbolList();

			//emit emitMainWindowUpdateColorList();
			m_ScMW->updateColorLists();

			if (!m_doc->m_Selection->isEmpty())
				m_doc->m_Selection->itemAt(0)->emitAllToGUI();
		//	view->DrawNew();
			m_ScMW->view->DrawNew();
		}


//		else
//		{
//			// Update tools colors if needed
//			m_prefsManager->replaceToolColors(replaceColorMap);
//			m_prefsManager->setColorSet(m_colorList);
//			propertiesFramePalette->colorPal->setColors(m_prefsManager->colorSet());
//			m_prefsManager->appPrefs.defaultGradients = dialogGradients;
//			m_prefsManager->appPrefs.defaultPatterns = dialogPatterns;
//			QString Cpfad = QDir::toNativeSeparators(ScPaths::applicationDataDir())+"DefaultColors.xml";
//			const FileFormat *fmt = LoadSavePlugin::getFormatById(FORMATID_SLA150EXPORT);
//			if (fmt)
//			{
//				ScribusDoc *s_doc = new ScribusDoc();
//				s_doc->setup(0, 1, 1, 1, 1, "Custom", "Custom");
//				s_doc->setPage(100, 100, 0, 0, 0, 0, 0, 0, false, false);
//				s_doc->addPage(0);
//				s_doc->setGUI(false, this, 0);
//				s_doc->PageColors = dia->m_colorList;
//				s_doc->setGradients(dia->dialogGradients);
//				s_doc->setPatterns(dia->dialogPatterns);
//				fmt->setupTargets(s_doc, 0, this, mainWindowProgressBar, &(PrefsManager::instance()->appPrefs.fontPrefs.AvailFonts));
//				fmt->savePalette(Cpfad);
//				delete s_doc;
//			}
//			m_prefsManager->setColorSetName(dia->getColorSetName());
//			m_doc = NULL;
//		}

	if (!m_haveDoc)
		m_doc = NULL;

//	m_undoManager->setUndoEnabled(true);


}


void ColorPickerColorList::selEditColor(QTreeWidgetItem *it)
{

	// No color edit necessary

//	if ((it) && (!paletteLocked))
//	{
//		switch(colorPaintMode){
//		case ColorPaintMode::Solid:
//		case ColorPaintMode::Gradient:
//		{
//			QString curCol = it->text(0);
//			bool enableDel  = (!isMandatoryColor(curCol)) && (m_colorList.count() > 1);
//			bool enableEdit = (!isMandatoryColor(curCol));
//			duplicateButton->setEnabled(curCol != "Registration");
//			deleteButton->setEnabled(enableDel);
//			editButton->setEnabled(enableEdit);
//			if(enableEdit)
//				editColorItem();
//			break;
//		}

//		}

//	}
}

void ColorPickerColorList::itemSelectionChanged()
{
	QList<QTreeWidgetItem *> selItems = dataTree->selectedItems();
	if (selItems.count() > 1)
		deleteButton->setEnabled(true);
}


void ColorPickerColorList::handleUpdateRequest(int updateFlags)
{
	if (updateFlags & reqColorsUpdate){

		init();

	}

}


/*********************************************************************
*
* Action Buttons
*
**********************************************************************/

void ColorPickerColorList::createNew()
{
	if (paletteLocked)
		return;
//	QTreeWidgetItem* it = dataTree->currentItem();
//	if (it)
//	{
		switch(colorPaintMode){
		case ColorPaintMode::Solid:{
			QString name = Color.getDisplayName();

			if(!isMandatoryColor(name)){

				// TODO:
				// Add Dialog that ask user to override existing color

				m_colorList.insert(name, Color);
				QTreeWidgetItem *lg = updateColorList(name);
				if (lg != 0)
				{
					dataTree->setCurrentItem(lg, 0, QItemSelectionModel::ClearAndSelect);
				}
				itemSelected(dataTree->currentItem());
				modified = true;

				updateColorList();
				itemSelected(0);
				updateDoc();
			}

			break;
		}
		case ColorPaintMode::Gradient:{

			// Use local copy of Gradent data that has been set by "ColorPickerModeEditor"
			// m_gradientName, m_gradient

			// TODO:
			// Add Dialog that ask user to override existing gradient

			dialogGradients.insert(m_gradientName, m_gradient);
			QTreeWidgetItem *lg = updateGradientList(m_gradientName);
			if (lg != 0)
			{
				dataTree->setCurrentItem(lg, 0, QItemSelectionModel::ClearAndSelect);
			}
			itemSelected(dataTree->currentItem());
			modified = true;

			updateGradientList();
			itemSelected(0);
			updateDoc();

			break;
		}
		case ColorPaintMode::Hatch:{

			break;
		}
		case ColorPaintMode::Pattern:{
			//loadPatternDir();

			updatePatternList();
			itemSelected(0);
			updateDoc();
			break;
		}


		}

	//}
}

void ColorPickerColorList::editColorItem()
{
	if (paletteLocked)
		return;
	QTreeWidgetItem* it = dataTree->currentItem();
	if (it)
	{
		switch(colorPaintMode){
		case ColorPaintMode::Solid:{
			QString name = Color.getDisplayName();

			if(!isMandatoryColor(name)){

				m_colorList[name] = Color;
				if (it->text(0) != name)
				{
					replaceColorMap.insert(it->text(0), name);
					m_colorList.remove(it->text(0));
				}
				updateGradientColors(name, it->text(0));
				updateGradientList();
				updatePatternList();
				QTreeWidgetItem *lg = updateColorList(name);
				if (lg != 0)
				{
					dataTree->expandItem(lg->parent());
					dataTree->setCurrentItem(lg, 0, QItemSelectionModel::ClearAndSelect);
				}
				itemSelected(dataTree->currentItem());
				modified = true;

				updateDoc();
			}

			break;
		}
		case ColorPaintMode::Gradient:{

			QString gradN = it->text(0);
			QString patternName = origNames[it->text(0)];
			QString newName = "";

			newName = m_gradientName;
			if (newName != gradN)
			{
				origNames.remove(patternName);
				origNames.insert(newName, patternName);
				replaceMap.insert(patternName, newName);
				dialogGradients.remove(gradN);
				dialogGradients.insert(newName, m_gradient);
			}
			else
				dialogGradients[gradN] = m_gradient;
			QStringList patterns = dialogPatterns.keys();
			for (int c = 0; c < dialogPatterns.count(); ++c)
			{
				ScPattern pa = dialogPatterns[patterns[c]];
				for (int o = 0; o < pa.items.count(); o++)
				{
					PageItem *ite = pa.items.at(o);
					if (ite->gradient() == gradN)
						ite->setGradient(newName);
					if (ite->strokeGradient() == gradN)
						ite->setStrokeGradient(newName);
					if (ite->gradientMask() == gradN)
						ite->setGradientMask(newName);
				}
				PageItem *ite = pa.items.at(0);
				dialogPatterns[patterns[c]].pattern = ite->DrawObj_toImage(pa.items, 1.0);
			}

			// Update gradient list
			QTreeWidgetItem *lg = updateGradientList(m_gradientName);
			if (lg != 0)
			{
				dataTree->expandItem(lg->parent());
				dataTree->setCurrentItem(lg, 0, QItemSelectionModel::ClearAndSelect);
			}
			itemSelected(dataTree->currentItem());
			modified = true;

			updateDoc();

			break;
		}
		case ColorPaintMode::Pattern:{

			QString patternName = origNames[it->text(0)];
			QString newName = "";
			Query dia(this, "tt", 1, tr("&Name:"), tr("Rename Entry"));
			dia.setEditText(it->text(0), true);
			dia.setTestList(dialogPatterns.keys());
			if (dia.exec())
			{
				newName = dia.getEditText();
				ScPattern pat = dialogPatterns.take(it->text(0));
				dialogPatterns.insert(newName, pat);
				replaceMapPatterns.insert(patternName, newName);
				origNamesPatterns.remove(it->text(0));
				origNamesPatterns.insert(newName, patternName);
				QStringList patterns = dialogPatterns.keys();
				for (int c = 0; c < dialogPatterns.count(); ++c)
				{
					ScPattern pa = dialogPatterns[patterns[c]];
					for (int o = 0; o < pa.items.count(); o++)
					{
						PageItem *ite = pa.items.at(o);
						if ((ite->pattern() == patternName) && ((ite->GrType == 8) || (ite->itemType() == PageItem::Symbol)))
							ite->setPattern(newName);
						if (!ite->strokePattern().isEmpty())
						{
							if (ite->strokePattern() == patternName)
								ite->setStrokePattern(newName);
						}
						if (!ite->patternMask().isEmpty())
						{
							if (ite->patternMask() == patternName)
								ite->setPatternMask(newName);
						}
					}
				}
				QTreeWidgetItem *lg = updatePatternList(newName);
				if (lg != 0)
				{
					dataTree->expandItem(lg->parent());
					dataTree->setCurrentItem(lg, 0, QItemSelectionModel::ClearAndSelect);
				}
				itemSelected(dataTree->currentItem());

				updateDoc();
			}
			else
				return;

			break;
		}
		case ColorPaintMode::Hatch:{

			break;
		}
		}

	}
}


void ColorPickerColorList::removeColorItem()
{
	if (paletteLocked)
		return;
	QList<QTreeWidgetItem *> selItems = dataTree->selectedItems();

	// Multiple Object Selection
	if (selItems.count() > 1)
	{
		QStringList usedColors;
		QStringList colors;
		QStringList gradients;
		QStringList patterns;
		for (int a = 0; a < selItems.count(); a++)
		{
			QTreeWidgetItem* it = selItems[a];

			if (isMandatoryColor(it->text(0)))
				continue; // skip mendatory colors

			switch(colorPaintMode){
			case ColorPaintMode::Solid:
				colors.append(it->text(0));
				break;
			case ColorPaintMode::Gradient:
				gradients.append(it->text(0));
				break;
			case ColorPaintMode::Hatch:
				break;
			case ColorPaintMode::Pattern:
				patterns.append(it->text(0));
				break;
			}

		}
		for (int a = 0; a < gradients.count(); a++)
		{
			dialogGradients.remove(gradients[a]);
			replaceMap.insert(gradients[a], "");
		}

		bool hasUsed = false;
		ColorList UsedCG = getGradientColors();
		for (int a = 0; a < colors.count(); a++)
		{
			if (UsedCG.contains(colors[a]) || inDocUsedColors.contains(colors[a]))
			{
				hasUsed = true;
				usedColors.append(colors[a]);
			}
		}
		if (hasUsed)
		{
			ColorList dCols = m_colorList;
			QString dColor = tr("Selected Colors");
			for (int a = 0; a < usedColors.count(); a++)
			{
				dCols.remove(usedColors[a]);
			}
			dCols.insert(dColor , ScColor());
			DelColor *dia = new DelColor(this, dCols, dColor, true);
			if (dia->exec())
			{
				QString replacementColor(dia->getReplacementColor());
				for (int a = 0; a < colors.count(); a++)
				{
					dColor = colors[a];
					if (replacementColor == CommonStrings::tr_NoneColor)
						replacementColor = CommonStrings::None;
					if (replaceColorMap.values().contains(dColor))
					{
						QMap<QString,QString>::Iterator itt;
						for (itt = replaceColorMap.begin(); itt != replaceColorMap.end(); ++itt)
						{
							if (itt.value() == dColor)
								itt.value() = replacementColor;
						}
					}
					replaceColorMap.insert(dColor, replacementColor);
					m_colorList.remove(dColor);
					updateGradientColors(replacementColor, dColor);
				}
				modified = true;
			}
			delete dia;
		}
		else
		{
			for (int a = 0; a < colors.count(); a++)
			{
				replaceColorMap.insert(colors[a], "Black");
				m_colorList.remove(colors[a]);
			}
			modified = true;
		}
		updatePatternList();
		updateGradientList();
		updateColorList();
		itemSelected(0);

		updateDoc();
	}
	else // Single Object Selection
	{
		QTreeWidgetItem* it = dataTree->currentItem();
		if (!it)
			return;

		switch(colorPaintMode){
		case ColorPaintMode::Solid:{
			QString dColor = it->text(0);
			if (isMandatoryColor(dColor))
				return;
			ColorList UsedCG = getGradientColors();
			if (inDocUsedColors.contains(dColor) || UsedCG.contains(dColor))
			{
				DelColor *dia = new DelColor(this, m_colorList, dColor, true);
				if (dia->exec())
				{
					QString replacementColor(dia->getReplacementColor());
					if (replacementColor == CommonStrings::tr_NoneColor)
						replacementColor = CommonStrings::None;
					if (replaceColorMap.values().contains(dColor))
					{
						QMap<QString,QString>::Iterator itt;
						for (itt = replaceColorMap.begin(); itt != replaceColorMap.end(); ++itt)
						{
							if (itt.value() == dColor)
								itt.value() = replacementColor;
						}
					}
					replaceColorMap.insert(dColor, replacementColor);
					m_colorList.remove(dColor);
					updateGradientColors(replacementColor, dColor);
				}
				delete dia;
			}
			else
			{
				replaceColorMap.insert(dColor, "Black");
				updateGradientColors("Black", dColor);
				m_colorList.remove(dColor);
			}
			updateColorList();
			break;
		}
		case ColorPaintMode::Gradient:{

			dialogGradients.remove(it->text(0));
			replaceMap.insert(it->text(0), "");

			updateGradientList();
			break;
		}
		case ColorPaintMode::Hatch:{

			break;
		}
		case ColorPaintMode::Pattern:{

			QStringList patterns2Del;
			QStringList mainPatterns = dialogPatterns.keys();
			for (int a = 0; a < mainPatterns.count(); a++)
			{
				if (mainPatterns[a] != it->text(0))
				{
					QStringList subPatterns;
					subPatterns = getUsedPatternsHelper(mainPatterns[a], subPatterns);
					if (subPatterns.contains(it->text(0)))
						patterns2Del.append(mainPatterns[a]);
				}
			}
			patterns2Del.append(it->text(0));
			for (int a = 0; a < patterns2Del.count(); a++)
			{
				dialogPatterns.remove(patterns2Del[a]);
			}

			updatePatternList();

			break;
		}

		}

		itemSelected(0);
		modified = true;

		updateDoc();

	}

}


void ColorPickerColorList::importColorItems()
{
	QTreeWidgetItem* it = dataTree->currentItem();
	if (it)
	{
		switch(colorPaintMode){
		case ColorPaintMode::Gradient:{

			QString fileName;
			QString allFormats = tr("All Supported Formats")+" (";
			allFormats += "*.sgr *.SGR";
			allFormats += " *.ggr *.GGR";
			allFormats += ");;";
			QString formats = tr("Scribus Gradient Files \"*.sgr\" (*.sgr *.SGR);;");
			formats += tr("Gimp Gradient Files \"*.ggr\" (*.ggr *.GGR);;");
			formats += tr("All Files (*)");
			allFormats += formats;
			PrefsContext* dirs = PrefsManager::instance()->prefsFile->getContext("dirs");
			QString wdir = dirs->get("gradients", ".");
			CustomFDialog dia(this, wdir, tr("Open"), allFormats, fdHidePreviewCheckBox | fdExistingFiles | fdDisableOk);
			if (dia.exec() == QDialog::Accepted)
				fileName = dia.selectedFile();
			else
				return;
			if (!fileName.isEmpty())
			{
				PrefsManager::instance()->prefsFile->getContext("dirs")->set("gradients", fileName.left(fileName.lastIndexOf("/")));
				QFileInfo fi(fileName);
				QString ext = fi.suffix().toLower();
				if (ext == "sgr")
					loadScribusFormat(fileName);
				else if (ext == "ggr")
					loadGimpFormat(fileName);
				updateGradientList();
				updateColorList();
				itemSelected(0);
				modified = true;
			}
		}
			break;


		case ColorPaintMode::Solid:{
			QStringList allFormatsV = LoadSavePlugin::getExtensionsForColors();
			allFormatsV.removeAll("sla");
			allFormatsV.removeAll("scd");
			allFormatsV.removeAll("sla.gz");
			allFormatsV.removeAll("scd.gz");
			allFormatsV.removeAll("ai");
			QString extra = allFormatsV.join(" *.");
			extra.prepend(" *.");
			QString fileName;
			PrefsContext* dirs = PrefsManager::instance()->prefsFile->getContext("dirs");
			QString wdir = dirs->get("colors", ".");
			QString docexts("*.sla *.sla.gz *.scd *.scd.gz");
			QString aiepsext(FormatsManager::instance()->extensionListForFormat(FormatsManager::EPS|FormatsManager::PS|FormatsManager::AI, 0));
			QString ooexts(" *.acb *.aco *.ase *.skp *.soc *.gpl *.xml *.sbz");
			ooexts += extra;
			QString filter = tr("All Supported Formats (%1);;Documents (%2);;Other Files (%3);;All Files (*)").arg(docexts+" "+aiepsext+ooexts).arg(docexts).arg(aiepsext+ooexts);
			CustomFDialog dia(this, wdir, tr("Import Colors"), filter, fdHidePreviewCheckBox | fdDisableOk);
			if (dia.exec() == QDialog::Accepted)
				fileName = dia.selectedFile();
			else
				return;
			if (!fileName.isEmpty())
				dirs->set("colors", fileName.left(fileName.lastIndexOf("/")));
			if (!importColorsFromFile(fileName, m_colorList))
				ScMessageBox::information(this, tr("Information"), "<qt>" + tr("The file %1 does not contain colors which can be imported.\nIf the file was a PostScript-based, try to import it with File -&gt; Import. \nNot all files have DSC conformant comments where the color descriptions are located.\n This prevents importing colors from some files.\nSee the Edit Colors section of the documentation for more details.").arg(fileName) + "</qt>");
			else
			{
				updateGradientList();
				updateColorList();
				modified = true;
			}
			itemSelected(0);
		}
			break;


		case ColorPaintMode::Pattern:{
			QString fileName;
			QStringList formats;
			QString allFormats = tr("All Supported Formats")+" (";
			int fmtCode = FORMATID_FIRSTUSER;
			const FileFormat *fmt = LoadSavePlugin::getFormatById(fmtCode);
			while (fmt != 0)
			{
				if (fmt->load)
				{
					formats.append(fmt->filter);
					int an = fmt->filter.indexOf("(");
					int en = fmt->filter.indexOf(")");
					while (an != -1)
					{
						allFormats += fmt->filter.mid(an+1, en-an-1)+" ";
						an = fmt->filter.indexOf("(", en);
						en = fmt->filter.indexOf(")", an);
					}
				}
				fmtCode++;
				fmt = LoadSavePlugin::getFormatById(fmtCode);
			}
			allFormats += "*.sce *.SCE ";
			formats.append("Scribus Objects (*.sce *.SCE)");
			QString form1 = "";
			QString form2 = "";
			QStringList imgFormats;
			bool jpgFound = false;
			bool tiffFound = false;
			for (int i = 0; i < QImageReader::supportedImageFormats().count(); ++i )
			{
				form1 = QString(QImageReader::supportedImageFormats().at(i)).toLower();
				form2 = QString(QImageReader::supportedImageFormats().at(i)).toUpper();
				if ((form1 == "png") || (form1 == "xpm") || (form1 == "gif"))
				{
					formats.append(form2 + " (*."+form1+" *."+form2+")");
					allFormats += "*."+form1+" *."+form2+" ";
					imgFormats.append(form1);
				}
				else if ((form1 == "jpg") || (form1 == "jpeg"))
				{
					// JPEG is a special case because both .jpg and .jpeg
					// are acceptable extensions.
					if (!jpgFound)
					{
						formats.append("JPEG (*.jpg *.jpeg *.JPG *.JPEG)");
						allFormats += "*.jpg *.jpeg *.JPG *.JPEG ";
						imgFormats.append("jpeg");
						imgFormats.append("jpg");
						jpgFound = true;
					}
				}
				else if ((form1 == "tif") || (form1 == "tiff"))
				{
					if (!tiffFound)
					{
						formats.append("TIFF (*.tif *.tiff *.TIF *.TIFF)");
						allFormats += "*.tif *.tiff *.TIF *.TIFF ";
						imgFormats.append("tif");
						imgFormats.append("tiff");
						tiffFound = true;
					}
				}
				else if (form1 != "svg")
				{
					imgFormats.append(form1);
					allFormats += "*."+form1+" *."+form2+" ";
				}
			}
			if (!tiffFound)
			{
				formats.append("TIFF (*.tif *.tiff *.TIF *.TIFF)");
				allFormats += "*.tif *.tiff *.TIF *.TIFF ";
			}
			if (!jpgFound)
			{
				formats.append("JPEG (*.jpg *.jpeg *.JPG *.JPEG)");
				allFormats += "*.jpg *.jpeg *.JPG *.JPEG ";
			}
			formats.append("PSD (*.psd *.PSD)");
			formats.append("Gimp Patterns (*.pat *.PAT)");
			allFormats += "*.psd *.PSD ";
			allFormats += "*.pat *.PAT);;";
			imgFormats.append("tif");
			imgFormats.append("tiff");
			imgFormats.append("pat");
			imgFormats.append("psd");
			//	imgFormats.append("pdf");
			imgFormats.append("eps");
			imgFormats.append("epsi");
			imgFormats.append("ps");
			qSort(formats);
			allFormats += formats.join(";;");
			PrefsContext* dirs = PrefsManager::instance()->prefsFile->getContext("dirs");
			QString wdir = dirs->get("patterns", ".");
			CustomFDialog dia(this, wdir, tr("Open"), allFormats, fdHidePreviewCheckBox | fdExistingFiles | fdDisableOk);
			if (dia.exec() != QDialog::Accepted)
				return;
			fileName = dia.selectedFile();
			if (fileName.isEmpty())
				return;
			qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
			PrefsManager::instance()->prefsFile->getContext("dirs")->set("patterns", fileName.left(fileName.lastIndexOf("/")));
			QFileInfo fi(fileName);
			if ((fi.suffix().toLower() == "sce") || (!imgFormats.contains(fi.suffix().toLower())))
			{
				loadVectors(fileName);
			}
			else
			{
				QString patNam = fi.baseName().trimmed().simplified().replace(" ", "_");
				ScPattern pat = ScPattern();
				pat.setDoc(m_doc);
				pat.setPattern(fileName);
				if (!dialogPatterns.contains(patNam))
				{
					dialogPatterns.insert(patNam, pat);
					origNamesPatterns.insert(patNam, patNam);
				}
			}
			updateColorList();
			updateGradientList();
			updatePatternList();
			itemSelected(0);
			qApp->restoreOverrideCursor();
		}
			break;
		}
	}
}

void ColorPickerColorList::removeUnusedColorItem()
{
	if (paletteLocked)
		return;
	QTreeWidgetItem* it = dataTree->currentItem();
	if (it)
	{
		switch(colorPaintMode){
		case ColorPaintMode::Solid:{
			ColorList::Iterator it;
			ColorList UsedCG = getGradientColors();
			if (m_ScMW->HaveDoc)
			{
				m_colorList = inDocUsedColors;
				for (it = UsedCG.begin(); it != UsedCG.end(); ++it)
				{
					if (!m_colorList.contains(it.key()))
						m_colorList.insert(it.key(), it.value());
				}
			}
			else
				m_colorList = UsedCG;

			m_colorList.ensureDefaultColors();
			updatePatternList();
			updateGradientList();
			updateColorList();
			itemSelected(0);
			modified = true;

			updateDoc();
			break;
		}
		}


	}
}

/*********************************************************************
*
* Members
*
**********************************************************************/

QString ColorPickerColorList::getColorTooltip(const ScColor& color)
{
	QString tooltip;
	if (color.getColorModel() == colorModelRGB)
	{
		int r, g, b;
		color.getRawRGBColor(&r, &g, &b);
		tooltip = tr("R: %1 G: %2 B: %3").arg(r).arg(g).arg(b);
	}
	else if (color.getColorModel() == colorModelCMYK)
	{
		int c, m, y, k;
		color.getCMYK(&c, &m, &y, &k);
		tooltip = tr("C: %1% M: %2% Y: %3% K: %4%").arg(qRound(c / 2.55)).arg(qRound(m / 2.55)).arg(qRound(y / 2.55)).arg(qRound(k / 2.55));
	}
	else if (color.getColorModel() == colorModelLab)
	{
		double L, a, b;
		color.getLab(&L, &a, &b);
		tooltip = tr("L: %1 a: %2 b: %3").arg(L, 0, 'f', 2).arg(a, 0, 'f', 2).arg(b, 0, 'f', 2);
	}
	return tooltip;
}

bool ColorPickerColorList::isMandatoryColor(QString colorName)
{
	if (colorName == "Black" || colorName == "White")
		return true;
	ScColor color = m_colorList.value(colorName, ScColor());
	if (color.isRegistrationColor())
		return true;
	return false;
}

ColorList ColorPickerColorList::getGradientColors()
{
	ColorList colorList;
	QHash<QString,VGradient>::Iterator itg;
	for (itg = dialogGradients.begin(); itg != dialogGradients.end(); ++itg)
	{
		QList<VColorStop*> cstops = itg.value().colorStops();
		for (uint cst = 0; cst < itg.value().Stops(); ++cst)
		{
			if ((!colorList.contains(cstops.at(cst)->name)) && (cstops.at(cst)->name != CommonStrings::None))
				colorList.insert(cstops.at(cst)->name, m_colorList[cstops.at(cst)->name]);
		}
	}
	QStringList patterns = dialogPatterns.keys();
	for (int c = 0; c < dialogPatterns.count(); ++c)
	{
		ScPattern pa = dialogPatterns[patterns[c]];
		for (int o = 0; o < pa.items.count(); o++)
		{
			PageItem *ite = pa.items.at(o);
			if ((!colorList.contains(ite->lineColor())) && (ite->lineColor() != CommonStrings::None))
				colorList.insert(ite->lineColor(), m_colorList[ite->lineColor()]);
			if ((!colorList.contains(ite->fillColor())) && (ite->fillColor() != CommonStrings::None))
				colorList.insert(ite->fillColor(), m_colorList[ite->fillColor()]);
			QString gCol1, gCol2, gCol3, gCol4;
			ite->get4ColorColors(gCol1, gCol2, gCol3, gCol4);
			if ((!colorList.contains(gCol1)) && (gCol1 != CommonStrings::None))
				colorList.insert(gCol1, m_colorList[gCol1]);
			if ((!colorList.contains(gCol2)) && (gCol2 != CommonStrings::None))
				colorList.insert(gCol2, m_colorList[gCol2]);
			if ((!colorList.contains(gCol3)) && (gCol3 != CommonStrings::None))
				colorList.insert(gCol3, m_colorList[gCol3]);
			if ((!colorList.contains(gCol4)) && (gCol4 != CommonStrings::None))
				colorList.insert(gCol4, m_colorList[gCol4]);
			for (int grow = 0; grow < ite->meshGradientArray.count(); grow++)
			{
				for (int gcol = 0; gcol < ite->meshGradientArray[grow].count(); gcol++)
				{
					meshPoint mp = ite->meshGradientArray[grow][gcol];
					if ((!colorList.contains(mp.colorName)) && (mp.colorName != CommonStrings::None))
						colorList.insert(mp.colorName, m_colorList[mp.colorName]);
				}
			}
			QList<VColorStop*> cstops = ite->fill_gradient.colorStops();
			for (uint cst = 0; cst < ite->fill_gradient.Stops(); ++cst)
			{
				if ((!colorList.contains(cstops.at(cst)->name)) && (cstops.at(cst)->name != CommonStrings::None))
					colorList.insert(cstops.at(cst)->name, m_colorList[cstops.at(cst)->name]);
			}
			cstops = ite->stroke_gradient.colorStops();
			for (uint cst = 0; cst < ite->stroke_gradient.Stops(); ++cst)
			{
				if ((!colorList.contains(cstops.at(cst)->name)) && (cstops.at(cst)->name != CommonStrings::None))
					colorList.insert(cstops.at(cst)->name, m_colorList[cstops.at(cst)->name]);
			}
			cstops = ite->mask_gradient.colorStops();
			for (uint cst = 0; cst < ite->mask_gradient.Stops(); ++cst)
			{
				if ((!colorList.contains(cstops.at(cst)->name)) && (cstops.at(cst)->name != CommonStrings::None))
					colorList.insert(cstops.at(cst)->name, m_colorList[cstops.at(cst)->name]);
			}
		}
	}
	colorList.remove(CommonStrings::None);
	return colorList;
}

void ColorPickerColorList::updateGradientColors(QString newName, QString oldName)
{
	QHash<QString,VGradient>::Iterator itg;
	for (itg = dialogGradients.begin(); itg != dialogGradients.end(); ++itg)
	{
		QList<VColorStop*> cstops = itg.value().colorStops();
		for (uint cst = 0; cst < itg.value().Stops(); ++cst)
		{
			if (oldName == cstops.at(cst)->name)
			{
				cstops.at(cst)->name = newName;
				const ScColor& col = m_colorList[cstops.at(cst)->name];
				cstops.at(cst)->color = ScColorEngine::getShadeColorProof(col, m_doc, cstops.at(cst)->shade);
			}
		}
	}
	ColorList colorListBack = m_doc->PageColors;
	m_doc->PageColors = m_colorList;
	QStringList patterns = dialogPatterns.keys();
	for (int c = 0; c < dialogPatterns.count(); ++c)
	{
		ScPattern pa = dialogPatterns[patterns[c]];
		for (int o = 0; o < pa.items.count(); o++)
		{
			PageItem *ite = pa.items.at(o);
			if (oldName == ite->lineColor())
				ite->setLineColor(newName);
			if (oldName == ite->fillColor())
				ite->setFillColor(newName);
			QString gCol1, gCol2, gCol3, gCol4;
			ite->get4ColorColors(gCol1, gCol2, gCol3, gCol4);
			if (oldName == gCol1)
				gCol1 = newName;
			if (oldName == gCol2)
				gCol2 = newName;
			if (oldName == gCol3)
				gCol3 = newName;
			if (oldName == gCol4)
				gCol4 = newName;
			ite->set4ColorColors(gCol1, gCol2, gCol3, gCol4);
			for (int grow = 0; grow < ite->meshGradientArray.count(); grow++)
			{
				for (int gcol = 0; gcol < ite->meshGradientArray[grow].count(); gcol++)
				{
					meshPoint mp = ite->meshGradientArray[grow][gcol];
					if (mp.colorName == oldName)
					{
						mp.colorName = newName;
						ite->setMeshPointColor(grow, gcol, mp.colorName, mp.shade, mp.transparency);
					}
				}
			}
			QList<VColorStop*> cstops = ite->fill_gradient.colorStops();
			for (uint cst = 0; cst < ite->fill_gradient.Stops(); ++cst)
			{
				if (oldName == cstops.at(cst)->name)
				{
					cstops.at(cst)->name = newName;
					ite->SetQColor(&cstops.at(cst)->color, cstops.at(cst)->name, cstops.at(cst)->shade);
				}
			}
			cstops = ite->stroke_gradient.colorStops();
			for (uint cst = 0; cst < ite->stroke_gradient.Stops(); ++cst)
			{
				if (oldName == cstops.at(cst)->name)
				{
					cstops.at(cst)->name = newName;
					ite->SetQColor(&cstops.at(cst)->color, cstops.at(cst)->name, cstops.at(cst)->shade);
				}
			}
			cstops = ite->mask_gradient.colorStops();
			for (uint cst = 0; cst < ite->mask_gradient.Stops(); ++cst)
			{
				if (oldName == cstops.at(cst)->name)
				{
					cstops.at(cst)->name = newName;
					ite->SetQColor(&cstops.at(cst)->color, cstops.at(cst)->name, cstops.at(cst)->shade);
				}
			}
		}
		PageItem *ite = pa.items.at(0);
		dialogPatterns[patterns[c]].pattern = ite->DrawObj_toImage(pa.items, 1.0);
	}
	m_doc->PageColors = colorListBack;
}

QStringList ColorPickerColorList::getUsedPatternsHelper(QString pattern, QStringList &results)
{
	ScPattern *pat = &dialogPatterns[pattern];
	QStringList pats;
	pats.clear();
	for (int c = 0; c < pat->items.count(); ++c)
	{
		if ((pat->items.at(c)->GrType == 8) || (pat->items.at(c)->itemType() == PageItem::Symbol))
		{
			const QString& patName = pat->items.at(c)->pattern();
			if (!patName.isEmpty() && !results.contains(patName))
				pats.append(patName);
		}
		const QString& pat2 = pat->items.at(c)->strokePattern();
		if (!pat2.isEmpty() && !results.contains(pat2))
			pats.append(pat->items.at(c)->strokePattern());
		const QString& pat3 = pat->items.at(c)->patternMask();
		if (!pat3.isEmpty() && !results.contains(pat3))
			pats.append(pat->items.at(c)->patternMask());
	}
	if (!pats.isEmpty())
	{
		results = pats;
		for (int c = 0; c < pats.count(); ++c)
		{
			getUsedPatternsHelper(pats[c], results);
		}
	}
/*	QStringList pats;
	pats.clear();
	for (int c = 0; c < pat->items.count(); ++c)
	{
		if ((!results.contains(pat->items.at(c)->pattern())) && ((pat->items.at(c)->GrType == 8) || (pat->items.at(c)->itemType() == PageItem::Symbol)))
			pats.append(pat->items.at(c)->pattern());
	}
	if (!pats.isEmpty())
	{
		results = pats;
		for (int c = 0; c < pats.count(); ++c)
		{
			getUsedPatternsHelper(pats[c], results);
		}
	} */
	return results;
}

void ColorPickerColorList::setColorPaintMode(ColorPaintMode mode, GradientTypes gradient)
{

	colorPaintMode = mode;
	gradientTypes = gradient;

	switch(mode){
	case ColorPaintMode::Hatch:
		this->setVisible(false);
		break;
	case ColorPaintMode::Gradient:
		switch(gradient){
		case GradientTypes::Linear:
		case GradientTypes::Radial:
		case GradientTypes::Diamond:
		case GradientTypes::Conical:
			this->setVisible(true);
			break;
		case GradientTypes::FourColors:
		case GradientTypes::Mesh:
		case GradientTypes::PatchMesh:
		default:
			this->setVisible(false);
			break;
		}
		break;
	case ColorPaintMode::Solid:
	case ColorPaintMode::Pattern:
		this->setVisible(true);
		break;
	}

	// refresh list
	init();

}

void ColorPickerColorList::setObjectPaintMode(ObjectPaintMode mode)
{

	objectPaintMode = mode;

}
/*********************************************************************
*
* Event Handler
*
**********************************************************************/

void ColorPickerColorList::slotRightClick(QPoint p)
{
	QTreeWidgetItem* it = dataTree->itemAt(p);
	if (it)
	{
		switch(colorPaintMode){
		case ColorPaintMode::Solid:
			QMenu *pmen = new QMenu();
			//			qApp->changeOverrideCursor(QCursor(Qt::ArrowCursor));
			pmen->addAction( tr("Sort by Name"));
			pmen->addAction( tr("Sort by Color"));
			pmen->addAction( tr("Sort by Type"));
			sortRule = pmen->actions().indexOf(pmen->exec(QCursor::pos()));
			delete pmen;
			updateColorList();
			break;
		}
	}
}

void ColorPickerColorList::toggleColorList(){

	if(this->isHidden()){
		this->show();
	}else{
		this->hide();
	}

}

void ColorPickerColorList::setColor(ScColor color){

	Color = color;

}

void ColorPickerColorList::itemSelected(QTreeWidgetItem* it)
{
	QList<QTreeWidgetItem *> selItems = dataTree->selectedItems();

	newButton->setText( tr("Add"));
	importButton->setText( tr("&Import"));

	if ((it) && (!paletteLocked))
	{

		switch(colorPaintMode){
		case ColorPaintMode::Solid:{

			QString curCol = it->text(0);
			bool enableDel  = (!isMandatoryColor(curCol)) && (m_colorList.count() > 1);
			bool enableEdit = (!isMandatoryColor(curCol));

			newButton->setEnabled(true);
			deleteButton->setEnabled(enableDel);
			editButton->setEnabled(enableEdit);
			duplicateButton->setEnabled(curCol != "Registration");

			importButton->setEnabled(true);
			deleteUnusedButton->setEnabled(it->childCount() > 0);

			emit emitColor(m_colorList[curCol]);

			break;
		}
		case ColorPaintMode::Gradient:{

			newButton->setEnabled(true);
			deleteButton->setEnabled(true);
			editButton->setEnabled(true);
			duplicateButton->setEnabled(true);

			importButton->setEnabled(false);
			deleteUnusedButton->setEnabled(false);

			QString gradientName = it->text(0);
			VGradient gradient = dialogGradients.value(gradientName);
			emit emitGradient(gradientName, gradient);

			break;
		}
		case ColorPaintMode::Hatch:

			break;
		case ColorPaintMode::Pattern:{

			newButton->setText( tr("Load Set"));
			newButton->setEnabled(true);
			if (selItems.count() < 2)
				deleteButton->setEnabled(it->childCount() > 0);
			else
				deleteButton->setEnabled(true);


			editButton->setEnabled(true);
			duplicateButton->setEnabled(false);

			importButton->setText( tr("Load File"));
			importButton->setEnabled(true);
			deleteUnusedButton->setEnabled(false);

			break;
		}
		}

		//		if ((it->parent() == colorItems) || (it->parent() == gradientItems))
		//		{
		//			importButton->setEnabled(false);
		//			newButton->setEnabled(true);
		//			deleteUnusedButton->setEnabled(it->parent() == colorItems);
		//			if (it->parent() == colorItems)
		//			{
		//				QString curCol = it->text(0);
		//				//ScColor tmpColor = m_colorList[curCol];
		//				bool enableDel  = (!isMandatoryColor(curCol)) && (m_colorList.count() > 1);
		//				bool enableEdit = (!isMandatoryColor(curCol));
		//				duplicateButton->setEnabled(curCol != "Registration");
		//				deleteButton->setEnabled(enableDel);
		//				editButton->setEnabled(enableEdit);

		//				emit emitColor(m_colorList[curCol]);
		//			}
		//			else
		//			{
		//				editButton->setEnabled(true);
		//				duplicateButton->setEnabled(true);
		//				deleteButton->setEnabled(true);
		//			}
		//		}
		//		else if (it->parent() == patternItems)
		//		{
		//			importButton->setText( tr("Load File"));
		//			newButton->setText( tr("Load Set"));
		//			editButton->setText( tr("Rename"));
		//			importButton->setEnabled(true);
		//			newButton->setEnabled(true);
		//			editButton->setEnabled(true);
		//			duplicateButton->setEnabled(false);
		//			deleteButton->setEnabled(true);
		//		}
		//		else if (it == patternItems)
		//		{
		//			importButton->setText( tr("Load File"));
		//			newButton->setText( tr("Load Set"));
		//			importButton->setEnabled(true);
		//			newButton->setEnabled(true);
		//			editButton->setEnabled(false);
		//			duplicateButton->setEnabled(false);
		//			if (selItems.count() < 2)
		//				deleteButton->setEnabled(it->childCount() > 0);
		//			else
		//				deleteButton->setEnabled(true);
		//		}
		//		else
		//		{
		//			importButton->setEnabled(true);
		//			newButton->setEnabled(true);
		//			editButton->setEnabled(false);
		//			duplicateButton->setEnabled(false);
		//			if (selItems.count() < 2)
		//				deleteButton->setEnabled(it->childCount() > 0);
		//			else
		//				deleteButton->setEnabled(true);
		//			deleteUnusedButton->setEnabled((it == colorItems) && (it->childCount() > 0));
		//		}
	}
	else
	{
		switch(colorPaintMode){
		case ColorPaintMode::Solid:
		case ColorPaintMode::Gradient:
			newButton->setEnabled(true);
			break;
		case ColorPaintMode::Hatch:
		case ColorPaintMode::Pattern:
		default:
			newButton->setEnabled(false);
			break;
		}

		deleteButton->setEnabled(false);
		editButton->setEnabled(false);
		duplicateButton->setEnabled(false);

		importButton->setEnabled(false);
		deleteUnusedButton->setEnabled(false);

		dataTree->clearSelection();
	}
}


void ColorPickerColorList::setGradientData(const QString name, VGradient gradient)
{
	m_gradient = gradient;
	m_gradientName = name;
}

void ColorPickerColorList::loadScribusFormat(QString fileName)
{
	QFile f(fileName);
	if(!f.open(QIODevice::ReadOnly))
		return;
	QDomDocument docu("scridoc");
	QTextStream ts(&f);
	ts.setCodec("UTF-8");
	QString errorMsg;
	int errorLine = 0, errorColumn = 0;
	if( !docu.setContent(ts.readAll(), &errorMsg, &errorLine, &errorColumn) )
	{
		f.close();
		return;
	}
	f.close();
	QDomElement elem = docu.documentElement();
	if (elem.tagName() != "SCRIBUSGRADIENT")
		return;
	QDomNode DOC = elem.firstChild();
	while(!DOC.isNull())
	{
		QDomElement dc = DOC.toElement();
		if (dc.tagName()=="COLOR")
		{
			ScColor lf = ScColor();
			if (dc.hasAttribute("CMYK"))
				lf.setNamedColor(dc.attribute("CMYK"));
			else
				lf.fromQColor(QColor(dc.attribute("RGB")));
			if (dc.hasAttribute("Spot"))
				lf.setSpotColor(static_cast<bool>(dc.attribute("Spot").toInt()));
			else
				lf.setSpotColor(false);
			if (dc.hasAttribute("Register"))
				lf.setRegistrationColor(static_cast<bool>(dc.attribute("Register").toInt()));
			else
				lf.setRegistrationColor(false);
			if (!m_colorList.contains(dc.attribute("NAME")))
			{
				m_colorList.insert(dc.attribute("NAME"), lf);
				hasImportedColors = true;
			}
		}
		if (dc.tagName() == "Gradient")
		{
			VGradient gra = VGradient(VGradient::linear);
			gra.clearStops();
			QDomNode grad = dc.firstChild();
			while(!grad.isNull())
			{
				QDomElement stop = grad.toElement();
				QString name = stop.attribute("NAME");
				double ramp  = ScCLocale::toDoubleC(stop.attribute("RAMP"), 0.0);
				int shade    = stop.attribute("SHADE", "100").toInt();
				double opa   = ScCLocale::toDoubleC(stop.attribute("TRANS"), 1.0);
				QColor color;
				if (name == CommonStrings::None)
					color = QColor(255, 255, 255, 0);
				else
				{
					const ScColor& col = m_colorList[name];
					color = ScColorEngine::getShadeColorProof(col, NULL, shade);
				}
				gra.addStop(color, ramp, 0.5, opa, name, shade);
				grad = grad.nextSibling();
			}
			if (!dialogGradients.contains(dc.attribute("Name")))
				dialogGradients.insert(dc.attribute("Name"), gra);
			else
			{
				QString tmp;
				QString name = dc.attribute("Name");
				name += "("+tmp.setNum(dialogGradients.count())+")";
				dialogGradients.insert(name, gra);
			}
		}
		DOC=DOC.nextSibling();
	}
}

void ColorPickerColorList::loadGimpFormat(QString fileName)
{
	QFile f(fileName);
	if (f.open(QIODevice::ReadOnly))
	{
		ScTextStream ts(&f);
		QString tmp, dummy;
		QString gradientName = "";
		int numEntrys = 0;
		int entryCount = 0;
		int stopCount = 0;
		double left, middle, right, r0, g0, b0, a0, r1, g1, b1, a1;
		double oldr1 = 0.0;
		double oldg1 = 0.0;
		double oldb1 = 0.0;
		double olda1 = 0.0;
		tmp = ts.readLine();
		if (tmp.startsWith("GIMP Gradient"))
		{
			tmp = ts.readLine();
			ScTextStream CoE(&tmp, QIODevice::ReadOnly);
			CoE >> dummy;
			gradientName = CoE.readAll().trimmed();
		}
		if (!gradientName.isEmpty())
		{
			QString stopName = gradientName+QString("_Stop%1");
			QString stopNameInUse;
			VGradient gra = VGradient(VGradient::linear);
			gra.clearStops();
			QColor color;
			tmp = ts.readLine();
			ScTextStream CoE(&tmp, QIODevice::ReadOnly);
			CoE >> numEntrys;
			while (!ts.atEnd())
			{
				entryCount++;
				tmp = ts.readLine();
				ScTextStream Cval(&tmp, QIODevice::ReadOnly);
				Cval >> left >> middle >> right >> r0 >> g0 >> b0 >> a0 >> r1 >> g1 >> b1 >> a1;
				if ((entryCount == 1) && (entryCount < numEntrys))
				{
					stopNameInUse = stopName.arg(stopCount);
					addGimpColor(stopNameInUse, r0, g0, b0);
					color = QColor(qRound(r0 * 255), qRound(g0 * 255), qRound(b0 * 255));
					gra.addStop(color, left, 0.5, a0, stopNameInUse, 100);
					stopCount++;
				}
				else if (entryCount == numEntrys)
				{
					if ((entryCount != 1) && ((r0 != oldr1) || (g0 != oldg1) || (b0 != oldb1) || (a0 != olda1)))
					{
						stopNameInUse = stopName.arg(stopCount);
						addGimpColor(stopNameInUse, oldr1, oldg1, oldb1);
						color = QColor(qRound(oldr1 * 255), qRound(oldg1 * 255), qRound(oldb1 * 255));
						gra.addStop(color, left, 0.5, olda1, stopNameInUse, 100);
						stopCount++;
					}
					stopNameInUse = stopName.arg(stopCount);
					addGimpColor(stopNameInUse, r0, g0, b0);
					color = QColor(qRound(r0 * 255), qRound(g0 * 255), qRound(b0 * 255));
					gra.addStop(color, left, 0.5, a0, stopNameInUse, 100);
					stopCount++;
					stopNameInUse = stopName.arg(stopCount);
					addGimpColor(stopNameInUse, r1, g1, b1);
					color = QColor(qRound(r1 * 255), qRound(g1 * 255), qRound(b1 * 255));
					gra.addStop(color, right, 0.5, a1, stopNameInUse, 100);
					stopCount++;
				}
				else
				{
					if ((r0 == oldr1) && (g0 == oldg1) && (b0 == oldb1) && (a0 == olda1))
					{
						stopNameInUse = stopName.arg(stopCount);
						addGimpColor(stopNameInUse, r0, g0, b0);
						color = QColor(qRound(r0 * 255), qRound(g0 * 255), qRound(b0 * 255));
						gra.addStop(color, left, 0.5, a0, stopNameInUse, 100);
						stopCount++;
					}
					else
					{
						stopNameInUse = stopName.arg(stopCount);
						addGimpColor(stopNameInUse, oldr1, oldg1, oldb1);
						color = QColor(qRound(oldr1 * 255), qRound(oldg1 * 255), qRound(oldb1 * 255));
						gra.addStop(color, left, 0.5, olda1, stopNameInUse, 100);
						stopCount++;
						stopNameInUse = stopName.arg(stopCount);
						addGimpColor(stopNameInUse, r0, g0, b0);
						color = QColor(qRound(r0 * 255), qRound(g0 * 255), qRound(b0 * 255));
						gra.addStop(color, left, 0.5, a0, stopNameInUse, 100);
						stopCount++;
					}
				}
				oldr1 = r1;
				oldg1 = g1;
				oldb1 = b1;
				olda1 = a1;
			}
			if (!dialogGradients.contains(gradientName))
				dialogGradients.insert(gradientName, gra);
			else
			{
				QString tmp;
				gradientName += "("+tmp.setNum(dialogGradients.count())+")";
				dialogGradients.insert(gradientName, gra);
			}
		}
		f.close();
	}
	/* File format is:
   *
   *   GIMP Gradient
   *   Name: name
   *   number_of_segments
   *   left middle right r0 g0 b0 a0 r1 g1 b1 a1 type coloring left_color_type
   *   left middle right r0 g0 b0 a0 r1 g1 b1 a1 type coloring right_color_type
   *   ...
   */
}

void ColorPickerColorList::addGimpColor(QString &colorName, double r, double g, double b)
{
	ScColor lf = ScColor();
	bool found = false;
	int Rc, Gc, Bc, hR, hG, hB;
	Rc = qRound(r * 255);
	Gc = qRound(g * 255);
	Bc = qRound(b * 255);
	lf.setColorRGB(Rc, Gc, Bc);
	for (ColorList::Iterator it = m_colorList.begin(); it != m_colorList.end(); ++it)
	{
		if (it.value().getColorModel() == colorModelRGB)
		{
			it.value().getRGB(&hR, &hG, &hB);
			if ((Rc == hR) && (Gc == hG) && (Bc == hB))
			{
				colorName = it.key();
				found = true;
				return;
			}
		}
	}
	if (!found)
	{
		m_colorList.insert(colorName, lf);
		hasImportedColors = true;
	}
}

void ColorPickerColorList::loadVectors(QString data)
{
	int storedPageNum = m_doc->currentPageNumber();
	int storedContentsX = m_doc->view()->contentsX();
	int storedContentsY = m_doc->view()->contentsY();
	double storedViewScale = m_doc->view()->scale();
	FPoint stored_minCanvasCoordinate = m_doc->minCanvasCoordinate;
	FPoint stored_maxCanvasCoordinate = m_doc->maxCanvasCoordinate;

	m_doc->PageColors = m_colorList;
	m_doc->docGradients = dialogGradients;
	UndoManager::instance()->setUndoEnabled(false);
	m_doc->setLoading(true);
	QFileInfo fi(data);
	QString patNam = fi.baseName().trimmed().simplified().replace(" ", "_");
	uint ac = m_doc->Items->count();
	uint ap = m_doc->docPatterns.count();
	bool savedAlignGrid = m_doc->SnapGrid;
	bool savedAlignGuides = m_doc->SnapGuides;
	bool savedAlignElement = m_doc->SnapElement;
	m_doc->SnapGrid = false;
	m_doc->SnapGuides = false;
	m_doc->SnapElement = false;
	if (fi.suffix().toLower() == "sce")
	{
		ScriXmlDoc ss;
		ss.ReadElem(data, PrefsManager::instance()->appPrefs.fontPrefs.AvailFonts, m_doc, m_doc->currentPage()->xOffset(), m_doc->currentPage()->yOffset(), true, true, PrefsManager::instance()->appPrefs.fontPrefs.GFontSub);
	}
	else
	{
		FileLoader *fileLoader = new FileLoader(data);
		int testResult = fileLoader->testFile();
		delete fileLoader;
		if ((testResult != -1) && (testResult >= FORMATID_FIRSTUSER))
		{
			const FileFormat * fmt = LoadSavePlugin::getFormatById(testResult);
			if( fmt )
			{
				fmt->setupTargets(m_doc, 0, m_ScMW, 0, &(PrefsManager::instance()->appPrefs.fontPrefs.AvailFonts));
				fmt->loadFile(data, LoadSavePlugin::lfUseCurrentPage|LoadSavePlugin::lfInteractive|LoadSavePlugin::lfScripted|LoadSavePlugin::lfKeepPatterns|LoadSavePlugin::lfLoadAsPattern);
			}
		}
	}
	m_doc->SnapGrid = savedAlignGrid;
	m_doc->SnapGuides = savedAlignGuides;
	m_doc->SnapElement = savedAlignElement;
	uint ae = m_doc->Items->count();
	if (ac != ae)
	{
		for (uint as = ac; as < ae; ++as)
		{
			PageItem* ite = m_doc->Items->at(ac);
			if (ite->itemType() == PageItem::PathText)
				ite->updatePolyClip();
			else
				ite->layout();
		}
		ScPattern pat = ScPattern();
		pat.setDoc(m_doc);
		PageItem* currItem = m_doc->Items->at(ac);
		double minx =  std::numeric_limits<double>::max();
		double miny =  std::numeric_limits<double>::max();
		double maxx = -std::numeric_limits<double>::max();
		double maxy = -std::numeric_limits<double>::max();
		double x1, x2, y1, y2;
		currItem->getVisualBoundingRect(&x1, &y1, &x2, &y2);
		minx = qMin(minx, x1);
		miny = qMin(miny, y1);
		maxx = qMax(maxx, x2);
		maxy = qMax(maxy, y2);
		pat.pattern = currItem->DrawObj_toImage(qMin(qMax(maxx - minx, maxy - miny), 500.0));
		pat.width = maxx - minx;
		pat.height = maxy - miny;
		currItem->setXYPos(0, 0, true);
		currItem->setWidthHeight(maxx - minx, maxy - miny, true);
		currItem->groupWidth = maxx - minx;
		currItem->groupHeight = maxy - miny;
		currItem->gWidth = maxx - minx;
		currItem->gHeight = maxy - miny;
		for (uint as = ac; as < ae; ++as)
		{
			pat.items.append(m_doc->Items->takeAt(ac));
		}
		if (!dialogPatterns.contains(patNam))
		{
			dialogPatterns.insert(patNam, pat);
			origNamesPatterns.insert(patNam, patNam);
		}
		for (QHash<QString, ScPattern>::Iterator it = m_doc->docPatterns.begin(); it != m_doc->docPatterns.end(); ++it)
		{
			if (!origPatterns.contains(it.key()))
			{
				dialogPatterns.insert(it.key(), it.value());
				origNamesPatterns.insert(it.key(), it.key());
			}
		}
	}
	else
	{
		uint ape = m_doc->docPatterns.count();
		if (ap != ape)
		{
			for (QHash<QString, ScPattern>::Iterator it = m_doc->docPatterns.begin(); it != m_doc->docPatterns.end(); ++it)
			{
				if (!origPatterns.contains(it.key()))
				{
					dialogPatterns.insert(it.key(), it.value());
					origNamesPatterns.insert(it.key(), it.key());
				}
			}
		}
	}
	m_doc->setLoading(false);
	m_colorList = m_doc->PageColors;
	dialogGradients = m_doc->docGradients;
	UndoManager::instance()->setUndoEnabled(true);

	m_doc->minCanvasCoordinate = stored_minCanvasCoordinate;
	m_doc->maxCanvasCoordinate = stored_maxCanvasCoordinate;
	m_doc->view()->setScale(storedViewScale);
	m_doc->setCurrentPage(m_doc->DocPages.at(storedPageNum));
	m_doc->view()->setContentsPos(static_cast<int>(storedContentsX * storedViewScale), static_cast<int>(storedContentsY * storedViewScale));
}
