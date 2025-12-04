#ifndef CX_CT_X2_U_RIBBON_BUILDER_H
#define CX_CT_X2_U_RIBBON_BUILDER_H

#include <QAction>
#include <QIcon>
#include <QMap>
#include <QString>
#include <QObject>
#include <vector>
#include "u_ribbon_block.h"

#include "base/icon_manager.h"
#include "u_ribbon_handler.h"

// 小工具：记录“组标题 -> URibbonActionGroup”
struct RibbonGroupStore {
    // key 用组标题
    QMap<QString, URibbonActionGroup> groups;

    // 取得组（若不存在则创建）
    URibbonActionGroup& getOrCreate(const QString& title) {
        auto it = groups.find(title);
        if (it == groups.end()) {
            URibbonActionGroup g;
            g.title = title;
            it = groups.insert(title, g);
        }
        return it.value();
    }

    // 导出为 QVector 以符合 block.groups 的类型
    QVector<URibbonActionGroup> toVector() const {
        QVector<URibbonActionGroup> v;
        v.reserve(groups.size());
        for (auto it = groups.cbegin(); it != groups.cend(); ++it) {
            v.push_back(it.value());
        }
        return v;
    }
};

// THandler 是你各模块对应的 Handler 类型，如 WindowingHandler / ReconstructionHandler ...
template <class THandler>
class RibbonBuilder {
public:
    RibbonBuilder(QObject* parent,
                  QMap<QString, QAction*>& actions, // key: id 的指针常量（与原设计一致）
                  IURibbonHandler* handler)
        : parent_(parent),
          actions_(actions),
          handler_(qobject_cast<THandler*>(handler)) {}

    // 声明一个组，返回“组标题字符串”（后面 make 时直接传这个就好）
    QString group(const QString& title) {
        groups_.getOrCreate(title); // 确保组存在
        return title;
    }

    // 创建/复用 QAction，并登记到组；slot 可为 nullptr
    // 注意：不再自动推断图标组，必须显式传入 iconGroupKey
    QAction* make(const QString& groupKey,
                  const char* id,
                  const QString& text,
                  void (THandler::*slot)() = nullptr,
                  const QString& groupTitle = QString())
    {
        QAction* a = actions_.value(id, nullptr);
        const QString key = QString::fromUtf8(id);

        if (!a) {
            // 直接使用调用方传入的 iconGroupKey 生成图标
            QIcon ic = IconManager::i().createColoredIcon(groupKey, key);
            a = new QAction(ic, text, parent_);
            actions_.insert(id, a);
        } else {
            a->setText(text);
            // 若已有 action 但还没有图标，补齐（仍然使用调用方传入的 iconGroupKey）
            if (a->icon().isNull()) {
                a->setIcon(IconManager::i().createColoredIcon(groupKey, key));
            }
        }

        if (handler_ && slot) {
            QObject::connect(a, &QAction::triggered, handler_, slot);
        }

        if (!groupTitle.isEmpty()) {
            auto& g = groups_.getOrCreate(groupTitle);
            g.actions.push_back(a);
        }
        return a;
    }

    // 生成最终的 URibbonBlock
    URibbonBlock buildBlock(const QString& blockTitle, AppIds::ERibbonBlock blockIndex) const {
        URibbonBlock b;
        b.title = blockTitle;
        b.blockIndex = blockIndex;
        b.groups = groups_.toVector();
        return b;
    }

private:
    QObject* parent_;
    QMap<QString, QAction*>& actions_;
    THandler* handler_;
    RibbonGroupStore groups_;
};

// 语法糖：宏可减少重复写 QObject::tr(...) / AppIds 常量
// RB_GROUP: 注册并返回分组标题
#define RB_GROUP(builder, groupTextConst) \
    (builder).group(QObject::tr(groupTextConst))

// RB_MAKE: 显式传入 iconGroupKey（不再 infer）
// 用法示例：RB_MAKE(b, AppIds::kWin_Reset, "重置窗宽窗位", "image", WindowingHandler, onReset, grpWin);
#define RB_MAKE(builder, groupKeyConst, idConst, textConst, handlerClass, slot, groupTitle) \
    (builder).make((groupKeyConst), (idConst), QObject::tr(textConst), &handlerClass::slot, (groupTitle))

#endif // CX_CT_X2_U_RIBBON_BUILDER_H
