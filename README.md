Raven-Qt
========

Rave-Qt is a C++/Qt5 for client (Sentry)[www.getsentry.com]. It is veriy easy to use and support captureMessage, tags, user interface.etc. With the help of the [Utils](192.168.1.6/ari.feng/utils) library, you can also send backtrace to the server.

## Usage

### Initalize

Raven client uses instance mode and can be initialized with a single DSN string.

    Raven* raven = Raven::instance();
    raven->initialize("http://public_key:secret@192.168.1.22:9000/4");

If you want to add some tags for every message to be sent, just set global tags:

    raven->set_global_tags("version", "3.4.1.3083");
    raven->set_global_tags("platform", "Windows 7 64-bit");
    //...
    
Also, user info can be easily set this way (optional):

    raven->set_user_id("1234567890");
    raven->set_user_name("ari.feng");
    raven->set_user_email("ari.feng@forensix.cn");
    raven->set_user_data("key1", "value1");
    raven->set_user_data("key2", "value2");
    
After these, send a log message to sentry server is rather simple:

    Raven::captureMessage(RAVEN_INFO, "something interesting...");
    
Or if you have some extra information:

    QJsonObject extra;
    extra["key"] = "value";
    Raven::captureMessage(RAVEN_INFO, "the message", extra);
    
That's all!
