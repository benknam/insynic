#include "insynic_draggablekey.h"
#include <QMouseEvent>
#include <QPainter>
#include <QCursor>

InsynicDraggableKey::InsynicDraggableKey(const VirtualKey &key, QWidget *parent)
    : QWidget(parent)
    , m_key(key)
    , m_dragging(false)
    , m_screenWidth(1080)
    , m_screenHeight(1920)
{
    setFixedSize(key.size, key.size);
    setCursor(Qt::OpenHandCursor);
    setAttribute(Qt::WA_TranslucentBackground);
}

InsynicDraggableKey::~InsynicDraggableKey()
{
}

VirtualKey InsynicDraggableKey::getKey() const
{
    return m_key;
}

void InsynicDraggableKey::setKey(const VirtualKey &key)
{
    m_key = key;
    setFixedSize(key.size, key.size);
    update();
}

void InsynicDraggableKey::updatePosition(int x, int y)
{
    m_key.relX = m_screenWidth > 0 ? static_cast<double>(x) / m_screenWidth : 0.5;
    m_key.relY = m_screenHeight > 0 ? static_cast<double>(y) / m_screenHeight : 0.5;
    move(x - m_key.size / 2, y - m_key.size / 2);
}

void InsynicDraggableKey::updateSize(int size)
{
    m_key.size = size;
    setFixedSize(size, size);
    int screenX = m_key.getScreenX(m_screenWidth);
    int screenY = m_key.getScreenY(m_screenHeight);
    move(screenX - size / 2, screenY - size / 2);
    update();
}

void InsynicDraggableKey::setScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

void InsynicDraggableKey::setScrcpyPosition(int x, int y)
{
    m_scrcpyX = x;
    m_scrcpyY = y;
}

void InsynicDraggableKey::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_globalDragStart = event->globalPos();
        m_globalWindowStart = this->pos();
        setCursor(Qt::ClosedHandCursor);
        emit draggingStarted();
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void InsynicDraggableKey::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        QPoint globalDelta = event->globalPos() - m_globalDragStart;
        QPoint newGlobalPos = m_globalWindowStart + globalDelta;
        
        int newX = newGlobalPos.x() + m_key.size / 2 - m_scrcpyX;
        int newY = newGlobalPos.y() + m_key.size / 2 - m_scrcpyY;
        
        newX = qMax(0, qMin(m_screenWidth, newX));
        newY = qMax(0, qMin(m_screenHeight, newY));
        
        updatePosition(newX, newY);
        move(m_scrcpyX + newX - m_key.size / 2, m_scrcpyY + newY - m_key.size / 2);
        emit positionChanged(newX, newY);
        event->accept();
    }
    QWidget::mouseMoveEvent(event);
}

void InsynicDraggableKey::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        setCursor(Qt::OpenHandCursor);
        emit draggingEnded();
        event->accept();
    }
    QWidget::mouseReleaseEvent(event);
}

void InsynicDraggableKey::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit configRequested(this);
    event->accept();
}

void InsynicDraggableKey::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QBrush brush(QColor(255, 100, 100, 150));
    painter.setBrush(brush);
    painter.setPen(QPen(QColor(255, 0, 0, 200), 2));
    
    painter.drawEllipse(rect().adjusted(2, 2, -2, -2));
    
    painter.setPen(QPen(Qt::white, 12, Qt::SolidLine, Qt::RoundCap));
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter, m_key.keyName);
}