#include "sidelistwidget.h"

SideListWidget::SideListWidget(QWidget *parent) : QListWidget(parent)
{
    setAcceptDrops(true);
    setDragEnabled(true);
}

void SideListWidget::dropEvent(QDropEvent *ev) {
    QListWidget::dropEvent(ev);
    delete takeItem(currentRow());
    emit itemMoved();
}

void SideListWidget::mouseReleaseEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::RightButton) {
        QMenu menu;

        QListWidgetItem *curItem = itemAt(ev->pos());
        if(curItem) {
            bool isEnabled = curItem->data(Qt::UserRole + 1).toBool();

            QAction *actSwitch = new QAction(isEnabled ? "隐藏" : "显示");
            menu.addAction(actSwitch);
            connect(actSwitch, &QAction::triggered, [this, curItem, isEnabled] {
                curItem->setData(Qt::UserRole + 1, !isEnabled);
                curItem->setForeground(isEnabled ? Qt::gray : Qt::black);
                emit itemEnabledChanged(curItem);
            });

            QAction *actRename = new QAction("重命名");
            menu.addAction(actRename);
            connect(actRename, &QAction::triggered, [this, curItem] {
                QInputDialog dialog;
                dialog.setInputMode(QInputDialog::InputMode::TextInput);
                dialog.resize(320, dialog.height());
                dialog.setWindowTitle("重命名");
                dialog.setLabelText("名称：");
                dialog.setOkButtonText("确定");
                dialog.setCancelButtonText("取消");
                if(!dialog.exec())
                    return;
                QString name = dialog.textValue();
                if(name == curItem->text())
                    return;
                if(name.isEmpty()) {
                    QMessageBox::warning(this, "错误", "名称为空");
                    return;
                }

                int cnt = count();
                for(int i = 0; i < cnt; i++) {
                    QListWidgetItem *sItem = item(i);
                    if(sItem != curItem && sItem->text() == name) {
                        QMessageBox::warning(this, "错误", "名称重复");
                        return;
                    }
                }

                QString oldName = curItem->text();
                curItem->setText(name);
                emit itemTextChanged(oldName, curItem);
            });

            QAction *actRemove = new QAction("删除");
            menu.addAction(actRemove);
            connect(actRemove, &QAction::triggered, [this, curItem] {
                takeItem(row(curItem));
                QString name = curItem->text();
                delete curItem;
                emit itemRemoved(name);
            });

            menu.addSeparator();

            connect(&menu, &QMenu::destroyed, [actSwitch, actRename, actRemove] {
                actSwitch->deleteLater();
                actRename->deleteLater();
                actRemove->deleteLater();
            });

            if(curItem->data(Qt::UserRole).toInt() == ItemType::Polygon) {
                QAction *actExport = new QAction("导出");
                menu.addAction(actExport);
                connect(actExport, &QAction::triggered, [this, curItem] {
                    emit itemExportRequested(curItem);
                });

                connect(&menu, &QMenu::destroyed, [actExport] { actExport->deleteLater(); });

                menu.addSeparator();
            }
        }

        QAction actNewPolygon("添加多边形");
        menu.addAction(&actNewPolygon);
        connect(&actNewPolygon, SIGNAL(triggered()), this, SIGNAL(newPolygon()));

        QAction actNewBackground("添加背景图片");
        menu.addAction(&actNewBackground);
        connect(&actNewBackground, SIGNAL(triggered()), this, SIGNAL(newBackground()));

        menu.move(cursor().pos());
        menu.exec();
    }

    QListWidget::mouseReleaseEvent(ev);
}
