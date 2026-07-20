#include "insynic_filebrowser.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>

InsynicFileBrowserDialog::InsynicFileBrowserDialog(InsynicFileManager *fm,
                                                   QWidget *parent)
    : QDialog(parent)
    , m_fileManager(fm)
    , m_currentPath("/mnt")
{
    setWindowTitle(tr("File Manager"));
    resize(700, 500);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    m_toolbar = new QToolBar(this);
    m_upBtn = new QPushButton(tr("Up"), this);
    m_refreshBtn = new QPushButton(tr("Refresh"), this);
    m_uploadBtn = new QPushButton(tr("Upload"), this);
    m_downloadBtn = new QPushButton(tr("Download"), this);
    m_deleteBtn = new QPushButton(tr("Delete"), this);
    m_newFolderBtn = new QPushButton(tr("New Folder"), this);

    m_toolbar->addWidget(m_upBtn);
    m_toolbar->addWidget(m_refreshBtn);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_uploadBtn);
    m_toolbar->addWidget(m_downloadBtn);
    m_toolbar->addWidget(m_deleteBtn);
    m_toolbar->addSeparator();
    m_toolbar->addWidget(m_newFolderBtn);

    layout->addWidget(m_toolbar);

    m_pathBar = new QLineEdit(this);
    m_pathBar->setText(m_currentPath);
    layout->addWidget(m_pathBar);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({tr("Name"), tr("Size"), tr("Date"), tr("Permissions")});
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSortingEnabled(true);
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_treeWidget->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    layout->addWidget(m_treeWidget);

    m_statusLabel = new QLabel(tr("Ready"), this);
    layout->addWidget(m_statusLabel);

    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked,
            this, &InsynicFileBrowserDialog::onItemDoubleClicked);
    connect(m_pathBar, &QLineEdit::returnPressed,
            this, &InsynicFileBrowserDialog::onPathReturnPressed);
    connect(m_upBtn, &QPushButton::clicked,
            this, &InsynicFileBrowserDialog::onUpClicked);
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &InsynicFileBrowserDialog::onRefreshClicked);
    connect(m_uploadBtn, &QPushButton::clicked,
            this, &InsynicFileBrowserDialog::onUploadClicked);
    connect(m_downloadBtn, &QPushButton::clicked,
            this, &InsynicFileBrowserDialog::onDownloadClicked);
    connect(m_deleteBtn, &QPushButton::clicked,
            this, &InsynicFileBrowserDialog::onDeleteClicked);
    connect(m_newFolderBtn, &QPushButton::clicked,
            this, &InsynicFileBrowserDialog::onNewFolderClicked);

    loadPath(m_currentPath);
}

InsynicFileBrowserDialog::~InsynicFileBrowserDialog()
{
}

void
InsynicFileBrowserDialog::retranslateUi()
{
    setWindowTitle(tr("File Manager"));
    m_upBtn->setText(tr("Up"));
    m_refreshBtn->setText(tr("Refresh"));
    m_uploadBtn->setText(tr("Upload"));
    m_downloadBtn->setText(tr("Download"));
    m_deleteBtn->setText(tr("Delete"));
    m_newFolderBtn->setText(tr("New Folder"));
    
    QStringList headers = {tr("Name"), tr("Size"), tr("Date"), tr("Permissions")};
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
        item->setData(0, Qt::UserRole, info.path);
        item->setData(0, Qt::UserRole + 1, info.isDir);
    }

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
    loadPath(parent);
}

void
InsynicFileBrowserDialog::onRefreshClicked()
{
    refresh();
}

void
InsynicFileBrowserDialog::refresh()
{
    loadPath(m_currentPath);
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
    QTreeWidgetItem *item = m_treeWidget->currentItem();
    if (!item) {
        QMessageBox::information(this, tr("Info"), tr("Select a file to download"));
        return;
    }
    bool isDir = item->data(0, Qt::UserRole + 1).toBool();
    if (isDir) {
        QMessageBox::information(this, tr("Info"), tr("Select a file, not a directory"));
        return;
    }
    QString remotePath = item->data(0, Qt::UserRole).toString();
    QString fileName = item->text(0);
    QString localFile = QFileDialog::getSaveFileName(this, tr("Download File"),
                                                     fileName);
    if (localFile.isEmpty()) {
        return;
    }
    m_statusLabel->setText(tr("Downloading..."));
    QApplication::processEvents();
    bool ok = m_fileManager->pullFile(remotePath, localFile);
    if (ok) {
        m_statusLabel->setText(tr("Download complete"));
    } else {
        m_statusLabel->setText(tr("Download failed"));
        QMessageBox::warning(this, tr("Error"), tr("Failed to download file"));
    }
}

void
InsynicFileBrowserDialog::onDeleteClicked()
{
    QTreeWidgetItem *item = m_treeWidget->currentItem();
    if (!item) {
        QMessageBox::information(this, tr("Info"), tr("Select a file to delete"));
        return;
    }
    QString name = item->text(0);
    auto ret = QMessageBox::question(this, tr("Confirm Delete"),
        QString(tr("Delete '%1'?")).arg(name));
    if (ret != QMessageBox::Yes) {
        return;
    }
    QString remotePath = item->data(0, Qt::UserRole).toString();
    m_statusLabel->setText(tr("Deleting..."));
    QApplication::processEvents();
    bool ok = m_fileManager->deleteFile(remotePath);
    if (ok) {
        m_statusLabel->setText(tr("Deleted"));
        refresh();
    } else {
        m_statusLabel->setText(tr("Delete failed"));
        QMessageBox::warning(this, tr("Error"), tr("Failed to delete"));
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
