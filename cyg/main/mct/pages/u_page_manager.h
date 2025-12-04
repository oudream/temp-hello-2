#ifndef CX_CT_X2_U_PAGE_MANAGER_H
#define CX_CT_X2_U_PAGE_MANAGER_H

#include <QObject>
#include <QMap>
#include <QStackedWidget>

#include "pages/u_page_base.h"


/**
 * @brief PageManager - 统一管理 UPageBase 的注册与切换（主窗只持有它）
 * 职责：注册→加入 QStackedWidget；提供 id↔index 映射；进入/离开生命周期
 */
class UPageManager : public QObject
{
Q_OBJECT
public:
    explicit UPageManager(QStackedWidget *stack, QObject *parent = 0);

    void registerPage(const QString &id, UPageBase *page);

    int indexOf(const QString &id) const;

    QString idAt(int index) const;

    void switchTo(int index);

    UPageBase *pageAt(int index) const;

    template<class TPage>
    TPage *pageTo() const
    {
        for (const auto &id : _orderedIds)
        {
            UPageBase *base = _pages.value(id, nullptr);
            if (auto casted = qobject_cast<TPage *>(base))
            {
                return casted;
            }
        }
        return nullptr;
    }

private:
    QStackedWidget *_stack = nullptr;
    QMap<QString, UPageBase *> _pages; // id->page
    QStringList _orderedIds;

};


#endif //CX_CT_X2_U_PAGE_MANAGER_H
