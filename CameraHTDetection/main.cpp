#include "ChooseDialog.h"
#include "ScanDialog.h"

int main(int argc, char *argv[])
{
    QSharedMemory lock("智华摄像头高温检测");
    if (!lock.create(sizeof(int))) {
        qui::MsgBox::warning("警告", "程序已在运行中,请勿重复启动!");
        return 0;
    }

    QApplication a(argc, argv);
	QUI_LOG_INIT("log", qApp->applicationDirPath() + "/log/log.txt");
	QUI_LOG->flushOn(utility::file::Log::ErrorLevel);

	QUI_INI_INIT(qApp->applicationDirPath() + "/config/config.ini");
    QUI_INI->newValue("配置/选择机种方式", 0, "0:手动选择;1:扫描选择");
	QUI_INI->newValue("产品代码/ZH-XXXXXXXXX", "这是一个测试样例");
	QUI_INI->newValue("设备绑定校验/启用", 0);
	for (int i = 0; i < 8; ++i) {
		QUI_INI->newValue(Q_SPRINTF("设备绑定校验/设备%d序列号", i + 1), "");
	}

    QString typeName;
	auto args = qApp->arguments();
    for (int i = 0; i < args.size(); ++i) {
		if (args[i] == "-type_name" && i + 1 < args.size()) {
            typeName = args[i + 1];
            break;
        }
    }

	if (!typeName.isEmpty()) {
		auto dirPath = qApp->applicationDirPath() + "/config";
		auto typePath = dirPath + "/" + typeName;
        if (!utility::existDir(typePath)) {
			qui::MsgBox::warning("警告", QString("机种%1不存在!").arg(typeName));
            return 0;
        }
        UTIL_JSON->setFilePath(typePath);
        UTIL_JSON->setTypeName(typeName);
		MainWindow* window = new MainWindow(typeName);
		window->maskCloseHint(true);
		if (!window->initialize()) {
			qui::MsgBox::warning("错误", window->getLastError());
			window->close();
			return 0;
		}
		window->maskCloseHint(false);
		qui::Widget::doModeless(window, QUI_MAIN_WINDOW_LAYOUT);
	}
    else {
		if (QUI_INI->value("配置/选择机种方式").toInt() == 0) {
			qui::Widget::doModeless<ChooseDialog, QUI_SIMPLE_WINDOW_LAYOUT>();
		}
		else {
			qui::Widget::doModeless<ScanDialog, QUI_SIMPLE_WINDOW_LAYOUT>();
		}
    }
    return a.exec();
}
