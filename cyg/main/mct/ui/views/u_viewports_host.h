#ifndef UI_VIEWS_UVIEWPORTS_HOST_H
#define UI_VIEWS_UVIEWPORTS_HOST_H

#include <QWidget>
#include <QGridLayout>

class UViewportAxial;

class UViewportCoronal;

class UViewportSagittal;

class UViewport3D;

/**
 * @brief UViewportsHost - 四视图宿主（3D页核心）
 *
 * 职责：
 *  - 管理四个子视图（Axial/Coronal/Sagittal/3D）
 *  - 支持布局模式切换：2×2、1&3、等分
 *  - 每个视图包裹在持久 Cell（含外部缩放按钮+滚动条）
 *
 * 生命周期：
 *  - 由 MainWindow 注册 ThreeDProxyPage 时创建
 *  - 与 3D 页共生共灭，不随布局切换销毁视图
 */
class UViewportsHost : public QWidget
{
Q_OBJECT
public:
    explicit UViewportsHost(QWidget *parent = nullptr);

    ~UViewportsHost() override;

    /// 切换布局模式（grid_2x2 / layout_1x3 / equalize）
    void setLayoutMode(const QString &mode);

    /// 当前布局模式
    QString currentLayoutMode() const
    { return _layoutMode; }

    /// 等分（窗口 resize 后可调用）
    void equalize();

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    // 四个视图
    UViewportAxial *_vAxial = nullptr;
    UViewportCoronal *_vCoronal = nullptr;
    UViewportSagittal *_vSagittal = nullptr;
    UViewport3D *_v3D = nullptr;

    // 四个 cell（持久容器：视图+右侧按钮条）
    QWidget *_cellAxial = nullptr;
    QWidget *_cellCoronal = nullptr;
    QWidget *_cellSagittal = nullptr;
    QWidget *_cell3D = nullptr;

    // 布局
    QGridLayout *_grid = nullptr;
    QString _layoutMode = "grid_2x2";

    // 内部工具
    QWidget *makeViewportCell(QWidget *view, const QString &title);

    void rebuild();
};

#endif // UI_VIEWS_UVIEWPORTS_HOST_H
