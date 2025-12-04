#ifndef CX_CT_X2_U_SCENE_PAGE_H
#define CX_CT_X2_U_SCENE_PAGE_H

#include "pages/u_page_base.h"

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

#include <memory>

#include "m_scene_volume.h"   // FourViewController（装配/联动/导出等）
#include "m_mpr3d_view.h"     // Mpr3DView::ViewMode


class MScenePage : public UPageBase
{
Q_OBJECT
public:
    explicit MScenePage(QWidget *parent);

    ~MScenePage() override;

    const char *getName() const override;

    QWidget *getWidget() override;

    void onEnter() override;

    void onLeave() override;

    // 数据入口（两种任选其一）
    bool loadVtiFile(const QString &file);         // 示例：加载 VTI 文件
    void setImageData(vtkImageData *img);          // 或外部直接塞入影像

    void set3DViewMode(Mpr3DView::ViewMode m);

    void setWindowLevel(double win, double lev);

    // 导出 Axial（which=0）
    bool exportAxialRendered(const QString &path, int scale = 1);

    bool exportAxialHighQuality(const QString &path,
                                double spacingXY = -1.0, int sizeX = -1, int sizeY = -1,
                                bool linear = true, bool rescale16 = true);

    MSceneVolume *controller() const
    { return _controller.get(); }

    void reLayout(const QString &layoutMode)
    { _controller->reLayout(layoutMode, _host); }

private:
    void buildUi(QWidget *parent);     // 创建 2×2 视窗 + 顶部工具条
    void wireUp();      // 创建控制器并与 UI 事件对接

    // UI <-> 控制器 同步辅助
    void applyWlToUi(double w, double l);

    void updateWlFromUi();

private:
    QWidget *_root;
    QWidget *_host;

private: // 视图区
    QGridLayout *_rootLayout = nullptr;
    QVTKOpenGLNativeWidget *_w3D = nullptr;
    QVTKOpenGLNativeWidget *_wAx = nullptr;
    QVTKOpenGLNativeWidget *_wCo = nullptr;
    QVTKOpenGLNativeWidget *_wSa = nullptr;

private: // 工具条
    QHBoxLayout *_bar = nullptr;
    QPushButton *_btnLoad = nullptr;
    QComboBox *_cmb3DMode = nullptr;
    QLabel *_lblWin = nullptr;
    QSlider *_sldWin = nullptr;
    QDoubleSpinBox *_spnWin = nullptr;
    QLabel *_lblLev = nullptr;
    QSlider *_sldLev = nullptr;
    QDoubleSpinBox *_spnLev = nullptr;
    QPushButton *_btnPresetBone = nullptr;
    QPushButton *_btnPresetSoft = nullptr;
    QPushButton *_btnPresetMetal = nullptr;
    QCheckBox *_chkThick = nullptr;
    QSpinBox *_spnThickPx = nullptr;
    QLabel *_lblScroll = nullptr;
    QDoubleSpinBox *_spnScroll = nullptr;
    QPushButton *_btnExportAxialRendered = nullptr;
    QPushButton *_btnExportAxialHQ = nullptr;

    QPointer<MSceneVolume> _controller;

};

#endif // CX_CT_X2_U_SCENE_PAGE_H
