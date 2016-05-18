/*
 Copyright 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QFileInfo>
#include <QMetaMethod>
#include <KPluginMetaData>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QPointer>
#include <QLocalSocket>

#include "helper.h"
#include <purpose/configuration.h>
#include <purpose/job.h>

static QString pluginType;
static KPluginMetaData md;

class Communication : public QObject
{
Q_OBJECT
public:
    Communication(const QString &serverName)
    {
        int pcIdx = metaObject()->indexOfSlot("propertyChanged()");
        Q_ASSERT(pcIdx>=0);
        const QMetaMethod propertyChangedMethod = metaObject()->method(pcIdx);

        m_socket.setServerName(serverName);
        m_socket.connectToServer(QIODevice::ReadWrite);
        connect(&m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(error()));

        bool b = m_socket.waitForConnected();
        Q_ASSERT(b);

        b = m_socket.waitForReadyRead();
        Q_ASSERT(b);

        Q_ASSERT(m_socket.canReadLine());
        QByteArray byteLine = m_socket.readLine();
        byteLine.chop(1); // Drop \n
        const qint64 bytes = byteLine.toLongLong();
        // QByteArray and QJsonDocument::from* can only handle int size.
        // If the payload is bigger we are screwed.
        Q_ASSERT(bytes <= std::numeric_limits<int>::max());

        QByteArray dataArray;
        dataArray.resize(bytes);
        int pos = 0;
        bool couldRead = false;
        while ((pos < bytes) && (couldRead = (m_socket.bytesAvailable() || m_socket.waitForReadyRead()))) {
            pos += m_socket.read(dataArray.data() + pos, qMin(m_socket.bytesAvailable(), bytes-pos));
        }
        Q_ASSERT(couldRead); // false if we hit a timeout before read-end.
        Q_ASSERT(pos == bytes);

        Purpose::Configuration config(QJsonDocument::fromJson(dataArray).object(), pluginType, md);
        config.setUseSeparateProcess(false);

        Q_ASSERT(config.isReady());

        m_job = config.createJob();
        m_job->start();

        const QMetaObject* m = m_job->metaObject();
        for(int i = 0, c = m->propertyCount(); i<c; ++i) {
            QMetaProperty prop = m->property(i);
                connect(m_job, prop.notifySignal(), this, propertyChangedMethod);
            if (prop.hasNotifySignal() && prop.isReadable()) {
            }
        }
    }

private Q_SLOTS:
    void error() {
        qWarning() << "socket error:" << m_socket.error();
    }

    void propertyChanged() {
        const int idx = senderSignalIndex();

        const QMetaObject* m = m_job->metaObject();
        QJsonObject toSend;
        for(int i = 0, c = m->propertyCount(); i<c; ++i) {
            QMetaProperty prop = m->property(i);
            if (prop.notifySignalIndex() == idx) {
                toSend[QString::fromLatin1(prop.name())] = fromVariant(prop.read(m_job));
            }
        }
        send(toSend);
    }

    static QJsonValue fromVariant(const QVariant &var) {
        if (var.canConvert<QJsonObject>()) {
            return var.toJsonObject();
        } else {
            return QJsonValue::fromVariant(var);
        }
    }

private:
    void send(const QJsonObject &object) {
        const QByteArray data = QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n';
//         qDebug() << "sending..." << data;
        m_socket.write(data);
    }

    Purpose::Job* m_job = nullptr;
    QLocalSocket m_socket;
};

int main(int argc, char** argv)
{
#warning make QGuiApplication, consider QCoreApplication?
    QApplication app(argc, argv);

    QString serverName;

    {
        QCommandLineParser parser;
        parser.addOption(QCommandLineOption(QStringLiteral("server"), QStringLiteral("Named socket to connect to"), QStringLiteral("name")));
        parser.addOption(QCommandLineOption(QStringLiteral("pluginPath"), QStringLiteral("Chosen plugin"), QStringLiteral("path")));
        parser.addOption(QCommandLineOption(QStringLiteral("pluginType"), QStringLiteral("Plugin type name "), QStringLiteral("path")));
        parser.addHelpOption();

        parser.process(app);

        serverName = parser.value(QStringLiteral("server"));
        pluginType = parser.value(QStringLiteral("pluginType"));

        const QString path = parser.value(QStringLiteral("pluginPath"));
        if (path.endsWith(QLatin1String("/metadata.json"))) {
            QFileInfo fi(path);
            md = Purpose::createMetaData(path);
        } else {
            md = KPluginMetaData(path);
            Q_ASSERT(md.isValid());
        }
    }

    Communication c(serverName);

    return app.exec();
}

#include "purposeprocess_main.moc"
