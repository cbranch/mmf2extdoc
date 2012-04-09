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
#include "mf2reader.h"
#include <windows.h>
#include "mmfstructs.h"
#include <tchar.h>
#include <QMessageBox>

using namespace std;

QDataStream & operator<< (QDataStream &stream, const MMFParam &item)
{
	stream << item.type;
	stream << item.id;
	stream << item.description;
	return stream;
}

QDataStream & operator>> (QDataStream &stream, MMFParam &item)
{
	stream >> item.type;
	stream >> item.id;
	stream >> item.description;
	return stream;
}

typedef QSharedPointer<MenuItem> (*DeserialiserFunction)(QDataStream &stream, QSharedPointer<SubMenuItem> parent);

DeserialiserFunction deserialisers[] = {
	&SubMenuItem::deserialise,
	&SeparatorMenuItem::deserialise,
	&StringMenuItem::deserialise,
	&ACEMenuItem::deserialise,
	&PageMenuItem::deserialise,
};

QSharedPointer<MenuItem> deserialiseUnknown(QDataStream &stream, QSharedPointer<SubMenuItem> parent)
{
	int type;
	stream >> type;
	return deserialisers[(int)type](stream, parent);
}

QSharedPointer<MenuItem> SeparatorMenuItem::deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent)
{
	return QSharedPointer<MenuItem>(new SeparatorMenuItem(parent));
}

QSharedPointer<MenuItem> StringMenuItem::deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent)
{
	QString text;
	stream >> text;
	return QSharedPointer<MenuItem>(new StringMenuItem(text, parent));
}

QSharedPointer<MenuItem> ACEMenuItem::deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent)
{
	ACE ace;
	QString text;
	stream >> text;
	stream >> ace.text;
	stream >> ace.menuID;
	stream >> ace.flags;
	stream >> ace.params;
	int type;
	stream >> type;
	ace.type = (ACE::Type)type;
	return QSharedPointer<MenuItem>(new ACEMenuItem(ace, text, parent));
}

QSharedPointer<MenuItem> PageMenuItem::deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent)
{
	QString text, content;
	stream >> text >> content;
	PageMenuItem *item = new PageMenuItem(text, parent);
	item->content = content;
	return QSharedPointer<MenuItem>(item);
}

QSharedPointer<MenuItem> SubMenuItem::deserialise(QDataStream &stream, QSharedPointer<SubMenuItem> parent)
 {
	QSharedPointer<SubMenuItem> subMenu(new SubMenuItem);
	stream >> subMenu->text >> subMenu->content;
	subMenu->parent = parent;
	int count;
	stream >> count;
	while (count-- > 0) {
		subMenu->children.append(deserialiseUnknown(stream, subMenu));
	}
	return subMenu;
}

QDataStream & operator<< (QDataStream &stream, const QSharedPointer<MenuItem> &item)
{
	item->serialise(stream);
	return stream;
}

QDataStream & operator>> (QDataStream &stream, QSharedPointer<MenuItem> &item)
{
	item = deserialiseUnknown(stream, QSharedPointer<SubMenuItem>());
	return stream;
}

LPTSTR paramTypes[] = {
	_T("Invalid"),
	_T("Object"),
	_T("Time"),
	_T("Border"),
	_T("Direction"),
	_T("Integer"),
	_T("Sample"),
	_T("Music"),
	_T("Invalid"),
	_T("Create Object"),
	_T("Animation"),
	_T("Nop"),
	_T("Player"),
	_T("Every"),
	_T("Key"),
	_T("Speed"),
	_T("Position"),
	_T("Joystick Direction"),
	_T("Shoot"),
	_T("Zone"),
	_T("Invalid"),
	_T("Sys Create"),
	_T("Numeric"),
	_T("Numeric comparison"),
	_T("Colour"),
	_T("Buffer4"),
	_T("Frame"),
	_T("Sample Loop"),
	_T("Music Loop"),
	_T("Direction"),
	_T("Invalid"),
	_T("Text Number"),
	_T("Click"),
	_T("Program"),
	_T("Invalid"),
	_T("Cnd Sample"),
	_T("Cnd Music"),
	_T("Remark"),
	_T("Group"),
	_T("Groupointer"),
	_T("Filename"),
	_T("String"),
	_T("Compare Time"),
	_T("Paste"),
	_T("Keyboard input"),
	_T("String"),
	_T("String comparison"),
	_T("Ink Effect"),
	_T("Menu"),
	_T("Global value"),
	_T("Alterable value"),
	_T("Flag"),
	_T("Global value expression"),
	_T("Alterable value expression"),
	_T("Flag expression"),
	_T("Extension"),
	_T("8 directions"),
	_T("Movement"),
	_T("Global string"),
	_T("Global string expression"),
	_T("Program"),
	_T("Alterable string"),
	_T("Alterable string expression"),
	_T("Filename"),
	_T("Effect"),
};

DWORD CALLBACK getMMFVersion() {
	return 0x02004110;
}

QSharedPointer<SubMenuItem> extractMenu(HMENU hMenu, QMap<short, ACE> aces)
{
	QSharedPointer<SubMenuItem> newMenu(new SubMenuItem);
	for (int i = 0; i < GetMenuItemCount(hMenu); i++) {
		MENUITEMINFO menuInfo = { sizeof(MENUITEMINFO), MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_SUBMENU, 0 };
		GetMenuItemInfo(hMenu, i, MF_BYPOSITION, &menuInfo);
		if (menuInfo.fType & MFT_SEPARATOR) {
			newMenu->children.append(QSharedPointer<MenuItem>(new SeparatorMenuItem(newMenu)));
		} else {
			menuInfo.cch++;
			menuInfo.dwTypeData = new TCHAR[menuInfo.cch + 1];
			GetMenuItemInfo(hMenu, i, MF_BYPOSITION, &menuInfo);
			QString menuName = QString::fromWCharArray(menuInfo.dwTypeData);
			delete [] menuInfo.dwTypeData;
			if (menuInfo.hSubMenu != NULL) {
				QSharedPointer<SubMenuItem> subMenu = extractMenu(menuInfo.hSubMenu, aces);
				subMenu->parent = newMenu;
				subMenu->text = menuName;
				newMenu->children.append(subMenu);
			} else {
				QMap<short, ACE>::iterator iter = aces.find(menuInfo.wID);
				if (iter != aces.end()) {
					newMenu->children.append(QSharedPointer<MenuItem>(new ACEMenuItem(iter.value(), menuName, newMenu)));
				}
			}
		}
	}
	return newMenu;
}

typedef QString (*GetParamString)(short);

QString getParamString(short type)
{
	return QString::fromWCharArray(paramTypes[type]);
}

QString getParamStringExp(short type)
{
	switch (type) {
		case 1:
			return QString("Numeric");
		case 3:
			return QString("String");
		default:
			return QString("Invalid");
	}
}

QMap<short, ACE> extractStrings(HMODULE hMfx, mv *mV, GetACEInfos proc, int count, GetParamString paramProc, QMap<int, QString> &paramText, ACE::Type type)
{
	QMap<short, ACE> aces;
	for (int i = 0; i < count; i++) {
		infosEventsV2 *cnd = proc(mV, i);
		eventInformations2 *cndInfos = (eventInformations2*)((char*)cnd - offsetof(eventInformations2, infos));
		TCHAR string[1024] = {0};
		LoadString(hMfx, cndInfos->string, string, 1023);
		ACE ace;
		ace.menuID = cndInfos->menu;
		ace.flags = cndInfos->infos.flags;
		ace.text = QString::fromWCharArray(string);
		short *paramType = (short*)&(&cndInfos->infos)[1];
		short *paramId = &paramType[cndInfos->infos.nParams];
		for (int x = 0; x < cndInfos->infos.nParams; x++) {
			LoadString(hMfx, paramId[x], string, 1023);
			if (paramText.find(paramId[x]) == paramText.end())
				paramText.insert(paramId[x], QString::fromWCharArray(string));
			MMFParam param;
			param.type = paramProc(paramType[x]);
			param.id = paramId[x];
			ace.params.append(param);
		}
		ace.type = type;
		aces.insert(ace.menuID, ace);
	}
	return aces;
}

MF2Description MF2Description::fromFile(QString file)
{
	HMODULE hMfx = LoadLibrary(file.utf16());
	if (hMfx == NULL) {
		LPTSTR str;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, (LPTSTR)&str, 1, NULL);
		QMessageBox::information(NULL, QApplication::applicationName(), QString::fromWCharArray(str));
		LocalFree(str);
		return MF2Description();
	}
	mv myMv;
	myMv.mvGetVersion = &getMMFVersion;
	GetRunObjectInfos infosProc = (GetRunObjectInfos)GetProcAddress(hMfx, "GetRunObjectInfos");
	GetACEInfos conditionsProc = (GetACEInfos)GetProcAddress(hMfx, "GetConditionInfos");
	GetACEInfos actionsProc = (GetACEInfos)GetProcAddress(hMfx, "GetActionInfos");
	GetACEInfos expressionsProc = (GetACEInfos)GetProcAddress(hMfx, "GetExpressionInfos");
	GetACEMenu conditionMenuProc = (GetACEMenu)GetProcAddress(hMfx, "GetConditionMenu");
	GetACEMenu actionMenuProc = (GetACEMenu)GetProcAddress(hMfx, "GetActionMenu");
	GetACEMenu expressionMenuProc = (GetACEMenu)GetProcAddress(hMfx, "GetExpressionMenu");
	if (!infosProc || !conditionsProc || !actionsProc || !expressionsProc) {
		return MF2Description();
	}
	kpxRunInfos infos;
	infosProc(&myMv, &infos);

	MF2Description desc;
	QMap<short, ACE> conditions = extractStrings(hMfx, &myMv, conditionsProc, infos.numOfConditions, getParamString, desc.paramText, ACE::Condition);
	QMap<short, ACE> actions = extractStrings(hMfx, &myMv, actionsProc, infos.numOfActions, getParamString, desc.paramText, ACE::Action);
	QMap<short, ACE> expressions = extractStrings(hMfx, &myMv, expressionsProc, infos.numOfExpressions, getParamStringExp, desc.paramText, ACE::Expression);
	desc.conditionMenu = extractMenu(conditionMenuProc(&myMv, NULL, NULL), conditions);
	desc.actionMenu = extractMenu(actionMenuProc(&myMv, NULL, NULL), actions);
	desc.expressionMenu = extractMenu(expressionMenuProc(&myMv, NULL, NULL), expressions);

	// Load icon
	HANDLE hImage = LoadImage(hMfx, MAKEINTRESOURCE(400), IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
	desc.image = QPixmap::fromWinHBITMAP((HBITMAP)hImage).toImage();
	DeleteObject(hImage);

	// load extension name
	TCHAR string[1024] = {0};
	LoadString(hMfx, 2, string, 1023);
	desc.menu->text = QString::fromWCharArray(string);

	desc.actionMenu->text = "Actions";
	desc.actionMenu->parent = desc.menu;
	desc.menu->children.append(desc.actionMenu);
	desc.conditionMenu->text = "Conditions";
	desc.conditionMenu->parent = desc.menu;
	desc.menu->children.append(desc.conditionMenu);
	desc.expressionMenu->text = "Expressions";
	desc.expressionMenu->parent = desc.menu;
	desc.menu->children.append(desc.expressionMenu);
	return desc;
}
