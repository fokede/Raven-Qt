Raven-Qt
========

Raven-Qt is a C++/Qt5 for client [Sentry](www.getsentry.com). It is very easy to use and support captureMessage, tags, user interface.etc.

Raven-Qt doesn't support backtrace itself, but with the help of the [Utils](git@github.com:arifeng/Utils.git) library, you can achieve this easily.

## Quick start

### Basic usage

Raven client uses instance mode and can be initialized with a single DSN string.

    Raven* raven = Raven::instance();
    raven->initialize("http://public_key:secret@192.168.1.22:9000/4");

After initialized, send log message to sentry server is rather simple:

    Raven::captureMessage(RAVEN_INFO, "something interesting...");

The first paramter is log level, which can be one of RAVEN_DEBUG, RAVEN_INFO, RAVEN_WARNING, RAVEN_ERROR and RAVEN_FATAL.

### Advaned usage

If you have some extra information to send:

    QJsonObject extra;
    extra["key"] = "value";
    Raven::captureMessage(RAVEN_INFO, "the message", extra);

If you want to add some tags for every message to be sent, just set global tags:

    raven->set_global_tags("version", "3.4.1.3083");
    raven->set_global_tags("CPU", "Intel(R) Core(TM) i7-4850HQ");
    raven->set_global_tags("operating system", "Windows NT 6.2 x86_64");
    //...

Also, user info can be easily set this way (optional):

    raven->set_user_id("24D37EC34543D29E84C");
    raven->set_user_name("your_name");
    raven->set_user_email("your_email");
    raven->set_user_data("key1", "value1");
    raven->set_user_data("key2", "value2");
    //...

That's all!
