#include "SettingDialog.h"

SettingDialog::SettingDialog(std::shared_ptr<rlg::Base> base, QWidget* parent)
	: m_rlg(base), QDialog(parent)
{
	ui.setupUi(this);
	ui.saveButton->setFocusPolicy(Qt::NoFocus);
	ui.exitButton->setFocusPolicy(Qt::NoFocus);
	QUI_LOG->info("打开设置界面");
}

SettingDialog::~SettingDialog()
{
	QUI_LOG->info("关闭设置界面");
}

bool SettingDialog::initialize()
{
	bool result = false;
	do
	{
		ui.treeWidget->initialize();

		ui.treeWidget->setQrcUrl(":/MainDlg/Resources/images");

		const QStringList primarys = { "基本配置", "测试配置", "启用配置", "用户配置", "绑定配置", "次数配置", "出图配置", "花屏检测配置" };
		ui.treeWidget->addLevelItems<0>(primarys);
		ui.treeWidget->setLevelIcons<0>(primarys, "light_green_folder.ico");

		for (auto& x : primarys) {
			if (x != "绑定配置") {
				ui.treeWidget->setLevelValueEditPrivilege(x, 1, true);
			}
		}
		QStringList keys;
		QString value, explain;

		//基本配置
		keys = UTIL_JSON->getBaseInfoKeys();
		for (int i = 0; i < keys.size(); ++i) {
			value = UTIL_JSON->getBaseInfoValue(keys[i]);
			explain = UTIL_JSON->getBaseInfoExplains()[i];
			ui.treeWidget->addLevelItem<1>("基本配置", keys[i], value, explain);
			if (keys[i] == "设备类型") {
				utility::QComboBoxEx* combo = new utility::QComboBoxEx(this);
				auto texts = UTIL_JSON->getDeviceTexts();
				for (int i = 0; i < texts.size(); ++i) {
					combo->addItem(QIcon(":/MainDlg/Resources/images/orange_file.ico"), texts[i]);
				}
				combo->setCurrentIndex(value.toInt());
				connect(combo, CAST_QCOMBOBOX_CHANGED_INT, [this](int index) {
					UTIL_JSON->setBaseInfoValue("设备类型", N_TO_Q_STR(index));
					m_restartFlag |= RF_CHANGE_DEVICE_TYPE;
					QUI_LOG->info("修改[基本配置/设备类型]=%d", index);
					QUI_LOG->flush();
				});
				ui.treeWidget->setLevelIcon<1>("基本配置", keys[i], "green_folder.ico", QString(), "empty_file.ico");
				ui.treeWidget->setLevelWidget<1>("基本配置", keys[i], 1, combo);
			}
			else if (keys[i] == "解码芯片") {
				utility::QComboBoxEx* combo = new utility::QComboBoxEx(this);
				auto texts = UTIL_JSON->getDeserializerTexts();
				for (int i = 0; i < texts.size(); ++i) {
					combo->addItem(QIcon(":/MainDlg/Resources/images/orange_file.ico"), texts[i]);
				}
				combo->setCurrentIndex(value.toInt());
				connect(combo, CAST_QCOMBOBOX_CHANGED_INT, [this](int index) {
					UTIL_JSON->setBaseInfoValue("解码芯片", N_TO_Q_STR(index));
					m_restartFlag |= RF_CHANGE_CHIP_TYPE;
					QUI_LOG->info("修改[基本配置/解码芯片]=%d", index);
					QUI_LOG->flush();
				});
				ui.treeWidget->setLevelIcon<1>("基本配置", keys[i], "green_folder.ico", QString(), "empty_file.ico");
				ui.treeWidget->setLevelWidget<1>("基本配置", keys[i], 1, combo);
			}
			else {
				ui.treeWidget->setLevelIcon<1>("基本配置", keys[i], "green_folder.ico", "orange_file.ico", "empty_file.ico");
			}
		}

		//测试配置
		keys = UTIL_JSON->getTestInfoKeys();
		for (int i = 0; i < keys.size(); ++i) {
			value = UTIL_JSON->getTestInfoValue(keys[i]);
			explain = UTIL_JSON->getTestInfoExplains()[i];
			ui.treeWidget->addLevelItem<1>("测试配置", keys[i], value, explain);
			ui.treeWidget->setLevelIcon<1>("测试配置", keys[i], "green_folder.ico", "orange_file.ico", "empty_file.ico");
		}

		//启用配置
		keys = UTIL_JSON->getEnableInfoKeys();
		for (int i = 0; i < keys.size(); ++i) {
			value = UTIL_JSON->getEnableInfoValue(keys[i]);
			explain = UTIL_JSON->getEnableInfoExplains()[i];
			ui.treeWidget->addLevelItem<1>("启用配置", keys[i], value, explain);
			ui.treeWidget->setLevelIcon<1>("启用配置", keys[i], "green_folder.ico", "orange_file.ico", "empty_file.ico");
		}

		//用户配置
		keys = UTIL_JSON->getUserInfoKeys();
		for (int i = 0; i < keys.size(); ++i) {
			value = UTIL_JSON->getUserInfoValue(keys[i]);
			explain = UTIL_JSON->getUserInfoExplains()[i];
			ui.treeWidget->addLevelItem<1>("用户配置", keys[i], value, explain);
			ui.treeWidget->setLevelIcon<1>("用户配置", keys[i], "green_folder.ico", "orange_file.ico", "empty_file.ico");
		}

		//绑定配置
		keys = UTIL_JSON->getBindInfoParentKeys();
		for (int i = 0; i < keys.size(); ++i) {
			QString key = Q_SPRINTF("设备%d", keys[i].toInt() + 1);
			ui.treeWidget->addLevelItem<1>("绑定配置", key);
			ui.treeWidget->setLevelIcon<1>("绑定配置", key, "green_folder.ico");
			auto childKey = UTIL_JSON->getBindInfoChildKeys();
			for (int j = 0; j < childKey.size(); ++j) {
				value = UTIL_JSON->getBindInfoValue(keys[i], childKey[j]);
				explain = UTIL_JSON->getBindInfoChildExplains()[j];
				ui.treeWidget->addLevelItem<2>("绑定配置", key, childKey[j], value, explain);
				ui.treeWidget->setLevelIcon<2>("绑定配置", key, childKey[j], "folder.ico", QString(), "empty_file.ico");
				if (childKey[j] == "启用") {
					utility::QCheckBoxEx* check(new utility::QCheckBoxEx(this));
					check->setObjectName(Q_SPRINTF("check_%d", i));
					check->setChecked(UTIL_JSON->getBindInfo()[i].enable);
					connect(check, &QCheckBox::stateChanged, this, [this](int state) {
						utility::QCheckBoxEx* check = dynamic_cast<utility::QCheckBoxEx*>(sender());
						QString parentKey = check->objectName().split("_")[1];
						UTIL_JSON->setBindInfoValue(parentKey, "启用", N_TO_Q_STR(state));
						m_restartFlag |= RF_CHANGE_BIND_ENABLE;
						QUI_LOG->qinfo("修改[绑定配置/设备%1/启用]=%2", parentKey.toInt() + 1, state);
						QUI_LOG->flush();
					});
					ui.treeWidget->setLevelWidget<2>("绑定配置", key, "启用", 1, check);
				}
				else if (childKey[j] == "序列号") {
					utility::QComboBoxEx* combo(new utility::QComboBoxEx(this));
					//QAbstractItemView* view = combo->view();
					//if (view) { 
					//	view->setContextMenuPolicy(Qt::CustomContextMenu);
					//	connect(view, &QAbstractItemView::customContextMenuRequested, [combo, view](const QPoint& pos) {
					//		QModelIndex index = view->indexAt(pos);
					//		if (!index.isValid()) {
					//			return;
					//		}
					//		QString text = index.data(Qt::DisplayRole).toString();
					//		QMenu menu;
					//		QAction* copyAction = menu.addAction("复制");
					//		connect(copyAction, &QAction::triggered, [text]() { QApplication::clipboard()->setText(text); });
					//		menu.exec(view->mapToGlobal(pos));
					//	});
					//}
					combo->setEditable(true);
					auto lineEdit = combo->lineEdit();
					if (lineEdit) {
						lineEdit->setReadOnly(true);
					}

					combo->setPreviousText(UTIL_JSON->getBindInfo()[i].sn);
					m_snList.append(combo);
					connect(combo, CAST_QCOMBOBOX_ACTIVATED_STR, this, [&](const QString& text) {
						utility::QComboBoxEx* combo = dynamic_cast<utility::QComboBoxEx*>(sender());
						int index0 = combo->objectName().split("_")[1].toInt();
						for (const auto& x : m_snList) {
							if (x->objectName() != combo->objectName() &&
								text != "未绑定" &&
								x->currentText() == text) {
								int index1 = x->objectName().split("_")[1].toInt();
								qui::MsgBox::warning("错误",
									Q_SPRINTF("绑定序列号失败,设备%d与设备%d冲突", index0 + 1, index1 + 1));
								combo->setCurrentText(combo->getPreviousText());
								return;
							}
							combo->setPreviousText(text);
						}
						UTIL_JSON->setBindInfoValue(N_TO_Q_STR(index0), "序列号", text);
						m_restartFlag |= RF_CHANGE_BIND_SN;
						QUI_LOG->qinfo("修改[绑定配置/设备%1/序列号]=%2", index0 + 1, text);
						QUI_LOG->flush();
					});
					combo->setObjectName(Q_SPRINTF("BindInfoCombo_%d", i));
					auto deviceSnList = m_rlg->getDeviceSnList();
					deviceSnList.append("未绑定");
					for (const auto& x : deviceSnList) {
						combo->addItem(QIcon(":/MainDlg/Resources/images/orange_file.ico"), x);
					}
					combo->setCurrentText(UTIL_JSON->getBindInfo()[i].sn);
					ui.treeWidget->setLevelWidget<2>("绑定配置", key, childKey[j], 1, combo);
				}
			}
		}

		//次数配置
		keys = UTIL_JSON->getCountInfoKeys();
		auto size = std::min(keys.size(), m_rlg->getChannelTotal());
		size = size == 0 ? keys.size() : size + 1;
		for (int i = 0; i < size; ++i) {
			value = UTIL_JSON->getCountInfoValue(keys[i]);
			explain = UTIL_JSON->getCountInfoExplains()[i];
			ui.treeWidget->addLevelItem<1>("次数配置", keys[i], value, explain);
			ui.treeWidget->setLevelIcon<1>("次数配置", keys[i], "green_folder.ico", "orange_file.ico", "empty_file.ico");
		}

		//出图配置
		keys = UTIL_JSON->getPlotInfoKeys();
		size = std::min(keys.size(), m_rlg->getChannelTotal());
		size = size == 0 ? keys.size() : size + 1;
		auto iniFiles = utility::getFileListBySuffixName(UTIL_JSON->getFilePath(), { ".ini" });
		for (int i = 0; i < size; ++i) {
			value = UTIL_JSON->getPlotInfoValue(keys[i]);
			explain = UTIL_JSON->getPlotInfoExplains()[i];
			utility::QComboBoxEx* combo = new utility::QComboBoxEx(this);
			if (keys[i] == "出图方式") {
				connect(combo, CAST_QCOMBOBOX_ACTIVATED_INT, this, [&](int index) {
					UTIL_JSON->setPlotInfoValue("出图方式", N_TO_Q_STR(index));
					QUI_LOG->info("修改[出图配置/出图方式]=%d", index);
					QUI_LOG->flush();
				});
				const QStringList plotWay = { "顺序方式", "奇偶方式" };
				for (auto& x : plotWay) {
					combo->addItem(QIcon(":/MainDlg/Resources/images/orange_file.ico"), x);
				}
				combo->setCurrentIndex(value.toInt());
			}
			else {
				connect(combo, CAST_QCOMBOBOX_ACTIVATED_STR, this, [&](const QString& text) {
					utility::QComboBoxEx* combo = dynamic_cast<utility::QComboBoxEx*>(sender());
					int index = combo->objectName().split("_")[1].toInt();
					UTIL_JSON->setPlotInfoValue(q_sprintf("通道%d", index + 1), text);
					m_restartFlag |= RF_CHANGE_CONFIG_FILE;
					QUI_LOG->qinfo("修改[出图配置/通道%1]=%2", index + 1, text);
					QUI_LOG->flush();
				});
				combo->setObjectName(Q_SPRINTF("PlotInfoCombo_%d", i - 1));
				for (const auto& x : iniFiles) {
					combo->addItem(QIcon(":/MainDlg/Resources/images/orange_file.ico"), utility::getBaseNameByPath(x));
				}
				combo->setCurrentText(UTIL_JSON->getPlotInfo()->plotFile[i - 1]);
			}
			ui.treeWidget->addLevelItem<1>("出图配置", keys[i], value, explain);
			ui.treeWidget->setLevelIcon<1>("出图配置", keys[i], "green_folder.ico", QString(), "empty_file.ico");
			ui.treeWidget->setLevelWidget<1>("出图配置", keys[i], 1, combo);
		}

		//花屏检测配置
		keys = UTIL_JSON->getCrashDetectionInfoKeys();
		for (int i = 0; i < keys.size(); ++i) {
			value = UTIL_JSON->getCrashDetectionInfoValue(keys[i]);
			explain = UTIL_JSON->getCrashDetectionInfoExplains()[i];
			ui.treeWidget->addLevelItem<1>("花屏检测配置", keys[i], value, explain);
			ui.treeWidget->setLevelIcon<1>("花屏检测配置", keys[i], "green_folder.ico", "orange_file.ico", "empty_file.ico");
		}

		ui.treeWidget->onItemChanged = [this](QTreeWidgetItem* item, int row, int column)->bool {
			bool result = false;
			QString primary = ui.treeWidget->currentParentKeys()[0];
			QString key = ui.treeWidget->currentText(0);
			QString value = ui.treeWidget->currentText(1);
			do
			{
				if (primary == "基本配置")
				{
					if (!UTIL_JSON->setBaseInfoValue(key, value))
					{
						qui::MsgBox::warning("错误", UTIL_JSON->getLastError());
						break;
					}

					if (key == "系统时钟")
					{
						m_restartFlag |= RF_CHANGE_SYSTEM_CLOCK;
					}

					if (key == "条码长度")
					{
						bool convert = false;
						value.toInt(&convert);
						if (!convert)
						{
							item->setText(1, N_TO_Q_STR(value.length()));
						}
					}
				}
				else if (primary == "测试配置")
				{
					if (!UTIL_JSON->setTestInfoValue(key, value))
					{
						qui::MsgBox::warning("错误", UTIL_JSON->getLastError());
						break;
					}

					if (key == "测试时间")
					{
						int time = value.toInt();
						int precision = UTIL_JSON->getTestInfoValue("时间精度").toInt();
						if (precision == utility::TP_MINUTE)
						{
							updateText(Q_SPRINTF("时间 %02d:00", time));
						}
						else
						{
							int min = 0, sec = 0;
							utility::secondCountdown(time, 0, min, sec);
							updateText(Q_SPRINTF("时间 %02d:%02d", min, sec));
						}
					}
					else if (key == "时间精度")
					{
						int time = UTIL_JSON->getTestInfoValue("测试时间").toInt();
						int precision = value.toInt();
						if (precision == utility::TP_MINUTE)
						{
							updateText(Q_SPRINTF("时间 %02d:00", time));
						}
						else
						{
							int min = 0, sec = 0;
							utility::secondCountdown(time, 0, min, sec);
							updateText(Q_SPRINTF("时间 %02d:%02d", min, sec));
						}
					}
				}
				else if (primary == "启用配置")
				{
					if (!UTIL_JSON->setEnableInfoValue(key, value))
					{
						qui::MsgBox::warning("错误", UTIL_JSON->getLastError());
						break;
					}

					if (key == "保存错误帧日志") {
						m_rlg->enableSaveErrorFrameLog(value.toInt());
					}
				}
				else if (primary == "用户配置")
				{
					if (!UTIL_JSON->setUserInfoValue(key, value))
					{
						qui::MsgBox::warning("错误", UTIL_JSON->getLastError());
						break;
					}
				}
				else if (primary == "次数配置") {
					if (!UTIL_JSON->setCountInfoValue(key, value)) {
						qui::MsgBox::warning("错误", UTIL_JSON->getLastError());
						break;
					}
				}
				else if (primary == "花屏检测配置") {
					if (!UTIL_JSON->setCrashDetectionInfoValue(key, value)) {
						qui::MsgBox::warning("错误", UTIL_JSON->getLastError());
						break;
					}
				}

				QUI_LOG->qinfo("修改[%1/%2]=%3", primary, key, value);
				QUI_LOG->flush();
				result = true;
			} while (false);
			return result;
		};

		addChannelWidget();

		connect(ui.saveButton, &QPushButton::clicked, this, &SettingDialog::saveButtonSlot);

		connect(ui.exitButton, &QPushButton::clicked, this, &SettingDialog::exitButtonSlot);

		result = true;
	} while (false);
	return result;
}

void SettingDialog::saveButtonSlot()
{
	if (!UTIL_JSON->initialize(true)) {
		QUI_LOG->qerror("保存配置失败");
		qui::MsgBox::warning("提示", "保存文件失败," + UTIL_JSON->getLastError());
	}
	else {
		QUI_LOG->qinfo("保存配置成功");
		QString text = "保存文件成功";
		if (m_restartFlag & RF_CHANGE_DEVICE_TYPE ||
			m_restartFlag & RF_CHANGE_CHIP_TYPE ||
			m_restartFlag & RF_CHANGE_SYSTEM_CLOCK ||
			m_restartFlag & RF_CHANGE_BIND_SN ||
			m_restartFlag & RF_CHANGE_BIND_ENABLE) {
			text.append(",请重启软件生效!");
		}
		else {
			if (m_restartFlag & RF_CHANGE_CONFIG_FILE) {
				text.append(",请重新连接,出图配置才会生效!");
			}
		}
		qui::MsgBox::information("提示", text);
	}
	QUI_LOG->flush();
	return;
}

void SettingDialog::exitButtonSlot()
{
	this->close();
}

void SettingDialog::addChannelWidget()
{
	m_channelList.clear();
	int count = UTIL_JSON->getBindInfoEnableCount();
	int best = m_rlg->getBestMatrix(count);
	int total = m_rlg->getChannelTotal();

	QGridLayout* layout = new QGridLayout;
	int row = best, column = best;
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < column; ++j) {
			int index = i * column + j;
			if (total != 0 && index >= total) {
				continue;
			}

			auto check = new QCheckBox(Q_SPRINTF("%d", index + 1));
			check->setObjectName(Q_SPRINTF("check_%d", index));
			check->setIcon(QIcon(":/MainDlg/Resources/images/id.ico"));
			connect(check, &QCheckBox::clicked, this, [&](bool on) {
				auto open = false;
				for (int i = 0; i < m_rlg->getDeviceCount(); ++i) {
					if (m_rlg->isOpen(i)) {
						open = true;
						break;
					}
				}

				auto checkbox = dynamic_cast<QCheckBox*>(sender());
				if (open) {
					qui::MsgBox::warning("警告", "请先断开连接,方可禁用或启用!");
					checkbox->setChecked(!on);
					return;
				}
				auto index = checkbox->objectName().split("_")[1].toInt();
				UTIL_JSON->setChannelInfoValue(N_TO_Q_STR(index), N_TO_Q_STR(on));
			});

			check->setChecked(UTIL_JSON->getChannelInfo()[index].enable);
			m_channelList.append(check);
			layout->addWidget(check, i, j);
		}
	}

	QCheckBox* all = new QCheckBox("全选");
	all->setIcon(QIcon(":/MainDlg/Resources/images/rework.ico"));
	connect(all, &QCheckBox::clicked, this, [&](bool checked) {
		auto open = false;
		for (int i = 0; i < m_rlg->getDeviceCount(); ++i) {
			if (m_rlg->isOpen(i)) {
				open = true;
				break;
			}
		}

		auto checkbox = dynamic_cast<QCheckBox*>(sender());
		if (open) {
			qui::MsgBox::warning("警告", "请先断开连接,方可禁用或启用全选!");
			checkbox->setChecked(!checked);
			return;
		}

		for (int i = 0; i < m_channelList.size(); i++) {
			m_channelList[i]->setChecked(checked);
			UTIL_JSON->setChannelInfoValue(N_TO_Q_STR(i), N_TO_Q_STR(checked));
		}
	});
	layout->addWidget(all, row, column - 1);
	ui.channelTab->setLayout(layout);
}
