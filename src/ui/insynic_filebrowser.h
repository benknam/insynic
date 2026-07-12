#ifndef INSYNIC_FILE_BROWSER_DIALOG_H
#define INSYNIC_FILE_BROWSER_DIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>

#include "insynic_filemanager.h"

class InsynicFileBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsynicFileBrowserDialog(InsynicFileManager *fm,
                                      QWidget *parent = nullptr);
    ~InsynicFileBrowserDialog();

public slots:
    void refresh();

private slots:
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onPathReturnPressed();
    void onUpClicked();
    void onRefreshClicked();
    void onUploadClicked();
    void onDownloadClicked();
    void onDeleteClicked();
    void onNewFolderClicked();

private:
    void loadPath(const QString &path);
    void updatePathBar();

    InsynicFileManager *m_fileManager;

    QToolBar *m_toolbar;
    QLineEdit *m_pathBar;
    QPushButton *m_upBtn;
    QPushButton *m_refreshBtn;
    QPushButton *m_uploadBtn;
    QPushButton *m_downloadBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_newFolderBtn;

    QTreeWidget *m_treeWidget;
    QLabel *m_statusLabel;

    QString m_currentPath;
};

#endif
