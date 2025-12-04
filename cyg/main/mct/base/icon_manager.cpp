#include "icon_manager.h"

#include "colored_svg_icon.h"

#include <yaml-cpp/yaml.h>

#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QScreen>
#include <QApplication>
#include <QRegularExpression>

IconManager& IconManager::i()
{
    static IconManager inst;
    return inst;
}

QString IconManager::resolvePath(const QString& baseDir, const QString& relative) const
{
    // 1. 绝对路径 / qrc 直接返回
    if (relative.startsWith(":/") || QFileInfo(relative).isAbsolute())
        return QDir::cleanPath(relative);

    QString resolvedBase = baseDir;

    // 2. 如果 baseDir 是相对路径，则**基于 yamlBaseDir** 来转为绝对路径
    if (!resolvedBase.startsWith(":/") && !QFileInfo(resolvedBase).isAbsolute())
    {
        if (!_yamlBaseDir.isEmpty())
        {
            resolvedBase = QDir(_yamlBaseDir).filePath(resolvedBase);
        }
    }

    resolvedBase = QDir::cleanPath(resolvedBase);
    QString full = QDir(resolvedBase).filePath(relative);
    return QDir::cleanPath(full);
}

bool IconManager::parseYaml(const QString& yamlPath, bool onlyOneGroup, const QString& groupNameOpt)
{
    try
    {
        YAML::Node root = YAML::LoadFile(yamlPath.toStdString());

        if (auto d = root["defaults"])
        {
            if (auto n = d["fallback"]; n) fallbackRel_ = QString::fromStdString(n.as<std::string>());

            if (auto n = d["action_icon_size"]; n && n.IsSequence() && n.size() == 2)
                sizes_.actionIcon = QSize(n[0].as<int>(), n[1].as<int>());

            if (auto n = d["button_size"]; n && n.IsSequence() && n.size() == 2)
                sizes_.buttonSize = QSize(n[0].as<int>(), n[1].as<int>());

            if (auto n = d["enable_dpi_scale"]; n) dpiScale_ = n.as<bool>();

            if (auto c = d["colors"])
            {
                auto fromHex = [](const YAML::Node& node, const char* key, QString& out)
                {
                    if (node[key])
                    {
                        out = QString::fromStdString(node[key].as<std::string>());
                    }
                };
                fromHex(c, "default", colDefault_);
                fromHex(c, "normal_off", colNormalOff_);
                fromHex(c, "normal_on", colNormalOn_);
                fromHex(c, "active_off", colActiveOff_);
                fromHex(c, "active_on", colActiveOn_);
                fromHex(c, "selected_off", colSelectedOff_);
                fromHex(c, "selected_on", colSelectedOn_);
                fromHex(c, "disabled_off", colDisabledOff_);
                fromHex(c, "disabled_on", colDisabledOn_);
            }
        }

        // DPI scale
        if (dpiScale_)
        {
            const qreal dpr = QGuiApplication::primaryScreen()
                                  ? QGuiApplication::primaryScreen()->devicePixelRatio()
                                  : 1.0;
            sizes_.actionIcon = QSize(static_cast<int>(sizes_.actionIcon.width() * dpr),
                                      static_cast<int>(sizes_.actionIcon.height() * dpr));
            sizes_.buttonSize = QSize(static_cast<int>(sizes_.buttonSize.width() * dpr),
                                      static_cast<int>(sizes_.buttonSize.height() * dpr));
        }

        auto gs = root["groups"];
        if (!gs) return true;

        // 记录 YAML 所在目录
        _yamlBaseDir = QFileInfo(yamlPath).absolutePath();

        for (auto it = gs.begin(); it != gs.end(); ++it)
        {
            const QString gname = QString::fromStdString(it->first.as<std::string>());
            if (onlyOneGroup && gname != groupNameOpt) continue;

            const YAML::Node g = it->second;
            Group G;
            G.baseDir = QString::fromStdString(g["base_dir"].as<std::string>(""));

            auto icons = g["icons"];
            if (icons && icons.IsMap())
            {
                for (auto i = icons.begin(); i != icons.end(); ++i)
                {
                    const QString key = QString::fromStdString(i->first.as<std::string>());
                    const QString rel = QString::fromStdString(i->second.as<std::string>());
                    G.relPaths.insert(key, rel);
                }
            }
            groups_.insert(gname, G);
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool IconManager::loadAll(const QString& yamlPath)
{
    groups_.clear();
    return parseYaml(yamlPath, /*onlyOneGroup*/false, {});
}

bool IconManager::loadGroup(const QString& yamlPath, const QString& groupName)
{
    groups_.remove(groupName);
    return parseYaml(yamlPath, /*onlyOneGroup*/true, groupName);
}

IconSizes IconManager::sizes() const { return sizes_; }

QIcon IconManager::createColoredIcon(const QString& group, const QString& key) const
{
    // 找组
    auto it = groups_.find(group);
    if (it == groups_.end()) return {};

    const Group& G = it.value();

    for (auto itRel = G.relPaths.constBegin(); itRel != G.relPaths.constEnd(); ++itRel)
    {
        qDebug() << itRel.key() << " = " << itRel.value();
    }

    // 取相对路径
    QString rel = G.relPaths.value(key);
    QString full;

    auto tryRead = [&](const QString& path)-> QByteArray
    {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly))
            return f.readAll();
        return {};
    };

    if (!rel.isEmpty())
    {
        full = resolvePath(G.baseDir, rel);
        if (full.startsWith(":/") || QFileInfo::exists(full))
        {
            QByteArray svg = tryRead(full);
            if (!svg.isEmpty())
            {
                // // 这样即使 YAML 没设置某些字段，也不会出现纯黑或不可见颜色的情况。
                // const QColor colDefault     = colors.value("default",      QColor("#C0C8D2")); // 未匹配到具体状态时作为兜底颜色
                //
                // // Normal（正常状态） - Off 表示未选中
                // const QColor colNormalOff   = colors.value("normal_off",   colDefault);
                // // Normal - On 表示选中（按钮 setCheckable & setChecked = true 时）
                // const QColor colNormalOn    = colors.value("normal_on",    colors.value("selected_on", QColor("#4FC3F7")));
                //
                // // Active（鼠标悬停状态） - Off 表示未选中但 hover 中
                // // ⚠ 建议不要使用纯白(#FFFFFF)，在浅背景 UI 中会“看不见”
                // const QColor colActiveOff   = colors.value("active_off",   colNormalOff.lighter(120));
                // const QColor colActiveOn    = colors.value("active_on",    colNormalOn.lighter(120)); // hover + 选中组合
                //
                // // Disabled（禁用状态） - UI 设 setEnabled(false) 时使用
                // const QColor colDisabledOff = colors.value("disabled_off", QColor("#6E737B"));
                // const QColor colDisabledOn  = colors.value("disabled_on",  colDisabledOff); // 选中 + 禁用几乎不用区分，保持一致
                //
                // // Selected（被选中/高亮激活，例如 QToolBar 中被激活的当前 Action）
                // const QColor colSelectedOff = colors.value("selected_off", colNormalOn);
                // const QColor colSelectedOn  = colors.value("selected_on",  colNormalOn.darker(120));

                // 验证 "#RRGGBB" 串；有效则转 QColor，否则返回 invalid
                auto colorFromHexIfValid = [](const QString& s) -> QColor {
                    static const QRegularExpression re(R"(^#([A-Fa-f0-9]{6})$)");
                    const QString t = s.trimmed();
                    if (re.match(t).hasMatch()) {
                        QColor c(t);
                        if (c.isValid()) return c;
                    }
                    return QColor(); // invalid
                };

                // 当成员色无效时，用“亮眼默认方案”
                auto pick = [&](const QString& memberHex, const QColor& fallback) -> QColor {
                    const QColor c = colorFromHexIfValid(memberHex);
                    return c.isValid() ? c : fallback;
                };
                // -----------------------------
                // 3) 亮眼默认方案（深色 UI 友好）
                // -----------------------------
                const QColor DEF_DEFAULT      ("#B0B8C3"); // 中性灰：兜底
                const QColor DEF_NORMAL_OFF   ("#C0C8D2"); // 未选中常态：淡灰蓝
                const QColor DEF_NORMAL_ON    ("#4FC3F7"); // 选中常态：品牌蓝
                const QColor DEF_ACTIVE_OFF   ("#7AB8FF"); // 悬停未选中：亮一些
                const QColor DEF_ACTIVE_ON    ("#29B6F6"); // 悬停选中：湖蓝
                const QColor DEF_SELECTED_OFF ("#64B5F6"); // 被激活（Selected）未选中
                const QColor DEF_SELECTED_ON  ("#1E88E5"); // 被激活选中：更深
                const QColor DEF_DISABLED     ("#6E737B"); // 禁用：冷灰

                // -----------------------------
                // 4) 从“成员字符串”提色；无效则回退到默认方案
                //    （成员变量示例：colNormalOff_、colActiveOn_ 等，类型为 QString）
                // -----------------------------
                const QColor cDefault     = pick(colDefault_,     DEF_DEFAULT);
                const QColor cNormalOff   = pick(colNormalOff_,   DEF_NORMAL_OFF);
                const QColor cNormalOn    = pick(colNormalOn_,    DEF_NORMAL_ON);
                const QColor cActiveOff   = pick(colActiveOff_,   DEF_ACTIVE_OFF);
                const QColor cActiveOn    = pick(colActiveOn_,    DEF_ACTIVE_ON);
                const QColor cSelectedOff = pick(colSelectedOff_, DEF_SELECTED_OFF);
                const QColor cSelectedOn  = pick(colSelectedOn_,  DEF_SELECTED_ON);
                const QColor cDisabledOff = pick(colDisabledOff_, DEF_DISABLED);
                const QColor cDisabledOn  = pick(colDisabledOn_,  DEF_DISABLED);

                // -----------------------------
                // 5) 构造引擎并写满 8 种状态
                //    QIcon::Mode    = Normal / Active(hover) / Disabled / Selected
                //    QIcon::State   = Off / On（未选中 / 选中；需要按钮 setCheckable(true) 才能触发 On）
                // -----------------------------
                auto* eng = new ColoredSvgIconEngine(svg); // 将 currentColor → #RRGGBB 后渲染
                eng->setDefaultColor(cDefault);            // 最兜底颜色（不匹配任何状态时）

                // Normal（正常状态） - Off 表示未选中
                eng->setColor(QIcon::Normal,   QIcon::Off, cNormalOff);
                eng->setColor(QIcon::Normal,   QIcon::On,  cNormalOn);

                // Active（鼠标悬停状态） - Off 表示未选中但 hover 中
                // ⚠ 建议不要使用纯白(#FFFFFF)，在浅背景 UI 中会“看不见”
                eng->setColor(QIcon::Active,   QIcon::Off, cActiveOff);
                eng->setColor(QIcon::Active,   QIcon::On,  cActiveOn);

                // Selected（被选中/高亮激活，例如 QToolBar 中被激活的当前 Action）
                eng->setColor(QIcon::Selected, QIcon::Off, cSelectedOff);
                eng->setColor(QIcon::Selected, QIcon::On,  cSelectedOn);

                // Disabled（禁用状态） - UI 设 setEnabled(false) 时使用
                // 选中 + 禁用几乎不用区分，保持一致
                eng->setColor(QIcon::Disabled, QIcon::Off, cDisabledOff);
                eng->setColor(QIcon::Disabled, QIcon::On,  cDisabledOn);

                return QIcon(eng);
            }
        }
    }

    // fallback
    if (!fallbackRel_.isEmpty())
    {
        full = resolvePath(G.baseDir, fallbackRel_);
        if (full.startsWith(":/") || QFileInfo::exists(full))
        {
            QByteArray svg = tryRead(full);
            if (!svg.isEmpty())
            {
                auto* eng = new ColoredSvgIconEngine(svg);
                eng->setDefaultColor(colDefault_);
                return QIcon(eng);
            }
        }
    }
    return {};
}

QString IconManager::inferGroupById(const QString& actionId)
{
    // 简单规则：以你的 AppIds 命名习惯划分（可按需增强/改表驱动）
    if (actionId.startsWith("kAction_Layout_")) return "windowing";
    if (actionId.startsWith("kAction_Panel_")) return "panels";
    return "common";
}
