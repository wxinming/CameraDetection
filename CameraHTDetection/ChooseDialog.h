#pragma once
#pragma execution_character_set("utf-8")

#include <QDialog>
#include "ui_ChooseDialog.h"
#include "MainWindow.h"

class ChooseDialog : public QWidget
{
	Q_OBJECT
public:
	ChooseDialog(QWidget *parent = Q_NULLPTR);
	~ChooseDialog();
	QString getLastError() const;
	bool initialize();

public slots:
	void setLastError(const QString& error);
	void addDirctoryList();
	bool checkInput();

protected slots:
	void on_addButton_clicked();
	void on_deleteButton_clicked();
	void on_confirmButton_clicked();
	void on_exitButton_clicked();

private:
	Ui::ChooseDialog ui;
	QString m_lastError = "未知错误";
	QString m_dirName;
	QString m_pathName;
	QString m_recordName;
	QStringList m_dirList;
	QJsonObject m_recordObj;
};
