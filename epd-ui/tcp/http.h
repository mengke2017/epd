#ifndef HTTP_H
#define HTTP_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>
#include <QEventLoop>
//#include <QSemaphore>
#include "client.h"

#define FALSE  0
#define TRUE   1

#define halfShift	10
#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF
/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF


typedef unsigned char   boolean;
typedef unsigned int	CharType ;
typedef unsigned char	UTF8;
typedef unsigned short	UTF16;
typedef unsigned int	UTF32;

static const UTF32 halfMask = 0x3FFUL;
static const UTF32 halfBase = 0x0010000UL;
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };
static const char trailingBytesForUTF8[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};
typedef enum
{
    strictConversion = 0,
    lenientConversion
} ConversionFlags;
typedef enum
{
    conversionOK, 		/* conversion successful */
    sourceExhausted,	/* partial character in source, but hit end */
    targetExhausted,	/* insuff. room in target for conversion */
    sourceIllegal,		/* source sequence is illegal/malformed */
    conversionFailed
} ConversionResult;

typedef struct Raw{
    QString En_coding;
    QString Content_Type;
    QString SOAPAction;
    QString Content_Length;
    QString Host;
    QString Connection;
    QString User_Agent;
    QByteArray Soap_XML;
    QByteArray Envelope_start;
    QByteArray Envelope_end;
}Raw_Header;

//typedef struct http_protocol{
//    QString result;
//    QString command;
//}Http_protocol;

class http : public QObject
{
    Q_OBJECT
public:
    explicit http(QString estationid);
    QByteArray qGzipUncompress(const QByteArray& data);
    QByteArray Bzip2DataHandle( QByteArray data);
    static http * httpInt(QString estationid);
    client *tcp_client;
    QList<QString> alarm_list;

//    QSemaphore cmd_sem;
private:
    QNetworkAccessManager *manager;
    void Webservice_Request_DownLoad(Raw_Header *raw);
    void Webservice_Request_UpLoad(QHttpMultiPart *image_part);
    QString unicodeToUtf_8(const QString &resStr);
    int Utf16_To_Utf8 (const UTF16* sourceStart, UTF8* targetStart, size_t outLen ,  ConversionFlags flags);
    QByteArray AddHttpCMD(QString arry, QString bound, QString id, const QByteArray& data);
    int  http_command;
    QList<int> command_list;
    QString Device_id;
    QTimer *timer;
    QNetworkReply* mReply;
    QFile *app_file;
signals:
    void recieved_data(int);
    void http_recall(int);
    void to_local(int);
    void to_battery_time(QString);
private slots:
    void ReadyRead_get();//Finished_get
    void Finished_get();
    void ReplyFinished(QNetworkReply *);
    void http_protocol_handle(int command);
    void _HttpPostRequest();
public slots:
    void HttpPostRequest(int command);

};

#endif // HTTP_H
