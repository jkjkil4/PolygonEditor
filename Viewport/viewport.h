#pragma once

#include <QWidget>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QListWidgetItem>
#include <QTimer>
#include <QtMath>

class Viewport : public QWidget
{
    Q_OBJECT
protected:
    void keyReleaseEvent(QKeyEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void mouseDoubleClickEvent(QMouseEvent *ev) override;
    void paintEvent(QPaintEvent *) override;

public:
    enum Mode { Arrow, Vertex, Move };

    struct Base
    {
        int x = 0, y = 0;
        bool enabled = true;
    };
    struct Polygon
    {
        Polygon() : base() {}
        Base base;
        QList<QPoint> list;
    };
    struct Background
    {
        Background() : base() {}
        Base base;
        QPixmap pix;
    };

    explicit Viewport(QWidget *parent = nullptr);

    void addPolygon(const QString &name, const Polygon &polygon = Polygon());
    void addBackground(const QString &name, const QString &filePath, const Background &background = Background());

    void setMode(Mode mode);
    void setOffset(QPoint offset);

    const Polygon& getPolygon(const QString &name) { return mMapPolygon[name]; }
//    const QMap<QString, Polygon>& getAllPolygon() { return mMapPolygon; }
    const Background& getBackground(const QString &name) { return mMapBackground[name]; }
    QPoint getOffset() { return mOffset; }
    void clear();

public slots:
    void onItemClicked(QListWidgetItem *item);
    void onItemEnabledChanged(QListWidgetItem *item);
    void onItemTextChanged(const QString &oldName, QListWidgetItem *item);
    void onItemRemoved(const QString &name);

signals:
    void dataChanged();

private:
    template<typename T>
    bool setItemEnabled(QMap<QString, T> &map, const QString &name, bool enabled);
    template<typename T>
    bool replace(QMap<QString, T> &map, const QString &oldName, const QString &newName);
    template<typename T>
    bool remove(QMap<QString, T> &map, const QString &name);

    QPoint mOffset, mLeftDragPos, mMiddleDragPos;

    bool contains(const QString &name);
    Base &getBase(const QString &name);
    QMap<QString, Polygon> mMapPolygon;
    QMap<QString, Background> mMapBackground;

    Mode mMode;
    QString mSelection;

    QTimer *mTimerTwinkle;
    bool mTwinkle = false;
    int mPolygonInd = 0;
};
