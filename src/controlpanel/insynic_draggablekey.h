#ifndef INSYNIC_DRAGGABLE_KEY_H
#define INSYNIC_DRAGGABLE_KEY_H

#include <QWidget>
#include <QPoint>
#include "insynic_virtualkey.h"

class InsynicDraggableKey : public QWidget
{
    Q_OBJECT

public:
    explicit InsynicDraggableKey(const VirtualKey &key, QWidget *parent = nullptr);
    ~InsynicDraggableKey();
    
    VirtualKey getKey() const;
    void setKey(const VirtualKey &key);
    
    void updatePosition(int x, int y);
    void updateSize(int size);
    void setScreenSize(int width, int height);
    void setScrcpyPosition(int x, int y);
    
    int getScreenX() const { return m_key.getScreenX(m_screenWidth); }
    int getScreenY() const { return m_key.getScreenY(m_screenHeight); }

signals:
    void positionChanged(int x, int y);
    void sizeChanged(int size);
    void configRequested(InsynicDraggableKey *key);
    void deleteRequested(InsynicDraggableKey *key);
    void draggingStarted();
    void draggingEnded();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    VirtualKey m_key;
    bool m_dragging;
    QPoint m_globalDragStart;
    QPoint m_globalWindowStart;
    int m_screenWidth;
    int m_screenHeight;
    int m_scrcpyX;
    int m_scrcpyY;
};

#endif