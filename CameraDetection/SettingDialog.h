#pragma once
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QCheckBox>
#include "ui_SettingDialog.h"
#include "Json.h"

class SettingDialog : public QDialog
{
	Q_OBJECT
public:
	SettingDialog(std::shared_ptr<rlg::Base> base, QWidget* parent = Q_NULLPTR);
	~SettingDialog();
	bool initialize();

signals:
	void updateText(const QString& text);

public slots:
	void saveButtonSlot();
	void exitButtonSlot();

protected:
	void addChannelWidget();

private:
	enum RestartFlag {
		RF_CHANGE_NULL = 0x00,
		RF_CHANGE_DEVICE_TYPE = 0x01,
		RF_CHANGE_CHIP_TYPE = 0x02,
		RF_CHANGE_SYSTEM_CLOCK = 0x04,
		RF_CHANGE_BIND_ENABLE = 0x08,
		RF_CHANGE_BIND_SN = 0x10,
		RF_CHANGE_CONFIG_FILE = 0x20
	};

	Ui::SettingDialog ui;
	QList<QCheckBox*> m_channelList;
	QList<QComboBox*> m_snList;
	int m_restartFlag = RF_CHANGE_NULL;
	std::shared_ptr<rlg::Base> m_rlg;
};

