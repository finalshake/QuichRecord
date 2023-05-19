#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTextStream>
#include <QTextBlock>
#include <QFileDialog>
#include <QVector>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


// fold and unfold function
void MainWindow::on_fold_clicked()
{
    if(!unfold)
    {
        showText = new QPlainTextEdit(ui->centralwidget);
        edit = new QPushButton(ui->centralwidget);
        // create widget, resize, and show them with content if exist
        this->resize(500, 650);
        ui->centralwidget->resize(500, 650);
        ui->fold->setGeometry(10, 625, 25,22);
        ui->about->setGeometry(430, 625, 60, 24);
        ui->isCalc->setGeometry(50, 625, 85, 20);
        showText->setGeometry(10, 90, 480, 510);
        showText->setStyleSheet("font-size: 14px; font: oblique; color: green; background-color: black");
        edit->setGeometry(440, 605, 50, 18);
        edit->setStyleSheet("background-color: gray; color: white");
        edit->setText("modify");
        edit->show();
        connect(edit, SIGNAL(clicked(bool)), this, SLOT(on_edit_clicked()));

        showText->show();
        if(saveFile)
            reloadPlainText();

        unfold = true;
    }
    else
    {
        delete showText;
        delete edit;
        showText = nullptr;
        edit = nullptr;
        this->resize(500, 115);
        ui->centralwidget->resize(500, 115);
        ui->fold->setGeometry(10, 90, 25, 22);
        ui->about->setGeometry(430, 90, 60, 24);
        ui->isCalc->setGeometry(50, 90, 85, 20);

        unfold = false;
    }
}


void MainWindow::on_about_clicked()
{
    QMessageBox::information(this, "试验快速记录计算软件QuickRecord v1.0.0", "试验快速记录和计算。\nCopyright © 2021-2022 Shake (Weng Kaiqiang).\n All Rights Reserved.");
}

// the following code implements line edit behavior when focus and enter pressed
void MainWindow::on_x_val_focus()
{
    qDebug()<<"ok!";
    ui->x_val->selectAll();
    if(!saveFile)
    {
        QMessageBox::warning(this, "Warning", "请先选择要保存数据的文件");
        ui->y_val->setFocus();
    }
}

void MainWindow::on_x_val_returnPressed()
{
    ui->y_val->setFocus();
    ui->y_val->selectAll();
}


void MainWindow::on_y_val_returnPressed()
{
    ui->h_val->setFocus();
    ui->h_val->selectAll();
}


void MainWindow::on_h_val_returnPressed()
{
    ui->d_val->setFocus();
    ui->d_val->selectAll();
}


void MainWindow::on_d_val_returnPressed()
{
    if(!saveFile)
    {
        ui->x_val->setFocus();
        ui->x_val->selectAll();
        return;
    }
    if(!saveFile->open(QIODevice::Append | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot open save file. Please check or rechoose the file.");
        return;
    }
    //first write x,y,h,d to save file
    QString lineToWrite = ui->x_val->text() + "\t" + ui->y_val->text() + "\t" + ui->h_val->text() + "\t" + ui->d_val->text();
    if(hasPointVal)
    {
        lineToWrite = lineToWrite + " @" + stdX + ' ' + stdY + ' ' + stdH + ' ' + stdD;
    }
    qDebug() << lineToWrite;
    // to write
    // don't forget to check if unfold
    QTextStream out(saveFile);
    out << lineToWrite << "\n";
    saveFile->close();
    if(unfold){
        showText->appendPlainText(lineToWrite);
    }

    //then setfocus to the beginning x
    ui->x_val->setFocus();
    ui->x_val->selectAll();

    /* if the checkbox needs calculation, then calculate the cep and h, may plus d
     * use the input and std to do calculation, then overwrite the
     * result file, print to ui.
     * result file name: saveFileName.res,
     * result file format: n,cep,h,d,sigmax,sigmay,sigmah,sigmad */
    if(ui->isCalc->isChecked())
    {
        // check if there's std point first
        if(!hasPointVal)
            return;

        // read the input, the main uncertainty lays between h and d
        bool hasX, hasY, hasH, hasD;
        double inX = ui->x_val->text().toDouble(&hasX);
        double inY = ui->y_val->text().toDouble(&hasY);
        double inH = ui->h_val->text().toDouble(&hasH);
        double inD = ui->d_val->text().toDouble(&hasD);
        if(!hasX || !hasY)		// if x and y is invaild, just abandon the calculation
            return;
        // then std point. d == -1 means there's no d.
        double std_x = MainWindow::cutStd(ui->x_val->text(), stdX).toDouble();
        double std_y = MainWindow::cutStd(ui->y_val->text(), stdY).toDouble();
        double std_h = MainWindow::cutStd(ui->h_val->text(), stdH).toDouble();
        double std_d = MainWindow::cutStd(ui->d_val->text(), stdD).toDouble();
        // check whether there's result file,
        // if has, read the file, and extract the content from that file to init sigma
        // if not, init the sigma with 0
        double sigmaX = 0.0, sigmaY = 0.0, sigmaH = 0.0, sigmaD = 0.0;
        int n = 0;
        QString resFileName = saveFileName + ".res";
        QFile resFile(resFileName);
        if(resFile.exists())		// result file exists
        {							// change the init para
            if(resFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&resFile);
                QStringList para = in.readLine().split(',');
                n = para.at(0).toInt();
                sigmaX = para.at(4).toDouble();
                sigmaY = para.at(5).toDouble();
                if(hasH)
                    sigmaH = para.at(6).toDouble();
                if(hasD && stdD != "-1")
                    sigmaD = para.at(7).toDouble();

                resFile.close();
            }
        }
        Sigma sigX(sigmaX, n), sigY(sigmaY, n), sigH(sigmaH, n), sigD(sigmaD, n);
        // start to calculate
        sigX.cal_sigma(inX, std_x);
        sigY.cal_sigma(inY, std_y);
        if(hasH)
            sigH.cal_sigma(inH, std_h);
        // as to d, there's  conditions
        // 1. you got standard d in standard point info, then it must have d in input info, calculate the sigma d
        // 2. you got d in input info, but no standard d info, so calculate % per cent cep
        // 3. otherwise, ignore d's calculation
        if(stdD != "-1")
        {					// condition 1
            sigD.cal_sigma(inD, std_d);
        }
        else if(hasD)	// condition 2
        {
            // 计算千分之cep

        }

        // print to ui and write to result file
        double cep = Sigma::cal_cep(sigX.old_sigma, sigY.old_sigma);
        QString cepRes = QString::number(cep, 'f', 2);
        ui->CEP_res->setText(cepRes);
        double h = 0;
        QString hRes;
        if(sigH.n > 0)
        {
            h = Sigma::cal_h(sigH.old_sigma);
            hRes = QString::number(h, 'f', 2);
        }
        else
            hRes = "";
        ui->H_res->setText(hRes);
        // write to file
        if(!resFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "error", "Cannot open result file.");
            return;
        }
        QTextStream outRes(&resFile);
        QString dRes = "";
        if(sigD.n > 0)
        {
            dRes = QString::number(sigD.old_sigma, 'f', 2);
        }
        QString resToWrite = QString::number(sigX.n) + ',' + cepRes + ',' + hRes + ',' + dRes + ','
                + QString::number(sigX.old_sigma, 'f', 3) + ',' + QString::number(sigY.old_sigma, 'f', 3)
                + ',' + QString::number(sigH.old_sigma, 'f', 2) + ',' + QString::number(sigD.old_sigma, 'f', 2);
        outRes << resToWrite;
        resFile.close();

    }
}


// choose the file you want to save data in
void MainWindow::on_saveFile_clicked()
{
    SelectSaveFile *saveFileDialog = new SelectSaveFile();
    connect(saveFileDialog, SIGNAL(sendFileName(QString)), this, SLOT(recvFileName(QString)));
    saveFileDialog->exec();

    if(saveFileName.isEmpty())
        return;

    // before assign a new saveFile, don't forget to release the old memory
    if(!saveFile)
    {
        delete saveFile;
        saveFile = nullptr;
    }

    saveFile = new QFile(saveFileName);
    if(unfold)
        reloadPlainText();
}

void MainWindow::recvFileName(QString filename)
{
    saveFileName = filename;
}

// reload data content from file you choose to plaintextedit
void MainWindow::reloadPlainText()
{
    showText->clear();

    if(!saveFile->open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(saveFile);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        showText->appendPlainText(line);
    }
    saveFile->close();
}

// modify the data from plaintextedit
void MainWindow::on_edit_clicked()
{
    qDebug() << "edit";
    QMessageBox::StandardButton ret = QMessageBox::warning(this, "Warning", "修改数据将写入文件中，结果不可逆，确定修改？", QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes)	// accept modifying
    {
        if(!saveFile)	// no file selected
            return;

        // start to modify
        if(!saveFile->open(QIODevice::WriteOnly | QIODevice::Text))
            return;
        QTextStream out(saveFile);
        int lineNum = showText->blockCount();
        for(int i = 0; i < lineNum; i++)
        {
            QString line = showText->document()->findBlockByNumber(i).text();
            qDebug() << line;
            out << line << "\n";
        }
        saveFile->close();
    }
}

// choose point.csv file and load it
void MainWindow::on_pointFile_clicked()
{
    // show messagebox that let you choose point file
    QString pointFileName = QFileDialog::getOpenFileName(this, "选择存有标准点的文件", "", tr("point csv file(*.csv)"));
    if(pointFileName.isEmpty())
    {
        return;
    }
    qDebug() << pointFileName;

    // open the file and print point name to combobox
    // release the old one first
    if(!pointFile)
    {
        delete pointFile;
        pointFile = nullptr;
    }
    pointFile = new QFile(pointFileName);
    if(!pointFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "Cannot open point file.");
        return;
    }

    // read point file and print point name to combobox
    QTextStream in(pointFile);
    QStringList res;
    while(!in.atEnd())
    {
        QString pointName = in.readLine().split(",").at(0);
        res.append(pointName);
    }
    ui->point->clear();
    ui->point->addItems(res);

    pointFile->close();
    MainWindow::on_point_activated(0);
}


void MainWindow::on_point_activated(int index)
{
    qDebug() << index;
    if(!pointFile->open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    int i = 0;
    QString res;
    QTextStream in(pointFile);
    while(i <= index && !in.atEnd())
    {
        res = in.readLine();
        i++;
    }
    qDebug() << res;
    // point.csv structure
    // point name, zone num, x, y, h, L, B  d(if exists)
    QStringList splitRes = res.split(",");
    if(splitRes.size() < 5)
    {
        pointFile->close();
        hasPointVal = false;
        QMessageBox::warning(this, "Error", "the format of point file is not correct");
        return;
    }
    hasPointVal = true;
    stdX = splitRes.at(2);
    stdY = (splitRes.at(1)+splitRes.at(3));
    stdH = splitRes.at(4);
    if(splitRes.size() > 7)
        stdD = splitRes.at(7);
    else
        stdD = "-1";

    //qDebug() << QString::number(stdX - 1, 'f', 3) << ' ' << QString::number(stdY - 1, 'f', 3) << ' ' << stdD;

    pointFile->close();
}


// calculation section
void MainWindow::on_isCalc_stateChanged(int arg1)
{
    if(arg1 == 0)
        return;
    //else
    //    qDebug() << "checked";
    if(!saveFile)
        return;
    MainWindow::calFromFile(saveFile);
}

void MainWindow::calFromFile(QFile *file)
{
    if(!file->open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    bool hasStd = false;
    QString recordSec = "";
    QString stdSec = "";
    QTextStream in(file);
    QString line;

    Sigma sigX, sigY, sigH, sigD;

    while(!in.atEnd())
    {
        line = in.readLine();
        if(line.split('@').size() > 1)
        {
            // there's std point in the save file
            hasStd = true;
            recordSec = line.split('@').at(0);
            stdSec = line.split('@').at(1);
        }
        else {
            recordSec = line;
            hasStd = false;
        }

        QStringList recList = recordSec.split('\t');
        if(recList.size() != 4)
        {
            // wrong format
            qDebug() << recList.size() << " wrong format with input section";
            continue;
        }

        bool recHasH, recHasD, stdHasD = false;
        double rec_x = recList.at(0).toDouble();
        double rec_y = recList.at(1).toDouble();
        double rec_h = recList.at(2).toDouble(&recHasH);
        double rec_d = recList.at(3).toDouble(&recHasD);

        if(hasStd) 		// has standard point in file
        {
            QStringList stdList = stdSec.split(" ");
            if(stdList.size() != 4)
            {
                qDebug() << stdList.size() << " wrong format with standard point section";
                continue;
            }
            double std_x = MainWindow::cutStd(recList.at(0), stdList.at(0)).toDouble();
            double std_y = MainWindow::cutStd(recList.at(1), stdList.at(1)).toDouble();
            double std_h = MainWindow::cutStd(recList.at(2), stdList.at(2)).toDouble();
            double std_d;
            if(stdList.at(3) != "-1")
            {
                std_d = MainWindow::cutStd(recList.at(3), stdList.at(3)).toDouble();
                stdHasD = true;
            }
            // start to calculate
            sigX.cal_sigma(rec_x, std_x);
            sigY.cal_sigma(rec_y, std_y);
            if(recHasH)
                sigH.cal_sigma(rec_h, std_h);
            // as to d, there's  conditions
            // 1. you got standard d in standard point info, then it must have d in input info, calculate the sigma d
            // 2. you got d in input info, but no standard d info, so calculate % per cent cep
            // 3. otherwise, ignore d's calculation
            if(stdHasD)
            {					// condition 1
                sigD.cal_sigma(rec_d, std_d);
            }
            else if(recHasD)	// condition 2
            {
                // 计算千分之cep
                // d 是计算rec的d的平均值，最终用cep/平均d
                sigD.cal_ave(rec_d);
            }
        }
        else if(hasPointVal)			// don't have standard point in file
        {								// use stdX member in mainwindow class
            double std_x = MainWindow::cutStd(recList.at(0), stdX).toDouble();
            double std_y = MainWindow::cutStd(recList.at(1), stdY).toDouble();
            double std_h = MainWindow::cutStd(recList.at(2), stdH).toDouble();
            double std_d;
            if(stdD != "-1")
            {
                std_d = MainWindow::cutStd(recList.at(3), stdD).toDouble();
                stdHasD = true;
            }
            // start to calculate
            sigX.cal_sigma(rec_x, std_x);
            sigY.cal_sigma(rec_y, std_y);
            if(recHasH)
                sigH.cal_sigma(rec_h, std_h);
            // as to d, there's  conditions
            // 1. you got standard d in standard point info, then it must have d in input info, calculate the sigma d
            // 2. you got d in input info, but no standard d info, so calculate % per cent cep
            // 3. otherwise, ignore d's calculation
            if(stdHasD)
            {					// condition 1
                sigD.cal_sigma(rec_d, std_d);
            }
            else if(recHasD)	// condition 2
            {
                // 计算千分之cep
                // d 是计算rec的d的平均值，最终用cep/平均d
                // cache 中的标准点不顶用，通过文件中的@后的标准点信息来整理数据文件，然后单独从文件计算

            }
        }
        else
            continue;

    }

    file->close();

    /* calculate cep, h. display to ui. and write to file.
     * file name: saveFileName.res
     * file format: n,cep,h,d,sigmax,sigmay,sigmah,sigmad */
    double cep;
    QString cepRes;
    if(sigX.n > 0)
    {
        cep = Sigma::cal_cep(sigX.old_sigma, sigY.old_sigma);
        if(sigD.old_ave != 0)
        {
            cep = cep / sigD.old_ave;
            cepRes = QString::number(cep * 100, 'f', 2) + "%";
        }
        else
            cepRes = QString::number(cep, 'f', 2);
    }
    else
        cepRes = "";
    ui->CEP_res->setText(cepRes);
    double h = 0;
    QString hRes;
    if(sigH.n > 0)
    {
        h = Sigma::cal_h(sigH.old_sigma);
        hRes = QString::number(h, 'f', 2);
    }
    else
        hRes = "";
    ui->H_res->setText(hRes);
    // write to file
    QString resFileName = saveFileName + ".res";
    QFile resFile(resFileName);
    if(!resFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "error", "Cannot open result file.");
        return;
    }
    QTextStream out(&resFile);
    QString dRes = "";
    if(sigD.n > 0)
    {
        if(sigD.old_ave - 0 < 0.0000000001)
            dRes = QString::number(sigD.old_sigma, 'f', 2);
        else
            dRes = QString::number(sigD.old_ave, 'f', 2);
    }
    QString resToWrite = QString::number(sigX.n) + ',' + cepRes + ',' + hRes + ',' + dRes + ','
            + QString::number(sigX.old_sigma, 'f', 3) + ',' + QString::number(sigY.old_sigma, 'f', 3)
            + ',' + QString::number(sigH.old_sigma, 'f', 2) + ',' + QString::number(sigD.old_sigma, 'f', 2);
    out << resToWrite;
    resFile.close();
}


// func: 根据输入的小数点前的字符位数，截取std的小数点前的字符位数
QString MainWindow::cutStd(QString input, QString std)
{
    QString iBeforeDecimal = input.split('.').at(0);
    QString sBeforeDecimal = std.split('.').at(0);
    int sizeToCut = sBeforeDecimal.size() - iBeforeDecimal.size();
    return std.remove(0, sizeToCut);
}

void MainWindow::on_adjustData_clicked()
{
    MainWindow::on_fold_clicked();
    if(!saveFile)
        return;
    int size = showText->blockCount();
    int append = 1;
    QVector<bool> flag(size, false);
    QString toWrite;
    qDebug() << toWrite << ' ' << size;
    for(int i = 0; i < size; i++)
    {
        if(flag[i])
            continue;
        QString splitFileName = saveFileName + '_' + QString::number(append);
        append++;
        QString curLine = showText->document()->findBlockByNumber(i).text();
        QString splitWrite = "";
        splitWrite = curLine;
        toWrite = toWrite  + curLine + '\n';
        flag[i] = true;
        QString findStd = curLine.split('@').at(1);
        for(int j = i + 1; j < size; j++)
        {
            if(flag[j])
                continue;
            QString line = showText->document()->findBlockByNumber(j).text();
            if(findStd == line.split('@').at(1))
            {
                splitWrite = splitWrite + '\n' + line;
                flag[j] = true;
            }
            else
                continue;
        }
        toWrite = toWrite + splitWrite;
        QFile splitFile(splitFileName);
        if(splitFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream in(&splitFile);
            in << splitWrite;
            splitFile.close();
        }

    }
    showText->clear();
    showText->appendPlainText(toWrite);
}

