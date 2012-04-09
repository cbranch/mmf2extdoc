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
#include "aceparammodel.h"
#include "mf2reader.h"

ACEParamModel::ACEParamModel(ACE &ace, const QMap<int, QString> &paramTitles, const QMap<int, QString> &paramDesc, QObject *parent)
	: QAbstractListModel(parent)
	, m_ace(ace)
	, m_paramTitles(paramTitles)
	, m_paramDesc(paramDesc)
{

}

ACEParamModel::~ACEParamModel()
{

}

int ACEParamModel::columnCount ( const QModelIndex & parent ) const
{
	return 3;
}

int ACEParamModel::rowCount ( const QModelIndex & parent ) const
{
	return m_ace.params.size();
}

QVariant ACEParamModel::data ( const QModelIndex & index, int role ) const
{
	if (index.isValid() && index.row() >= 0 && index.row() < m_ace.params.size()) {
		const MMFParam &param = m_ace.params.at(index.row());
		switch (role) {
			case Qt::DisplayRole:
			case Qt::EditRole:
				switch (index.column()) {
					case 0:
						return param.type;
					case 1:
						return m_paramTitles.value(param.id);
					case 2:
						if (!param.description.isEmpty() || role == Qt::EditRole)
							return param.description;
						return m_paramDesc.value(param.id);
				}
				break;
		}
	}
	return QVariant();
}

Qt::ItemFlags ACEParamModel::flags ( const QModelIndex & index) const
{
	return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

QVariant ACEParamModel::headerData ( int section, Qt::Orientation orientation, int role ) const
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

bool ACEParamModel::setData ( const QModelIndex & index, const QVariant & value, int role)
{
	if (index.isValid() && index.row() >= 0 && index.row() < m_ace.params.size()) {
		MMFParam &param = m_ace.params[index.row()];
		switch (role) {
			case Qt::EditRole:
				switch (index.column()) {
					case 0:
						param.type = value.toString();
						emit dataChanged(index, index);
						return true;
					case 2:
						param.description = value.toString();
						emit dataChanged(index, index);
						return true;
				}
		}
	}
	return false;
}