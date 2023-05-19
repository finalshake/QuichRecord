#ifndef SELECTSAVEFILE_H
#define SELECTSAVEFILE_H

#include <QDialog>

namespace Ui {
class SelectSaveFile;
}

class SelectSaveFile : public QDialog
{
    Q_OBJECT

public:
    explicit SelectSaveFile(QWidget *parent = nullptr);
    ~SelectSaveFile();

private slots:
    void on_pushButton_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

signals:
    void sendFileName(QString);

private:
    Ui::SelectSaveFile *ui;
};

#endif // SELECTSAVEFILE_H
