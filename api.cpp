/**
 * Log2Log Chat Log Converter
 *  Libraries
 *   API Handler
 *
 * @author Deltik
 * @remarks Ported from PHP
 *
 * License:
 *  This file is part of Log2Log.
 *
 *  Log2Log is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Log2Log is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Log2Log.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "api.h"

/**
 * Constructor
 */
Api::Api()
{
    netHandler = new QNetworkAccessManager(this);
    moveToThread(this);
}

/**
 * Thread Runner
 */
void Api::run()
{
    netHandler = new QNetworkAccessManager(this);
    connect(netHandler, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    getURL();

    exec();
}

/**
 * Set URL for use with QNetworkRequest
 * @param string $url Full URL of the API request
 */
void Api::setURL(QUrl url)
{
    $url = url;
    request = QNetworkRequest($url);
}
/// Alias
void Api::setURL(QString url)
{
    return setURL(QUrl(url, QUrl::TolerantMode));
}

/**
 * Add HTTP Query Property
 * @param string index Index of the query property
 * @param string data Content of the query property
 */
void Api::addPost(QString index, QString data)
{
    params.addQueryItem(index, data);
    $_POST = params.encodedQuery();
}

/**
 * Set Header
 */
void Api::setHeader(QString key, QString value)
{
    request.setRawHeader(key.toAscii(), value.toAscii());
}

/**
 * Get URL using QNetworkRequest
 */
void Api::getURL()
{
    jar = new QNetworkCookieJar();
    if (this->hed.contains("Set-Cookie"))
        jar->setCookiesFromUrl(QNetworkCookie::parseCookies(this->hed["Set-Cookie"].toAscii()), this->$url);
    netHandler->setCookieJar(jar);

    if (!$_POST.isEmpty())
    {
        netHandler->post(request, $_POST);qDebug()<<"POST IS: "<<$_POST;
    }
    else
    {
        netHandler->get(request);
    }
    params.clear();
    $_POST.clear();
}

void Api::replyFinished(QNetworkReply* pReply)
{
    // Localize reply into a variable
    QByteArray data = pReply->readAll();

    // Store header
    hed = QHash<QString, QString>();
    QList<QPair<QByteArray, QByteArray> > hedPair = pReply->rawHeaderPairs();
    for (int i = 0; i < hedPair.size(); i ++)
    {
        QPair<QByteArray, QByteArray> temp = hedPair[i];
        hed[QString(temp.first)] = QString(temp.second);
    }

    // Store content
    str = QVariant(data).toString();
    emit requestComplete(str);

    emit finished();
    this->quit();
}

QString Api::getReply()
{
    return str;
}
