#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include "ui_ScanDialog.h"

#include "MainWindow.h"

class ScanDialog : public QDialog
{
	Q_OBJECT
public:
	ScanDialog(QWidget *parent = Q_NULLPTR);
	~ScanDialog();

protected slots:
	void on_enterButton_clicked();
	void on_exitButton_clicked();
	void on_lineEdit_returnPressed();

private:
	Ui::ScanDialog ui;
};
