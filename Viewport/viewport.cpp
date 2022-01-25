#include "viewport.h"

Viewport::Viewport(QWidget *parent) : QWidget(parent), mTimerTwinkle(new QTimer(this))
{
    connect(mTimerTwinkle, &QTimer::timeout, [this] {
        mTwinkle = !mTwinkle;
        update();
    });

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setFocus();
}

void Viewport::addPolygon(const QString &name, const Polygon &polygon) {
    mMapPolygon[name] = polygon;
    dataChanged();
    update();
}
void Viewport::addBackground(const QString &name, const QString &filePath, const Background &background) {
    Background &bg = mMapBackground[name] = background;
    bg.pix.load(filePath);
    emit dataChanged();
    update();
}

void Viewport::setMode(Mode mode) {
    mMode = mode;
}
void Viewport::setOffset(QPoint offset) {
    mOffset = offset;
    update();
}

void Viewport::clear() {
    mSelection = "";
    mPolygonInd = 0;
    mMapPolygon.clear();
    mMapBackground.clear();
    mOffset = QPoint();
}

void Viewport::onItemClicked(QListWidgetItem *item) {
    mSelection = item->text();
    if(mMapBackground.contains(mSelection)) {
        mTwinkle = true;
        mPolygonInd = 0;
        mTimerTwinkle->start(500);
    } else mTimerTwinkle->stop();
    update();
}
void Viewport::onItemEnabledChanged(QListWidgetItem *item) {
    if(!setItemEnabled(mMapPolygon, item->text(), item->data(Qt::UserRole + 1).toBool()))
        setItemEnabled(mMapBackground, item->text(), item->data(Qt::UserRole + 1).toBool());
    emit dataChanged();
    update();
}
void Viewport::onItemTextChanged(const QString &oldName, QListWidgetItem *item) {
    if(!replace(mMapPolygon, oldName, item->text()))
        replace(mMapBackground, oldName, item->text());
    if(mSelection == oldName)
        mSelection = item->text();
    emit dataChanged();
    update();
}
void Viewport::onItemRemoved(const QString &name) {
    if(!remove(mMapPolygon, name))
        remove(mMapBackground, name);
    if(mSelection == name)
        mSelection = "";
    emit dataChanged();
    update();
}

template<typename T>
bool Viewport::setItemEnabled(QMap<QString, T> &map, const QString &name, bool enabled) {
    auto iter = map.find(name);
    if(iter != map.end()) {
        (*iter).base.enabled = enabled;
        update();
        return true;
    }
    return false;
}
template<typename T>
bool Viewport::replace(QMap<QString, T> &map, const QString &oldName, const QString &newName) {
    if(map.contains(oldName)) {
        T t = map.take(oldName);
        map[newName] = t;
        return true;
    }
    return false;
}
template<typename T>
bool Viewport::remove(QMap<QString, T> &map, const QString &name) {
    if(map.contains(name)) {
        map.remove(name);
        update();
        return true;
    }
    return false;
}

bool Viewport::contains(const QString &name) {
    return mMapPolygon.contains(name) || mMapBackground.contains(name);
}
Viewport::Base& Viewport::getBase(const QString &name) {
    if(mMapPolygon.contains(name))
        return mMapPolygon[name].base;
    if(mMapBackground.contains(name))
        return mMapBackground[name].base;
    throw;
}

void Viewport::keyReleaseEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_I) {
        if(mMode == Vertex && mMapPolygon.contains(mSelection)) {
            Polygon &polygon = mMapPolygon[mSelection];
            polygon.list.insert(
                        mPolygonInd,
                        mapFromGlobal(cursor().pos()) - mOffset - QPoint(polygon.base.x + width() / 2, polygon.base.y + height() / 2)
                        );
            emit dataChanged();
            update();
        }
    } else if(event->key() == Qt::Key_R) {
        if(mMode == Vertex && mMapPolygon.contains(mSelection)) {
            Polygon &polygon = mMapPolygon[mSelection];
            polygon.list.removeAt(mPolygonInd);
            if(mPolygonInd >= polygon.list.size())
                mPolygonInd = 0;
            emit dataChanged();
            update();
        }
    }
}

void Viewport::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::LeftButton) {
        if(mMode == Move && contains(mSelection))
            mLeftDragPos = ev->pos();
    } else if(ev->button() == Qt::MiddleButton) {
        mMiddleDragPos = ev->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}
void Viewport::mouseMoveEvent(QMouseEvent *ev) {
    if(ev->buttons() & Qt::LeftButton) {
        if(mMode == Move && contains(mSelection)) {
            Base &base = getBase(mSelection);
            base.x += ev->x() - mLeftDragPos.x();
            base.y += ev->y() - mLeftDragPos.y();
            mLeftDragPos = ev->pos();
            emit dataChanged();
            update();
        }
    } else if(ev->buttons() & Qt::MiddleButton) {
        mOffset += ev->pos() - mMiddleDragPos;
        mMiddleDragPos = ev->pos();
        emit dataChanged();
        update();
    }
}
void Viewport::mouseReleaseEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::LeftButton) {
        if(mMode == Vertex && mMapPolygon.contains(mSelection)) {
            Polygon &polygon = mMapPolygon[mSelection];
            if(!polygon.list.isEmpty()) {
                QPoint mouse = ev->pos() - mOffset - QPoint(polygon.base.x + width() / 2, polygon.base.y + height() / 2);
                auto sqr = [](double x) { return x * x; };
                auto getDis = [sqr, &mouse](const QPoint &prev, const QPoint &cur) -> double {
                    if( (prev.x() - mouse.x()) * (prev.x() - cur.x()) + (prev.y() - mouse.y()) * (prev.y() - cur.y()) < 0 )
                        return qSqrt(sqr(prev.x() - mouse.x()) + sqr(prev.y() - mouse.y()));
                    if( (cur.x() - mouse.x()) * (cur.x() - prev.x()) + (cur.y() - mouse.y()) * (cur.y() - prev.y()) < 0 )
                        return qSqrt(sqr(cur.x() - mouse.x()) + sqr(cur.y() - mouse.y()));
                    QPoint a = cur - prev;
                    QPoint v = mouse - prev;
                    return qSqrt(sqr(v.x()) + sqr(v.y()) - sqr((a.x() * v.x() + a.y() * v.y())) / (sqr(a.x()) + sqr(a.y())));
                };

                double minDis = getDis(*polygon.list.crbegin(), *polygon.list.cbegin());
                mPolygonInd = 0;

                QPoint prev = *polygon.list.cbegin();
                int ind = 1;
                for(auto iter = polygon.list.cbegin() + 1; iter != polygon.list.cend(); ++iter) {
                    const QPoint &cur = *iter;
                    double dis = getDis(prev, cur);

                    if(dis < minDis) {
                        minDis = dis;
                        mPolygonInd = ind;
                    }

                    prev = cur;
                    ind++;
                }
                update();
            }
        }
    } else if(ev->button() == Qt::MiddleButton) {
        setCursor(Qt::ArrowCursor);
    }
}
void Viewport::mouseDoubleClickEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::MiddleButton) {
        mOffset = QPoint();
        emit dataChanged();
        update();
    }
}

void Viewport::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(0, 0, width(), height(), QColor(250, 250, 250));

    QPoint offset = mOffset + QPoint(width() / 2, height() / 2);

    for(const Background &background : mMapBackground) {
        if(!background.base.enabled)
            continue;
        QPoint pos = QPoint(background.base.x - background.pix.width() / 2, background.base.y - background.pix.height() / 2) + offset;
        p.drawPixmap(pos.x(), pos.y(), background.pix);
    }
    for(auto iter = mMapPolygon.cbegin(); iter != mMapPolygon.cend(); ++iter) {
        const QString &name = iter.key();
        const Polygon &polygon = iter.value();
        if(!polygon.base.enabled)
            continue;

        QPoint prev = *polygon.list.crbegin();
        int ind = 0;
        for(const QPoint &cur : polygon.list) {
            QColor color = name == mSelection ? (mPolygonInd == ind ? QColor(128, 128, 255) : Qt::magenta) : Qt::black;
            p.setPen(QPen(color, 2));
            p.setBrush(color);

            QPoint pos = QPoint(polygon.base.x, polygon.base.y) + offset;
            p.drawLine(pos + prev, pos + cur);
            p.drawRect(QRect(pos - QPoint(3, 3) + cur, QSize(6, 6)));
            prev = cur;
            ind++;
        }

        p.setPen(Qt::black);
    }

    if(mTwinkle && mMapBackground.contains(mSelection)) {
        const Background &background = mMapBackground[mSelection];
        if(background.base.enabled) {
            int left = background.base.x - background.pix.width() / 2 + offset.x();
            int top = background.base.y - background.pix.height() / 2 + offset.y();
            int right = background.base.x + background.pix.width() / 2 + offset.x();
            int bottom = background.base.y + background.pix.height() / 2 + offset.y();
            p.setBrush(QColor(255, 0, 255, 20));
            p.setPen(QColor(255, 0, 255));
            p.drawRect(left, top, background.pix.width(), background.pix.height());
            p.fillRect(left - 1, top - 1, background.pix.width() + 2, 1, Qt::magenta);
            p.fillRect(left - 1, top - 1, 1, background.pix.height() + 2, Qt::magenta);
            p.fillRect(right, top, 1, background.pix.height(), Qt::magenta);
            p.fillRect(left, bottom, background.pix.width(), 1, Qt::magenta);
            p.setPen(Qt::black);
        }
    }

    p.fillRect(-8 + offset.x(), -1 + offset.y(), 16, 2, Qt::red);
    p.fillRect(-1 + offset.x(), -8 + offset.y(), 2, 16, Qt::red);
    if(mMapPolygon.contains(mSelection)) {
        const Polygon &polygon = mMapPolygon[mSelection];
        int x = polygon.base.x + offset.x();
        int y = polygon.base.y + offset.y();
        p.fillRect(x - 6, y - 1, 12, 2, Qt::blue);
        p.fillRect(x - 1, y - 6, 2, 12, Qt::blue);
    }

    p.drawText(QRect(0, 0, width(), height()), Qt::AlignLeft | Qt::AlignTop, "已选择:" + mSelection);
}
