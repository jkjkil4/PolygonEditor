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

    void addPolygonItem(const QString &name, bool enabled = true);
    void addBackgroundItem(const QString &name, const QString &filePath, bool enabled = true);

    void setChanged(bool changed);

    bool verify();
    bool save(const QString &filePath);
    bool open(const QString &filePath);
    void clear();

    QString fmt(const QString &name, const Viewport::Polygon &polygon, double scale = 1);
    QString fmtAll(double scale = 1);

    double askScale(bool *ok = nullptr);
    void showPlainText(const QString &text);

private slots:
    void onAddPolygonClicked();
    void onAddBackgroundClicked();

private:
    Ui::MainWindow *ui;

    bool mIsChanged = false;
    QString mFilePath;
};

