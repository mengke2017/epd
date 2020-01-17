#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <QObject>
#include <QStringList>
#include <QFileInfo>
#include "customize.h"

class FileUtils : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE static bool exists(const QString& fileName);

public:
    FileUtils();
    virtual ~FileUtils();
    static QString getAppInstancePath() ;
    static QByteArray readLocalFile(const QString& path);
    static QString readAllText(const QString &fileName);
    static QString getFullPath(const QString& fileName);

    static bool writeText(const QString& fileName, QString& content);
    static bool existsFullPath(const QString& fileName);
    static bool makeDir(const QString& strDir);
    static bool makePath(const QString& strPath);
    static void rmPath(const QString& strDir);
    static bool copyFile(const QString& srcFile, const QString& destFile);
    static bool saveFile(const QByteArray* content, const QString& fileName, bool bCover = true);
    static bool renameFile(const QString& fileName, const QString& newName );
    static bool createFile(const QString& fileName, int32_t initSize = 0, bool bForce = false);
    static bool rmFile(const QString& fileName);
    static bool isAbsolute(const QString& filePath);
    static int64_t getFileSize(const QString& filePath);
    static int32_t fileCount(const QString& filePath);
    static QStringList getFileList(const QString& dir);
    static void log(QString fileName, QString message);
    static void StringToXML(QFile *, QByteArray);
    static QString ReadAllXmlNode(QFile *, int command);
private:
    static void delDir(QFileInfo fileList);
    static void fileCounter(const QString& filePath, int32_t* pCounter);
    static QString readAllTextFullPath(const QString& fileName);
};


#endif // FILEUTILS_H
