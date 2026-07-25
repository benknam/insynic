#include "insynic_filebrowser.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QShortcut>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QMenuBar>

InsynicFileBrowserDialog::InsynicFileBrowserDialog(InsynicFileManager *fm,
                                                   const QString &deviceName,
                                                   QWidget *parent)
    : QDialog(parent)
    , m_fileManager(fm)
    , m_currentPath("/mnt")
    , m_deviceName(deviceName)
    , m_currentViewMode(ViewMode::Detail)
    , m_clipboardMode(ClipboardMode::None)
    , m_sortOrder(Qt::AscendingOrder)
    , m_sortColumn(0)
    , m_lastSelectedItem(nullptr)
    , m_selectedTile(nullptr)
{
    QString title = tr("File Manager");
    if (!m_deviceName.isEmpty()) {
        title += " - " + m_deviceName;
    }
    setWindowTitle(title);
    resize(1100, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    setupToolbar();

    m_pathBar = new QLineEdit(this);
    m_pathBar->setText(m_currentPath);
    layout->addWidget(m_pathBar);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({tr("Name"), tr("Size"), tr("Date"), tr("Permissions"), tr("Type")});
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSortingEnabled(true);
    m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    layout->addWidget(m_treeWidget);

    m_tileWidget = new QWidget(this);
    m_tileLayout = new QGridLayout(m_tileWidget);
    m_tileLayout->setSpacing(8);
    m_tileLayout->setContentsMargins(4, 4, 4, 4);
    m_tileLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_tileWidget->setVisible(false);
    layout->addWidget(m_tileWidget);

    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusLabel->setFixedHeight(24);
    m_statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(m_statusLabel);

    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked,
            this, &InsynicFileBrowserDialog::onItemDoubleClicked);
    connect(m_treeWidget, &QTreeWidget::itemClicked,
            this, &InsynicFileBrowserDialog::onItemClicked);
    connect(m_treeWidget, &QTreeWidget::itemPressed,
            this, &InsynicFileBrowserDialog::onItemPressed);
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested,
            this, &InsynicFileBrowserDialog::onContextMenuRequested);
    connect(m_pathBar, &QLineEdit::returnPressed,
            this, &InsynicFileBrowserDialog::onPathReturnPressed);

    QShortcut *copyShortcut = new QShortcut(QKeySequence::Copy, this);
    QShortcut *cutShortcut = new QShortcut(QKeySequence::Cut, this);
    QShortcut *pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    QShortcut *delShortcut = new QShortcut(QKeySequence::Delete, this);
    QShortcut *selectAllShortcut = new QShortcut(QKeySequence::SelectAll, this);

    connect(copyShortcut, &QShortcut::activated, this, &InsynicFileBrowserDialog::onCopyClicked);
    connect(cutShortcut, &QShortcut::activated, this, &InsynicFileBrowserDialog::onCutClicked);
    connect(pasteShortcut, &QShortcut::activated, this, &InsynicFileBrowserDialog::onPasteClicked);
    connect(delShortcut, &QShortcut::activated, this, &InsynicFileBrowserDialog::onDeleteClicked);
    connect(selectAllShortcut, &QShortcut::activated, this, &InsynicFileBrowserDialog::onSelectAllClicked);

    loadPath(m_currentPath);
}

InsynicFileBrowserDialog::~InsynicFileBrowserDialog()
{
}

void
InsynicFileBrowserDialog::setupToolbar()
{
    m_toolbar = new QToolBar(this);

    m_toolbar->addAction(tr("Up"), this, &InsynicFileBrowserDialog::onUpClicked);
    m_toolbar->addAction(tr("Refresh"), this, &InsynicFileBrowserDialog::onRefreshClicked);
    m_toolbar->addSeparator();
    m_toolbar->addAction(tr("Upload"), this, &InsynicFileBrowserDialog::onUploadClicked);
    m_toolbar->addAction(tr("Download"), this, &InsynicFileBrowserDialog::onDownloadClicked);
    m_toolbar->addAction(tr("Delete"), this, &InsynicFileBrowserDialog::onDeleteClicked);
    m_toolbar->addSeparator();
    m_toolbar->addAction(tr("New Folder"), this, &InsynicFileBrowserDialog::onNewFolderClicked);
    m_toolbar->addSeparator();

    m_sortNameAction = m_toolbar->addAction(tr("Name"));
    m_sortSizeAction = m_toolbar->addAction(tr("Size"));
    m_sortDateAction = m_toolbar->addAction(tr("Date"));
    m_sortTypeAction = m_toolbar->addAction(tr("Type"));
    m_viewModeAction = m_toolbar->addAction(tr("Tile"));
    m_selectAllAction = m_toolbar->addAction(tr("Select All"));
    m_deselectAllAction = m_toolbar->addAction(tr("Deselect All"));

    m_sortNameAction->setCheckable(true);
    m_sortSizeAction->setCheckable(true);
    m_sortDateAction->setCheckable(true);
    m_sortTypeAction->setCheckable(true);
    m_sortNameAction->setChecked(true);

    m_toolbar->addSeparator();

    connect(m_sortNameAction, &QAction::triggered, this, &InsynicFileBrowserDialog::onSortByNameClicked);
    connect(m_sortSizeAction, &QAction::triggered, this, &InsynicFileBrowserDialog::onSortBySizeClicked);
    connect(m_sortDateAction, &QAction::triggered, this, &InsynicFileBrowserDialog::onSortByDateClicked);
    connect(m_sortTypeAction, &QAction::triggered, this, &InsynicFileBrowserDialog::onSortByTypeClicked);
    connect(m_viewModeAction, &QAction::triggered, this, &InsynicFileBrowserDialog::onViewModeChanged);
    connect(m_selectAllAction, &QAction::triggered, this, &InsynicFileBrowserDialog::onSelectAllClicked);
    connect(m_deselectAllAction, &QAction::triggered, this, &InsynicFileBrowserDialog::onDeselectAllClicked);

    layout()->addWidget(m_toolbar);
}

void
InsynicFileBrowserDialog::retranslateUi()
{
    QString title = tr("File Manager");
    if (!m_deviceName.isEmpty()) {
        title += " - " + m_deviceName;
    }
    setWindowTitle(title);

    m_sortNameAction->setText(tr("Name"));
    m_sortSizeAction->setText(tr("Size"));
    m_sortDateAction->setText(tr("Date"));
    m_sortTypeAction->setText(tr("Type"));
    m_viewModeAction->setText(m_currentViewMode == ViewMode::Detail ? tr("Tile") : tr("Detail"));
    m_selectAllAction->setText(tr("Select All"));
    m_deselectAllAction->setText(tr("Deselect All"));

    QStringList headers = {tr("Name"), tr("Size"), tr("Date"), tr("Permissions"), tr("Type")};
    m_treeWidget->setHeaderLabels(headers);
}

void
InsynicFileBrowserDialog::loadPath(const QString &path)
{
    bool ok;
    QVector<AdbFileInfo> files = m_fileManager->listFiles(path, &ok);
    if (!ok) {
        m_statusLabel->setText(tr("Failed to list files"));
        return;
    }

    m_currentPath = path;
    m_pathBar->setText(path);
    m_treeWidget->clear();

    for (const AdbFileInfo &info : files) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_treeWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Unchecked);
        item->setText(0, info.name);
        if (info.isDir) {
            item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            item->setText(1, "");
        } else {
            item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
            if (info.size < 1024) {
                item->setText(1, QString("%1 B").arg(info.size));
            } else if (info.size < 1024 * 1024) {
                item->setText(1, QString("%1 KB").arg(info.size / 1024));
            } else {
                item->setText(1, QString("%1 MB").arg(info.size / (1024 * 1024)));
            }
        }
        item->setText(2, info.date);
        item->setText(3, info.permissions);
        QString fileType = info.isDir ? tr("Folder") : QFileInfo(info.name).suffix().toUpper();
        if (fileType.isEmpty() && !info.isDir) {
            fileType = tr("Unknown");
        }
        item->setText(4, fileType);
        item->setData(0, Qt::UserRole, info.path);
        item->setData(0, Qt::UserRole + 1, info.isDir);
        item->setData(0, Qt::UserRole + 2, info.size);
        item->setData(0, Qt::UserRole + 3, info.date);
        item->setData(0, Qt::UserRole + 4, info.owner);
        item->setData(0, Qt::UserRole + 5, info.group);
        item->setData(0, Qt::UserRole + 6, info.time);
        item->setData(0, Qt::UserRole + 7, info.isLink);
    }

    m_treeWidget->sortByColumn(m_sortColumn, m_sortOrder);

    m_statusLabel->setText(QString("%1 items").arg(files.size()));
}

void
InsynicFileBrowserDialog::onItemDoubleClicked(QTreeWidgetItem *item, int)
{
    bool isDir = item->data(0, Qt::UserRole + 1).toBool();
    if (isDir) {
        QString path = item->data(0, Qt::UserRole).toString();
        loadPath(path);
    }
}

void
InsynicFileBrowserDialog::onItemClicked(QTreeWidgetItem *item, int column)
{
    if (column == 0) {
        item->setCheckState(0, item->checkState(0) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
    }
}

void
InsynicFileBrowserDialog::onItemPressed(QTreeWidgetItem *item, int)
{
    if (QApplication::keyboardModifiers() & Qt::ShiftModifier && m_lastSelectedItem) {
        QList<QTreeWidgetItem*> allItems;
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
            allItems.append(m_treeWidget->topLevelItem(i));
        }
        int start = allItems.indexOf(m_lastSelectedItem);
        int end = allItems.indexOf(item);
        int min = qMin(start, end);
        int max = qMax(start, end);
        for (int i = min; i <= max; ++i) {
            allItems[i]->setSelected(true);
            allItems[i]->setCheckState(0, Qt::Checked);
        }
    }
    m_lastSelectedItem = item;
}

void
InsynicFileBrowserDialog::onPathReturnPressed()
{
    loadPath(m_pathBar->text());
}

void
InsynicFileBrowserDialog::onUpClicked()
{
    if (m_currentPath == "/" || m_currentPath.isEmpty()) {
        return;
    }
    QString parent = QDir::cleanPath(m_currentPath + "/..");
    if (parent.isEmpty() || parent == ".") {
        parent = "/";
    }
    m_currentPath = parent;
    m_pathBar->setText(parent);
    refresh();
}

void
InsynicFileBrowserDialog::onRefreshClicked()
{
    refresh();
}

void
InsynicFileBrowserDialog::refresh()
{
    if (m_currentViewMode == ViewMode::Detail) {
        loadPath(m_currentPath);
    } else {
        setupTileView();
    }
}

void
InsynicFileBrowserDialog::onSortByNameClicked()
{
    m_sortNameAction->setChecked(true);
    m_sortSizeAction->setChecked(false);
    m_sortDateAction->setChecked(false);
    m_sortTypeAction->setChecked(false);

    if (m_sortColumn == 0) {
        m_sortOrder = m_sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        m_sortOrder = Qt::AscendingOrder;
    }
    m_sortColumn = 0;
    m_treeWidget->sortByColumn(m_sortColumn, m_sortOrder);
}

void
InsynicFileBrowserDialog::onSortBySizeClicked()
{
    m_sortNameAction->setChecked(false);
    m_sortSizeAction->setChecked(true);
    m_sortDateAction->setChecked(false);
    m_sortTypeAction->setChecked(false);

    if (m_sortColumn == 1) {
        m_sortOrder = m_sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        m_sortOrder = Qt::AscendingOrder;
    }
    m_sortColumn = 1;
    m_treeWidget->sortByColumn(m_sortColumn, m_sortOrder);
}

void
InsynicFileBrowserDialog::onSortByDateClicked()
{
    m_sortNameAction->setChecked(false);
    m_sortSizeAction->setChecked(false);
    m_sortDateAction->setChecked(true);
    m_sortTypeAction->setChecked(false);

    if (m_sortColumn == 2) {
        m_sortOrder = m_sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        m_sortOrder = Qt::AscendingOrder;
    }
    m_sortColumn = 2;
    m_treeWidget->sortByColumn(m_sortColumn, m_sortOrder);
}

void
InsynicFileBrowserDialog::onSortByTypeClicked()
{
    m_sortNameAction->setChecked(false);
    m_sortSizeAction->setChecked(false);
    m_sortDateAction->setChecked(false);
    m_sortTypeAction->setChecked(true);

    if (m_sortColumn == 4) {
        m_sortOrder = m_sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        m_sortOrder = Qt::AscendingOrder;
    }
    m_sortColumn = 4;
    m_treeWidget->sortByColumn(m_sortColumn, m_sortOrder);
}

void
InsynicFileBrowserDialog::onViewModeChanged()
{
    if (m_currentViewMode == ViewMode::Detail) {
        switchViewMode(ViewMode::Tile);
    } else {
        switchViewMode(ViewMode::Detail);
    }
}

void
InsynicFileBrowserDialog::switchViewMode(ViewMode mode)
{
    m_currentViewMode = mode;

    if (mode == ViewMode::Detail) {
        m_treeWidget->setVisible(true);
        m_tileWidget->setVisible(false);
        m_viewModeAction->setText(tr("Tile"));
        loadPath(m_currentPath);
    } else {
        m_treeWidget->setVisible(false);
        m_tileWidget->setVisible(true);
        m_viewModeAction->setText(tr("Detail"));
        setupTileView();
    }
}

void
InsynicFileBrowserDialog::setupTileView()
{
    QLayoutItem *item;
    while ((item = m_tileLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    m_selectedTile = nullptr;

    bool ok;
    QVector<AdbFileInfo> files = m_fileManager->listFiles(m_currentPath, &ok);
    if (!ok) {
        m_statusLabel->setText(tr("Failed to list files"));
        return;
    }

    int row = 0;
    int col = 0;
    int tileWidth = 120;
    int tileSpacing = 8;
    int availableWidth = m_tileWidget->width();
    int maxCols = qMax(1, availableWidth / (tileWidth + tileSpacing));
    if (maxCols < 1) maxCols = 1;

    for (const AdbFileInfo &info : files) {
        QWidget *tile = new QWidget(m_tileWidget);
        QVBoxLayout *tileLayout = new QVBoxLayout(tile);
        tileLayout->setContentsMargins(4, 4, 4, 4);
        tileLayout->setSpacing(2);
        tileLayout->setAlignment(Qt::AlignCenter);

        QLabel *iconLabel = new QLabel(tile);
        QIcon icon = info.isDir ? style()->standardIcon(QStyle::SP_DirIcon)
                                : style()->standardIcon(QStyle::SP_FileIcon);
        iconLabel->setPixmap(icon.pixmap(48, 48));
        iconLabel->setAlignment(Qt::AlignCenter);

        QLabel *nameLabel = new QLabel(info.name, tile);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setWordWrap(true);
        nameLabel->setMaximumWidth(100);
        nameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        tileLayout->addWidget(iconLabel);
        tileLayout->addWidget(nameLabel);

        tile->setProperty("filePath", info.path);
        tile->setProperty("isDir", info.isDir);
        tile->setProperty("fileName", info.name);
        tile->setProperty("fileSize", info.size);
        tile->setProperty("fileDate", info.date);
        tile->setProperty("fileTime", info.time);
        tile->setProperty("fileOwner", info.owner);
        tile->setProperty("fileGroup", info.group);
        tile->setProperty("filePermissions", info.permissions);
        tile->setProperty("fileIsLink", info.isLink);

        tile->setStyleSheet("QWidget { background-color: transparent; border: 1px solid transparent; }");
        tile->setMouseTracking(true);
        tile->installEventFilter(this);
        tile->setFixedSize(120, 100);
        tile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        m_tileLayout->addWidget(tile, row, col);

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }

    m_statusLabel->setText(QString("%1 items").arg(files.size()));
}

bool
InsynicFileBrowserDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            QWidget *tile = qobject_cast<QWidget*>(obj);
            if (tile) {
                onTileContextMenuRequested(tile->mapToParent(mouseEvent->pos()));
                return true;
            }
        }
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QWidget *tile = qobject_cast<QWidget*>(obj);
            if (tile) {
                onTileDoubleClicked(tile);
                return true;
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QWidget *tile = qobject_cast<QWidget*>(obj);
            if (tile) {
                onTileClicked(tile);
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

void
InsynicFileBrowserDialog::onTileDoubleClicked(QWidget *tile)
{
    if (!tile) return;

    bool isDir = tile->property("isDir").toBool();
    if (isDir) {
        QString path = tile->property("filePath").toString();
        loadPath(path);
        switchViewMode(ViewMode::Tile);
    }
}

void
InsynicFileBrowserDialog::onTileClicked(QWidget *tile)
{
    if (!tile) return;

    clearTileSelection();
    setTileSelected(tile, true);
    m_selectedTile = tile;
}

void
InsynicFileBrowserDialog::onTileContextMenuRequested(const QPoint &pos)
{
    QWidget *tile = getTileAtPosition(pos);
    if (!tile) return;

    QMenu menu(this);

    m_copyAction = menu.addAction(tr("Copy"));
    m_cutAction = menu.addAction(tr("Cut"));
    m_pasteAction = menu.addAction(tr("Paste"));
    menu.addSeparator();
    m_deleteAction = menu.addAction(tr("Delete"));
    menu.addSeparator();

    QMenu *sortMenu = menu.addMenu(tr("Sort"));
    QAction *sortByName = sortMenu->addAction(tr("By Name"));
    QAction *sortBySize = sortMenu->addAction(tr("By Size"));
    QAction *sortByDate = sortMenu->addAction(tr("By Date"));
    QAction *sortByType = sortMenu->addAction(tr("By Type"));

    menu.addSeparator();
    m_propertiesAction = menu.addAction(tr("Properties"));

    m_copyAction->setEnabled(true);
    m_cutAction->setEnabled(true);
    m_deleteAction->setEnabled(true);
    m_propertiesAction->setEnabled(true);
    m_pasteAction->setEnabled(m_clipboardMode != ClipboardMode::None);

    QAction *selectedAction = menu.exec(m_tileWidget->mapToGlobal(pos));

    if (selectedAction == m_copyAction) {
        m_clipboardPaths.clear();
        m_clipboardPaths.append(tile->property("filePath").toString());
        m_clipboardSourceDir = m_currentPath;
        m_clipboardMode = ClipboardMode::Copy;
        m_statusLabel->setText(tr("Copied 1 item"));
    } else if (selectedAction == m_cutAction) {
        m_clipboardPaths.clear();
        m_clipboardPaths.append(tile->property("filePath").toString());
        m_clipboardSourceDir = m_currentPath;
        m_clipboardMode = ClipboardMode::Cut;
        m_statusLabel->setText(tr("Cut 1 item"));
    } else if (selectedAction == m_pasteAction) {
        onPasteClicked();
    } else if (selectedAction == m_deleteAction) {
        QString name = tile->property("fileName").toString();
        auto ret = QMessageBox::question(this, tr("Confirm Delete"),
            QString(tr("Delete '%1'?")).arg(name));
        if (ret == QMessageBox::Yes) {
            QString remotePath = tile->property("filePath").toString();
            bool ok = m_fileManager->deleteFile(remotePath);
            if (ok) {
                refresh();
            } else {
                QMessageBox::warning(this, tr("Error"), tr("Failed to delete"));
            }
        }
    } else if (selectedAction == sortByName) {
        onSortByNameClicked();
    } else if (selectedAction == sortBySize) {
        onSortBySizeClicked();
    } else if (selectedAction == sortByDate) {
        onSortByDateClicked();
    } else if (selectedAction == sortByType) {
        onSortByTypeClicked();
    } else if (selectedAction == m_propertiesAction) {
        AdbFileInfo info;
        info.name = tile->property("fileName").toString();
        info.path = tile->property("filePath").toString();
        info.isDir = tile->property("isDir").toBool();
        info.size = tile->property("fileSize").toLongLong();
        info.date = tile->property("fileDate").toString();
        info.time = tile->property("fileTime").toString();
        info.owner = tile->property("fileOwner").toString();
        info.group = tile->property("fileGroup").toString();
        info.permissions = tile->property("filePermissions").toString();
        info.isLink = tile->property("fileIsLink").toBool();
        showPropertiesDialog(info);
    }
}

void
InsynicFileBrowserDialog::clearTileSelection()
{
    for (int i = 0; i < m_tileLayout->count(); ++i) {
        QLayoutItem *item = m_tileLayout->itemAt(i);
        if (item && item->widget()) {
            setTileSelected(item->widget(), false);
        }
    }
}

void
InsynicFileBrowserDialog::setTileSelected(QWidget *tile, bool selected)
{
    if (selected) {
        tile->setStyleSheet("QWidget { background-color: #3366CC; border: 1px solid #003399; }");
    } else {
        tile->setStyleSheet("QWidget { background-color: transparent; border: 1px solid transparent; }");
    }
}

QWidget *
InsynicFileBrowserDialog::getTileAtPosition(const QPoint &pos)
{
    for (int i = 0; i < m_tileLayout->count(); ++i) {
        QLayoutItem *item = m_tileLayout->itemAt(i);
        if (item && item->widget()) {
            QWidget *widget = item->widget();
            if (widget->geometry().contains(pos)) {
                return widget;
            }
        }
    }
    return nullptr;
}

AdbFileInfo
InsynicFileBrowserDialog::getTileFileInfo(QWidget *tile)
{
    AdbFileInfo info;
    info.name = tile->property("fileName").toString();
    info.path = tile->property("filePath").toString();
    info.isDir = tile->property("isDir").toBool();
    info.size = tile->property("fileSize").toLongLong();
    info.date = tile->property("fileDate").toString();
    info.time = tile->property("fileTime").toString();
    info.owner = tile->property("fileOwner").toString();
    info.group = tile->property("fileGroup").toString();
    info.permissions = tile->property("filePermissions").toString();
    info.isLink = tile->property("fileIsLink").toBool();
    return info;
}

void
InsynicFileBrowserDialog::onSelectAllClicked()
{
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);
        item->setSelected(true);
        item->setCheckState(0, Qt::Checked);
    }
}

void
InsynicFileBrowserDialog::onDeselectAllClicked()
{
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);
        item->setSelected(false);
        item->setCheckState(0, Qt::Unchecked);
    }
}

void
InsynicFileBrowserDialog::onCopyClicked()
{
    QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    m_clipboardPaths.clear();
    for (QTreeWidgetItem *item : selectedItems) {
        m_clipboardPaths.append(item->data(0, Qt::UserRole).toString());
    }
    m_clipboardSourceDir = m_currentPath;
    m_clipboardMode = ClipboardMode::Copy;

    m_statusLabel->setText(tr("Copied %1 items").arg(m_clipboardPaths.size()));
}

void
InsynicFileBrowserDialog::onCutClicked()
{
    QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    m_clipboardPaths.clear();
    for (QTreeWidgetItem *item : selectedItems) {
        m_clipboardPaths.append(item->data(0, Qt::UserRole).toString());
    }
    m_clipboardSourceDir = m_currentPath;
    m_clipboardMode = ClipboardMode::Cut;

    m_statusLabel->setText(tr("Cut %1 items").arg(m_clipboardPaths.size()));
}

void
InsynicFileBrowserDialog::onPasteClicked()
{
    if (m_clipboardMode == ClipboardMode::None || m_clipboardPaths.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Nothing to paste"));
        return;
    }

    m_statusLabel->setText(tr("Pasting..."));
    QApplication::processEvents();

    bool allSuccess = true;
    for (const QString &sourcePath : m_clipboardPaths) {
        QString fileName = QFileInfo(sourcePath).fileName();
        QString destPath = m_currentPath + "/" + fileName;

        if (m_clipboardMode == ClipboardMode::Copy) {
            bool ok = m_fileManager->pullFile(sourcePath, destPath);
            if (!ok) {
                bool pushOk = m_fileManager->pushFile(sourcePath, m_currentPath);
                if (!pushOk) {
                    allSuccess = false;
                }
            }
        } else if (m_clipboardMode == ClipboardMode::Cut) {
            bool ok = m_fileManager->rename(sourcePath, destPath);
            if (!ok) {
                allSuccess = false;
            }
        }
    }

    if (allSuccess) {
        m_statusLabel->setText(tr("Paste complete"));
        m_clipboardMode = ClipboardMode::None;
        m_clipboardPaths.clear();
        refresh();
    } else {
        m_statusLabel->setText(tr("Paste failed"));
        QMessageBox::warning(this, tr("Error"), tr("Failed to paste"));
    }
}

void
InsynicFileBrowserDialog::onContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = m_treeWidget->itemAt(pos);

    QMenu menu(this);

    m_copyAction = menu.addAction(tr("Copy"));
    m_cutAction = menu.addAction(tr("Cut"));
    m_pasteAction = menu.addAction(tr("Paste"));
    menu.addSeparator();
    m_deleteAction = menu.addAction(tr("Delete"));
    menu.addSeparator();

    QMenu *sortMenu = menu.addMenu(tr("Sort"));
    QAction *sortByName = sortMenu->addAction(tr("By Name"));
    QAction *sortBySize = sortMenu->addAction(tr("By Size"));
    QAction *sortByDate = sortMenu->addAction(tr("By Date"));
    QAction *sortByType = sortMenu->addAction(tr("By Type"));

    menu.addSeparator();
    m_propertiesAction = menu.addAction(tr("Properties"));

    bool hasSelection = !m_treeWidget->selectedItems().isEmpty();
    m_copyAction->setEnabled(hasSelection);
    m_cutAction->setEnabled(hasSelection);
    m_deleteAction->setEnabled(hasSelection);
    m_propertiesAction->setEnabled(item != nullptr);
    m_pasteAction->setEnabled(m_clipboardMode != ClipboardMode::None);

    QAction *selectedAction = menu.exec(m_treeWidget->viewport()->mapToGlobal(pos));

    if (selectedAction == m_copyAction) {
        onCopyClicked();
    } else if (selectedAction == m_cutAction) {
        onCutClicked();
    } else if (selectedAction == m_pasteAction) {
        onPasteClicked();
    } else if (selectedAction == m_deleteAction) {
        onDeleteClicked();
    } else if (selectedAction == sortByName) {
        onSortByNameClicked();
    } else if (selectedAction == sortBySize) {
        onSortBySizeClicked();
    } else if (selectedAction == sortByDate) {
        onSortByDateClicked();
    } else if (selectedAction == sortByType) {
        onSortByTypeClicked();
    } else if (selectedAction == m_propertiesAction) {
        onPropertiesClicked();
    }
}

void
InsynicFileBrowserDialog::onPropertiesClicked()
{
    QTreeWidgetItem *item = m_treeWidget->currentItem();
    if (!item) {
        return;
    }

    AdbFileInfo info;
    info.name = item->text(0);
    info.path = item->data(0, Qt::UserRole).toString();
    info.isDir = item->data(0, Qt::UserRole + 1).toBool();
    info.size = item->data(0, Qt::UserRole + 2).toLongLong();
    info.date = item->data(0, Qt::UserRole + 3).toString();
    info.owner = item->data(0, Qt::UserRole + 4).toString();
    info.group = item->data(0, Qt::UserRole + 5).toString();
    info.time = item->data(0, Qt::UserRole + 6).toString();
    info.isLink = item->data(0, Qt::UserRole + 7).toBool();
    info.permissions = item->text(3);

    showPropertiesDialog(info);
}

void
InsynicFileBrowserDialog::showPropertiesDialog(const AdbFileInfo &info)
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Properties - %1").arg(info.name));
    dialog->resize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    QLabel *nameLabel = new QLabel(QString("<b>%1</b>").arg(info.name), dialog);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);

    gridLayout->addWidget(new QLabel(tr("Type:")), 0, 0);
    gridLayout->addWidget(new QLabel(info.isDir ? tr("Directory") : tr("File")), 0, 1);

    gridLayout->addWidget(new QLabel(tr("Location:")), 1, 0);
    gridLayout->addWidget(new QLabel(info.path), 1, 1);

    if (!info.isDir) {
        gridLayout->addWidget(new QLabel(tr("Size:")), 2, 0);
        QString sizeStr;
        if (info.size < 1024) {
            sizeStr = QString("%1 B").arg(info.size);
        } else if (info.size < 1024 * 1024) {
            sizeStr = QString("%1 KB").arg(info.size / 1024);
        } else {
            sizeStr = QString("%1 MB").arg(info.size / (1024 * 1024));
        }
        gridLayout->addWidget(new QLabel(sizeStr), 2, 1);
    }

    gridLayout->addWidget(new QLabel(tr("Permissions:")), 3, 0);
    gridLayout->addWidget(new QLabel(info.permissions), 3, 1);

    gridLayout->addWidget(new QLabel(tr("Owner:")), 4, 0);
    gridLayout->addWidget(new QLabel(info.owner), 4, 1);

    gridLayout->addWidget(new QLabel(tr("Group:")), 5, 0);
    gridLayout->addWidget(new QLabel(info.group), 5, 1);

    gridLayout->addWidget(new QLabel(tr("Modified:")), 6, 0);
    gridLayout->addWidget(new QLabel(info.date + " " + info.time), 6, 1);

    if (info.isLink) {
        gridLayout->addWidget(new QLabel(tr("Symbolic Link:")), 7, 0);
        gridLayout->addWidget(new QLabel(tr("Yes")), 7, 1);
    }

    layout->addWidget(nameLabel);
    layout->addLayout(gridLayout);

    QPushButton *closeBtn = new QPushButton(tr("Close"), dialog);
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::close);

    dialog->exec();
}

void
InsynicFileBrowserDialog::onUploadClicked()
{
    QString localFile = QFileDialog::getOpenFileName(this, tr("Upload File"));
    if (localFile.isEmpty()) {
        return;
    }
    m_statusLabel->setText(tr("Uploading..."));
    QApplication::processEvents();
    bool ok = m_fileManager->pushFile(localFile, m_currentPath);
    if (ok) {
        m_statusLabel->setText(tr("Upload complete"));
        refresh();
    } else {
        m_statusLabel->setText(tr("Upload failed"));
        QMessageBox::warning(this, tr("Error"), tr("Failed to upload file"));
    }
}

void
InsynicFileBrowserDialog::onDownloadClicked()
{
    QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Select a file to download"));
        return;
    }

    for (QTreeWidgetItem *item : selectedItems) {
        bool isDir = item->data(0, Qt::UserRole + 1).toBool();
        if (isDir) {
            continue;
        }
        QString remotePath = item->data(0, Qt::UserRole).toString();
        QString fileName = item->text(0);
        QString localFile = QFileDialog::getSaveFileName(this, tr("Download File"),
                                                         fileName);
        if (localFile.isEmpty()) {
            continue;
        }
        m_statusLabel->setText(tr("Downloading..."));
        QApplication::processEvents();
        bool ok = m_fileManager->pullFile(remotePath, localFile);
        if (!ok) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to download file"));
        }
    }
    m_statusLabel->setText(tr("Download complete"));
}

void
InsynicFileBrowserDialog::onDeleteClicked()
{
    QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Select files to delete"));
        return;
    }

    QString names;
    for (QTreeWidgetItem *item : selectedItems) {
        if (!names.isEmpty()) names += ", ";
        names += item->text(0);
    }

    auto ret = QMessageBox::question(this, tr("Confirm Delete"),
        QString(tr("Delete %1 item(s): %2?")).arg(selectedItems.size()).arg(names));
    if (ret != QMessageBox::Yes) {
        return;
    }

    m_statusLabel->setText(tr("Deleting..."));
    QApplication::processEvents();

    bool allSuccess = true;
    for (QTreeWidgetItem *item : selectedItems) {
        QString remotePath = item->data(0, Qt::UserRole).toString();
        bool ok = m_fileManager->deleteFile(remotePath);
        if (!ok) {
            allSuccess = false;
        }
    }

    if (allSuccess) {
        m_statusLabel->setText(tr("Deleted"));
        refresh();
    } else {
        m_statusLabel->setText(tr("Delete failed"));
        QMessageBox::warning(this, tr("Error"), tr("Failed to delete some files"));
    }
}

void
InsynicFileBrowserDialog::onNewFolderClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("New Folder"),
        tr("Folder name:"), QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) {
        return;
    }
    QString path = m_currentPath + "/" + name;
    if (m_fileManager->mkdir(path)) {
        refresh();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create folder"));
    }
}
