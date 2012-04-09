#ifndef PARAMEDITORMODEL_H
#define PARAMEDITORMODEL_H

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

struct ParamData
{
	int id;
	QString text;
	QString desc;
	bool overwriteDesc;
};

class ParamEditorModel : public QAbstractListModel
{
	Q_OBJECT

public:
	ParamEditorModel(QMap<int, QString> paramText, QMap<int, QString> paramDescription, QObject *parent);
	~ParamEditorModel();

	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	Qt::ItemFlags flags ( const QModelIndex & index) const;
	QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	QList<ParamData> getParamData() const { return params; }

private:
	QList<ParamData> params;
	
};

#endif // PARAMEDITORMODEL_H
