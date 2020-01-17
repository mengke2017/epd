#ifndef XML_H
#define XML_H
#include <QObject>
class xml : public QObject
{
    Q_OBJECT
public:
    static void StringToXML(QString text);
    static QString ReadAllXmlNode(int command);
    //QString ReadSingleXmlNode(int command,QString text);
};

#endif // XML_H
