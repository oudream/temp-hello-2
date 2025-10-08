#include "qt_utils.h"

#include <ccxx/cxcontainer.h>
#include <ccxx/cxfile.h>
#include <ccxx/cxqtutil.h>

using namespace std;

int
QtUtils::refresh(QListWidget* lw, const std::vector<std::string>& vs, bool defEnable)
{
	lw->clear();
	for (int i = 0; i < vs.size(); ++i)
	{
		const string& v = vs[i];
		QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(v.data(), v.size()));
		if (defEnable)
		{
			item->setFlags(Qt::ItemIsEnabled);
		}
		else
		{
			item->setFlags(Qt::ItemIsSelectable);
		}
		lw->addItem(item);
	}
}

void
QtUtils::toScreenCenter(QDialog& dialog)
{
//    dialog.resize( 380, 220 );
	dialog.setGeometry(
		QStyle::alignedRect(
			Qt::LeftToRight,
			Qt::AlignCenter,
			dialog.size(),
			qApp->desktop()->availableGeometry()
		)
	);
}

int
QtUtils::refresh2disable(QListWidget* lw, const vector<std::string>& vs)
{
	for (int i = 0; i < lw->count(); ++i)
	{
		QByteArray data = lw->item(i)->text().toUtf8();
//		cxDebug() << vs;
		if (! CxContainer::contain(vs, std::string( data.data(), data.size() )))
		{
			lw->item(i)->setFlags(Qt::ItemIsSelectable);
		}
		else
		{
			lw->item(i)->setFlags(Qt::ItemIsEnabled);
		}
	}
	return 0;
}

void
QtUtils::colunmnEnable(QTableWidget *tb,int column,bool defEnable)
{
	if(tb==NULL) return;
	if(column<tb->columnCount()) return;
    if(!defEnable){

    		QTableWidgetItem *item = tb->item(0,column);
			if(item){
				item->setFlags(item->flags() & (~Qt::ItemIsEditable));
				item->setFlags(item->flags() & (~Qt::ItemIsSelectable));
//				item->set
			}

//		for(int i=0;i<tb->rowCount();++i){
//			QTableWidgetItem *item = tb->item(i,column);
//			if(item){
//				item->setFlags(item->flags() & (~Qt::ItemIsEditable));
//				item->setFlags(item->flags() & (~Qt::ItemIsSelectable));
//			}
//		}
	}else{

	}

}

int
QtUtils::toItemValue(const int& v)
{
	return v;
}
double
QtUtils::toItemValue(const double& v)
{
	return v;
}
long long
QtUtils::toItemValue(const long long& v)
{
	return v;
}
QString
QtUtils::toItemValue(const char* v)
{
	return QString::fromUtf8(v);
}
QString
QtUtils::toItemValue(const string& v)
{
	return QString::fromUtf8(v.data(), v.size());
}
QString
QtUtils::toItemValue(const QString& v)
{
	return v;
}
std::string
QtUtils::fromItemValue(const QString& value)
{
	QByteArray data = value.toUtf8();
	return string(data.data(), data.count());
}

int
QtUtils::loadImage(QLabel * lb, const std::string & fp)
{
	QPixmap p;
	p.load(CxQString::gbkToQString(fp));
	p.scaled(lb->size(), Qt::KeepAspectRatio);
	lb->setScaledContents(true);
	lb->setPixmap(p);
	return 0;
}

int
QtUtils::loadImage(QLabel* lb, QPixmap& p)
{
	p.scaled(lb->size(), Qt::KeepAspectRatio);
//	p.scaled(lb->width(), lb->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	lb->setPixmap(p);
	lb->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	lb->setScaledContents(true);
	return 0;
}

int
QtUtils::loadImage_v2(QLabel* lb, QPixmap& p)
{
	p.scaled(lb->size(), Qt::KeepAspectRatio);
//	p.scaled(lb->width(), lb->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	lb->setPixmap(p);
	lb->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	lb->setScaledContents(false);
	lb->setAlignment(Qt::AlignLeft);
	return 0;
}

void
QtUtils::toItemValue(QPlainTextEdit* ed, const string& v)
{
	ed->setPlainText(QString::fromUtf8(v.data(), v.size()));
}

void
QtUtils::toItemValue(QLineEdit* ed, const string& v)
{
	ed->setText(QString::fromUtf8(v.data(), v.size()));
}

void
QtUtils::toItemValue(QLineEdit* ed, const int& v)
{
	ed->setText(QString::number(v));
}

void
QtUtils::toItemValue(QCheckBox* ed, const int& v)
{
	if (v > 0)
	{
		ed->setCheckState(Qt::Checked);
	}
	else
	{
		ed->setCheckState(Qt::Unchecked);
	}
}

void
QtUtils::toItemValue(QCheckBox* ed, const bool& v)
{
	if (v)
	{
		ed->setCheckState(Qt::Checked);
	}
	else
	{
		ed->setCheckState(Qt::Unchecked);
	}
}

void
QtUtils::toItemValue(QComboBox* cb, const std::string& v){
	if(cb){
		cb->setCurrentText(QString::fromUtf8(v.data(), v.size()));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool
QtUtils::fromItemValue(QPlainTextEdit* ed, string& v, bool empty, const QString & title, std::string& msg)
{
	v = fromItemValue(ed->toPlainText());
	if (v.empty() && !empty)
	{
		string s = fromItemValue(title);
		msg = CxString::format("[ %s ]不能为空！", s.c_str());
		return false;
	}
	return true;
}

bool
QtUtils::fromItemValue(QLineEdit* ed, string& v, bool empty, const QString & title, std::string& msg)
{
	v = fromItemValue(ed->text());
	if (v.empty() && !empty)
	{
		string s = fromItemValue(title);
		msg = CxString::format("[ %s ]不能为空！", s.c_str());
		return false;
	}
	return true;
}

bool
QtUtils::fromItemValue(QLineEdit* ed, int& v, bool empty, const QString & title, std::string& msg)
{
	v = ed->text().toInt();
	if (ed->text().isEmpty() && !empty)
	{
		string s = fromItemValue(title);
		msg = CxString::format("[ %s ]不能为空！", s.c_str());
		return false;
	}
	return true;
}

bool
QtUtils::fromItemValue(QCheckBox* ed, int& v, bool empty, const QString & title, std::string& msg)
{
	if (ed->isChecked())
	{
		v = 1;
	}
	else
	{
		v = 0;
	}
	return true;
}

bool
QtUtils::fromItemValue(QCheckBox* ed, bool& v, bool empty, const QString & title, std::string& msg)
{
	if (ed->isChecked())
	{
		v = true;
	}
	else
	{
		v = false;
	}
	return true;
}

int
QtUtils::refresh(QComboBox* cb, const string& fp)
{
	return 0;
}

bool
QtUtils::fromItemValue(QComboBox* cb, std::string& v, bool empty, const QString & title, std::string& msg)
{
	v = fromItemValue(cb->currentText());
	if (v.empty() && !empty)
	{
		string s = fromItemValue(title);
		msg = CxString::format("[ %s ]不能为空！", s.c_str());
		return false;
	}
	return true;
}

