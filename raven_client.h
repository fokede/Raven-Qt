// Sentry client for Qt5
// Author: ari.feng (ari.feng@forensix.cn)
//
// This client uses sentry protocol version 5

#ifndef RAVEN_QT5_H
#define RAVEN_QT5_H

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>
#include <QEventLoop>
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
    Raven();
    ~Raven();

    // Log level
    enum Level { kDebug = 0, kInfo, kWarning, kError, kFatal };

    static Raven* instance();

    // Initialize raven client with a standard DSN string
    // Note: Call this *after* constructing QApplication
    bool initialize(const QString& DSN);
    void waitForIdle();


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

    // Convert from UNIX log level (0-7) to sentry log level (0-4)
    // 0 ~ 7 represents: Fatal, Alert, Critcial, Error, Warning, Notice, Info, Debug
    static Level fromUnixLogLevel(int i);

    // Do not use manually, used by the RAVEN_HERE macro only.
    static QString locationInfo(const char* file, const char* func, int line);

public:
    static const char kClientName[];

    static const char kClientVersion[];

private slots:
    void slotFinished();
    void slotSslError(const QList<QSslError>&);

private:
    // Send to Sentry server
    void Send(const QJsonObject& jbody);

    QJsonObject GetUserInfo();

    QString LocalAddress();

    void ConnectReply(QNetworkReply* reply);

    int GetRequestId(QNetworkReply* reply);

    static QString GetLevelString(Level level);

private:
    // The sentry protocol version
    static const int kProtocolVersion = 5;

    // Maximium retry to send the log message when network failure occured.
    static const int kMaxSendRetry = 3;

    static const int kInvalidRequestId = -1;

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

    QMap<int, QByteArray> pending_request_;

    QNetworkAccessManager network_access_manager_;
    QEventLoop *waitLoop_;
};

#endif // SENTRYCLIENT_H
