#ifndef CX_CT_X2_MAINWINDOW_H
#define CX_CT_X2_MAINWINDOW_H


#include <QMainWindow>
#include <QVBoxLayout>

#include "imagecanvas.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpen();
    void onSaveAs();
    void onToolSelect();
    void onToolRect();
    void onToolCircle();
    void onClearShapes();

private:
    Ui::MainWindow *ui;

    QVBoxLayout *_layoutCentral;
    ImageCanvas *_imageCanvas;

};

#endif //CX_CT_X2_MAINWINDOW_H
