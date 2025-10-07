#pragma once
#pragma execution_character_set("utf-8")

#include <QLabel>
#include <QMouseEvent>

class QLabelEx : public QLabel
{
	Q_OBJECT
public:
	QLabelEx(QWidget* parent = nullptr);
	~QLabelEx();

protected:
	void mouseDoubleClickEvent(QMouseEvent* event) override;

signals:
	void mouseLeftDoubleClick(int id);
	void mouseRightDoubleClick(int id);
};
