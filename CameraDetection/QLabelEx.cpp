#include "QLabelEx.h"

QLabelEx::QLabelEx(QWidget* parent)
	: QLabel(parent)
{

}

QLabelEx::~QLabelEx()
{

}

void QLabelEx::mouseDoubleClickEvent(QMouseEvent* event)
{
	auto split = objectName().split("_");
	if (split.size() == 2)
	{
		int id = split[1].toInt();
		if (event->button() == Qt::LeftButton) {
			emit mouseLeftDoubleClick(id);
		}
		else if (event->button() == Qt::RightButton) {
			emit mouseRightDoubleClick(id);
		}
	}
	return __super::mouseDoubleClickEvent(event);
}
