#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QFile>
#include "selectsavefile.h"
#include "sigma.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void reloadPlainText();
    void calFromFile(QFile *file);
    QString cutStd(QString input, QString std);

    bool unfold = false;
    QPlainTextEdit *showText = nullptr;
    QPushButton *edit = nullptr;
    QFile *saveFile = nullptr;
    QFile *pointFile = nullptr;
    QString saveFileName = "";

    QString stdX, stdY, stdH, stdD;
    bool hasPointVal = false;

private slots:
    void on_fold_clicked();

    void on_about_clicked();

    void on_x_val_focus();

    void on_x_val_returnPressed();

    void on_y_val_returnPressed();

    void on_h_val_returnPressed();

    void on_d_val_returnPressed();

    void on_saveFile_clicked();

    void recvFileName(QString);

    void on_edit_clicked();

    void on_pointFile_clicked();

    void on_point_activated(int index);

    void on_isCalc_stateChanged(int arg1);

    void on_adjustData_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
