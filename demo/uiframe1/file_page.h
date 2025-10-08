#ifndef CX_CT_X2_FILE_PAGE_H
#define CX_CT_X2_FILE_PAGE_H


#include <QWidget>

class FilePage : public QWidget {
Q_OBJECT
public:
    explicit FilePage(QWidget *parent=nullptr);

private:
    QWidget* buildLeftButtons();
    QWidget* buildRightArea();
};


#endif //CX_CT_X2_FILE_PAGE_H
