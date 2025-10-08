#include "Json.h"

Json* Json::getInstance()
{
	static Json json;
	return &json;
}

void Json::setFilePath(const QString& filePath)
{
	m_filePath = filePath;
}

QString Json::getFilePath() const
{
	return m_filePath;
}

void Json::setTypeName(const QString& typeName)
{
	m_typeName = typeName;
}

QString Json::getTypeName() const
{
	return m_typeName;
}

bool Json::initialize(bool update)
{
	bool result = false;
	do
	{
		const QString typeName = m_filePath + "\\type.json";
		const QString shareName = QString("%1\\%2").arg(utility::getCurrentDirectory(), "Config\\share.json");
		if (!utility::file::exist(typeName) || update)
		{
			if (update ? !updateTypeFile(typeName) : !writeTypeFile(typeName))
			{
				break;
			}
		}

		if (!utility::file::exist(shareName) || update)
		{
			if (update ? !updateShareFile(shareName) : !writeShareFile(shareName))
			{
				break;
			}
		}

		utility::file::setHidden(shareName);

		if (!readTypeFile(typeName) || !readShareFile(shareName))
		{
			break;
		}
		result = true;
	} while (false);
	return result;
}

QString Json::getLastError() const
{
	return m_lastError;
}

bool Json::setBaseInfoValue(const QString& key, const QString& _value)
{
	bool result = false, convert = false, success = true;
	do
	{
		QString value = _value;
		RUN_BREAK(!m_baseInfoObj.contains(key), QString("非法的键%1").arg(key));
		if (key == "设备类型" || key == "解码芯片")
		{
			int number = value.toInt(&convert);
			RUN_BREAK(!convert, QString("%1必须为整数").arg(key));
			if (key == "设备类型")
			{
				RUN_BREAK(number < 0 || number > rlg::DEVICE_TYPE_VM16K, QString("%1不在有效范围内").arg(key));
			}
			else if (key == "解码芯片")
			{
				//RUN_BREAK(number < 0 || number > _HisFX3_Deserializer_MAXIM96722 - 1, QString("%1不在有效范围内").arg(key));
			}
		}
		else if (key == "条码长度")
		{
			int number = value.toInt(&convert);
			if (!convert)
			{
				value = N_TO_Q_STR(value.length());
			}
		}
		else if (key == "系统时钟")
		{
			auto number = value.toDouble(&convert);
			RUN_BREAK(!convert, "系统时钟必须为数字", VOID_LAMBDA(success = false;));
			RUN_BREAK(number < 0 || number > 136, "系统时钟必须在0~136MHz范围内", VOID_LAMBDA(success = false;));
		}

		if (!success)
		{
			break;
		}
		m_baseInfoObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getBaseInfoValue(const QString& key) const
{
	return m_baseInfoObj.value(key).toString();
}

BaseInfo* Json::getBaseInfo() const
{
	return const_cast<BaseInfo*>(&m_baseInfo);
}

QStringList Json::getBaseInfoKeys() const
{
	return m_baseInfoKeys;
}

QStringList Json::getBaseInfoExplains() const
{
	return { 
		"设备类型",
		"解串器类型",
		"奇数通道文件",
		"偶数通道文件",
		"采集卡时钟",
		"判断前N个字符",
		"可扫描直接获取" 
	};
}

bool Json::setTestInfoValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		RUN_BREAK(!m_testInfoObj.contains(key), QString("非法的键%1").arg(key));
		if (key == "测试时间" || key == "循环次数" || key == "循环间隔" ||
			key == "判定次数" || key == "采集间隔" || key == "奇偶间隔" ||
			key == "时间精度" || key == "抓图超时")
		{
			int number = value.toInt(&convert);
			RUN_BREAK(!convert, QString("%1必须为整数").arg(key));
			if (key == "采集间隔")
			{
				RUN_BREAK(number < 0, "采集间隔必须大于等于0");
			}
			else if (key == "时间精度")
			{
				RUN_BREAK(number != 0 && number != 1, "时间精度只能为0或1");
			}
		}
		else if (key == "帧率范围" || key == "电流范围")
		{
			QStringList split = value.split("~", QString::SkipEmptyParts);
			RUN_BREAK(split.size() != 2, "格式必须为X0~X1,中间以~区分");
			double v1 = split[0].toDouble(&convert);
			RUN_BREAK(!convert, QString("%1必须为数字").arg(key));
			double v2 = split[1].toDouble(&convert);
			RUN_BREAK(!convert, QString("%1必须为数字").arg(key));
		}
		m_testInfoObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getTestInfoValue(const QString& key) const
{
	return m_testInfoObj.value(key).toString();
}

TestInfo* Json::getTestInfo() const
{
	return const_cast<TestInfo*>(&m_testInfo);
}

QStringList Json::getTestInfoKeys() const
{
	return m_testInfoKeys;
}

QStringList Json::getTestInfoExplains() const
{
	return { 
		"0:分级 1:秒级",//2
		"单位:分或秒",//1
		"单位:分或秒",//4
		"循环测试多少次",//3
		"超过此次数则NG",//5
		"单位:FPS",//6
		"单位:毫安",
		"单位:毫秒",//7
		"单位:毫秒",//8
		"单位:毫秒",//9
		"单位:毫秒"//10
	};
}

bool Json::setEnableInfoValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		int number = value.toInt(&convert);
		RUN_BREAK(!convert, QString("%1必须为整数").arg(key));
		RUN_BREAK(number != 0 && number != 1, QString("%1只能为0或1").arg(key));
		RUN_BREAK(!m_enableInfoObj.contains(key), QString("非法的键%1").arg(key));
		m_enableInfoObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getEnableInfoValue(const QString& key) const
{
	return m_enableInfoObj.value(key).toString();
}

EnableInfo* Json::getEnableInfo() const
{
	return const_cast<EnableInfo*>(&m_enableInfo);
}

QStringList Json::getEnableInfoKeys() const
{
	return m_enableInfoKeys;
}

QStringList Json::getEnableInfoExplains() const
{
	QStringList explains;
	for (int i = 0; i < 32; ++i) {
		explains.append("0禁用 1启用");
	}
	return explains;
}

bool Json::setUserInfoValue(const QString& key, const QString& value)
{
	bool result = false;
	do
	{
		RUN_BREAK(!m_userInfoObj.contains(key), QString("非法的键%1").arg(key));
		m_userInfoObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getUserInfoValue(const QString& key) const
{
	return m_userInfoObj.value(key).toString();
}

UserInfo* Json::getUserInfo() const
{
	return const_cast<UserInfo*>(&m_userInfo);
}

QStringList Json::getUserInfoKeys() const
{
	return m_userInfoKeys;
}

QStringList Json::getUserInfoExplains() const
{
	return { "管理员认证密码" };
}

bool Json::setBindInfoValue(const QString& parentKey, const QString& childKey, const QString& value)
{
	bool result = false;
	do
	{
		if (!m_bindInfoObj.contains(parentKey))
		{
			setLastError(QString("非法的父键%1").arg(parentKey));
			break;
		}

		QJsonObject object = m_bindInfoObj[parentKey].toObject();
		if (!object.contains(childKey))
		{
			setLastError(QString("非法的子键%1").arg(childKey));
			break;
		}
		object[childKey] = value;
		m_bindInfoObj.insert(parentKey, object);
		result = true;
	} while (false);
	return result;
}

QString Json::getBindInfoValue(const QString& parentKey, const QString& childKey)
{
	return m_bindInfoObj.value(parentKey).toObject().value(childKey).toString();
}

BindInfo* Json::getBindInfo() const
{
	return const_cast<BindInfo*>(m_bindInfo);
}

int Json::getBindInfoEnableCount() const
{
	int count = 0;
	for (int i = 0; i < MAX_BIND_COUNT; ++i)
	{
		if (m_bindInfo[i].enable)
		{
			++count;
		}
	}
	return count;
}

QStringList Json::getBindInfoParentKeys() const
{
	return m_bindInfoParentKeys;
}

QStringList Json::getBindInfoChildKeys() const
{
	return m_bindInfoChildKeys;
}

QStringList Json::getBindInfoChildExplains() const
{
	return { "设备启用禁用","设备序列号" };
}

bool Json::setChannelInfoValue(const QString& key, const QString& value)
{
	bool result = false;
	do
	{
		RUN_BREAK(!m_channelInfoObj.contains(key), QString("非法的键%1").arg(key));
		m_channelInfoObj[key] = value;
		result = true;
	} while (false);
	return result;
}

int Json::getChannelInfoValue(const QString& key) const
{
	return m_channelInfoObj.value(key).toString().toInt();
}

ChannelInfo* Json::getChannelInfo() const
{
	return const_cast<ChannelInfo*>(m_channelInfo);
}

bool Json::setCountInfoValue(const QString& key, const QString& value)
{
	bool result = false;
	do
	{
		RUN_BREAK(!m_countInfoObj.contains(key), QString("非法的键%1").arg(key));
		m_countInfoObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getCountInfoValue(const QString& key) const
{
	return m_countInfoObj[key].toString();
}

CountInfo* Json::getCountInfo() const
{
	return const_cast<CountInfo*>(&m_countInfo);
}

QStringList Json::getCountInfoKeys() const
{
	return m_countInfoKeys;
}

QStringList Json::getCountInfoExplains() const
{
	QStringList explains = { "线束插拔上限" };
	for (int i = 0; i < MAX_CHANNEL_COUNT; ++i) {
		explains.append("线束插拔次数");
	}
	return explains;
}

bool Json::setPlotInfoValue(const QString& key, const QString& value)
{
	bool result = false;
	do
	{
		RUN_BREAK(!m_plotInfoObj.contains(key), QString("非法的键%1").arg(key));
		m_plotInfoObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getPlotInfoValue(const QString& key) const
{
	return m_plotInfoObj.value(key).toString();
}

PlotInfo* Json::getPlotInfo() const
{
	return const_cast<PlotInfo*>(&m_plotInfo);
}

QStringList Json::getPlotInfoKeys() const
{
	return m_plotInfoKeys;
}

QStringList Json::getPlotInfoExplains() const
{
	QStringList explains = { "出图方式" };
	for (int i = 0; i < MAX_CHANNEL_COUNT; ++i) {
		explains.append("出图文件");
	}
	return explains;
}

bool Json::setCrashDetectionInfoValue(const QString& key, const QString& value)
{
	bool result = false, convert = false;
	do
	{
		if (key == "HSV最小RGB范围" || key == "HSV最大RGB范围") {
			auto split = value.split(",", QString::SkipEmptyParts);
			if (split.size() != 3) {
				setLastError("范围格式错误,参考100,255,255");
				break;
			}

			for (auto& x : split) {
				x.toInt(&convert);
				if (!convert) {
					setLastError(QString("%1中不可存在非整数参数").arg(key));
					break;
				}
			}

			if (!convert) {
				break;
			}
		}
		else if (key == "最大轮廓面积") {
			value.toInt(&convert);
			if (!convert) {
				setLastError(QString("%1必须为整数").arg(key));
				break;
			}
		}
		else if (key == "是否启用检测") {
			auto number = value.toInt(&convert);
			if (!convert) {
				setLastError(QString("%1必须为整数").arg(key));
				break;
			}

			if (number != 0 && number != 1) {
				setLastError(QString("%1只能为0或1").arg(key));
				break;
			}
		}

		RUN_BREAK(!m_crashDetectionObj.contains(key), QString("非法的键%1").arg(key));
		m_crashDetectionObj[key] = value;
		result = true;
	} while (false);
	return result;
}

QString Json::getCrashDetectionInfoValue(const QString& key) const
{
	return m_crashDetectionObj.value(key).toString();
}

CrashDetectionInfo* Json::getCrashDetectionInfo() const
{
	return const_cast<CrashDetectionInfo*>(&m_crashDetectionInfo);
}

QStringList Json::getCrashDetectionInfoKeys() const
{
	return m_crashDetectionInfoKeys;
}

QStringList Json::getCrashDetectionInfoExplains() const
{
	return { "HSV三原色范围", "HSV三原色范围", "最大轮廓面积", "0禁用1启用" };
}

rlg::DeviceType Json::getDeviceType() const
{
	rlg::DeviceType type = rlg::DEVICE_TYPE_VM16F_16;
	switch (m_baseInfo.deviceType.toInt())
	{
	case 0: type = rlg::DEVICE_TYPE_VM16F_16; break;
	case 1: type = rlg::DEVICE_TYPE_VM16F_8; break;
	case 2: type = rlg::DEVICE_TYPE_R2C; break;
	case 3: type = rlg::DEVICE_TYPE_R9U; break;
	case 4: type = rlg::DEVICE_TYPE_VM16K; break;
	default:break;
	}
	return type;
}

QStringList Json::getDeviceTexts() const
{
	return { "VM16F_16", "VM16F_8", "R2C", "R9U", "VM16K" };
}

rlg::DeserializerType Json::getDeserializerType() const
{
	auto decode = m_baseInfo.decodeChip.toInt();
	return static_cast<rlg::DeserializerType>(decode + 1);
}

QStringList Json::getDeserializerTexts() const
{
	const QStringList text =
	{
		"TI954", //0
		"MAXIM9296", //1
		"TI972", //2
		"MAXIM96706", //3
		"MAXIM9280", //4
		"MAXIM96716", //5
		"ROHMBU18RM84", //6
		"TI9702", //7
		"MAXIM9296C16", //8
		"MAXIM96722", //9
		"AHD9922B", //10
		"NS6603_C2_D8", //11
		"MAXIM9296AndTI954_9296_C16", //12
		"MAXIM9296AndTI954_954_C16", //13
		"RLC99602_C2_D8", //14
		"TI9702_C16", //15
	};
	return text;
}

void Json::setLastError(const QString& error)
{
	m_lastError = error;
}

bool Json::readTypeFile(const QString& fileName)
{
	bool result = false;
	do
	{
		QJsonObject jsonCopy;
		utility::file::readJson(fileName, jsonCopy);

		enum UpgradeJsonNode {
			UJN_V2_0_0_1 = 0x01,
			UJN_V2_0_0_14 = 0x02,
		};

		QString oddFile, evenFile;
		int caseEo = 0, needToUpgrade = 0;
		RUN_BREAK(!utility::file::repairJson1LevelNode(fileName,
			{
				"基本信息",
				"测试信息",
				"启用信息",
				"花屏检测信息"
			},
			{
				m_baseInfoKeys,
				m_testInfoKeys,
				m_enableInfoKeys,
				m_crashDetectionInfoKeys
			},
			{
				m_baseInfoValues,
				m_testInfoValues,
				m_enableInfoValues,
				m_crashDetectionInfoValues
			}, 
			[this, fileName, &oddFile, &evenFile, &caseEo, &needToUpgrade](const QString& node,
				const QString& key, const QJsonValue& value, QJsonObject& obj) {
				if (node == "基本信息") {
					if (key == "配置文件") {
						//1.0.0.12升级到2.0.0.1,删除[配置文件]增加[奇数文件]&[偶数文件]节点
						auto string = value.toString();
						auto split = string.split(";");
						if (split.size() == 2) {
							obj.insert("奇数文件", utility::getBaseNameByPath(split.value(0)));
							obj.insert("偶数文件", utility::getBaseNameByPath(split.value(1)));
						}
						else {
							obj.insert("奇数文件", utility::getBaseNameByPath(string));
							obj.insert("偶数文件", utility::getBaseNameByPath(string));
						}
						needToUpgrade |= UJN_V2_0_0_1;
					}
					else if (key == "奇数文件") {
						//2.0.0.13升级到2.0.0.14,删除[奇数文件],独立一个出图配置
						oddFile = value.toString();
						needToUpgrade |= UJN_V2_0_0_14;
					}
					else if (key == "偶数文件") {
						//2.0.0.13升级到2.0.0.14,删除[偶数文件],独立一个出图配置
						evenFile = value.toString();
						needToUpgrade |= UJN_V2_0_0_14;
					}
					else if (key == "条码判断") {
						//2.0.0.13升级到2.0.0.14,将条码判断更改为条码头部
						obj.insert("条码头部", value.toString());
						needToUpgrade |= UJN_V2_0_0_14;
					}
				}
				else if (node == "测试信息") {
					//2.0.0.13升级到2.0.0.14,将奇偶间隔更改为出图间隔
					if (key == "奇偶间隔") {
						obj.insert("出图间隔", value);
						needToUpgrade |= UJN_V2_0_0_14;
					}
				}
				else if (node == "启用信息") {
					//2.0.0.13升级到2.0.0.14,区分奇偶删除,出图信息增加出图方式
					if (key == "区分奇偶") {
						caseEo = value.toString().toInt();
						needToUpgrade |= UJN_V2_0_0_14;
					}
				}
			}),
			utility::getLastError());

		if (needToUpgrade & UJN_V2_0_0_1) {
			utility::makeDir(m_filePath + "\\backup");
			utility::file::writeJson(m_filePath + "\\backup\\upgrade_to_v2.0.0.1.json", jsonCopy);
		}

		if (needToUpgrade & UJN_V2_0_0_14) {
			for (int i = 0; i < m_plotInfoKeys.size(); ++i) {
				if (m_plotInfoKeys[i] == "出图方式") {
					m_plotInfoValues[i] = N_TO_Q_STR(caseEo);
				}
				else {
					QRegularExpression expr("通道(\\d+)");
					auto match = expr.match(m_plotInfoKeys[i]);
					if (match.hasMatch()) {
						auto channel = match.captured(1).toInt();
						if (channel % 2 != 0) {
							m_plotInfoValues[i] = oddFile;
						}
						else {
							m_plotInfoValues[i] = evenFile;
						}
					}
				}
			}
			utility::makeDir(m_filePath + "\\backup");
			utility::file::writeJson(m_filePath + "\\backup\\upgrade_to_v2.0.0.14.json", jsonCopy);
		}

		utility::file::repairJson1LevelNode(fileName, { "出图信息" }, { m_plotInfoKeys }, { m_plotInfoValues });

		QJsonObject root;
		RUN_BREAK(!utility::file::readJson(fileName, root), "读取类型配置文件失败," + utility::getLastError());

		RUN_BREAK(!root.contains("基本信息"), "丢失对象名基本信息");
		m_baseInfoObj = root.value("基本信息").toObject();
		RUN_BREAK(!parseBaseInfo(), "解析基本信息失败," + getLastError());

		RUN_BREAK(!root.contains("测试信息"), "丢失对象名测试信息");
		m_testInfoObj = root.value("测试信息").toObject();
		RUN_BREAK(!parseTestInfo(), "解析测试信息失败," + getLastError());

		RUN_BREAK(!root.contains("启用信息"), "丢失对象名启用信息");
		m_enableInfoObj = root.value("启用信息").toObject();
		RUN_BREAK(!parseEnableInfo(), "解析启用信息失败," + getLastError());

		RUN_BREAK(!root.contains("通道信息"), "丢失对象名通道信息");
		m_channelInfoObj = root.value("通道信息").toObject();
		RUN_BREAK(!parseChannelInfo(), "解析通道信息失败," + getLastError());

		RUN_BREAK(!root.contains("出图信息"), "丢失对象名出图信息");
		m_plotInfoObj = root.value("出图信息").toObject();
		RUN_BREAK(!parsePlotInfo(), "解析出图信息失败," + getLastError());

		RUN_BREAK(!root.contains("花屏检测信息"), "丢失对象名花屏检测信息");
		m_crashDetectionObj = root.value("花屏检测信息").toObject();
		RUN_BREAK(!parseCrashDetectionInfo(), "解析花屏检测信息失败," + getLastError());

		result = true;
	} while (false);
	return result;
}

bool Json::writeTypeFile(const QString& fileName)
{
	bool result = false;
	do
	{
		QJsonObject root, base, test, enable, channel, plot;

		for (int i = 0; i < m_baseInfoKeys.size(); i++)
		{
			base.insert(m_baseInfoKeys[i], m_baseInfoValues[i]);
		}

		for (int i = 0; i < m_testInfoKeys.size(); i++)
		{
			test.insert(m_testInfoKeys[i], m_testInfoValues[i]);
		}

		for (int i = 0; i < m_enableInfoKeys.size(); i++)
		{
			enable.insert(m_enableInfoKeys[i], m_enableInfoValues[i]);
		}

		for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			channel.insert(N_TO_Q_STR(i), "1");
		}

		for (int i = 0; i < m_plotInfoKeys.size(); ++i) {
			plot.insert(m_plotInfoKeys[i], m_plotInfoValues[i]);
		}

		root.insert("基本信息", base);
		root.insert("测试信息", test);
		root.insert("启用信息", enable);
		root.insert("通道信息", channel);
		root.insert("出图信息", plot);

		RUN_BREAK(!utility::file::writeJson(fileName, root), "写入类型配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::updateTypeFile(const QString& fileName)
{
	bool result = false;
	do
	{
		QJsonObject root;
		root.insert("基本信息", m_baseInfoObj);
		root.insert("测试信息", m_testInfoObj);
		root.insert("启用信息", m_enableInfoObj);
		root.insert("通道信息", m_channelInfoObj);
		root.insert("出图信息", m_plotInfoObj);
		root.insert("花屏检测信息", m_crashDetectionObj);
		RUN_BREAK(!utility::file::writeJson(fileName, root), "更新类型配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::readShareFile(const QString& fileName)
{
	bool result = false;
	do
	{
		RUN_BREAK(!utility::file::repairJson1LevelNode(fileName,
			{
				"用户信息",
				"次数信息"
			},
			{
				m_userInfoKeys,
				m_countInfoKeys
			},
			{
				m_userInfoValues,
				m_countInfoValues
			}),
			utility::getLastError());

		QJsonObject root;
		RUN_BREAK(!utility::file::readJson(fileName, root), "读取共享配置文件失败," + utility::getLastError());

		RUN_BREAK(!root.contains("用户信息"), "丢失对象名用户信息");
		m_userInfoObj = root.value("用户信息").toObject();
		RUN_BREAK(!parseUserInfo(), "解析用户信息失败," + getLastError());

		RUN_BREAK(!root.contains("绑定信息"), "丢失对象名绑定信息");
		m_bindInfoObj = root.value("绑定信息").toObject();
		RUN_BREAK(!parseBindInfo(), "解析绑定信息失败," + getLastError());

		RUN_BREAK(!root.contains("次数信息"), "丢失对象名次数信息");
		m_countInfoObj = root.value("次数信息").toObject();
		RUN_BREAK(!parseCountInfo(), "解析次数信息失败," + getLastError());

		result = true;
	} while (false);
	return result;
}

bool Json::writeShareFile(const QString& fileName)
{
	bool result = false;
	do
	{
		QJsonObject root, user, bind, count;
		for (int i = 0; i < m_userInfoKeys.size(); i++) {
			user.insert(m_userInfoKeys[i], m_userInfoValues[i]);
		}

		for (int i = 0; i < MAX_BIND_COUNT; i++) {
			QJsonObject obj;
			for (int j = 0; j < m_bindInfoChildKeys.size(); j++) {
				obj.insert(m_bindInfoChildKeys[j], m_bindInfoChildValues[j]);
			}
			bind.insert(N_TO_Q_STR(i), obj);
		}

		for (int i = 0; i < m_countInfoKeys.size(); ++i) {
			count.insert(m_countInfoKeys[i], m_countInfoValues[i]);
		}

		root.insert("用户信息", user);
		root.insert("绑定信息", bind);
		root.insert("次数信息", count);
		RUN_BREAK(!utility::file::writeJson(fileName, root), "写入共享配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::updateShareFile(const QString& fileName)
{
	bool result = false;
	do
	{
		QJsonObject root;
		root.insert("用户信息", m_userInfoObj);
		root.insert("绑定信息", m_bindInfoObj);
		root.insert("次数信息", m_countInfoObj);
		RUN_BREAK(!utility::file::writeJson(fileName, root), "更新共享配置文件失败," + utility::getLastError());
		result = true;
	} while (false);
	return result;
}

bool Json::parseBaseInfo()
{
	bool result = false;
	do
	{
		QString* valuePtr = reinterpret_cast<QString*>(&m_baseInfo);
		for (int i = 0; i < m_baseInfoKeys.size(); i++, valuePtr++)
		{
			*valuePtr = getBaseInfoValue(m_baseInfoKeys[i]);
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseTestInfo()
{
	bool result = false, convert = false;
	do
	{
		m_testInfo.testTime = getTestInfoValue("测试时间").toInt();

		m_testInfo.timePrecision = getTestInfoValue("时间精度").toInt();

		m_testInfo.cycleCount = getTestInfoValue("循环次数").toInt();

		m_testInfo.cycleInterval = getTestInfoValue("循环间隔").toInt();

		m_testInfo.determineCount = getTestInfoValue("判定次数").toInt();

		auto fpsRange = getTestInfoValue("帧率范围").split("~", QString::SkipEmptyParts);
		RUN_BREAK(fpsRange.size() != 2, "帧率范围格式错误");

		m_testInfo.fps0 = fpsRange[0].toDouble(&convert);
		RUN_BREAK(!convert, "帧率范围只能为数字");

		m_testInfo.fps1 = fpsRange[1].toDouble(&convert);
		RUN_BREAK(!convert, "帧率范围只能为数字");

		auto currentRange = getTestInfoValue("电流范围").split("~", QString::SkipEmptyParts);
		RUN_BREAK(currentRange.size() != 2, "电流范围格式错误");

		m_testInfo.current0 = currentRange[0].toDouble(&convert);
		RUN_BREAK(!convert, "电流范围只能为数字");

		m_testInfo.current1 = currentRange[1].toDouble(&convert);
		RUN_BREAK(!convert, "电流范围只能为数字");

		m_testInfo.captureInterval = getTestInfoValue("采集间隔").toInt();

		m_testInfo.plotInterval = getTestInfoValue("出图间隔").toInt();

		m_testInfo.grabTimeout = getTestInfoValue("抓图超时").toInt();

		m_testInfo.startDelay = getTestInfoValue("启动延时").toInt();

		result = true;
	} while (false);
	return result;
}

bool Json::parseEnableInfo()
{
	bool result = false;
	do
	{
		int* valuePtr = reinterpret_cast<int*>(&m_enableInfo);
		for (int i = 0; i < m_enableInfoKeys.size(); i++, valuePtr++) {
			*valuePtr = getEnableInfoValue(m_enableInfoKeys[i]).toInt();
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseUserInfo()
{
	bool result = false;
	do
	{
		QString* valuePtr = reinterpret_cast<QString*>(&m_userInfo);
		for (int i = 0; i < m_userInfoKeys.size(); i++, valuePtr++)
		{
			*valuePtr = getUserInfoValue(m_userInfoKeys[i]);
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseBindInfo()
{
	bool result = false;
	do
	{
		for (int i = 0; i < MAX_BIND_COUNT; i++)
		{
			m_bindInfo[i].enable = getBindInfoValue(N_TO_Q_STR(i), "启用").toInt();
			strcpy_s(m_bindInfo[i].sn, Q_TO_C_STR(getBindInfoValue(N_TO_Q_STR(i), "序列号")));
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseChannelInfo()
{
	bool result = false;
	do
	{
		for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			m_channelInfo[i].enable = getChannelInfoValue(N_TO_Q_STR(i));
		}
		result = true;
	} while (false);
	return result;
}

bool Json::parseCountInfo()
{
	m_countInfo.upperlimit = getCountInfoValue("次数上限").toInt();
	for (int i = 0; i < MAX_CHANNEL_COUNT; ++i) {
		m_countInfo.channelSum[i] = getCountInfoValue(q_sprintf("通道%d", i + 1)).toInt();
	}
	return true;
}

bool Json::parsePlotInfo()
{
	m_plotInfo.plotWay = getPlotInfoValue("出图方式").toInt();
	for (int i = 0; i < MAX_CHANNEL_COUNT; ++i) {
		m_plotInfo.plotFile[i] = getPlotInfoValue(q_sprintf("通道%d", i + 1));
	}
	return true;
}

bool Json::parseCrashDetectionInfo()
{
	auto split = getCrashDetectionInfoValue("HSV最小RGB范围").split(",", QString::SkipEmptyParts);
	m_crashDetectionInfo.hsvLower = cv::Scalar(split[0].toInt(), split[1].toInt(), split[2].toInt());
	split = getCrashDetectionInfoValue("HSV最大RGB范围").split(",", QString::SkipEmptyParts);
	m_crashDetectionInfo.hsvUpper = cv::Scalar(split[0].toInt(), split[1].toInt(), split[2].toInt());
	m_crashDetectionInfo.maxArea = getCrashDetectionInfoValue("最大轮廓面积").toInt();
	m_crashDetectionInfo.enableDetection = getCrashDetectionInfoValue("是否启用检测").toInt();
	return true;
}

Json::Json(QObject* parent)
	: QObject(parent)
{

}

Json::~Json()
{

}
