#ifndef CX_CT_X2_QT_HELPER_H
#define CX_CT_X2_QT_HELPER_H


#include <c_global.h>


#include <QDateTime>
#include <QHash>

#include <QListView>
#include <QListWidget>
#include <QTableView>
#include <QTableWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QHeaderView>
#include <QItemDelegate>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QtCore/QMutex>


class QStringHelper
{
public:
    static QString gbkToQString(const std::string &s);

    static QStringList gbkToQStrings(const std::vector<std::string> &ss);

    static QStringList gbkToQStrings(const std::string *oSs, int iCount);

    static std::vector<std::string> gbkToStdStrings(const QStringList &ss);

    static std::string gbkToStdString(const QString &s);

    static const QString DateTimeFormat;

    static QString ToString(const int &i);

    static QString ToString(const unsigned int &i);

    static QString ToString(const int &i, int iZeroCount);

    static QString ToString(const bool &b);

    static QString ToString(const double &d);

    static QString ToString(const QString &s);

    static QString ToString(const std::string &s);

    static QString ToString(const qlonglong &dt);

    static QString ToString(const QHash<QString, QString> &keyValues);

    static QString ToString(const QStringList &keyValues);

    static QString ToString(const char *pData, int iLength);

    static QString ToString(const char *pData);

    static QString ToString(const cx::uchar *pData, int iLength);

    static int FromString(const QString &s, const int &iDefault, bool *ok = nullptr);

    static bool FromString(const QString &s, const bool &bDefault, bool *ok = nullptr);

    static float FromString(const QString &s, const float &fDefault, bool *ok = nullptr);

    static double FromString(const QString &s, const double &dDefault, bool *ok = nullptr);

    ///attention error with : QString s = CommonString::FromString(s, s);
    static QString FromString(const QString &s, const QString &sDefault, bool *ok = nullptr);

    ///attention error with : qlonglong l = CommonString::FromString(s, l);
    static qlonglong FromString(const QString &s, const qlonglong &dtDefault, bool *ok = nullptr);

    ///attention error with : QHash<QString, QString> hash = CommonString::FromString(s, hash);
    static QHash<QString, QString>
    FromString(const QString &s, const QHash<QString, QString> &hashDefault, bool *ok = nullptr);

    ///attention error with : QStringList sl = CommonString::FromString(s, sl);
    static QStringList FromString(const QString &s, const QStringList &hashDefault, bool *ok = nullptr);

    static QString DateTimeToString(const qlonglong &ms);

    static QString DateTimeToString(int y, int m, int d, int h, int mi, int ms);

    static qlonglong DateTimeFromString(const QString &str);

    static QString FirstToken(QString &s, const QChar &sep);

    static QString RightToken(const QString &s, const QChar &sep);

    static QString ToHexString(const char *pData, int iLength);

    static QString ToHexString(const cx::uchar *pData, int iLength);

    static QString ToHexString(const QByteArray &data);

    static QString ToHexString(const char &v);

    static QString ToHexString(const cx::uchar &v);

    static QString ToHexString(const short &v);

    static QString ToHexString(const cx::ushort &v);

    static QString ToHexString(const int &v);

    static QString ToHexString(const cx::uint &v);

    static QString ToHexString(const float &v);

    static QString ToHexString(const double &v);

    static QByteArray FromHexString(const QString &s);

    static bool CompareName(const QString &name1, const QString &name2);

};


class QDialogHelper
{

public:
    static void ShowPrompt(const QString &sText, const QString &sTitle = "Prompt");

    static void ShowWarning(const QString &sText);

    static void ShowError(const QString &sText);

    static bool ShowQuery(const QString &sQuery, const QString &sTitle = "系统信息", const QString &sOk = "确定",
                          const QString &sCancle = "取消");

    static int ShowQuery3(const QString &sQuery);

    static void ShowTerminate(const QString &sText);

    static bool DialogOpen(QString &sFilePath, const QString &sExtension = QString{});

    static bool DialogSave(QString &sFilePath, const QString &sDefaultPath, const QString &sExtension = QString{});

    static QString DialogDir(const QString &sRootDir, const QString &sTitle = QString{});

    static bool DialogInput(const QString &sPrompt, QString &sVaule);

    static bool
    DialogInput2(const QString &sTitle, const QString &sValue1Title, const QString &sValue2Title, QString &sValue1,
                 QString &sValue2, int iValue1EchoMode = 0, int iValue2EchoMode = 0);

    static bool DialogInput3(const QString &sTitle, const QString &sValue1Title, const QString &sValue2Title,
                             const QString &sValue3Title, QString &sValue1, QString &sValue2, QString &sValue3,
                             int iValue1EchoMode = 0, int iValue2EchoMode = 0, int iValue3EchoMode = 0);

    static bool DialogInput4(const QString &sTitle, const QString &sValue1Title, const QString &sValue2Title,
                             const QString &sValue3Title, const QString &sValue4Title, QString &sValue1,
                             QString &sValue2, QString &sValue3, QString &sValue4, int iValue1EchoMode = 0,
                             int iValue2EchoMode = 0, int iValue3EchoMode = 0, int iValue4EchoMode = 0);

    static bool
    DialogCombox(const QString &sTitle, const QString &sPrompt, const QStringList &sVaules, QString &sValue);

};


class QWidgetHelper
{
public:
    static void setQSS(QWidget *widget, const QString &fp);

    static void setQSS(QWidget *widget, const std::string &fp);

    //set
    static int setItemValue16(QTableWidgetItem *oItem, const int &value);

    static int setItemValue(QTableWidgetItem *oItem, const int &value);

    static int setItemValue(QTableWidgetItem *oItem, const double &value);

    static int setItemValue(QTableWidgetItem *oItem, const long long &value);

    static int setItemValue(QTableWidgetItem *oItem, const char *value);

    static int setItemValue(QTableWidgetItem *oItem, const std::string &value);

    static int setItemValue(QTableWidgetItem *oItem, const QString &value);

    static void setItemValueDateTime(QTableWidgetItem *oItem, const long long &value);

    //get
    static int getItemValue16(QTableWidgetItem *oItem, const int &value);

    static int getItemValue(QTableWidgetItem *oItem, const int &value);

    static double getItemValue(QTableWidgetItem *oItem, const double &value);

    static long long getItemValue(QTableWidgetItem *oItem, const long long &value);

    static std::string getItemValue(QTableWidgetItem *oItem, const std::string &value);

    static QString getItemValue(QTableWidgetItem *oItem, const QString &value);

    static long long getItemValueDateTime(QTableWidgetItem *oItem, const long long &value);


    static int setItemValue16(QListWidgetItem *oItem, const int &value);

    static int setItemValue(QListWidgetItem *oItem, const int &value);

    static int setItemValue(QListWidgetItem *oItem, const double &value);

    static int setItemValue(QListWidgetItem *oItem, const long long &value);

    static int setItemValue(QListWidgetItem *oItem, const char *value);

    static int setItemValue(QListWidgetItem *oItem, const std::string &value);

    static int setItemValue(QListWidgetItem *oItem, const QString &value);

    static void setItemValueDateTime(QListWidgetItem *oItem, const long long &value);

    //

    static int setItemValue16(QStandardItem *oItem, const int &value);

    static int setItemValue(QStandardItem *oItem, const int &value);

    static int setItemValue(QStandardItem *oItem, const double &value);

    static int setItemValue(QStandardItem *oItem, const long long &value);

    static int setItemValue(QStandardItem *oItem, const char *value);

    static int setItemValue(QStandardItem *oItem, const std::string &value);

    static int setItemValue(QStandardItem *oItem, const QString &value);

    static int setItemValueEditRole(QStandardItem *oItem, const QString &value);

    static void setItemValueDateTime(QStandardItem *oItem, const long long &value);

    static std::vector<int> getSelectRows(QTableWidget *oGrid, QString *pRows = nullptr);

    static std::vector<int> deleteSelectRows(QTableWidget *oGrid);

    //扩展数据
    static void setItemValueUserEx(QTableWidgetItem *oItem, const int id, const int value);

    static int getItemValueUserEx(QTableWidgetItem *oItem, const int id, const int value);

    static void setItemValueUserEx(QListWidgetItem *oItem, const int id, const int value);

    static int getItemValueUserEx(QListWidgetItem *oItem, const int id, const int value);

};


class QMutexScope
{
private:
    QMutex *m_mutex;

public:
    inline QMutexScope(QMutex *mutex) : m_mutex(mutex)
    {
        m_mutex->lock();
    }

    inline QMutexScope(QMutex &mutex) : m_mutex(&mutex)
    {
        m_mutex->lock();
    }

    inline ~QMutexScope()
    { m_mutex->unlock(); }

};


#endif //CX_CT_X2_QT_HELPER_H
