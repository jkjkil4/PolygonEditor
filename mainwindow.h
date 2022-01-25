#pragma once

#include <QMainWindow>
#include <QInputDialog>
#include <QMessageBox>
#include <QPlainTextEdit>

#include <QCloseEvent>
#include <QFileInfo>
#include <QXmlStreamWriter>
#include <QDomDocument>

#include "AddBackgroundDialog/addbackgrounddialog.h"
#include "Viewport/viewport.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent *ev);

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addPolygonItem(const QString &name);
    void addBackgroundItem(const QString &name, const QString &filePath);

    void setChanged(bool changed);

    bool verify();
    bool save(const QString &filePath);
    bool open(const QString &filePath);
    void clear();

    QString fmt(const QString &name, const Viewport::Polygon &polygon);
    QString fmtAll();

    void showPlainText(const QString &text);

private slots:
    void onAddPolygonClicked();
    void onAddBackgroundClicked();

private:
    Ui::MainWindow *ui;

    bool mIsChanged = false;
    QString mFilePath;
};

