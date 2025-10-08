#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imagecanvas.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QLayout>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _layoutCentral = new QVBoxLayout(ui->centralwidget);
    _layoutCentral->setObjectName("LayoutCentral");
    _imageCanvas = new ImageCanvas(ui->centralwidget);
    _imageCanvas->setObjectName("imageCanvas");
    _imageCanvas->setMinimumSize(QSize(400, 300));
    _layoutCentral->addWidget(_imageCanvas);
    _layoutCentral->setStretch(0, 1);

    if (0)
    {
        // --- 通用替换逻辑：兼容有/无布局 ---
        if (auto placeholder = this->findChild<QWidget*>("imageCanvas")) {
            QWidget* parentW   = placeholder->parentWidget();
            QLayout* parentLay = parentW ? parentW->layout() : nullptr;

            auto* canvas = new ImageCanvas(parentW);
            canvas->setObjectName("imageCanvas");
            // 继承占位控件的一些属性，避免伸缩/尺寸策略改变
            canvas->setSizePolicy(placeholder->sizePolicy());
            canvas->setMinimumSize(placeholder->minimumSize());
            canvas->setMaximumSize(placeholder->maximumSize());
            canvas->setFocusPolicy(Qt::StrongFocus);

            if (parentLay) {
                // ✅ 不关心布局类型（VBox/Grid/Stacked…）
                if (QLayoutItem* it = parentLay->replaceWidget(placeholder, canvas)) {
                    delete it; // 删除旧的布局项包装（不是 widget 本体）
                }
                placeholder->hide();
                placeholder->deleteLater();
            } else {
                // 父控件没有布局 → 直接按原几何放置
                canvas->setGeometry(placeholder->geometry());
                placeholder->hide();
                placeholder->deleteLater();
            }
        }
    }

    // 连接菜单/工具栏动作
    connect(ui->actionOpen,    &QAction::triggered, this, &MainWindow::onOpen);
    connect(ui->actionSaveAs,  &QAction::triggered, this, &MainWindow::onSaveAs);
    connect(ui->actionSelect,  &QAction::triggered, this, &MainWindow::onToolSelect);
    connect(ui->actionRect,    &QAction::triggered, this, &MainWindow::onToolRect);
    connect(ui->actionCircle,  &QAction::triggered, this, &MainWindow::onToolCircle);
    connect(ui->actionClear,   &QAction::triggered, this, &MainWindow::onClearShapes);

    // 初始工具为选择/无绘制
    ui->actionSelect->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("打开图像"), QString(),
                                              tr("图像文件 (*.png *.jpg *.bmp *.tif *.tiff);;所有文件 (*.*)"));
    if (fn.isEmpty()) return;

    if (!_imageCanvas->loadImage(fn)) {
        QMessageBox::warning(this, tr("打开失败"), tr("无法加载：%1").arg(fn));
    }

//    auto *canvas = findChild<ImageCanvas*>("imageCanvas");
//    auto *canvas = ui->imageCanvas;
//    if (!canvas) return;
//
//    if (!canvas->loadImage(fn)) {
//        QMessageBox::warning(this, tr("打开失败"), tr("无法加载：%1").arg(fn));
//    }
}

void MainWindow::onSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("另存为"), QString(),
                                              tr("图像文件 (*.png *.jpg *.bmp *.tif *.tiff)"));
    if (fn.isEmpty()) return;

    auto *canvas = findChild<ImageCanvas*>("imageCanvas");
    if (!canvas) return;

    if (!canvas->saveComposited(fn)) {
        QMessageBox::warning(this, tr("保存失败"), tr("无法保存：%1").arg(fn));
    }
}

void MainWindow::onToolSelect()
{
    auto *canvas = findChild<ImageCanvas*>("imageCanvas");
    if (canvas) canvas->setTool(ImageCanvas::Tool::Select);
    ui->actionSelect->setChecked(true);
    ui->actionRect->setChecked(false);
    ui->actionCircle->setChecked(false);
}

void MainWindow::onToolRect()
{
    auto *canvas = findChild<ImageCanvas*>("imageCanvas");
    if (canvas) canvas->setTool(ImageCanvas::Tool::Rect);
    ui->actionSelect->setChecked(false);
    ui->actionRect->setChecked(true);
    ui->actionCircle->setChecked(false);
}

void MainWindow::onToolCircle()
{
    auto *canvas = findChild<ImageCanvas*>("imageCanvas");
    if (canvas) canvas->setTool(ImageCanvas::Tool::Circle);
    ui->actionSelect->setChecked(false);
    ui->actionRect->setChecked(false);
    ui->actionCircle->setChecked(true);
}

void MainWindow::onClearShapes()
{
    auto *canvas = findChild<ImageCanvas*>("imageCanvas");
    if (canvas) canvas->clearShapes();
}