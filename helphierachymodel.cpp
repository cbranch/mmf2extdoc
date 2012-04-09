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
#include "helphierachymodel.h"
#include "mf2reader.h"
#include <QWeakPointer>

HelpHierachyModel::HelpHierachyModel(QSharedPointer<SubMenuItem> root, QObject *parent)
	: QAbstractItemModel(parent)
	, m_root(root)
{

}

HelpHierachyModel::~HelpHierachyModel()
{

}

Qt::ItemFlags HelpHierachyModel::flags ( const QModelIndex & index ) const
{
	Qt::ItemFlags f = Qt::ItemIsDropEnabled;
	if (index.isValid()) {
		f |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		if (index.parent().isValid()) {
			f |= Qt::ItemIsDragEnabled;
		}
		if (index.column() == 0 && getItem(index)->hasEditableString()) {
			f |= Qt::ItemIsEditable;
		}
	}
	return f;
}

QVariant HelpHierachyModel::data ( const QModelIndex & index, int role ) const
{
	QSharedPointer<MenuItem> item = getItem(index);
	switch (role) {
		case Qt::DisplayRole:
		case Qt::EditRole:
			return item->displayString();
		case Qt::UserRole:
			return QVariant::fromValue(item);
	}
	return QVariant();
}

QVariant HelpHierachyModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
	return QVariant();
}

int HelpHierachyModel::rowCount ( const QModelIndex & parent ) const
{
	if (parent.isValid())
		if (getItem(parent)->type() == MenuItem::SubMenu)
			return qSharedPointerCast<SubMenuItem>(getItem(parent))->children.size();
		else
			return 0;
	else
		return 1;
}

int HelpHierachyModel::columnCount ( const QModelIndex & parent ) const
{
	return 1;
}

QModelIndex HelpHierachyModel::index ( int row, int column, const QModelIndex & parent ) const
{
	QSharedPointer<SubMenuItem> subItem;
	if (!parent.isValid()) {
		if (row == 0)
			return createIndex(row, column, (void *)&m_root);
	} else {
		QSharedPointer<MenuItem> item = getItem(parent);
		if (item->type() == MenuItem::SubMenu) {
			subItem = qSharedPointerCast<SubMenuItem>(item);
		}
	}
	if (subItem && row >= 0 && row < subItem->children.size())
		return createIndex(row, column, (void *)&subItem->children.at(row));
	return QModelIndex();
}

QModelIndex HelpHierachyModel::parent ( const QModelIndex & index ) const
{
	if (index.isValid()) {
		QWeakPointer<MenuItem> parent = getItem(index)->parent.toStrongRef();
		if (!parent)
			return QModelIndex();
		QWeakPointer<MenuItem> grandparent = parent.data()->parent;
		if (!grandparent) {
			return createIndex(0, 0, (void*)&m_root);
		} else {
			// If the item doesn't have the root as its parent, then it must have a grandparent
			QList<QSharedPointer<MenuItem> > &list = grandparent.toStrongRef().staticCast<SubMenuItem>()->children;
			int parentRow = list.indexOf(parent);
			return createIndex(parentRow, 0, &list[parentRow]);
		}
	} else {
		return QModelIndex();
	}
}

QSharedPointer<MenuItem> HelpHierachyModel::getItem ( const QModelIndex & index ) const
{
	if (index.isValid()) {
		QSharedPointer<MenuItem> item = *(QSharedPointer<MenuItem> *)index.internalPointer();
		if (item->parent) {
			return item->parent.toStrongRef().staticCast<SubMenuItem>()->children.at(index.row());
		} else {
			return m_root;
		}
	}
	return QSharedPointer<MenuItem>();
}

bool HelpHierachyModel::removeRows ( int row, int count, const QModelIndex & parent )
{
	QSharedPointer<SubMenuItem> item = qSharedPointerCast<SubMenuItem>(getItem(parent));
	if (item.isNull() || row < 0 || row >= item->children.size())
		return false;
	beginRemoveRows(parent, row, row + count - 1);
	while (count-- > 0) {
		item->children.removeAt(row);
	}
	endRemoveRows();
	return true;
}

void HelpHierachyModel::appendItem (const QModelIndex &parent, QSharedPointer<MenuItem> item)
{
	QSharedPointer<SubMenuItem> subMenu = qSharedPointerCast<SubMenuItem>(getItem(parent));
	if (subMenu.isNull())
		return;
	beginInsertRows(parent, subMenu->children.size(), subMenu->children.size());
	item->parent = subMenu;
	subMenu->children.append(item);
	endInsertRows();
}

void HelpHierachyModel::insertItem (const QModelIndex &before, QSharedPointer<MenuItem> item)
{
	QModelIndex parent(before.parent());
	QSharedPointer<SubMenuItem> subMenu = qSharedPointerCast<SubMenuItem>(getItem(parent));
	if (subMenu.isNull())
		return;
	beginInsertRows(parent, before.row(), before.row());
	item->parent = subMenu;
	subMenu->children.insert(before.row(), item);
	endInsertRows();
}

bool HelpHierachyModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
	QSharedPointer<MenuItem> item = getItem(index);
	if (item) {
		switch (role) {
			case Qt::EditRole:
				// Name of the item edited
				if (index.column() == 0 && item->hasEditableString()) {
					QSharedPointer<StringMenuItem> stringItem = qSharedPointerCast<StringMenuItem>(item);
					stringItem->text = value.toString();
					emit dataChanged(index, index);
					return true;
				}
				break;

			case Qt::UserRole:
				// don't support replacing the top level item
				if (index.parent().isValid()) {
					QSharedPointer<SubMenuItem> parentItem = qSharedPointerCast<SubMenuItem>(getItem(index.parent()));
					Q_ASSERT(!parentItem.isNull() && parentItem->type() == MenuItem::SubMenu);
					Q_ASSERT(index.row() < parentItem->children.size());
					QSharedPointer<MenuItem> newItem = value.value<QSharedPointer<MenuItem> >();
					// If we are a sub-menu item, we need to remove ourselves!
					if (item->type() == MenuItem::SubMenu) {
						QSharedPointer<SubMenuItem> subItem = qSharedPointerCast<SubMenuItem>(item);
						beginRemoveRows(index, 0, subItem->children.size() - 1);
						subItem->children.clear();
						endRemoveRows();
					}
					parentItem->children[index.row()] = newItem->deepCopy(parentItem);
					emit dataChanged(index, index);
					// If this is now a sub-menu, insert the children
					//if (!newItem.isNull() && newItem->type() == MenuItem::SubMenu) {
					//	QSharedPointer<SubMenuItem> subItem = qSharedPointerCast<SubMenuItem>(newItem);
					//}
				}
				break;
		}
	}
	return false;
}

Qt::DropActions HelpHierachyModel::supportedDragActions () const
{
	return Qt::MoveAction | Qt::CopyAction;
}

bool HelpHierachyModel::insertRows ( int row, int count, const QModelIndex & parent )
{
	QSharedPointer<SubMenuItem> item = qSharedPointerCast<SubMenuItem>(getItem(parent));
	if (item.isNull() || item->type() != MenuItem::SubMenu)
		return false;
	if (row < 0 || row > item->children.size())
		return false;
	beginInsertRows(parent, row, row + count - 1);
	while (count-- > 0) {
		item->children.insert(row, QSharedPointer<MenuItem>(new SeparatorMenuItem(item)));
	}
	endInsertRows();
	return true;
}

bool HelpHierachyModel::setItemData ( const QModelIndex & index, const QMap<int, QVariant> & roles )
{
	// if UserRole is in the map, do this first since it overwrites all other data
	QMap<int, QVariant> newRoles = roles;
	QMap<int, QVariant>::iterator iter = newRoles.find(Qt::UserRole);
	if (iter != roles.end()) {
		setData(index, iter.value(), iter.key());
		newRoles.erase(iter);
	}
	return QAbstractItemModel::setItemData(index, newRoles);
}

QMap<int, QVariant> HelpHierachyModel::itemData ( const QModelIndex & index ) const
{
	QMap<int, QVariant> items = QAbstractItemModel::itemData(index);
	items[Qt::UserRole] = data(index, Qt::UserRole);
	return items;
}

QStringList HelpHierachyModel::mimeTypes () const
{
	return QAbstractItemModel::mimeTypes();
}

QMimeData * HelpHierachyModel::mimeData ( const QModelIndexList & indexes ) const
{
	return QAbstractItemModel::mimeData(indexes);
}

bool HelpHierachyModel::dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
	return  QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}