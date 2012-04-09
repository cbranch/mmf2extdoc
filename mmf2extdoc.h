#ifndef MMF2EXTDOC_H
#define MMF2EXTDOC_H

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

#include <QtGui/QDialog>
namespace Ui { class MMF2ExtDocClass; }
class MF2Description;

class MMF2ExtDoc : public QDialog
{
	Q_OBJECT

public:
	MMF2ExtDoc(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MMF2ExtDoc();

protected slots:
	void loadFile();
	void saveFile();
	void importExtension();
	void showPage(const QModelIndex &);
	void updatePageText();
	void editParams();
	void insertPage();
	void appendPage();
	void deletePage();
	void resetModel();
	void setExtensionName(QString);

private:
	Ui::MMF2ExtDocClass *ui;
	QString m_lastOpenedFile;
	MF2Description *m_data;
};

#endif // MMF2EXTDOC_H
