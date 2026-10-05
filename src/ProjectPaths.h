#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>

inline QString LayoutShopFindAsset(const QString& relativePath)
{
    QStringList roots;
#ifdef LAYOUTSHOP_ASSETS_DIR
    roots << QStringLiteral(LAYOUTSHOP_ASSETS_DIR);
#endif
    roots << QDir(QDir::currentPath()).absoluteFilePath("assets");
    roots << QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../assets");
    roots << QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../assets");

    for (const QString& root : roots)
    {
        const QString path = QDir(root).absoluteFilePath(relativePath);
        if (QFileInfo::exists(path))
        {
            return path;
        }
    }

    return QDir(roots.first()).absoluteFilePath(relativePath);
}
