#ifndef ACEPARAMMODEL_H
#define ACEPARAMMODEL_H

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

#include <QAbstractListModel>
class ACE;

/*
 * A model containing each of the action's parameters with editable titles and descriptions.
 * A reference to the global title/description repository is taken so that default strings can be
 * displayed when no title is provided for an action.
 */
class ACEParamModel : public QAbstractListModel
{
	Q_OBJECT

public:
	ACEParamModel(ACE &ace, const QMap<int, QString> &paramTitles, const QMap<int, QString> &paramDesc, QObject *parent);
	~ACEParamModel();

	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	Qt::ItemFlags flags ( const QModelIndex & index) const;
	QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

private:
	ACE &m_ace;
	const QMap<int, QString> &m_paramTitles;
	const QMap<int, QString> &m_paramDesc;
};

#endif // ACEPARAMMODEL_H
