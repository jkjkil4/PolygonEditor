#pragma once

#include <QListWidget>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>

#include <QMouseEvent>

class SideListWidget : public QListWidget
{
    Q_OBJECT
protected:
    void dropEvent(QDropEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

public:
    enum ItemType { Polygon, Background };

    explicit SideListWidget(QWidget *parent = nullptr);

signals:
    void newPolygon();
    void newBackground();

    void itemMoved();
    void itemEnabledChanged(QListWidgetItem *item);
    void itemTextChanged(const QString &oldName, QListWidgetItem *item);
    void itemRemoved(const QString &name);
    void itemExportRequested(QListWidgetItem *item);
};
