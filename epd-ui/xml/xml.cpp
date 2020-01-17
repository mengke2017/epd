#include "xml/xml.h"
#include <QtXml>
#include <QTextStream>
#include <QXmlStreamReader>
#include "tcp/StationCommand.h"
void xml::StringToXML(QString text)
{
    QFile file("./http_data.xml");
    file.open(QIODevice::WriteOnly);
    QTextStream xml_file(&file);
    xml_file<<text;
    file.close();
}

QString xml::ReadAllXmlNode(int command)
{
    QFile file("./http_data.xml");
   // QString Node_Attribute;
    QString Node_Name;
    QString data = " ";
    if(file.open(QIODevice::ReadOnly)){
        QXmlStreamReader xml_file(&file);
        switch(command){
            case UPDATE_LINE_HTTP:
                //Node_Attribute = "E_station_line_AndroidResponse";
                Node_Name     = "E_station_line_AndroidResult";
                break;
            case GET_INI_HTTP:
                Node_Name = "getStationInitializationResult";
                break;
            case WEATHER_HTTP:
                Node_Name = "getES_SMSForecostResult";
                break;
        }
        while(!xml_file.atEnd()){
            if(xml_file.isStartElement()){//判断是否是节点的开始
               // QXmlStreamAttributes attributes = xml_file.attributes();
                //if(attributes.hasAttribute(Node_Attribute)){//判断是否存在属性
                    if(xml_file.name().toString() == Node_Name)
                        data = xml_file.readElementText();
              // }
            }
            xml_file.readNext();
        }
        file.close();
    }
       return data;

}

//QString xml::ReadSingleXmlNode(QString attribute)
//{

//}
