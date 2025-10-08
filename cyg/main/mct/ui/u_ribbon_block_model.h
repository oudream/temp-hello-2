#ifndef CX_CT_X2_RIBBON_BLOCK_MODEL_H
#define CX_CT_X2_RIBBON_BLOCK_MODEL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QAction>
#include <QMap>

/**
 * @brief RibbonActionGroup - “功能名分组”
 * 例：布局组包含 2×2 / 1&3 / 等分；面板组包含 相机/切面/渲染/测量/日志/资产 显隐勾选
 */
struct URibbonActionGroup
{
    QString title;
    QList<QAction *> actions;
};

/**
 * @brief RibbonBlock - “功能块”
 * 上层功能块（项目/原图/重建/体编/体选/切面/渲染/测量/分割/配准/窗体/转换/试检）
 * 绑定到某个主窗口页面（0~4），Ribbon 顶栏点击后，Ribbon 会据此发出切页请求。
 */
struct URibbonBlock
{
    QString title;
    int pageIndex;                  ///< 0=项目, 1=原图, 2=重建, 3=3D, 4=试检
    QList<URibbonActionGroup> groups;
};

class URibbonBlockModel : public QObject
{
Q_OBJECT
public:
    explicit URibbonBlockModel(QObject *parent = nullptr);

    /// 注册所有 QAction（一次性）
    void createActions(QObject *parent);

    /// 获取功能块建模（懒构建）
    const QList<URibbonBlock> &getBlocks();

    /// 通过字符串 ID 取 QAction（供 Ribbon 绑定 UI）
    QAction *action(const QString &id) const;

private:
    QMap<QString, QAction *> _actions; ///< 全局动作表（按 ID 检索）
    QList<URibbonBlock> _blocks;       ///< 功能块列表（上层）
    bool _isBuilt = false;

    void buildModel();                 ///< 组织功能块与分组
};

#endif // CX_CT_X2_RIBBON_BLOCK_MODEL_H
