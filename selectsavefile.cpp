#include "selectsavefile.h"
#include "ui_selectsavefile.h"
#include <QFileDialog>
#include <QMessageBox>

SelectSaveFile::SelectSaveFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectSaveFile)
{
    ui->setupUi(this);
}

SelectSaveFile::~SelectSaveFile()
{
    delete ui;
}

void SelectSaveFile::on_pushButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "选择保存的文件", "", "", nullptr, QFileDialog::DontConfirmOverwrite);
    ui->filename->setText(filename);
}


void SelectSaveFile::on_buttonBox_accepted()
{
    if(ui->filename->text().isEmpty())
    {
        QMessageBox::warning(this, "Warning", "未选择文件");
    }
    else
    {
        emit sendFileName(ui->filename->text());
    }
}


void SelectSaveFile::on_buttonBox_rejected()
{
    this->destroy();
}

