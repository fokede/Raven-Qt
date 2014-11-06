#include "sentryclient.h"

#include <QApplication>
#include <QDebug>
#include <QHostAddress>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUuid>
#include <time.h>

const char Raven::kClientName[] = "Raven-Qt";
const char Raven::kClientVersion[] = "0.2";

Raven* g_client;

Raven::Raven(QObject *parent)
    : QObject(parent),
    initialized_(false) {
}

Raven::~Raven() {

}

// static
Raven* Raven::instance() {
    if (!g_client)
        g_client = new Raven(NULL);

    return g_client;
}

bool Raven::initialize(const QString& DSN) {
    // If an empty DSN is passed, you should treat it as valid option 
    // which signifies disabling the client.
    if (DSN.isEmpty()) {
        qDebug() << "Raven client is disabled.";
        return true;
    }

    QUrl url(DSN);
    if (!url.isValid())
        return false;

    protocol_ = url.scheme();
    public_key_ = url.userName();
    secret_key_ = url.password();
    host_ = url.host();
    path_ = url.path();
    project_id_ = url.fileName();

    int port = url.port(80);
    if (port != 80)
        host_.append(":").append(QString::number(port));

    qDebug() << "Raven client is ready.";

    initialized_ = true;
    return true;
}

void Raven::set_global_tags(const QString& key, const QString& value) {
    if (!key.isEmpty() && !value.isEmpty())
        global_tags_[key] = value;
}

void Raven::set_user_data(const QString& key, const QString& value) {
    if (!key.isEmpty() && !value.isEmpty())
        user_data_[key] = value;
}

// static
// According to http://sentry.readthedocs.org/en/latest/developer/client/
void Raven::captureMessage(
    Level level,
    const QString& from_here,
    const QString& message,
    QJsonObject extra,
    QJsonObject tags) {
    Raven* instance = Raven::instance();
    if (!instance->initialized_) {
        qDebug() << "Sentry has not been initialized succefully. Failed to send message.";
        return;
    }

    QJsonObject jdata;
    jdata["event_id"] = QUuid::createUuid().toString();
    jdata["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    jdata["server_name"] = QHostInfo::localHostName();
    jdata["user"] = instance->GetUserInfo();
    jdata["level"] = GetLevelString(level);
    jdata["culprit"] = from_here;
    jdata["message"] = message;
    jdata["logger"] = kClientName;
    jdata["platform"] = "C++/Qt";

    // Add global tags
    const QJsonObject& gtags = instance->global_tags_;
    if (!gtags.isEmpty()) {
        for (auto it = gtags.begin(); it != gtags.end(); ++it)
            tags[it.key()] = *it;
    }

    if (!tags.isEmpty())
        jdata["tags"] = tags;
    if (!extra.isEmpty())
        jdata["extra"] = extra;

    //jdata["modules"] = QJsonObject();

    instance->Send(jdata);
}

void Raven::Send(const QJsonObject& jbody) {
    QString client_info = QString("%1/%2").
        arg(kClientName).arg(kClientVersion);

    QString auth_info = QString("Sentry sentry_version=5,sentry_client=%1,sentry_timestamp=%2,sentry_key=%3,sentry_secret=%4").
        arg(client_info, QString::number(time(NULL)), public_key_, secret_key_);

    // FIXME(ari): Should add path_?
    // http://192.168.1.22:9000/api/4/store/
    QString url = QString("%1://%2/api/%3/store/").arg(protocol_).arg(host_).arg(project_id_);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, client_info);
    request.setRawHeader("X-Sentry-Auth", auth_info.toUtf8());

    QByteArray body = QJsonDocument(jbody).toJson(QJsonDocument::Indented);

    QNetworkReply* reply = network_access_manager_.post(request, body);
    connect(reply, SIGNAL(finished()), this, SLOT(slotFinished()));
}

void Raven::slotFinished() {
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());

    QNetworkReply::NetworkError e = reply->error();
    QByteArray res = reply->readAll();
    if (e == QNetworkReply::NoError) {
        qDebug() << "Sentry log ID " << res;
    } else {
        qDebug() << "Failed to send log to sentry: [" << e << "]:" << res ;
    }

    reply->deleteLater();
}

QJsonObject Raven::GetUserInfo() {
    QJsonObject user;
    if (!user_id_.isEmpty())
        user["id"] = user_id_;
    if (!user_name_.isEmpty())
        user["username"] = user_name_;
    if (!user_email_.isEmpty())
        user["email"] = user_email_;
    
    if (!user_data_.isEmpty()) {
        for (auto it = user_data_.begin(); it != user_data_.end(); ++it)
            user[it.key()] = *it;
    }

    user["ip_address"] = LocalAddress();
    return user;
}

QString Raven::LocalAddress() {
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    for (int nIter = 0; nIter < list.count(); nIter++) {
        if (!list[nIter].isLoopback()) {
            if (list[nIter].protocol() == QAbstractSocket::IPv4Protocol)
                return list[nIter].toString();
        }
    }

    return "0.0.0.0";
}

//static
QString Raven::locationInfo(const char* file, const char* func, int line) {
    return QString("%1 in %2 at %3").arg(file).arg(func).arg(QString::number(line));
}

// static
QString Raven::GetLevelString(Level level) {
    QString s;
    switch (level) {
    case kDebug:
        s = "debug";
        break;
    case kInfo:
        s = "info";
        break;
    case kWarning:
        s = "warning";
        break;
    case kError:
        s = "error";
        break;
    case kFatal:
        s = "fatal";
        break;
    default:
        qDebug() << "Wrong sentry log level, default to error.";
        s = "error";
        break;
    }

    return s;
}
