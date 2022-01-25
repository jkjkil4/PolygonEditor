#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

#include <QHBoxLayout>
#include <QVBoxLayout>

class AddBackgroundDialog : public QDialog
{
public:
    explicit AddBackgroundDialog(QWidget *parent = nullptr);

    QString name() { return mEdit->text(); }
    QString filePath() { return mEditSelect->text(); }

private:
    QLabel *mLabel;
    QLineEdit *mEdit;

    QLabel *mLabelSelect;
    QLineEdit *mEditSelect;
    QPushButton *mBtnSelect;

    QPushButton *mBtnOK, *mBtnCancel;
};
