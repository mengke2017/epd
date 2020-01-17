#include "http.h"
#include <QDebug>
#include <QtNetwork>
#include <QtZlib/zlib.h>
#include "tcp/StationCommand.h"
#include "bzlib.h"
#include "fileutils.h"
#include "tcp/client.h"
#include <stdio.h>
#include "systemutils.h"
http::http(QString estationid)
{
    manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(ReplyFinished(QNetworkReply *)));//http_recall
    connect(this,SIGNAL(recieved_data(int)),this,SLOT(http_protocol_handle(int)));
    connect(this,SIGNAL(http_recall(int)),this,SLOT(HttpPostRequest(int)));
    Device_id = estationid;
    tcp_client = client::getInstance();
}

void http::Webservice_Request(Raw_Header *raw)
{
    QNetworkRequest request;
    QUrl url = "http://" + tcp_client->my_syspam.ip + ":" +QString::number(tcp_client->my_syspam.port-1) + "/webservices/SiteServWebService.asmx";
//    request.setUrl(QUrl("http://60.191.59.13:7000/webservices/SiteServWebService.asmx"));
//
//    qWarning()<<"http://" + tcp_client->my_syspam.ip + ":" +QString::number(tcp_client->my_syspam.port) + "/webservices/SiteServWebService.asmx";
    request.setUrl(url);
    request.setRawHeader("Accept-Encoding",raw->En_coding.toUtf8());
    request.setRawHeader("Content-Type",raw->Content_Type.toUtf8());
    request.setRawHeader("User-Agent", raw->User_Agent.toUtf8());
    request.setRawHeader("Content-Length",raw->Content_Length.toUtf8());
    request.setRawHeader("Host",raw->Host.toUtf8());
    request.setRawHeader("Connection",raw->Connection.toUtf8());
    request.setRawHeader("SOAPAction",raw->SOAPAction.toUtf8());
    manager->post(request,raw->Soap_XML.toUtf8());
    //qDebug()<<raw->Soap_XML;
}

QString http::unicodeToUtf_8(const QString &resStr)
{

    //  unicode转utf-8  思路: 一个unicode编码占用2字节。所以只需要用4个16进制数字结合表示就行了

    QString temp;
    for (int i = 0; i< resStr.length();)
    {
        if (resStr.at(i) == '\\')
        {
            QString str = resStr.mid(i+2, 4);
            temp.append(str.toUShort(0, 16));
            i += 6;
        }
        else
        {
            temp.append(resStr.at(i));
            ++i;
        }
    }
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString desStr = codec->fromUnicode(temp);
    return desStr;
}

int http::Utf16_To_Utf8(const UTF16 *sourceStart, UTF8 *targetStart, size_t outLen, ConversionFlags flags)
{

    int result = 0;
    const UTF16* source = sourceStart;
    UTF8* target        = targetStart;
    UTF8* targetEnd     = targetStart + outLen;
    if ((NULL == source) || (NULL == targetStart)){
        printf("ERR, Utf16_To_Utf8: source=%p, targetStart=%p\n", source, targetStart);
        return conversionFailed;
    }
    unsigned short bytesToWrite = 0;
    while ( *source ) {
        UTF32 ch;
        const UTF32 byteMask = 0xBF;
        const UTF32 byteMark = 0x80;
        const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if ( *source ){
                UTF32 ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++source;
                }else if (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    break;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                result = sourceExhausted;
                break;
            }
        } else if (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END){
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                break;
            }
        }
        /* Figure out how many bytes the result will require */
        if(ch < (UTF32)0x80){
            bytesToWrite = 1;
        } else if (ch < (UTF32)0x800) {
            bytesToWrite = 2;
        } else if (ch < (UTF32)0x10000) {
            bytesToWrite = 3;
        } else if (ch < (UTF32)0x110000){
            bytesToWrite = 4;
        } else {
            bytesToWrite = 3;
            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        if (target > targetEnd) {
            source = oldSource; /* Back up source pointer! */
            target -= bytesToWrite; result = targetExhausted; break;
        }
        switch (bytesToWrite) { /* note: everything falls through. */
            case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
            case 1: *--target = (UTF8)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
       // count += bytesToWrite;
    }
    return result;
}


void http::http_protocol_handle(int command)
{
    QString data;
    QByteArray xml_data;

    QFile file("./http_data.xml");
    data = FileUtils::ReadAllXmlNode(&file,command);
    if(!data.isEmpty()){
        //QTextCodec *codec = QTextCodec::codecForName("utf-8");
       // codec->fromUnicode()
        xml_data = Bzip2DataHandle(data.toUtf8());
       //  qDebug()<<data;
    }
}

QByteArray http::qGzipUncompress(const QByteArray &data)
{
    if(!data.data()){
        qDebug()<<"no data!";
        return  QByteArray();
    }
    //初始化流结构体
    z_stream unGzipStream;
    unGzipStream.next_in = (z_Bytef*)data.data();//输入字节起始地址
    unGzipStream.avail_in = data.size();//输入字节大小
    unGzipStream.zalloc = Z_NULL;
    unGzipStream.zfree = Z_NULL;
    unGzipStream.opaque = Z_NULL;
    //初始化内部流状态
    int ret = inflateInit2(&unGzipStream,16);
    if(ret != Z_OK)
    {
        qDebug()<<"qGzipUncompress: The call to inflateInit2 returns the wrong value";
        return QByteArray();
    }
    unsigned char buffer[4096];
    QByteArray gzipUnomprData;
    //使用 4MB 的缓冲区循环去接收数据,并把获取到的数据追加给 unComprData,直到获取的数据为空
    do
    {
        unGzipStream.avail_out = 4096;//接收解压数据缓冲区大小
        unGzipStream.next_out = buffer;//接收解压数据缓冲区起始地址
        memset(buffer,0,4096);
        //解压数据
        ret = inflate(&unGzipStream,Z_NO_FLUSH);
        switch(ret)
        {
        case Z_MEM_ERROR:
            qDebug()<<"qGzipUncompress: Z_DATA_ERROR: Not enough memory";
            return QByteArray();
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;
        case Z_DATA_ERROR:
            qDebug()<<"qGzipUncompress: Z_DATA_ERROR: Input data is corrupted";
            return QByteArray();
        }
        if(ret != Z_FINISH)
        {
            gzipUnomprData.append((char*)buffer);
        }
    }while(unGzipStream.avail_out == 0);  
    return gzipUnomprData;
}

QByteArray http::Bzip2DataHandle(QByteArray data)
{
    QString file_name;
    if(http_command == WEATHER_HTTP){
        file_name = "./weather_information.xml";
        QFile weather_file(file_name);
        weather_file.open(QIODevice::WriteOnly);
        QTextStream xml_file(&weather_file);
        xml_file.setCodec("utf-8");

        xml_file << data;
        weather_file.close();
    } else {
        if(http_command == UPDATE_LINE_HTTP)
            file_name = "./station_line.xml";
        else if(http_command == GET_INI_HTTP)
            file_name = "./init_station.xml";
        QFile line_file(file_name);
        line_file.open(QIODevice::WriteOnly);
        QTextStream xml_line_file(&line_file);
        xml_line_file.setCodec("utf-8");
        QByteArray data1 = QByteArray::fromBase64(data);
        bz_stream  bz_uncompress;
        bz_uncompress.avail_in = data1.size();
        bz_uncompress.next_in  = (char *)data1.data();
        bz_uncompress.bzfree   = 0;
        bz_uncompress.bzalloc  = 0;
        bz_uncompress.opaque   = 0;
        int ret = BZ2_bzDecompressInit(&bz_uncompress,0,0);
        if(ret != BZ_OK){
            qDebug()<<"bz_uncompress init error!";
            return QByteArray();
        }
        char buffer[4003];
        ushort *buf_utf16;
        unsigned char buffer_utf8[8000];
        do{
            bz_uncompress.avail_out = 4000;
            bz_uncompress.next_out  = buffer;
            memset(buffer,0,sizeof(buffer));
            memset(buffer_utf8,0,sizeof(buffer_utf8));
            ret = BZ2_bzDecompress(&bz_uncompress);
            if(ret != BZ_FINISH_OK){
                buf_utf16 = (ushort *)buffer;
                Utf16_To_Utf8(buf_utf16, buffer_utf8, 8000, lenientConversion);
                xml_line_file<< (char *)buffer_utf8;
                qWarning((char *)buffer_utf8);
            }
        }while(bz_uncompress.avail_out == 0);
        BZ2_bzDecompressEnd(&bz_uncompress);
        line_file.close();
    }
    qWarning("to_local");
    emit to_local(http_command);
    QByteArray bzipUnomprData;
    return bzipUnomprData;
}

http *http::httpInt(QString estationid)
{
    static http http_int(estationid);
    return &http_int;
}

void http::ReplyFinished(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply :: NoError){
        QFile file("./http_data.xml");
        QByteArray byte = reply->readAll();
//        QString unicode;
//        QTextCodec *codec = QTextCodec::codecForName("utf-8");
//        unicode = codec->toUnicode(byte);
//        byte = unicode.toLocal8Bit();
//        qWarning()<<"code :"<<byte.toHex();
        FileUtils::StringToXML(&file, byte);
        emit recieved_data(http_command);
    } else {
        qDebug()<<"reply error!";
    }
    reply->deleteLater();
}

void http::HttpPostRequest(int command)
{
    Raw_Header raw;
    QString verifycode;
    QString estationid;
    switch(command){
        case GET_INI_HTTP:
            estationid = Device_id;
            raw.Content_Type = "text/xml;charset=UTF-8";
            raw.SOAPAction = "\"http://www.56gps.cn/getStationInitialization\"";
            raw.Envelope_start = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soapenv:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append("<soapenv:Header/><soapenv:Body><gps:getStationInitialization><gps:estation_id>");
            raw.Soap_XML = raw.Soap_XML.append(estationid);
            raw.Soap_XML = raw.Soap_XML.append("</gps:estation_id></gps:getStationInitialization></soapenv:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            Webservice_Request(&raw);
            http_command = GET_INI_HTTP;
            break;
        case UPDATE_LINE_HTTP://更新线路
            verifycode = "FANGYUXI";
            estationid = Device_id;
            raw.Content_Type = "text/xml;charset=UTF-8";
            raw.SOAPAction = "\"http://www.56gps.cn/E_station_line_Android\"";
            raw.Envelope_start = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soapenv:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append("<soapenv:Header/><soapenv:Body><gps:E_station_line_Android><!--Optional:--><gps:verifycode>");
            raw.Soap_XML = raw.Soap_XML.append(verifycode);
            raw.Soap_XML = raw.Soap_XML.append("</gps:verifycode><!--Optional:--><gps:estationid>");
            raw.Soap_XML = raw.Soap_XML.append(estationid);
            raw.Soap_XML = raw.Soap_XML.append("</gps:estationid></gps:E_station_line_Android></soapenv:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            raw.Content_Length = raw.Soap_XML.size();
            Webservice_Request(&raw);
            http_command = UPDATE_LINE_HTTP;
            break;
        case WEATHER_HTTP:
            raw.Content_Type = "text/xml;charset=UTF-8";
            raw.SOAPAction = "\"http://www.56gps.cn/getES_SMSForecost\"";
            raw.Envelope_start = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soapenv:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append("<soapenv:Header/><soapenv:Body><gps:getES_SMSForecost/></soapenv:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            raw.Content_Length = raw.Soap_XML.size();
            Webservice_Request(&raw);
            http_command = WEATHER_HTTP;
            break;
    }
}
