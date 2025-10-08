#ifndef QT_UTILS_H
#define QT_UTILS_H

#include <QApplication>
#include <QScreen>
#include <QStringList>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QShortcut>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QTableWidget>
#include <QListWidget>
#include <QLabel>
#include <QTreeWidgetItem>

class QtUtils
{
public:
	static int
	refresh(QListWidget* lw, const std::vector<std::string>& vs, bool defEnable = true);

	static int
	refresh2disable(QListWidget* lw, const std::vector<std::string>& vs);

	static void
	toScreenCenter(QDialog& win);

	static int
	refresh(QComboBox* cb, const std::string& fp);

	/**
	 * toItemValue
	 * @param value
	 * @return
	 */
	static int
	toItemValue(const int& value);

	static double
	toItemValue(const double& value);

	static long long
	toItemValue(const long long& value);

	static QString
	toItemValue(const char* value);

	static QString
	toItemValue(const std::string& value);

	static QString
	toItemValue(const QString& value);

	/**
	 * fromItemValue
	 * @param value
	 * @return
	 */
	static std::string
	fromItemValue(const QString& value);

	static int
	loadImage(QLabel * lb, const std::string & fp);

	static int
	loadImage(QLabel * lb, QPixmap & p);

	static int
	loadImage_v2(QLabel * lb, QPixmap & p);
	/**
	 * toItemValue
	 * @param ed
	 * @param v
	 */
	static void
	toItemValue(QPlainTextEdit* ed, const std::string& v);

	static void
	toItemValue(QLineEdit* ed, const std::string& v);

	static void
	toItemValue(QLineEdit* ed, const int& v);

	static void
	toItemValue(QCheckBox* ed, const int& v);

	static void
	toItemValue(QCheckBox* ed, const bool& v);

	static void
	toItemValue(QComboBox* cb, const std::string& v);

	static bool
	fromItemValue(QPlainTextEdit* ed, std::string& v, bool empty, const QString & title, std::string& msg);

	static bool
	fromItemValue(QLineEdit* ed, std::string& v, bool empty, const QString & title, std::string& msg);

	static bool
	fromItemValue(QLineEdit* ed, int& v, bool empty, const QString & title, std::string& msg);

	static bool
	fromItemValue(QCheckBox* ed, int& v, bool empty, const QString & title, std::string& msg);

	static bool
	fromItemValue(QCheckBox* ed, bool& v, bool empty, const QString & title, std::string& msg);

	static bool
	fromItemValue(QComboBox* cb, std::string& v, bool empty, const QString & title, std::string& msg);


	static void colunmnEnable(QTableWidget *tb,int column,bool defEnable = true);

};


#endif //QT_UTILS_H
