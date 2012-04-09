#ifndef PARAMEDITOR_H
#define PARAMEDITOR_H

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

#include <QDialog>
namespace Ui {class ParamEditor;};

class MF2Description;

class ParamEditor : public QDialog
{
	Q_OBJECT

public:
	ParamEditor(QWidget *parent = 0);
	~ParamEditor();

	void setDescription(MF2Description *);
	void accept();

private:
	Ui::ParamEditor *ui;
	MF2Description *m_desc;
};

#endif // PARAMEDITOR_H
