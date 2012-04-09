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
#include "parameditormodel.h"

ParamEditorModel::ParamEditorModel(QMap<int, QString> paramText, QMap<int, QString> paramDescription, QObject *parent)
	: QAbstractListModel(parent)
{
	for (QMap<int, QString>::iterator i = paramText.begin(); i != paramText.end(); i++) {
		ParamData pd;
		pd.id = i.key();
		pd.text = i.value();
		pd.desc = paramDescription.value(i.key());
		pd.overwriteDesc = false;
		params.append(pd);
	}
}

ParamEditorModel::~ParamEditorModel()
{

}

int ParamEditorModel::columnCount ( const QModelIndex & parent ) const
{
	return 3;
}

int ParamEditorModel::rowCount ( const QModelIndex & parent ) const
{
	return params.size();
}

QVariant ParamEditorModel::data ( const QModelIndex & index, int role ) const
{
	if (index.isValid() && index.row() >= 0 && index.row() < params.size()) {
		const ParamData &param = params.at(index.row());
		switch (role) {
			case Qt::DisplayRole:
			case Qt::EditRole:
				switch (index.column()) {
					case 0:
						return param.id;
					case 1:
						return param.text;
					case 2:
						return param.desc;
				}
				break;
			case Qt::CheckStateRole:
				switch (index.column()) {
					case 2:
						return param.overwriteDesc ? Qt::Checked : Qt::Unchecked;
				}
				break;
		}
	}
	return QVariant();
}

Qt::ItemFlags ParamEditorModel::flags ( const QModelIndex & index) const
{
	Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	if (index.column() > 0)
		flags |= Qt::ItemIsEditable;
	if (index.column() == 2)
		flags |= Qt::ItemIsUserCheckable;
	return flags;
}

QVariant ParamEditorModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
	switch (role) {
		case Qt::DisplayRole:
			switch (section) {
				case 0:
					return tr("Type");
				case 1:
					return tr("Title");
				case 2:
					return tr("Description");
			}
	}
	return QVariant();
}

bool ParamEditorModel::setData ( const QModelIndex & index, const QVariant & value, int role)
{
	bool changed = false;
	if (index.isValid() && index.row() >= 0 && index.row() < params.size()) {
		ParamData &param = params[index.row()];
		switch (role) {
			case Qt::EditRole:
				switch (index.column()) {
					case 1:
						param.text = value.toString();
						changed = true;
						break;
					case 2:
						param.desc = value.toString();
						changed = true;
						break;
				}
				break;
			case Qt::CheckStateRole:
				switch (index.column()) {
					case 2:
						param.overwriteDesc = value.toInt() == Qt::Checked;
						changed = true;
						break;
				}
				break;
		}
	}
	if (changed) {
		emit dataChanged(index, index);
		return true;
	}
	return false;
}