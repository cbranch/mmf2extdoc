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
#include "StdAfx.h"
#include "parameditor.h"
#include "ui_parameditor.h"
#include "parameditormodel.h"
#include "mf2reader.h"

ParamEditor::ParamEditor(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::ParamEditor();
	ui->setupUi(this);
}

ParamEditor::~ParamEditor()
{
	delete ui;
}

void ParamEditor::setDescription(MF2Description *desc)
{
	m_desc = desc;
	ui->treeView->setModel(new ParamEditorModel(desc->paramText, desc->paramDescription, ui->treeView));
}

void updateMenuItems(QSharedPointer<SubMenuItem> menu, QMap<int, QString> &newDesc)
{
	foreach (QSharedPointer<MenuItem> item, menu->children) {
		switch (item->type()) {
			case MenuItem::SubMenu:
				updateMenuItems(qSharedPointerCast<SubMenuItem>(item), newDesc);
				break;
			case MenuItem::Ace:
				QList<MMFParam> &params = qSharedPointerCast<ACEMenuItem>(item)->ace.params;
				for (QList<MMFParam>::iterator iter = params.begin(); iter != params.end(); iter++) {
					QMap<int, QString>::iterator descIter = newDesc.find(iter->id);
					if (descIter != newDesc.end()) {
						iter->description = descIter.value();
					}
				}
				break;
		}
	}
}

void ParamEditor::accept()
{
	QList<ParamData> data = static_cast<ParamEditorModel *>(ui->treeView->model())->getParamData();
	QMap<int, QString> overwriteDesc;
	foreach (const ParamData &d, data) {
		m_desc->paramText[d.id] = d.text;
		m_desc->paramDescription[d.id] = d.desc;
		if (d.overwriteDesc)
			overwriteDesc[d.id] = d.desc;
	}
	updateMenuItems(m_desc->menu, overwriteDesc);

	QDialog::accept();
}