#ifndef CX_CT_X2_ICON_MANAGER_H
#define CX_CT_X2_ICON_MANAGER_H


#include <QIcon>
#include <QHash>
#include <QSize>
#include <QString>

struct IconSizes {
    QSize actionIcon; // 给 QToolButton::setIconSize
    QSize buttonSize; // 给 QToolButton::setFixedSize
};

class IconManager {
public:
    static IconManager& i();

    // 加载完整配置（含所有分组）
    bool loadAll(const QString& yamlPath);

    // 只加载/更新某分组
    bool loadGroup(const QString& yamlPath, const QString& groupName);

    // 统一 UI 尺寸
    IconSizes sizes() const;

    // 创建“彩色”QIcon（根据 YAML 主题颜色 + SVG currentColor）
    QIcon createColoredIcon(const QString& group, const QString& key) const;

    // 辅助：判断某 key 属于哪个分组（可选自定义）
    static QString inferGroupById(const QString& actionId);

private:
    IconManager() = default;

    struct Group {
        QString baseDir;                   // 分组根（可 qrc 或磁盘）
        QHash<QString, QString> relPaths;  // key -> 相对路径(svg)
    };

    // YAML 解析
    bool parseYaml(const QString& yamlPath, bool onlyOneGroup, const QString& groupNameOpt);

    // 路径解析：baseDir + relative（支持 ../ 与 :/）
    QString resolvePath(const QString& baseDir, const QString& relative) const;

private:
    // 配置数据
    QString _yamlBaseDir;
    QHash<QString, Group> groups_;     // name -> Group
    QString fallbackRel_;              // 默认相对路径（按组 baseDir 解析）
    bool dpiScale_ = true;
    IconSizes sizes_{QSize(20,20), QSize(90,32)};

    // 主题色（状态映射）：Normal/Active/Selected/Disabled × Off/On
    QString colDefault_     = "#CFD3DC";
    QString colNormalOff_   = "#CFD3DC";
    QString colNormalOn_    = "#4FC3F7";
    QString colActiveOff_   = "#FFFFFF";
    QString colActiveOn_    = "#9CD2FF";
    QString colSelectedOff_ = "#4FC3F7";
    QString colSelectedOn_  = "#4FC3F7";
    QString colDisabledOff_ = "#6E737B";
    QString colDisabledOn_  = "#6E737B";

};



#endif //CX_CT_X2_ICON_MANAGER_H
