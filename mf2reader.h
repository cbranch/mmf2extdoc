#ifndef MF2READER_H
#define MF2READER_H

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

#include <QSharedPointer>
#include <QWeakPointer>
#include <QString>
#include <QMap>
#include <QList>
#include <QDataStream>
#include <QTextStream>

class MF2Description;

struct MMFParam {
	QString type;
	int id;
	QString description;
};

QDataStream & operator<< (QDataStream &stream, const MMFParam &item);
QDataStream & operator>> (QDataStream &stream, MMFParam &item);

class ACE {
public:
	QString text;
	short menuID;
	short flags;
	QList<MMFParam> params;
	enum Type {
		Action,
		Condition,
		Expression
	} type;

	bool returnInt() const { return (flags & 0x3) == 0; }
	bool returnString() const { return (flags & 0x3) == 1; }
	bool returnDouble() const { return (flags & 0x3) == 2; }
	bool isTrueEvent() const { return !(flags & 0x20); }
	bool isAlways() const { return (flags & 0x20); }
	bool isNotable() const { return (flags & 0x200); }
};

class SubMenuItem;

class MenuItem
{
public:
	MenuItem(QWeakPointer<MenuItem> parent) : parent(parent) {}

	QWeakPointer<MenuItem> parent;
	enum Type {
		SubMenu,
		Separator,
		String,
		Ace,
		Page,
	};

	virtual Type type() const = 0;
	virtual QSharedPointer<MenuItem> instantiate(QWeakPointer<MenuItem> newParent) const = 0;
	virtual QSharedPointer<MenuItem> deepCopy(QWeakPointer<MenuItem> newParent) const {
		return instantiate(newParent);
	}
	virtual QString displayString() const = 0;
	virtual bool hasEditableString() const { return false; }
	virtual void serialise(QDataStream &stream) { stream << type(); }
	virtual void exportHtml(QTextStream &stream, MF2Description &desc) {}
};

Q_DECLARE_METATYPE(QSharedPointer<MenuItem>)

QDataStream & operator<< (QDataStream &stream, const QSharedPointer<MenuItem> &item);
QDataStream & operator>> (QDataStream &stream, QSharedPointer<MenuItem> &item);

class StringMenuItem : public MenuItem
{
public:
	StringMenuItem(QString text, QWeakPointer<MenuItem> parent) : MenuItem(parent), text(text) {}

	QString text;
	virtual Type type() const { return String; }
	virtual QSharedPointer<MenuItem> instantiate(QWeakPointer<MenuItem> newParent) const {
		return QSharedPointer<MenuItem>(new StringMenuItem(text, parent));
	}
	virtual QSharedPointer<MenuItem> deepCopy(QWeakPointer<MenuItem> newParent) const {
		QSharedPointer<StringMenuItem> item =
			MenuItem::deepCopy(newParent).staticCast<StringMenuItem>();
		//item->text = text; // done in instantiate
		return item;
	}
	virtual QString displayString() const { return text; }
	virtual bool hasEditableString() const { return true; }
	virtual void serialise(QDataStream &stream) {
		MenuItem::serialise(stream);
		stream << text;
	}
	static QSharedPointer<MenuItem> deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent);
};

class SeparatorMenuItem : public MenuItem
{
public:
	SeparatorMenuItem(QWeakPointer<MenuItem> parent) : MenuItem(parent) {}

	virtual Type type() const { return Separator; }
	virtual QSharedPointer<MenuItem> instantiate(QWeakPointer<MenuItem> newParent) const {
		return QSharedPointer<MenuItem>(new SeparatorMenuItem(newParent));
	}
	virtual QString displayString() const { return "-----"; }
	static QSharedPointer<MenuItem> deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent);
};

class ACEMenuItem : public StringMenuItem
{
public:
	ACEMenuItem(ACE ace, QString text, QWeakPointer<MenuItem> parent) : StringMenuItem(text, parent), ace(ace) {}

	ACE ace;
	virtual Type type() const { return Ace; }
	virtual QSharedPointer<MenuItem> instantiate(QWeakPointer<MenuItem> newParent) const {
		return QSharedPointer<MenuItem>(new ACEMenuItem(ace, text, newParent));
	}
	virtual QSharedPointer<MenuItem> deepCopy(QWeakPointer<MenuItem> newParent) const {
		QSharedPointer<ACEMenuItem> item =
			StringMenuItem::deepCopy(newParent).staticCast<ACEMenuItem>();
		//item->ace = ace; // done in instantiate
		return item;
	}
	virtual void serialise(QDataStream &stream) {
		StringMenuItem::serialise(stream);
		stream << ace.text;
		stream << ace.menuID;
		stream << ace.flags;
		stream << ace.params;
		stream << (int)ace.type;
	}
	virtual void exportHtml(QTextStream &stream, MF2Description &desc);
	static QSharedPointer<MenuItem> deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent);
};

class PageMenuItem : public StringMenuItem
{
public:
	PageMenuItem(QString text, QWeakPointer<MenuItem> parent = QWeakPointer<MenuItem>()) : StringMenuItem(text, parent) {}

	QString content;
	virtual Type type() const { return Page; }
	virtual QSharedPointer<MenuItem> instantiate(QWeakPointer<MenuItem> newParent) const {
		return QSharedPointer<MenuItem>(new PageMenuItem(text, newParent));
	}
	virtual QSharedPointer<MenuItem> deepCopy(QWeakPointer<MenuItem> newParent) const {
		QSharedPointer<PageMenuItem> item =
			StringMenuItem::deepCopy(newParent).staticCast<PageMenuItem>();
		item->content = content;
		return item;
	}
	virtual void serialise(QDataStream &stream) {
		StringMenuItem::serialise(stream);
		stream << content;
	}
	virtual void exportHtml(QTextStream &stream, MF2Description &desc);
	static QSharedPointer<MenuItem> deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent);
};

class SubMenuItem : public PageMenuItem
{
public:
	SubMenuItem(QString text = QString(), QWeakPointer<MenuItem> parent = QWeakPointer<MenuItem>()) : PageMenuItem(text, parent) {}

	QList<QSharedPointer<MenuItem> > children;
	virtual Type type() const { return SubMenu; }
	virtual QSharedPointer<MenuItem> instantiate(QWeakPointer<MenuItem> newParent) const {
		return QSharedPointer<MenuItem>(new SubMenuItem(text, newParent));
	}
	virtual QSharedPointer<MenuItem> deepCopy(QWeakPointer<MenuItem> newParent) const {
		QSharedPointer<SubMenuItem> item =
			PageMenuItem::deepCopy(newParent).staticCast<SubMenuItem>();
		foreach (const QSharedPointer<MenuItem> &child, children) {
			item->children.append(child->deepCopy(item));
		}
		return item;
	}
	virtual void serialise(QDataStream &stream) {
		PageMenuItem::serialise(stream);
		stream << children.size();
		foreach (QSharedPointer<MenuItem> child, children) {
			child->serialise(stream);
		}
	}
	virtual void exportHtml(QTextStream &stream, MF2Description &desc);
	static QSharedPointer<SubMenuItem> deserialise(QDataStream &stream)
	{
		int type;
		stream >> type;
		Q_ASSERT(type == SubMenu);
		return qSharedPointerCast<SubMenuItem>(deserialise(stream, QSharedPointer<SubMenuItem>()));
	}
	static QSharedPointer<MenuItem> deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent);
};

class MF2Description {
public:
	MF2Description() : menu(new SubMenuItem) { menu->text = "New Object"; }

	QMap<int, QString> paramText;
	QMap<int, QString> paramDescription;
	QSharedPointer<SubMenuItem> conditionMenu;
	QSharedPointer<SubMenuItem> actionMenu;
	QSharedPointer<SubMenuItem> expressionMenu;
	QSharedPointer<SubMenuItem> menu;
	QImage image;

	QString getExtName() { return menu->text; }

	static MF2Description fromFile(QString file);
};

#endif // MF2READER_H