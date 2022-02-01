#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    ui->actNew->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_FileIcon));
//    ui->actOpen->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirOpenIcon));
//    ui->actSave->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DialogSaveButton));

    setWindowTitle("PolygonEditor");

    connect(ui->actNew, &QAction::triggered, [this] {
        if(!verify())
            return;
        setChanged(false);
        clear();
    });
    connect(ui->actOpen, &QAction::triggered, [this] {
        QString filePath = QFileDialog::getOpenFileName(this, "打开文件", "", "PolygonEditor Project (*pep)");
        if(filePath.isEmpty())
            return;
        if(!verify())
            return;
        if(open(filePath)) {
            mFilePath = filePath;
            setChanged(false);
        } else QMessageBox::warning(this, "错误", "打开失败");
    });
    connect(ui->actSave, &QAction::triggered, [this] {
        if(mFilePath.isEmpty())
            mFilePath = QFileDialog::getSaveFileName(this, "保存文件", "untitled.pep", "PolygonEditor Project (*pep)");
        if(mFilePath.isEmpty())
            return;
        if(save(mFilePath)) {
            setChanged(false);
        } else QMessageBox::warning(this, "错误", "保存失败");
    });
    connect(ui->actSaveAs, &QAction::triggered, [this] {
        QString filePath = QFileDialog::getSaveFileName(this, "另存为", "untitled.pep", "PolygonEditor Project (*pep)");
        if(filePath.isEmpty())
            return;
        if(save(filePath)) {
            mFilePath = filePath;
            setChanged(false);
        }
    });
    connect(ui->actExportAll, &QAction::triggered, [this] {
        bool ok;
        double scale = askScale(&ok);
        if(ok) showPlainText(fmtAll(scale));
    });
    connect(ui->actAbout, &QAction::triggered, [this] {
        QString info;
        info += "PolygonEditor v0.1.0<br>";
        info += "作者: jkjkil4<br>";
        info += "gitee: <a href=https://gitee.com/jkjkil4/PolygonEditor>https://gitee.com/jkjkil4/PolygonEditor</a>";
        QMessageBox::about(this, "关于", info);
    });
    connect(ui->actAboutQt, &QAction::triggered, [this] {
        QMessageBox::aboutQt(this);
    });
    connect(ui->listWidget, &SideListWidget::newPolygon, this, &MainWindow::onAddPolygonClicked);
    connect(ui->listWidget, &SideListWidget::newBackground, this, &MainWindow::onAddBackgroundClicked);

    connect(ui->viewport, &Viewport::dataChanged, [this] { setChanged(true); });

    connect(ui->listWidget, &SideListWidget::itemMoved, [this] { setChanged(true); });
    connect(ui->listWidget, &SideListWidget::itemClicked, ui->viewport, &Viewport::onItemClicked);
    connect(ui->listWidget, &SideListWidget::itemEnabledChanged, ui->viewport, &Viewport::onItemEnabledChanged);
    connect(ui->listWidget, &SideListWidget::itemTextChanged, ui->viewport, &Viewport::onItemTextChanged);
    connect(ui->listWidget, &SideListWidget::itemRemoved, ui->viewport, &Viewport::onItemRemoved);
    connect(ui->listWidget, &SideListWidget::itemExportRequested, [this](QListWidgetItem *item) {
        bool ok;
        double scale = askScale(&ok);
        if(ok) showPlainText(fmt(item->text(), ui->viewport->getPolygon(item->text()), scale));
    });

    connect(ui->rbArrow, QOverload<bool>::of(&QRadioButton::clicked), [this](bool enabled) {
        if(enabled)
            ui->viewport->setMode(Viewport::Arrow);
    });
    connect(ui->rbVertex, QOverload<bool>::of(&QRadioButton::clicked), [this](bool enabled) {
        if(enabled)
            ui->viewport->setMode(Viewport::Vertex);
    });
    connect(ui->rbMove, QOverload<bool>::of(&QRadioButton::clicked), [this](bool enabled) {
        if(enabled)
            ui->viewport->setMode(Viewport::Move);
    });

    connect(ui->actAddPolygon, &QAction::triggered, this, &MainWindow::onAddPolygonClicked);
    connect(ui->actAddBackground, &QAction::triggered, this, &MainWindow::onAddBackgroundClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addPolygonItem(const QString &name, bool enabled) {
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, SideListWidget::Polygon);
    item->setData(Qt::UserRole + 1, enabled);
    item->setBackground(QColor(235, 255, 245));
    item->setForeground(enabled ? Qt::black : Qt::gray);
    ui->listWidget->addItem(item);
}
void MainWindow::addBackgroundItem(const QString &name, const QString &filePath, bool enabled) {
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, SideListWidget::Background);
    item->setData(Qt::UserRole + 1, enabled);
    item->setData(Qt::UserRole + 2, filePath);
    item->setForeground(enabled ? Qt::black : Qt::gray);
    ui->listWidget->addItem(item);
}

void MainWindow::setChanged(bool changed) {
    QString title = "PolygonEditor";
    if(!mFilePath.isEmpty())
        title += " - " + QFileInfo(mFilePath).fileName();
    if(changed)
        title += " *";
    setWindowTitle(title);
    mIsChanged = changed;
}

bool MainWindow::verify() {
    if(!mIsChanged)
        return true;
    int ret = QMessageBox::information(this, "提示", "文件未保存，要保存吗?", "是", "否", "取消");
    switch(ret) {
    case 0: {
        if(mFilePath.isEmpty())
            mFilePath = QFileDialog::getSaveFileName(this, "保存文件", "untitled.pep", "PolygonEditor Project (*pep)");
        if(mFilePath.isEmpty())
            return false;
        bool ok = save(mFilePath);
        if(!ok) {
            int ret = QMessageBox::warning(this, "错误", "文件保存失败，放弃吗?", "是", "否");
            if(ret == 1)
                return false;
        }
        return true;
    }
    case 1:
        return true;
    }
    return false;
}
bool MainWindow::save(const QString &filePath) {
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QPoint offset = ui->viewport->getOffset();

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("PolygonEditorProject");
    xml.writeAttribute("Version", "1");
    xml.writeAttribute("x", QString::number(offset.x()));
    xml.writeAttribute("y", QString::number(offset.y()));
    int count = ui->listWidget->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QString name = item->text();
        SideListWidget::ItemType type = (SideListWidget::ItemType)item->data(Qt::UserRole).toInt();
        if(type == SideListWidget::Polygon) {
            const Viewport::Polygon &polygon = ui->viewport->getPolygon(name);
            xml.writeStartElement("Polygon");
            xml.writeAttribute("name", name);
            xml.writeAttribute("x", QString::number(polygon.base.x));
            xml.writeAttribute("y", QString::number(polygon.base.y));
            xml.writeAttribute("enabled", QString::number(polygon.base.enabled));
            for(const QPoint &pos : polygon.list)
                xml.writeTextElement("v", QString::number(pos.x()) + "," + QString::number(pos.y()));
            xml.writeEndElement();
        } else {
            const Viewport::Background &background = ui->viewport->getBackground(name);
            xml.writeStartElement("Background");
            xml.writeAttribute("name", name);
            xml.writeAttribute("x", QString::number(background.base.x));
            xml.writeAttribute("y", QString::number(background.base.y));
            xml.writeAttribute("enabled", QString::number(background.base.enabled));
            xml.writeCharacters(item->data(Qt::UserRole + 2).toString());
            xml.writeEndElement();
        }
    }
    xml.writeEndElement();
    xml.writeEndDocument();

    file.close();
    return true;
}
bool MainWindow::open(const QString &filePath) {
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QDomDocument doc;
    if(!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomNode root = doc.documentElement();
    if(root.nodeName() != "PolygonEditorProject")
        return false;

    ui->listWidget->clear();
    ui->viewport->clear();

    QDomElement elem = root.toElement();
    ui->viewport->setOffset(QPoint(elem.attribute("x").toInt(), elem.attribute("y").toInt()));

    QDomNode node = root.firstChild();
    while(!node.isNull()) {
        QDomElement elem = node.toElement();
        if(!elem.isNull()) {
            if(elem.tagName() == "Polygon") {
                Viewport::Polygon polygon;

                QString name = elem.attribute("name");
                polygon.base.x = elem.attribute("x").toInt();
                polygon.base.y = elem.attribute("y").toInt();
                polygon.base.enabled = elem.attribute("enabled").toInt();

                QDomNode node = elem.firstChild();
                while(!node.isNull()) {
                    QDomElement elem = node.toElement();
                    if(!elem.isNull() && elem.tagName() == "v") {
                        QStringList xy = elem.text().split(",");
                        if(xy.size() == 2)
                            polygon.list << QPoint(xy[0].toInt(), xy[1].toInt());
                    }
                    node = node.nextSibling();
                }

                addPolygonItem(name, polygon.base.enabled);
                ui->viewport->addPolygon(name, polygon);
            } else if(elem.tagName() == "Background") {
                Viewport::Background background;

                QString name = elem.attribute("name");
                QString filePath = elem.text();
                background.base.x = elem.attribute("x").toInt();
                background.base.y = elem.attribute("y").toInt();
                background.base.enabled = elem.attribute("enabled").toInt();

                addBackgroundItem(name, filePath, background.base.enabled);
                ui->viewport->addBackground(name, filePath, background);
            }
        }
        node = node.nextSibling();
    }

    return true;
}
void MainWindow::clear() {
    ui->listWidget->clear();
    ui->viewport->clear();
    mFilePath = "";
}

QString MainWindow::fmt(const QString &name, const Viewport::Polygon &polygon, double scale) {
    QString result;
    result += QString("%1  [x: %2  y: %3]\n").arg(name)
            .arg(qRound(polygon.base.x * scale * 100) / 100.0).arg(qRound(polygon.base.y * scale * 100) / 100.0);
    result += "ds_list_add(\n\tlistVertex,\n\t";
    int times = 0;
    bool hasPrev = false;
    for(const QPoint &pos : polygon.list) {
        if(hasPrev) {
            result += ", ";
        } hasPrev = true;

        times++;
        if(times == 6) {
            result += "\n\t";
            times = 0;
        }

        result += QString("[%1, %2]").arg(qRound(pos.x() * scale * 100) / 100.0).arg(qRound(pos.y() * scale * 100) / 100.0);
    }
    result += "\n);";
    return result;
}

QString MainWindow::fmtAll(double scale) {
    QString total;
    int count = ui->listWidget->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem *item = ui->listWidget->item(i);
        SideListWidget::ItemType type = (SideListWidget::ItemType)item->data(Qt::UserRole).toInt();
        if(type == SideListWidget::ItemType::Polygon) {
            const Viewport::Polygon &polygon = ui->viewport->getPolygon(item->text());
            total += fmt(item->text(), polygon, scale);
            total += "\n\n";
        }
    }
    return total;
}

double MainWindow::askScale(bool *ok) {
    QInputDialog dialog;
    dialog.setInputMode(QInputDialog::DoubleInput);
    dialog.resize(320, dialog.height());
    dialog.setWindowTitle("输入缩放");
    dialog.setLabelText("缩放倍数：");
    dialog.setDoubleMinimum(0.5);
    dialog.setDoubleValue(1);
    dialog.setOkButtonText("确定");
    dialog.setCancelButtonText("取消");
    if(dialog.exec()) {
        if(ok) *ok = true;
        return dialog.doubleValue();
    }
    if(ok) *ok = false;
    return 1;
}
void MainWindow::showPlainText(const QString &text) {
    QPlainTextEdit *edit = new QPlainTextEdit(text);
    edit->setReadOnly(true);
    edit->setAttribute(Qt::WA_DeleteOnClose);
    edit->setWordWrapMode(QTextOption::NoWrap);
    edit->resize(800, 608);
    edit->show();
}

void MainWindow::onAddPolygonClicked() {
    QInputDialog dialog;
    dialog.setInputMode(QInputDialog::InputMode::TextInput);
    dialog.resize(320, dialog.height());
    dialog.setWindowTitle("添加多边形");
    dialog.setLabelText("名称：");
    dialog.setOkButtonText("确定");
    dialog.setCancelButtonText("取消");
    if(!dialog.exec())
        return;
    QString name = dialog.textValue().trimmed();
    if(name.isEmpty()) {
        QMessageBox::warning(this, "错误", "名称为空");
        return;
    }

    int cnt = ui->listWidget->count();
    for(int i = 0; i < cnt; i++) {
        if(ui->listWidget->item(i)->text() == name) {
            QMessageBox::warning(this, "错误", "名称重复");
            return;
        }
    }

    addPolygonItem(name);
    ui->viewport->addPolygon(name);
}
void MainWindow::onAddBackgroundClicked() {
    AddBackgroundDialog dialog;
    dialog.setWindowTitle("添加背景图片");
    if(!dialog.exec())
        return;
    QString name = dialog.name().trimmed();
    QString filePath = dialog.filePath();
    if(name.isEmpty()) {
        QMessageBox::warning(this, "错误", "名称为空");
        return;
    }
    if(!QFileInfo(filePath).exists()) {
        QMessageBox::warning(this, "错误", "路径不存在");
        return;
    }

    int cnt = ui->listWidget->count();
    for(int i = 0; i < cnt; i++) {
        if(ui->listWidget->item(i)->text() == name) {
            QMessageBox::warning(this, "错误", "名称重复");
            return;
        }
    }

    addBackgroundItem(name, filePath);
    ui->viewport->addBackground(name, filePath);
}

void MainWindow::closeEvent(QCloseEvent *ev) {
    if(mIsChanged && !verify())
        ev->ignore();
}
