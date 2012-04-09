/*
 * Copyright 2012 Chris Branch
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "stdafx.h"
#include "mmf2extdoc.h"
#include "ui_mmf2extdoc.h"
#include "mf2reader.h"
#include "helphierachymodel.h"
#include "aceparammodel.h"
#include "parameditor.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QDataStream>
#include <QRegExp>
#include <QDir>

MMF2ExtDoc::MMF2ExtDoc(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
	, ui(new Ui::MMF2ExtDocClass)
	, m_data(new MF2Description)
{
	ui->setupUi(this);
	ui->treePages->addAction(ui->actionAppendPage);
	ui->treePages->addAction(ui->actionInsertPage);
	ui->treePages->addAction(ui->actionDeletePage);
	resetModel();
}

MMF2ExtDoc::~MMF2ExtDoc()
{
	delete ui;
	delete m_data;
}

void MMF2ExtDoc::loadFile()
{
	QString result = QFileDialog::getOpenFileName(this,
		tr("Open a MMF2ExtDoc project file"),
		m_lastOpenedFile,
		tr("MMF2ExtDoc project (*.mmf2extdoc)"));
	if (!result.isNull()) {
		m_lastOpenedFile = result;
		QFile file(result);
		if (file.open(QIODevice::ReadOnly)) {
			QDataStream stream(&file);
			stream >> m_data->image;
			stream >> m_data->paramText;
			stream >> m_data->paramDescription;
			m_data->menu = SubMenuItem::deserialise(stream);
			foreach (QSharedPointer<MenuItem> child, m_data->menu->children) {
				if (child->type() == MenuItem::SubMenu) {
					QSharedPointer<SubMenuItem> subMenu = qSharedPointerCast<SubMenuItem>(child);
					if (subMenu->text == "Conditions")
						m_data->conditionMenu = subMenu;
					else if (subMenu->text == "Actions")
						m_data->actionMenu = subMenu;
					else if (subMenu->text == "Expressions")
						m_data->expressionMenu = subMenu;
				}
			}
			resetModel();
		} else {
			QMessageBox::critical(this, QApplication::applicationName(), tr("Could not open file: %1").arg(file.errorString()));
		}
	}
}

QByteArray readResource(QString res)
{
	QFile leadin(res);
	leadin.open(QIODevice::ReadOnly);
	return leadin.readAll();
}

QString sanitiseFileName(QString fileName)
{
	return fileName.replace(QRegExp("[" + QRegExp::escape("\\/:*?\"<>|.") + "]"), "_");
}

QString getPathForItem(QSharedPointer<MenuItem> item)
{
	if (item->displayString().isEmpty())
		return "";
	QString fileName = sanitiseFileName(item->displayString());
	if (item->parent)
		return QDir::toNativeSeparators(getPathForItem(item->parent.toStrongRef()) + "/" + fileName);
	return fileName;
}

QString getPathForItem(QSharedPointer<MenuItem> item, MenuItem *relativeTo)
{
	if (item->displayString().isEmpty())
		return "";
	QString fileName = sanitiseFileName(item->displayString());
	if (item->parent && item != relativeTo)
		return QDir::toNativeSeparators(getPathForItem(item->parent.toStrongRef(), relativeTo) + "/" + fileName);
	return fileName;
}

void writeTOC(QSharedPointer<SubMenuItem> item, QTextStream &stream)
{
	stream << QString("<LI><OBJECT type=\"text/sitemap\">");
	stream << QString("<param name=\"Name\" value=\"%1\">").arg(item->text);
	stream << QString("<param name=\"Local\" value=\"%1.htm\">").arg(getPathForItem(item));
	stream << QString("<param name=\"ImageNumber\" value=\"1\"></OBJECT>\n");
	stream << QString("<UL>\n");
	foreach (QSharedPointer<MenuItem> child, item->children) {
		switch (child->type()) {
			case MenuItem::SubMenu:
				writeTOC(qSharedPointerCast<SubMenuItem>(child), stream);
				break;
			case MenuItem::Ace:
			case MenuItem::Page:
				stream << QString("<LI><OBJECT type=\"text/sitemap\">");
				stream << QString("<param name=\"Name\" value=\"%1\">").arg(child->displayString());
				stream << QString("<param name=\"Local\" value=\"%1.htm\"></OBJECT>").arg(getPathForItem(child));
				break;
			case MenuItem::Separator:
				stream << "<HR>\n";
				break;
		}
	}
	stream << QString("</UL>\n");
}

void writeFileList(QSharedPointer<SubMenuItem> item, QTextStream &stream)
{
	stream << QString("%1.htm\n").arg(getPathForItem(item));
	foreach (QSharedPointer<MenuItem> child, item->children) {
		switch (child->type()) {
			case MenuItem::SubMenu:
				writeFileList(qSharedPointerCast<SubMenuItem>(child), stream);
				break;
			case MenuItem::Ace:
			case MenuItem::Page:
				stream << QString("%1.htm\n").arg(getPathForItem(child));
				break;
		}
	}
}

QString levelsToRoot(QSharedPointer<MenuItem> item)
{
	if (item->parent)
		return QString("../") + levelsToRoot(item->parent.toStrongRef());
	return QString();
}

void writeFiles(QSharedPointer<MenuItem> item, MF2Description &desc, QDir outDir)
{
	if (item->type() == MenuItem::Separator)
		return;
	QFileInfo info(outDir, getPathForItem(item) + ".htm");
	outDir.mkpath(info.path());
	QFile file(info.filePath());
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QTextStream stream(&file);
		stream << readResource(":/templates/html_header");
		stream << QString("<title>%1</title>").arg(item->displayString());
		stream << QString("<link rel=\"stylesheet\" href=\"%1style.css\" type=\"text/css\">").arg(levelsToRoot(item));
		stream << "</head>\n<body>\n";
		stream << QString("<h1>%1</h1>\n").arg(item->displayString());
		item->exportHtml(stream, desc);
		stream << readResource(":/templates/html_footer");
		if (item->type() == MenuItem::SubMenu) {
			foreach (QSharedPointer<MenuItem> child, qSharedPointerCast<SubMenuItem>(item)->children) {
				writeFiles(child, desc, outDir);
			}
		}
	}
}

void SubMenuItem::exportHtml(QTextStream &stream, MF2Description &desc)
{
	PageMenuItem::exportHtml(stream, desc);
	stream << "<UL>\n";
	foreach (QSharedPointer<MenuItem> child, children) {
		if (child->type() != MenuItem::Separator)
			stream << QString("<LI><A HREF=\"%1.htm\">%2</A>").arg(getPathForItem(child, this)).arg(child->displayString());
		else
			stream << "<HR>";
	}
	stream << "</UL>\n";
}

void PageMenuItem::exportHtml(QTextStream &stream, MF2Description &desc)
{
	stream << content;
}

void ACEMenuItem::exportHtml(QTextStream &stream, MF2Description &desc)
{
	QString type;
	if (ace.type == ACE::Condition) {
		if (ace.isTrueEvent())
			type = "Event";
		else if (ace.isNotable())
			type = "Negatable condition";
		else
			type = "Condition";
	} else if (ace.type == ACE::Expression) {
		if (ace.returnInt()) {
			type = "Integer expression";
		} else if (ace.returnDouble()) {
			type = "Decimal expression";
		} else {
			type = "String expression";
		}
	} else {
		type = "Action";
	}
	stream << QString("<h2>%1</h2>\n").arg(type);

	int splitPoint = ace.text.indexOf("\n");
	QString objectText = ace.text.left(splitPoint);
	objectText.replace(QString("%o"), QString("(%1)").arg(desc.getExtName()));
	for (int i = 0; i < ace.params.size(); i++) {
		QString paramText = QString("<a href=\"#%1\">(%2)</a>").arg(i).arg(desc.paramText.value(ace.params.at(i).id));
		objectText = objectText.arg(paramText);
	}
	stream << QString("<div class=\"eventEditorText\">%1</div>\n").arg(objectText);
	if (splitPoint != -1)
		stream << ace.text.mid(splitPoint);

	stream << "<ul>\n";
	for (int i = 0; i < ace.params.size(); i++) {
		const MMFParam &param = ace.params.at(i);
		QString paramTitle = desc.paramText.value(param.id);
		QString paramDesc = param.description;
		if (paramDesc.isEmpty())
			paramDesc = desc.paramDescription.value(param.id);
		stream << QString("<li><h3><a name=\"%1\"></a>(%2) %3</h3>\n<p>%4</p>\n").arg(i).arg(param.type).arg(paramTitle).arg(paramDesc);
	}
	stream << "</ul>\n";
}

void MMF2ExtDoc::saveFile()
{
	QString result = QFileDialog::getSaveFileName(this,
		tr("Save a MMF2ExtDoc project file"),
		m_lastOpenedFile,
		tr("MMF2ExtDoc project (*.mmf2extdoc)"));
	if (!result.isNull()) {
		m_lastOpenedFile = result;
		QFileInfo fileInfo(result);
		fileInfo.dir().mkpath(fileInfo.baseName() + "_files");
		QFile file(result);
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			QDataStream stream(&file);
			stream << m_data->image;
			stream << m_data->paramText;
			stream << m_data->paramDescription;
			m_data->menu->serialise(stream);
			QDir outDir(fileInfo.dir());
			outDir.cd(fileInfo.baseName() + "_files");
			{
				QFile file(outDir.filePath("style.css"));
				if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					QTextStream stream(&file);
					stream << readResource(":/templates/stylesheet");
				}
			}
			m_data->image.save(outDir.filePath("icon.png"), "PNG");
			{
				QFile file(outDir.filePath("toc.hhc"));
				if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					QTextStream stream(&file);
					stream << readResource(":/templates/toc_header");
					stream << "<UL>\n";
					writeTOC(m_data->menu, stream);
					stream << "</UL></BODY></HTML>\n";
				}
			}
			{
				QFile file(outDir.filePath("project.hhp"));
				if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					QTextStream stream(&file);
					QString title(m_data->menu->text);
					QString indexPage(getPathForItem(m_data->menu) + ".htm");
					stream << readResource(":/templates/project_header");
					stream << QString("Default topic=%1\n").arg(indexPage);
					stream << QString("Title=%1\n").arg(title);
					stream << QString("\n[WINDOWS]\n");
					stream << QString("Default=\"%1\",\"toc.hhc\",\"index.hhk\",\"%2\",\"%3\",,,,,0x62420,,0x384e,[0,0,640,480],,,,,,,0\n").arg(title).arg(indexPage).arg(indexPage);
					stream << "\n[FILES]\n";
					stream << "icon.png\nstyle.css\n";
					writeFileList(m_data->menu, stream);
					stream << QString("\n[INFOTYPES]\n");
				}
			}
			writeFiles(m_data->menu, *m_data, outDir);
		} else {
			QMessageBox::critical(this, QApplication::applicationName(), tr("Could not open file: %1").arg(file.errorString()));
		}
	}
}

void MMF2ExtDoc::importExtension()
{
	QString result = QFileDialog::getOpenFileName(this,
		tr("Import an MMF2 extension"),
		m_lastOpenedFile,
		tr("MMF2 extension (*.mfx)"));
	if (!result.isNull()) {
		*m_data = MF2Description::fromFile(result);
		resetModel();
	}
}

void MMF2ExtDoc::showPage(const QModelIndex &index)
{
	QSharedPointer<MenuItem> item = static_cast<HelpHierachyModel*>(ui->treePages->model())->getItem(index);
	if (!item) {
		return;
	} else if (item->type() == MenuItem::Ace) {
		QSharedPointer<ACEMenuItem> aceItem = qSharedPointerCast<ACEMenuItem>(item);
		ui->textEdit->setEnabled(true);
		ui->listParams->setEnabled(true);
		ui->listParams->setVisible(true);
		ui->textEdit->setPlainText(aceItem->ace.text);
		ui->listParams->setModel(new ACEParamModel(aceItem->ace, m_data->paramText, m_data->paramDescription, ui->listParams));
	} else if (item->type() == MenuItem::Page || item->type() == MenuItem::SubMenu) {
		QSharedPointer<PageMenuItem> pageItem = qSharedPointerCast<PageMenuItem>(item);
		ui->textEdit->setEnabled(true);
		ui->listParams->setEnabled(false);
		ui->listParams->setVisible(false);
		ui->textEdit->setPlainText(pageItem->content);
	} else {
		ui->textEdit->setEnabled(false);
		ui->listParams->setEnabled(false);
	}
}

void MMF2ExtDoc::updatePageText()
{
	QSharedPointer<MenuItem>item = static_cast<HelpHierachyModel*>(ui->treePages->model())->getItem(ui->treePages->currentIndex());
	switch (item->type()) {
		case MenuItem::Ace:
			{
				QSharedPointer<ACEMenuItem> aceItem = qSharedPointerCast<ACEMenuItem>(item);
				aceItem->ace.text = ui->textEdit->toPlainText();
			}
			break;
		case MenuItem::SubMenu:
		case MenuItem::Page:
			{
				QSharedPointer<PageMenuItem> pageItem = qSharedPointerCast<PageMenuItem>(item);
				pageItem->content = ui->textEdit->toPlainText();
			}
	}
}

void MMF2ExtDoc::editParams()
{
	ParamEditor editor(this);
	editor.setDescription(m_data);
	if (editor.exec() == QDialog::Accepted) {
		showPage(ui->treePages->currentIndex());
	}
}

void MMF2ExtDoc::insertPage()
{
	HelpHierachyModel *model = static_cast<HelpHierachyModel *>(ui->treePages->model());
	QModelIndex index = ui->treePages->currentIndex();
	model->insertItem(index, QSharedPointer<MenuItem>(new PageMenuItem(tr("New page"))));
}

void MMF2ExtDoc::appendPage()
{
	HelpHierachyModel *model = static_cast<HelpHierachyModel *>(ui->treePages->model());
	QModelIndex index = ui->treePages->currentIndex();
	QSharedPointer<MenuItem> item = model->getItem(index);
	if (item && item->type() != MenuItem::SubMenu)
		index = index.parent();
	model->appendItem(index, QSharedPointer<MenuItem>(new PageMenuItem(tr("New page"))));
}

void MMF2ExtDoc::deletePage()
{
	QModelIndex index(ui->treePages->currentIndex());
	ui->treePages->model()->removeRows(index.row(), 1, index.parent());
}

void MMF2ExtDoc::resetModel()
{
	ui->objectName->setText(m_data->menu->text);
	ui->treePages->setModel(new HelpHierachyModel(m_data->menu, ui->treePages));
	connect(ui->treePages->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(showPage(QModelIndex)));
	showPage(QModelIndex());
}

void MMF2ExtDoc::setExtensionName(QString newName)
{
	m_data->menu->text = newName;
}