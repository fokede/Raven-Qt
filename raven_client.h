// Sentry client for Qt5
// Author: ari.feng (ari.feng@forensix.cn)
//
// This client uses protocol version 5
// With the help of the Utils library, exceptions can be handled by setting a external backtrace handler.
//
// // Basic usage
// Raven::captureMessage(RAVEN_INFO, "something interesting...");
// 
// // With extra data:
// QJsonObject extra;
// extra["key"] = "value";
// Raven::captureMessage(RAVEN_INFO, "the message", extra);

#ifndef RAVEN_QT5_H
#define RAVEN_QT5_H

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

#define RAVEN_HERE Raven::locationInfo(__FILE__, __FUNCTION__, __LINE__)

#define RAVEN_DEBUG Raven::kDebug, RAVEN_HERE
#define RAVEN_INFO Raven::kInfo, RAVEN_HERE
#define RAVEN_WARNING Raven::kWarning, RAVEN_HERE
#define RAVEN_ERROR Raven::kError, RAVEN_HERE
#define RAVEN_FATAL Raven::kFatal, RAVEN_HERE

class Raven : public QObject
{
    Q_OBJECT

public:
    Raven(QObject *parent);
    ~Raven();

    // Log level
    enum Level { kDebug = 0, kInfo, kWarning, kError, kFatal };

    static Raven* instance();

    // Initialize raven client with a standard DSN string
    bool initialize(const QString& DSN);

    // Log to Sentry server
    // Logs are grouped by parameter message
    static void captureMessage(Level level, const QString& from_here, 
        const QString& message, 
        QJsonObject extra = QJsonObject(),
        QJsonObject tags = QJsonObject());

    // Tags that will be sent always. e.g Software version
    void set_global_tags(const QString& key, const QString& value);

    // Set user info
    void set_user_id(const QString& id) { user_id_ = id; }
    void set_user_name(const QString& name) { user_name_ = name; }
    void set_user_email(const QString& email) { user_email_ = email; }
    void set_user_data(const QString& key, const QString& value);

    // Do not use manually, used by the RAVEN_HERE macro only.
    static QString locationInfo(const char* file, const char* func, int line);

public:
    static const char kClientName[];

    static const char kClientVersion[];

private slots:
    void slotFinished();

private:
    // Send to Sentry server
    void Send(const QJsonObject& jbody);

    QJsonObject GetUserInfo();

    QString LocalAddress();

    static QString GetLevelString(Level level);

private:
    // The sentry protocol version
    static const int kProtocolVersion = 5;

    // Maximium retry to send the log message when network failure occured.
    static const int kMaxSendRetry = 3;

    bool initialized_;

    // DSN components
    QString protocol_;
    QString public_key_;
    QString secret_key_;
    QString host_;
    QString path_;
    QString project_id_;

    // User Interface
    QString user_id_;
    QString user_name_;
    QString user_email_;
    QJsonObject user_data_;

    // Global tags
    QJsonObject global_tags_;

    QNetworkAccessManager network_access_manager_;
};

#endif // SENTRYCLIENT_H
