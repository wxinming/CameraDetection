#pragma once
#pragma execution_character_set("utf-8")

#include <QObject>
#include <Utility/Utility.h>
#pragma comment(lib, "Utility.lib")

#include <librlg/librlg.h>
#include <libqui/libqui.h>
#ifdef QT_DEBUG
#pragma comment(lib, "librlgd.lib")
#pragma comment(lib, "libquid.lib")
#else
#pragma comment(lib, "librlg.lib")
#pragma comment(lib, "libqui.lib")
#endif

#define MAX_BIND_COUNT 0x08
#define MAX_CHANNEL_COUNT 0x40

//运行状态
enum class RunStatus
{
	//开始
	RS_BEGIN,

	//预览
	RS_PREVIEW,

	//开始
	RS_START,

	//停止
	RS_STOP,

	//终止
	RS_ABORT,

	//结束
	RS_END,
};

//测试状态
enum class TestStatus
{
	//没有
	TS_NO,

	//等待
	TS_WT,

	//测试
	TS_TS,

	//良品
	TS_OK,

	//疵品
	TS_NG
};

//按钮类型
enum class ButtonType
{
	//退出按钮
	BT_EXIT,

	//连接按钮
	BT_CONN,

	//预览按钮
	BT_PREV,

	//测试按钮
	BT_TEST
};

//基本信息
struct BaseInfo 
{
	//设备类型
	QString deviceType;

	//解码芯片
	QString decodeChip;

	//系统时钟
	QString systemClock;

	//条码头部
	QString barcodeHeader;

	//条码长度
	QString barcodeLength;
};

//测试信息
struct TestInfo
{
	//时间精度
	int timePrecision;

	//测试时间
	int testTime;

	//循环间隔
	int cycleInterval;

	//循环次数
	int cycleCount;

	//判断次数
	int determineCount;

	//帧率0
	double fps0;

	//帧率1
	double fps1;

	//电流0
	double current0;

	//电流1
	double current1;

	//采集间隔
	int captureInterval;

	//出图间隔
	int plotInterval;

	//抓图超时
	int grabTimeout;

	//启动延时
	int startDelay;
};

//启用信息
struct EnableInfo
{
	//判断条码
	int judgeCode;

	//显示数据
	int showData;

	//显示通道
	int showChannel;

	//查询工站
	int queryWorkstation;

	//跳过测试
	int skipTest;

	//保存错误帧日志
	int saveErrorFrameLog;

	//不良保存记录提示
	int ngSaveRecordHint;

	//保存测试日志
	int saveTestLog;

	//保存不良图像
	int saveNgImage;

	//不良状态重置
	int ngStatusReset;
};

//用户信息
struct UserInfo
{
	QString adminPassword;
};

//绑定信息
struct BindInfo 
{
	//启用
	int enable;

	//序列号
	char sn[128];
};

//通道信息
struct ChannelInfo
{
	//是否启用
	bool enable = false;

	//结果
	int result = false;

	//判定次数
	int count = false;

	//单帧正确帧
	double srfps = 0;

	//单帧错误帧
	double sefps = 0;

	//总帧正确帧
	uint trfps = 0;

	//总帧错误帧
	uint tefps = 0;

	//电流
	double current = 0;

	//耗时
	int elapsedTime = 0;

	//花屏面积
	int crashAcreage = 0;

	//状态
	RunStatus status = RunStatus::RS_BEGIN;

	//开始时间
	quint64 startTime = 0;

	//循环次数
	int cycleCount = 1;

	//是否超时
	bool timeout(int interval)
	{
		if (GetTickCount64() > startTime + interval)
		{
			startTime = GetTickCount64();
			return true;
		}
		return false;
	}

	//所有通过
	static bool allPass(rlg::Base* rlg, ChannelInfo* info)
	{
		QList<bool> list;
		for (int i = 0; i < rlg->getChannelTotal(); ++i) {
			if (info[i].enable) {
				list.append(info[i].result);
			}
		}
		
		int sum = 0;
		for(const auto& x :list) {
			if (x) {
				sum += 1;
			}
		}
		return sum == list.size();
	}

	//零启用
	static bool zeroEnable(rlg::Base* rlg, ChannelInfo* info)
	{
		bool result = false;
		for (int i = 0; i < rlg->getChannelTotal(); ++i) {
			if (info[i].enable) {
				info[i].status = RunStatus::RS_BEGIN;
				result = true;
			}
		}
		return result;
	}

	static bool allStatusEqual(rlg::Base* rlg, ChannelInfo* info, RunStatus status)
	{
		QList<RunStatus> list;
		for (int i = 0; i < rlg->getChannelTotal(); ++i) {
			if (info[i].enable) {
				list.append(info[i].status);
			}
		}

		int sum = 0;
		for (const auto& x : list) {
			if (x == status) {
				sum += 1;
			}
		}
		return sum == list.size();
	}
};

struct CountInfo {
	int upperlimit = 0;
	int channelSum[MAX_CHANNEL_COUNT] = { 0 };
};

struct PlotInfo {
	//出图方式
	int plotWay;

	//出图文件
	QString plotFile[MAX_CHANNEL_COUNT];
};

struct CrashDetectionInfo {
	cv::Scalar hsvLower;
	cv::Scalar hsvUpper;
	int maxArea;
	bool enableDetection;
};

/*
* @brief,JSON文件
*/
class Json : public QObject
{
	Q_OBJECT
public:
	/*
	* @brief 获取实例
	* @return Json*
	*/
	static Json* getInstance();

	/*
	* @brief 设置文件路径
	* @param[in] filePath 路径
	* @return void
	*/
	void setFilePath(const QString& filePath);

	/*
	* @brief 获取文件路径
	* @return QString
	*/
	QString getFilePath() const;

	/*
	* @brief 设置机种名
	* @param[in] typeName 机种名
	* @return void
	*/
	void setTypeName(const QString& typeName);

	/*
	* @brief 获取机种名
	* @return QString
	*/
	QString getTypeName() const;

	/*
	* @brief 初始化
	* @param[in] update 是否更新
	* @return bool
	*/
	bool initialize(bool update = false);

	/*
	* @brief 获取最终错误
	* @return QString
	*/
	QString getLastError() const;

	/*
	* @brief 设置基本信息值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setBaseInfoValue(const QString& key, const QString& value);

	/*
	* @brief 获取基本信息值
	* @param[in] key 键
	* @return QString
	*/
	QString getBaseInfoValue(const QString& key) const;

	/*
	* @brief 获取基本信息
	* @return BaseInfo*
	*/
	BaseInfo* getBaseInfo() const;

	/*
	* @brief 获取基本信息键
	* @return QStringList
	*/
	QStringList getBaseInfoKeys() const;

	/*
	* @brief 获取基本信息说明
	* @return QStringList
	*/
	QStringList getBaseInfoExplains() const;

	/*
	* @brief 设置测试信息值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setTestInfoValue(const QString& key, const QString& value);

	/*
	* @brief 获取测试信息值
	* @param[in] key 键
	* @return QString
	*/
	QString getTestInfoValue(const QString& key) const;

	/*
	* @brief 获取测试信息
	* @return TestInfo*
	*/
	TestInfo* getTestInfo() const;

	/*
	* @brief 获取测试信息键
	* @return QStringList
	*/
	QStringList getTestInfoKeys() const;

	/*
	* @brief 获取测试信息说明
	* @return QStringList
	*/
	QStringList getTestInfoExplains() const;

	/*
	* @brief 设置启用信息值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setEnableInfoValue(const QString& key, const QString& value);

	/*
	* @brief 获取启用信息值
	* @param[in] key 键
	* @return QString
	*/
	QString getEnableInfoValue(const QString& key) const;

	/*
	* @brief 获取启用信息
	* @return EnableInfo*
	*/
	EnableInfo* getEnableInfo() const;

	/*
	* @brief 获取启用信息键
	* @return QStringList
	*/
	QStringList getEnableInfoKeys() const;

	/*
	* @brief 获取启用信息说明
	* @return QStringList
	*/
	QStringList getEnableInfoExplains() const;

	/*
	* @brief 设置用户信息值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setUserInfoValue(const QString& key, const QString& value);

	/*
	* @brief 获取用户信息值
	* @param[in] key 键
	* @return QString
	*/
	QString getUserInfoValue(const QString& key) const;

	/*
	* @brief 获取用户信息
	* @return UserInfo*
	*/
	UserInfo* getUserInfo() const;

	/*
	* @brief 获取用户信息键
	* @return QStringList
	*/
	QStringList getUserInfoKeys() const;

	/*
	* @brief 获取用户信息说明
	* @return QStringList
	*/
	QStringList getUserInfoExplains() const;

	/*
	* @brief 设置绑定信息值
	* @param[in] parentKey 父键
	* @param[in] childKey 子键
	* @param[in] value 值
	* @return bool
	*/
	bool setBindInfoValue(const QString& parentKey, const QString& childKey, const QString& value);

	/*
	* @brief 获取绑定信息值
	* @param[in] parentKey 父键
	* @param[in] childKey 子键
	* @return QString
	*/
	QString getBindInfoValue(const QString& parentKey, const QString& childKey);

	/*
	* @brief 获取绑定信息
	* @return BindInfo*
	*/
	BindInfo* getBindInfo() const;

	/*
	* @brief 获取绑定信息启用数量
	* @return int
	*/
	int getBindInfoEnableCount() const;

	/*
	* @brief 获取绑定信息父键
	* @return QStringList
	*/
	QStringList getBindInfoParentKeys() const;

	/*
	* @brief 获取绑定信息子键
	* @return QStringList
	*/
	QStringList getBindInfoChildKeys() const;

	/*
	* @brief 获取绑定信息说明
	* @return QStringList
	*/
	QStringList getBindInfoChildExplains() const;

	/*
	* @brief 设置通道信息值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setChannelInfoValue(const QString& key, const QString& value);

	/*
	* @brief 获取通道信息值
	* @param[in] key 键
	* @return int
	*/
	int getChannelInfoValue(const QString& key) const;

	/*
	* @brief 获取通道信息
	* @return ChannelInfo*
	*/
	ChannelInfo* getChannelInfo() const;

	/*
	* @brief 设置次数信息值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setCountInfoValue(const QString& key, const QString& value);

	/*
	* @brief 获取次数信息值
	* @param[in] key 键
	* @return int
	*/
	QString getCountInfoValue(const QString& key) const;

	/*
	* @brief 获取次数信息
	* @return CountInfo*
	*/
	CountInfo* getCountInfo() const;

	/*
	* @brief 获取次数信息键
	* @return QStringList
	*/
	QStringList getCountInfoKeys() const;

	/*
	* @brief 获取次数信息说明
	* @return QStringList
	*/
	QStringList getCountInfoExplains() const;

	/*
	* @brief 设置出图信息值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setPlotInfoValue(const QString& key, const QString& value);

	/*
	* @brief 获取出图信息值
	* @param[in] key 键
	* @return QString
	*/
	QString getPlotInfoValue(const QString& key) const;

	/*
	* @brief 获取出图信息
	* @return PlotInfo*
	*/
	PlotInfo* getPlotInfo() const;

	/*
	* @brief 获取出图信息键
	* @return QStringList
	*/
	QStringList getPlotInfoKeys() const;

	/*
	* @brief 获取出图信息说明
	* @return QStringList
	*/
	QStringList getPlotInfoExplains() const;

	/*
	* @brief 设置花屏检测信息值
	* @param[in] key 键
	* @param[in] value 值
	* @return bool
	*/
	bool setCrashDetectionInfoValue(const QString& key, const QString& value);

	/*
	* @brief 获取花屏检测信息值
	* @param[in] key 键
	* @return QString
	*/
	QString getCrashDetectionInfoValue(const QString& key) const;

	/*
	* @brief 获取花屏检测信息
	* @return CrashDetectionInfo*
	*/
	CrashDetectionInfo* getCrashDetectionInfo() const;

	/*
	* @brief 获取花屏检测键
	* @return QStringList
	*/
	QStringList getCrashDetectionInfoKeys() const;

	/*
	* @brief 获取花屏检测说明
	* @return QStringList
	*/
	QStringList getCrashDetectionInfoExplains() const;

	/*
	* @brief 获取设备类型
	* @return RolongoDeviceType
	*/
	rlg::DeviceType getDeviceType() const;

	/*
	* @brief 获取设备文本
	* @return QStringList
	*/
	QStringList getDeviceTexts() const;

	/*
	* @brief 获取解串器类型
	* @return HisDeserializerType
	*/
	rlg::DeserializerType getDeserializerType() const;

	/*
	* @brief 获取解串器文本
	* @return QStringList
	*/
	QStringList getDeserializerTexts() const;
protected:
	/*
	* @brief 设置最终错误
	* @param[in] error 错误信息
	* @return void
	*/
	void setLastError(const QString& error);

	/*
	* @brief 读取类型文件
	* @param[in] fileName 文件名
	* @return bool
	*/
	bool readTypeFile(const QString& fileName);

	/*
	* @brief 写入类型文件
	* @param[in] fileName 文件名
	* @return bool
	*/
	bool writeTypeFile(const QString& fileName);

	/*
	* @brief 更新类型文件
	* @param[in] fileName 文件名
	* @return bool
	*/
	bool updateTypeFile(const QString& fileName);

	/*
	* @brief 读取共享文件
	* @param[in] fileName 文件名
	* @return bool
	*/
	bool readShareFile(const QString& fileName);

	/*
	* @brief 写入共享文件
	* @param[in] fileName 文件名
	* @return bool
	*/
	bool writeShareFile(const QString& fileName);

	/*
	* @brief 更新共享文件
	* @param[in] fileName 文件名
	* @return bool
	*/
	bool updateShareFile(const QString& fileName);

	/*
	* @brief 解析基本信息
	* @return bool
	*/
	bool parseBaseInfo();

	/*
	* @brief 解析测试信息
	* @return bool
	*/
	bool parseTestInfo();

	/*
	* @brief 解析启用信息
	* @return bool
	*/
	bool parseEnableInfo();

	/*
	* @brief 解析用户信息
	* @return bool
	*/
	bool parseUserInfo();

	/*
	* @brief 解析绑定信息
	* @return bool
	*/
	bool parseBindInfo();

	/*
	* @brief 解析通道信息
	* @return bool
	*/
	bool parseChannelInfo();

	/*
	* @brief 解析次数信息
	* @return bool
	*/
	bool parseCountInfo();

	/*
	* @brief 解析出图信息
	* @return bool
	*/
	bool parsePlotInfo();

	/*
	* @brief 解析花屏检测信息
	* @return bool
	*/
	bool parseCrashDetectionInfo();

private:
	Json(QObject* parent = nullptr);

	~Json();

private:
	QString m_filePath;

	QString m_typeName;

	BaseInfo m_baseInfo;

	TestInfo m_testInfo;

	EnableInfo m_enableInfo;

	UserInfo m_userInfo;

	BindInfo m_bindInfo[MAX_BIND_COUNT] = { 0 };

	ChannelInfo m_channelInfo[MAX_CHANNEL_COUNT] = { 0 };

	CountInfo m_countInfo;

	PlotInfo m_plotInfo;

	CrashDetectionInfo m_crashDetectionInfo;

	QJsonObject m_baseInfoObj;

	QJsonObject m_testInfoObj;

	QJsonObject m_enableInfoObj;

	QJsonObject m_userInfoObj;

	QJsonObject m_bindInfoObj;

	QJsonObject m_channelInfoObj;

	QJsonObject m_countInfoObj;

	QJsonObject m_plotInfoObj;

	QJsonObject m_crashDetectionObj;

	QString m_lastError = "未知错误";

	const QStringList m_baseInfoKeys = 
	{
		"设备类型",//1
		"解码芯片",//2
		"系统时钟",//3
		"条码头部",//5
		"条码长度"//6
	};

	const QStringList m_baseInfoValues = 
	{
		"0",//1
		"0",//2
		"24",//3
		"NULL",//5
		"0"//6
	};

	const QStringList m_testInfoKeys = 
	{
		"时间精度",//2
		"测试时间",//1
		"循环间隔",//4
		"循环次数",//3
		"判定次数",//5
		"帧率范围",//6
		"电流范围",//7
		"采集间隔",//8
		"出图间隔",//9
		"抓图超时",//10
		"启动延时"//11
	};

	const QStringList m_testInfoValues = 
	{
		"0",//1
		"30",//2
		"1",//3
		"3",//4
		"1",//5
		"25~35",//6
		"40~70",//7
		"1000",//8
		"3000",//9
		"3000",//10
		"5000"//11
	};

	const QStringList m_enableInfoKeys = 
	{
		"判断条码",//1
		"显示数据",//2
		"显示通道",//3
		"查询工站",//4
		"跳过测试",//5
		"保存错误帧日志",//6
		"不良保存记录提示",//7
		"保存测试日志",//8
		"保存不良图像",//9
		"不良状态重置",//10
	};

	const QStringList m_enableInfoValues = 
	{
		"1",//1
		"1",//2
		"1",//3
		"1",//4
		"0",//5
		"0",//6
		"0",//7
		"1",//8
		"1",//9
		"0",//10
	};

	const QStringList m_userInfoKeys = 
	{
		"管理员密码"
	};

	const QStringList m_userInfoValues = 
	{
		"1."
	};

	const QStringList m_bindInfoParentKeys =
	{
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7"
		//"8",
		//"9",
		//"10",
		//"11",
		//"12",
		//"13",
		//"14",
		//"15"
	};

	const QStringList m_bindInfoChildKeys = 
	{
		"启用",
		"序列号"
	};

	const QStringList m_bindInfoChildValues = 
	{
		"0",
		"未绑定"
	};

	const QStringList m_crashDetectionInfoKeys = 
	{
		"HSV最小RGB范围",
		"HSV最大RGB范围",
		"最大轮廓面积",
		"是否启用检测"
	};

	const QStringList m_crashDetectionInfoValues = 
	{
		"50,100,100",
		"70,255,255",
		"100",
		"0"
	};

	const QStringList m_countInfoKeys = {
		"次数上限",
		"通道1",
		"通道2",
		"通道3",
		"通道4",
		"通道5",
		"通道6",
		"通道7",
		"通道8",
		"通道9",
		"通道10",
		"通道11",
		"通道12",
		"通道13",
		"通道14",
		"通道15",
		"通道16",
		"通道17",
		"通道18",
		"通道19",
		"通道20",
		"通道21",
		"通道22",
		"通道23",
		"通道24",
		"通道25",
		"通道26",
		"通道27",
		"通道28",
		"通道29",
		"通道30",
		"通道31",
		"通道32",
		"通道33",
		"通道34",
		"通道35",
		"通道36",
		"通道37",
		"通道38",
		"通道39",
		"通道40",
		"通道41",
		"通道42",
		"通道43",
		"通道44",
		"通道45",
		"通道46",
		"通道47",
		"通道48",
		"通道49",
		"通道50",
		"通道51",
		"通道52",
		"通道53",
		"通道54",
		"通道55",
		"通道56",
		"通道57",
		"通道58",
		"通道59",
		"通道60",
		"通道61",
		"通道62",
		"通道63",
		"通道64"
	};

	const QStringList m_countInfoValues = {
		"5000",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0",
		"0"
	};

	const QStringList m_plotInfoKeys = {
		"出图方式",
		"通道1",
		"通道2",
		"通道3",
		"通道4",
		"通道5",
		"通道6",
		"通道7",
		"通道8",
		"通道9",
		"通道10",
		"通道11",
		"通道12",
		"通道13",
		"通道14",
		"通道15",
		"通道16",
		"通道17",
		"通道18",
		"通道19",
		"通道20",
		"通道21",
		"通道22",
		"通道23",
		"通道24",
		"通道25",
		"通道26",
		"通道27",
		"通道28",
		"通道29",
		"通道30",
		"通道31",
		"通道32",
		"通道33",
		"通道34",
		"通道35",
		"通道36",
		"通道37",
		"通道38",
		"通道39",
		"通道40",
		"通道41",
		"通道42",
		"通道43",
		"通道44",
		"通道45",
		"通道46",
		"通道47",
		"通道48",
		"通道49",
		"通道50",
		"通道51",
		"通道52",
		"通道53",
		"通道54",
		"通道55",
		"通道56",
		"通道57",
		"通道58",
		"通道59",
		"通道60",
		"通道61",
		"通道62",
		"通道63",
		"通道64"
	};

	QStringList m_plotInfoValues = {
		"0",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor",
		"sensor"
	};
};
