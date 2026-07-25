#ifndef INSYNIC_FILE_BROWSER_DIALOG_H
#define INSYNIC_FILE_BROWSER_DIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QObject>
#include <QEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QGridLayout>

#include "insynic_filemanager.h"

enum class ViewMode {
    Detail,
    Tile
};

enum class ClipboardMode {
    None,
    Copy,
    Cut
};

class InsynicFileBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsynicFileBrowserDialog(InsynicFileManager *fm,
                                      const QString &deviceName = QString(),
                                      QWidget *parent = nullptr);
    ~InsynicFileBrowserDialog();

    void retranslateUi();

public slots:
    void refresh();

private slots:
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onItemPressed(QTreeWidgetItem *item, int column);
    void onPathReturnPressed();
    void onUpClicked();
    void onRefreshClicked();
    void onUploadClicked();
    void onDownloadClicked();
    void onDeleteClicked();
    void onNewFolderClicked();
    void onSortByNameClicked();
    void onSortBySizeClicked();
    void onSortByDateClicked();
    void onViewModeChanged();
    void onSelectAllClicked();
    void onDeselectAllClicked();
    void onCopyClicked();
    void onCutClicked();
    void onPasteClicked();
    void onContextMenuRequested(const QPoint &pos);
    void onPropertiesClicked();
    void onTileDoubleClicked();
    void onTileClicked();
    void onTileContextMenuRequested(const QPoint &pos);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void loadPath(const QString &path);
    void updatePathBar();
    void setupToolbar();
    void setupDetailView();
    void setupTileView();
    void switchViewMode(ViewMode mode);
    void updateSelectionState();
    void showPropertiesDialog(const AdbFileInfo &info);
    void clearTileSelection();
    void setTileSelected(QWidget *tile, bool selected);
    QWidget *getTileAtPosition(const QPoint &pos);
    AdbFileInfo getTileFileInfo(QWidget *tile);

    InsynicFileManager *m_fileManager;

    QToolBar *m_toolbar;
    QLineEdit *m_pathBar;
    QPushButton *m_upBtn;
    QPushButton *m_refreshBtn;
    QPushButton *m_uploadBtn;
    QPushButton *m_downloadBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_newFolderBtn;

    QPushButton *m_sortNameBtn;
    QPushButton *m_sortSizeBtn;
    QPushButton *m_sortDateBtn;
    QPushButton *m_viewModeBtn;
    QPushButton *m_selectAllBtn;
    QPushButton *m_deselectAllBtn;

    QAction *m_copyAction;
    QAction *m_cutAction;
    QAction *m_pasteAction;
    QAction *m_deleteAction;
    QAction *m_propertiesAction;

    QTreeWidget *m_treeWidget;
    QWidget *m_tileWidget;
    QGridLayout *m_tileLayout;

    QLabel *m_statusLabel;

    QString m_currentPath;
    QString m_deviceName;
    ViewMode m_currentViewMode;
    ClipboardMode m_clipboardMode;
    QStringList m_clipboardPaths;
    QString m_clipboardSourceDir;

    Qt::SortOrder m_sortOrder;
    int m_sortColumn;

    QTreeWidgetItem *m_lastSelectedItem;
    QWidget *m_selectedTile;
};

#endif
