#include "http.h"
#include <QDebug>
#include <QtNetwork>
#include <QHttpMultiPart>
//#include <QtZlib/zlib.h>
#include "tcp/StationCommand.h"
#include "bzlib.h"
#include "fileutils.h"
#include "tcp/client.h"
#include <stdio.h>
#include <QSemaphore>
#include "systemutils.h"

QSemaphore sem(1);

#define HTTP_TIME 100

http::http(QString estationid)
{
    manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(ReplyFinished(QNetworkReply *)));//http_recall
   // connect(manager,SIGNAL(readyRead(QNetworkReply*)), this, SLOT(onReadyRead(QNetworkReply*)));
    connect(this,SIGNAL(recieved_data(int)),this,SLOT(http_protocol_handle(int)));
    connect(this,SIGNAL(http_recall(int)),this,SLOT(HttpPostRequest(int)));
    Device_id = estationid;
    tcp_client = client::getInstance();
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(_HttpPostRequest()));
    timer->start(HTTP_TIME);
}

void http::Webservice_Request_DownLoad(Raw_Header *raw)
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
    manager->post(request,raw->Soap_XML);
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

QByteArray http::AddHttpCMD(QString arry,QString bound,QString id,const QByteArray& data)
{
    QByteArray body=QByteArray();
    body.append("--"+bound+"\r\n");
    body.append(QString("Content-Disposition: form-data; name=\"")+"cmd"+QString("\"")+QString("\r\n"));
    body.append("\r\n");
    body.append(arry+QString("\r\n"));

    body.append("--"+bound+"\r\n");
    body.append(QString("Content-Disposition: form-data; name=\"")+"estationid"+QString("\"")+QString("\r\n"));
    body.append("\r\n");
    body.append(id+QString("\r\n"));
    //上传文件的头部
    body.append("--"+bound+"\r\n");
    body.append(QString("Content-Disposition: form-data; filename=\"")+"file.jpg"+QString("\"")+QString("\r\n"));
    body.append("\r\n");
    body.append(data);
    body.append("\r\n");
    body.append("--"+bound+"\r\n");
    //上传文件内容
    return body;
}

void http::ReadyRead_get()
{
    if(app_file)
    {
        app_file->write(mReply->readAll());
    }
}

void http::Finished_get()
{
    app_file->flush();
    app_file->close();
    disconnect(mReply,SIGNAL(finished()),this,SLOT(Finished_get()));
    disconnect(mReply,SIGNAL(readyRead()),this,SLOT(ReadyRead_get()));
    mReply->deleteLater();
    mReply = 0;
    delete app_file;
    app_file = 0;
    if(GET_SOFT_FILE == http_command)
        emit to_local(GET_SOFT_FILE);
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
    }
}
QByteArray http::Bzip2DataHandle(QByteArray data)
{
    QString file_name;
    if(http_command == GET_WEATHER_HTTP){
        file_name = "./weather_information.xml";
        QFile weather_file(file_name);
        weather_file.open(QIODevice::WriteOnly);
        QTextStream xml_file(&weather_file);
        xml_file.setCodec("utf-8");

        xml_file << data;
        weather_file.close();
    } else if (http_command == GET_SERVICE_TIME) {
        QString date = "date -s \"" + data + "\"";
        system(date.toLatin1().data());
        system("hwclock -w");

    } else {
        if(http_command == GET_LINE_HTTP)
            file_name = "./station_line.xml";
        else if(http_command == GET_INI_HTTP)
            file_name = "./init_station.xml";
        else if(http_command == GET_LINE_STYLE)//LineStyle.xml
             file_name = "./LineStyle.xml";
        else if(http_command == GET_VERSION)//LineStyle.xml
            file_name = "./Version.xml";
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
               // xml_line_file<< (char *)buffer_utf8;
                QString s = (char *)buffer_utf8;
             //   qDebug()<<s;
                QTextCodec *codec = QTextCodec::codecForName("utf-8");
                QByteArray byte = codec->fromUnicode(s);
                xml_line_file << byte;
            //    qWarning()<<"222"<<byte;
            }
        }while(bz_uncompress.avail_out == 0);
        BZ2_bzDecompressEnd(&bz_uncompress);
        line_file.close();
    }
   // qWarning("to_local");
    emit to_local(http_command);
    QByteArray bzipUnomprData;
    return bzipUnomprData;
}

http *http::httpInt(QString estationid)
{
    static http http_int(estationid);
    return &http_int;
}

void http::Webservice_Request_UpLoad(QHttpMultiPart *image_part)
{
    QUrl url = "http://" + tcp_client->my_syspam.ip + ":" +QString::number(tcp_client->my_syspam.port) + "/webservices/UpLodeFile.aspx";
    QNetworkRequest request(url);
    manager->post(request,image_part);
}

void http::ReplyFinished(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply :: NoError){
 //       qWarning("ReplyFinished http_command: %d",http_command);
        QFile file("./http_data.xml");
        QByteArray byte = reply->readAll();
        FileUtils::StringToXML(&file, byte);
    //    qDebug()<<"###########http_data = "<<byte;
        emit recieved_data(http_command);
    } else {
        qDebug()<<"reply error!";//<<reply->readAll();
    }
    if(sem.available() == 0)
        sem.release(1);
    reply->deleteLater();
}
//

void http::HttpPostRequest(int command) {
//    static bool flag = false;
    command_list.append(command);
//    if(command_list.length() == 1) {
//        _HttpPostRequest();
//        if(flag) {
//            timer->stop();
//            flag = false;
//        } else {
//            flag = true;
//        }
//    }
//    if(flag) {
//        timer->start(HTTP_TIME);  //
//    }
}

void http::_HttpPostRequest()
{
    Raw_Header raw;
    QString verifycode;
    QString estationid;
    if(command_list.isEmpty()) {
        return;
    }
    if(0 == sem.available()) {
        return;
    }
    sem.acquire(1);
//    qWarning("send_http_command: %d",command_list.first());
    switch(command_list.first()){
        case GET_INI_HTTP:
            estationid = Device_id;
            raw.Content_Type = "text/xml;charset=UTF-8";
            raw.SOAPAction = "\"http://www.56gps.cn/getStationInitialization\"";
            raw.Envelope_start = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soapenv:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append("<soapenv:Header/><soapenv:Body><gps:getStationInitialization><gps:estationid>");
            raw.Soap_XML = raw.Soap_XML.append(estationid);
            raw.Soap_XML = raw.Soap_XML.append("</gps:estationid></gps:getStationInitialization></soapenv:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            Webservice_Request_DownLoad(&raw);
            http_command = GET_INI_HTTP;
            break;
        case GET_LINE_HTTP://更新线路
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
            Webservice_Request_DownLoad(&raw);
            http_command = GET_LINE_HTTP;
            break;
        case GET_WEATHER_HTTP:
            raw.Content_Type = "text/xml;charset=UTF-8";
            raw.SOAPAction = "\"http://www.56gps.cn/getES_SMSForecost\"";
            raw.Envelope_start = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soapenv:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append("<soapenv:Header/><soapenv:Body><gps:getES_SMSForecost/></soapenv:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            raw.Content_Length = raw.Soap_XML.size();
            Webservice_Request_DownLoad(&raw);
            http_command = GET_WEATHER_HTTP;
            break;
        case GET_SERVICE_TIME:
            raw.Content_Type = "application/soap+xml;charset=UTF-8;action=\"http://www.56gps.cn/getServiceTime\"";
            raw.Envelope_start = "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soap:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append(" <soap:Header/><soap:Body><gps:getServiceTime/></soap:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            raw.Content_Length = raw.Soap_XML.size();
            Webservice_Request_DownLoad(&raw);
            http_command = GET_SERVICE_TIME;
            break;
        case GET_LINE_STYLE:
            estationid = Device_id;
            raw.Content_Type = "text/xml;charset=UTF-8";
            raw.SOAPAction = "\"http://www.56gps.cn/getLineStytleXML\"";
            raw.Envelope_start = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soapenv:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append("<soapenv:Header/><soapenv:Body><gps:getLineStytleXML><gps:estationid>");
            raw.Soap_XML = raw.Soap_XML.append(estationid);
            raw.Soap_XML = raw.Soap_XML.append("</gps:estationid></gps:getLineStytleXML></soapenv:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            raw.Content_Length = raw.Soap_XML.size();
            Webservice_Request_DownLoad(&raw);
            http_command = GET_LINE_STYLE;
            break;
        case PUT_SHOT_SCREEN:
        {
            estationid = Device_id;
            char* m_buf;
            m_buf = NULL;
            QFile file("./Screenshot.jpg");
            if(!file.open(QIODevice::ReadOnly)){
                return;
            }
            int file_len = file.size();
            QDataStream in(&file);
            m_buf = new char[file_len];
            in.readRawData( m_buf, file_len);
            QByteArray DataContent =QByteArray (m_buf,file_len);
            QString BOUNDARY=QUuid::createUuid().toString();
            QByteArray body;
            /* Content-Disposition: form-data; name="cmd"\r\n
             * \r\n
             * set_monitor_imge\r\n
             * Content-Disposition: form-data; filename="file.jpg"\r\n
             * \r\n
             * 文件内容
             */
            body.append(AddHttpCMD(QString("set_monitor_imge"),BOUNDARY,estationid,DataContent));
            QUrl url = "http://" + tcp_client->my_syspam.ip + ":" +QString::number(tcp_client->my_syspam.port-1) + "/webservices/UpLodeFile.aspx";
            //QNetworkRequest request(url);
            QNetworkRequest request=QNetworkRequest(QUrl(url));
            request.setRawHeader(QString("Content-Type").toUtf8(),QString("multipart/form-data; boundary="+BOUNDARY).toUtf8());
            request.setRawHeader(QString("Content-Length").toUtf8(),QString::number(body.length()).toUtf8());
            manager->post(request,body);
            http_command = PUT_SHOT_SCREEN;
            break;
        }
        case PUT_ERROR_MSG:
        {
            estationid = Device_id;
            QString alarm = "1";
            raw.Content_Type = "text/xml;charset=UTF-8";
            raw.SOAPAction = "\"http://www.56gps.cn/insertIntoAlarm\"";
            raw.Envelope_start = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soapenv:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append("<soapenv:Header/><soapenv:Body><gps:insertIntoAlarm><gps:ESid>");
            raw.Soap_XML = raw.Soap_XML.append(estationid);
            raw.Soap_XML = raw.Soap_XML.append("</gps:ESid><gps:itype>");
            raw.Soap_XML = raw.Soap_XML.append(alarm);
             raw.Soap_XML = raw.Soap_XML.append("</gps:itype></gps:insertIntoAlarm></soapenv:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            raw.Content_Length = raw.Soap_XML.size();
            Webservice_Request_DownLoad(&raw);
            http_command = PUT_ERROR_MSG;
            break;
        }
        case GET_VERSION:
            verifycode = "FANGYUXI";
            raw.Content_Type = "text/xml;charset=UTF-8";
            raw.SOAPAction = "\"http://www.56gps.cn/GetVersion\"";
            raw.Envelope_start = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:gps=\"http://www.56gps.cn/\">";
            raw.Envelope_end = "</soapenv:Envelope>";
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_start);
            raw.Soap_XML = raw.Soap_XML.append("<soapenv:Header/><soapenv:Body><gps:GetVersion><!--Optional:--><gps:verifycode>");
            raw.Soap_XML = raw.Soap_XML.append(verifycode);
            raw.Soap_XML = raw.Soap_XML.append("</gps:verifycode>");
            raw.Soap_XML = raw.Soap_XML.append("</gps:GetVersion></soapenv:Body>");
            raw.Soap_XML = raw.Soap_XML.append(raw.Envelope_end);
            raw.Content_Length = raw.Soap_XML.size();
            Webservice_Request_DownLoad(&raw);
            http_command = GET_VERSION;
            break;
        case GET_SOFT_FILE:
        {
            QFile file("./Version.xml");
            QString data;
            data = FileUtils::ReadAllXmlNode(&file,GET_SOFT_FILE);

            QNetworkRequest request;
            QUrl url = data;
            QFileInfo info(url.path());
            QString fileName(info.fileName());  //获取文件名
            if(fileName.isEmpty())
            {
                fileName = "index.html";
            }
            app_file = new QFile("./epd-ui.apk");
            if(!app_file->open(QIODevice::WriteOnly))
            {
                qDebug()<<"file open error";
                delete app_file;
                app_file = 0;
                return ;
            }
            request.setUrl(url);
            mReply = manager->get(request);
            http_command = GET_SOFT_FILE;
            connect(mReply,SIGNAL(finished()),this,SLOT(Finished_get()));
            connect(mReply,SIGNAL(readyRead()),this,SLOT(ReadyRead_get()));
            break;
        }
        default:
            sem.release(1);  // 无效命令
            break;
    }
    if(!command_list.isEmpty())
       command_list.removeFirst();
}
