#ifndef HELPHIERACHYMODEL_H
#define HELPHIERACHYMODEL_H

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

#include <QAbstractItemModel>
#include <QSharedPointer>
class MenuItem;
class SubMenuItem;

class HelpHierachyModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	HelpHierachyModel(QSharedPointer<SubMenuItem> root, QObject *parent);
	~HelpHierachyModel();

	Qt::ItemFlags flags ( const QModelIndex & index ) const;
	QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
	int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	QModelIndex parent ( const QModelIndex & index ) const;
	QSharedPointer<MenuItem> getItem ( const QModelIndex & index ) const;
	void appendItem (const QModelIndex &parent, QSharedPointer<MenuItem> item);
	void insertItem (const QModelIndex &before, QSharedPointer<MenuItem> item);
	bool setData ( const QModelIndex & index, const QVariant & value, int role );
	Qt::DropActions supportedDragActions () const;
	bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
	bool setItemData ( const QModelIndex & index, const QMap<int, QVariant> & roles );
	QMap<int, QVariant> itemData ( const QModelIndex & index ) const;
	QStringList mimeTypes () const;
	QMimeData * mimeData ( const QModelIndexList & indexes ) const;
	bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );

private:
	QSharedPointer<SubMenuItem> m_root;
};

#endif // HELPHIERACHYMODEL_H
