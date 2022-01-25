#include "addbackgrounddialog.h"

AddBackgroundDialog::AddBackgroundDialog(QWidget *parent)
    : QDialog(parent),
      mLabel(new QLabel("名称：")), mEdit(new QLineEdit),
      mLabelSelect(new QLabel("路径：")), mEditSelect(new QLineEdit), mBtnSelect(new QPushButton("...")),
      mBtnOK(new QPushButton("确定")), mBtnCancel(new QPushButton("取消"))
{
    mBtnSelect->setMaximumWidth(36);
    mBtnSelect->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *layTop = new QHBoxLayout;
    layTop->addWidget(mLabel);
    layTop->addWidget(mEdit);

    QHBoxLayout *laySelect = new QHBoxLayout;
    laySelect->addWidget(mLabelSelect);
    laySelect->addWidget(mEditSelect);
    laySelect->addWidget(mBtnSelect);

    QHBoxLayout *layBottom = new QHBoxLayout;
    layBottom->addStretch();
    layBottom->addWidget(mBtnOK);
    layBottom->addWidget(mBtnCancel);

    QVBoxLayout *layMain = new QVBoxLayout;
    layMain->addLayout(layTop);
    layMain->addLayout(laySelect);
    layMain->addLayout(layBottom);

    setLayout(layMain);

    connect(mBtnSelect, &QPushButton::clicked, [this] {
        QString filePath = QFileDialog::getOpenFileName(this, "选择图片");
        if(filePath.isEmpty())
            return;
        mEditSelect->setText(filePath);
    });
    connect(mBtnOK, &QPushButton::clicked, this, &AddBackgroundDialog::accept);
    connect(mBtnCancel, &QPushButton::clicked, this, &AddBackgroundDialog::reject);
}
