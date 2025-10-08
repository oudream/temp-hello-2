#include "u_page_manager.h"
#include <QStackedWidget>
#include "ui/pages/u_page_base.h"

UPageManager::UPageManager(QStackedWidget *stack, QObject *parent)
        : QObject(parent), _stack(stack)
{}

void UPageManager::registerPage(const QString &id, UPageBase *page)
{
    _pages.insert(id, page);
    _orderedIds << id;
    _stack->addWidget(page->getWidget());
}

int UPageManager::indexOf(const QString &id) const
{ return _orderedIds.indexOf(id); }

QString UPageManager::idAt(int index) const
{
    if (index >= 0 && index < _orderedIds.size()) return _orderedIds.at(index);
    return QString();
}

void UPageManager::switchTo(int index)
{
    if (index < 0 || index >= _stack->count()) return;
    const int cur = _stack->currentIndex();
    if (cur == index) return;
    if (cur >= 0)
    {
        const QString curId = idAt(cur);
        if (_pages.contains(curId)) _pages[curId]->onLeave();
    }
    _stack->setCurrentIndex(index);
    const QString newId = idAt(index);
    if (_pages.contains(newId)) _pages[newId]->onEnter();
}
