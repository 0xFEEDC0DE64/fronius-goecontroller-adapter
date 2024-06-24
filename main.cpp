#include <QCoreApplication>
#include <QDebug>
#include <QMqttClient>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    QMqttClient client;
    client.setHostname("localhost");
    client.setPort(1883);

    QNetworkAccessManager manager;

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&](){
        QNetworkRequest request{QUrl{"http://192.168.1.110/solar_api/v1/GetPowerFlowRealtimeData.fcgi"}};
        auto reply = manager.get(request);
        QObject::connect(reply, &QNetworkReply::finished, [&,reply](){
            struct Helper {
                ~Helper() { reply->deleteLater(); }
                QNetworkReply *reply;
            } helper { reply };

            auto content = reply->readAll();

            if (reply->error() != QNetworkReply::NoError)
            {
                qCritical() << "request failed:" << reply->error();
                qDebug() << content;
            }

            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(content, &error);
            if (error.error != QJsonParseError::NoError)
            {
                qCritical() << "json parsing failed:" << error.errorString();
                qDebug() << content;
                return;
            }

            if (!doc.isObject())
            {
                qCritical() << "json is not an object";
                qDebug() << content;
                return;
            }

            const auto &rootObj = doc.object();
            qDebug() << "finished";

            if (!rootObj.contains("Body"))
            {
                qCritical() << "json does not contain Body";
                qDebug() << content;
                return;
            }

            const auto &body = rootObj.value("Body");

            if (!body.isObject())
            {
                qCritical() << "json Body is not an object";
                qDebug() << content;
                return;
            }

            const auto &bodyObj = body.toObject();

            if (!bodyObj.contains("Data"))
            {
                qCritical() << "json does not contain Data";
                qDebug() << content;
                return;
            }

            const auto &data = bodyObj.value("Data");

            if (!data.isObject())
            {
                qCritical() << "json Data is not an object";
                qDebug() << content;
                return;
            }

            const auto &dataObj = data.toObject();

            if (!dataObj.contains("Site"))
            {
                qCritical() << "json does not contain Site";
                qDebug() << content;
                return;
            }

            const auto &site = dataObj.value("Site");

            if (!site.isObject())
            {
                qCritical() << "json Site is not an object";
                qDebug() << content;
                return;
            }

            const auto &siteObj = site.toObject();

            if (!siteObj.contains("P_PV"))
            {
                qCritical() << "json does not contain P_PV";
                qDebug() << content;
                return;
            }

            const auto &p_pv = siteObj.value("P_PV").toDouble();

            qDebug() << "P_PV" << p_pv;
            client.publish(QMqttTopicName{"go-eController/909648/ecp/set"}, QString{"[%0,null,null,null,%0]"}.arg(p_pv).toUtf8());
        });
    });
    timer.setInterval(1000);
    
    QObject::connect(&client, &QMqttClient::connected, &timer, qOverload<>(&QTimer::start));
    QObject::connect(&client, &QMqttClient::connected, &timer, [](){
        qDebug() << "mqtt connected";
    });
    QObject::connect(&client, &QMqttClient::disconnected, &timer, &QTimer::stop);
    QObject::connect(&client, &QMqttClient::disconnected, &timer, [](){
        qDebug() << "mqtt disconnected";
    });
    QObject::connect(&client, &QMqttClient::errorChanged, [](QMqttClient::ClientError error){
        qCritical() << "mqtt error:" << error;
    });
    qDebug() << "connecting to mqtt...";
    client.connectToHost();

    return app.exec();
}