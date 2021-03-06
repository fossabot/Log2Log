/**
 * Log2Log
 *  Formats
 *   AIM
 *
 * @author Deltik
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
 */

#include "aim.h"
#include "helper.h"

Aim::Aim()
{
    final = new StdFormat();
    final->setClient("AIM");
}

/**
 * Load a Chat Log
 */
void Aim::load(QVariant $log_raw)
{
    // If $log_raw does not contain an AIM chat log...
    if (!$log_raw.toString().startsWith("<?xml version=\"1.0\" standalone=\"yes\" encoding=\"UTF-8\" ?>\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n<html>\n<head>\n<meta http-equiv=\"content-type\" content=\"application/xhtml+xml;charset=utf-8\"/>\n<title>IM History with buddy "))
    {
        return;
    }

    // Import the chat log.
    QString $log_proc = $log_raw.toString();

    // Remove cruft that will interfere with QXmlStreamReader.
    QStringList $log_split = $log_proc.split("</head>");
    $log_split.pop_front();
    $log_proc = $log_split.join("</head>");
    $log_proc.replace("</body></html>", "</body>");

    // Get _self and _with
    QString $self, $with;
    // Non-group chat (_self and _with defined)
    if ($log_proc.contains("<a href=\"aim:goim?screenName="))
    {
        QString $users_proc = $log_proc;
        QStringList $users_split = $users_proc.split("<a href=\"aim:goim?screenName=");
        $users_split.pop_front();
        $users_proc = $users_split.join("<a href=\"aim:goim?screenName=");
        $users_split = $users_proc.split("&targetBuddyList=");
        $with = $users_split.takeFirst();
        $users_proc = $users_split.join("&targetBuddyList");
        $users_split = $users_proc.split("\">");
        $self = $users_split.takeFirst();
    }
    // Group chat (sorry; there's no _with information)
    else
    {
        $self = accountGuess;
        $with = buddyGuess;
    }

    // Remove more cruft that will interfere with QXmlStreamReader.
    $log_split = $log_proc.split("</h1>");
    $log_split.pop_front();
    $log_proc = $log_split.join("</h1>");
    $log_proc = "<body>" + $log_proc;

    // Remove even more cruft that will interfere with QXmlStreamReader.
    $log_proc = html_entity_decode($log_proc);

    // Create HTML reader
    QXmlStreamReader xml($log_proc);

    // Read the HTML file.
    while (!xml.atEnd())
    {
        // Read next item
        QXmlStreamReader::TokenType token = xml.readNext();

        // Just some items that might be used in this scope
        qlonglong               $datetime_base;
        QDate                   $date_base;
        QTime                   $time_base;
        QString                 $self_alias;
        QString                 $with_alias;

        // Looking at element beginnings...
        if (token == QXmlStreamReader::StartElement)
        {
            // If token is the chat entry indicator / time indicator
            if (xml.attributes().value("class").toString() == "time")
            {
                // CONSTRUCT: _entry
                final->newEntry();
                // CONSTRUCT: _self
                final->setSelf($self);
                // CONSTRUCT: _with
                final->setWith($with);
                // CONSTRUCT: _protocol
                final->setProtocol("aim");

                // Point to the datestamp
                xml.readNext();

                // Reset upper scope variables
                $datetime_base = NULL;
                $date_base     = QDate();
                $time_base     = QTime();
                $self_alias    . clear();
                $with_alias    . clear();

                QString $date_raw = xml.text().toString();
                $date_base = interpretDate($date_raw);
            }

            // If token is a chat row
            if (xml.attributes().value("class").toString() == "local" ||
                xml.attributes().value("class").toString() == "remote")
            {
                // What kind of chat row, again?
                QString $class = xml.attributes().value("class").toString();

                // New chat row
                final->newRow();

                // Point to the sender alias and timestamp.
                xml.readNext();

                QString $meta_raw = xml.text().toString();

                // Handle Time
                QStringList $meta_split = $meta_raw.split(" (");
                QString $timestamp = $meta_split.takeLast();
                QStringList $timestamp_split = $timestamp.split("):");
                $timestamp = $timestamp_split.takeFirst().trimmed();
                $time_base = interpretTime($timestamp);

                // Handle Sender
                QString $sender = $meta_split.join(" (").trimmed();

                // CONSTRUCT: _sender
                if ($class == "local")
                {
                    final->setSender("_self");
                    // CONSTRUCT: _self_alias
                    $self_alias = $sender;
                    final->setSelfAlias($self_alias);
                }
                else if ($class == "remote")
                {
                    final->setSender("_with");
                    // CONSTRUCT: _with_alias
                    $with_alias = $sender;
                    final->setWithAlias($with_alias);
                }
                else
                    final->setSender("_unknown");
                // CONSTRUCT: _sender_alias
                final->setAlias($sender);

                // Entering: </td>
                xml.readNext();
                // Entering: <td class="msg" width="100%">
                xml.readNext();

                // Content variable
                QString $message;
                // Grab all message HTML
                while (xml.tokenType() != QXmlStreamReader::EndElement &&
                       xml.qualifiedName().toString() != "</td>")
                {
                    // Entering next token
                    token = xml.readNext();
                    // If: <FONT>
                    if (token == QXmlStreamReader::StartElement)
                    {
                        QXmlStreamAttributes attrs = xml.attributes();
                        QList<QXmlStreamAttribute> attrs_list = attrs.toList();

                        $message += "<" +
                                    xml.qualifiedName().toString().toLower();

                        if (attrs_list.size() > 0)
                        {
                            $message += " ";

                            for (int i = 0; i < attrs_list.size(); i ++)
                            {
                                $message += attrs_list[i].qualifiedName().toString() +
                                            "=\"" +
                                            attrs_list[i].value().toString() +
                                            "\"";
                                if (i < attrs_list.size() - 1)
                                    $message += " ";
                            }
                            $message += ">";
                        }
                    }
                    // If </FONT>
                    if (token == QXmlStreamReader::EndElement &&
                        xml.qualifiedName().toString().toLower() != "td")
                    {
                        $message += "</" +
                                    xml.qualifiedName().toString().toLower() +
                                    ">";
                    }
                    // If message content, the good stuff...
                    else
                        $message += xml.text().toString();
                }

                // CONSTRUCT: _message
                final->setContent($message);

                // Handle DateTime
                // @precondition Date base is valid.
                if ($date_base.isValid())
                {
                    QDateTime dt($date_base, $time_base);
                    // Next-day rollover
                    while (dt.toMSecsSinceEpoch() < $datetime_base)
                    {
                        dt.addDays(1);
                    }
                    $datetime_base = dt.toMSecsSinceEpoch();

                    // CONSTRUCT: _time
                    final->setTime($datetime_base);
                }
            }

            // If token is an event
            if (xml.attributes().value("class").toString() == "event")
            {
                // New chat row
                final->newRow();

                // Get the raw event.
                xml.readNext();
                QString $event_raw = xml.text().toString();

                // Process Log2Log-supported events. TODO
                QHash<QString, QString> $sweep_terms;
                $sweep_terms["signed off at"] = "_evt_offline";
                $sweep_terms["signed on at"]  = "_evt_online";
                $sweep_terms["went away at"]  = "_evt_away";
                $sweep_terms["returned at"]   = "_evt_available";

                QHashIterator<QString, QString> i($sweep_terms);
                while (i.hasNext())
                {
                    i.next();
                    QStringList testplosion = $event_raw.split(i.key());
                    // If term matched...
                    if (testplosion.length() == 2)
                    {
                        QString $sender_alias = testplosion.takeFirst().trimmed();
                        QString $timestamp_raw = testplosion.takeLast().trimmed();
                        $time_base = interpretTime($timestamp_raw);
                        if ($sender_alias == $self_alias)
                            // CONSTRUCT: _sender
                            final->setContent("_self");
                        else if ($sender_alias == $with_alias)
                            // CONSTRUCT: _sender
                            final->setContent("_with");

                        // CONSTRUCT: _code
                        final->setCode(1);
                        // CONSTRUCT: _evt
                        final->setSender(i.value());
                        // CONSTRUCT: _sender_alias
                        final->setAlias($sender_alias);

                        // Handle DateTime
                        // @precondition Date base is valid.
                        if ($date_base.isValid())
                        {
                            QDateTime dt($date_base, $time_base);
                            // Next-day rollover
                            while (dt.toMSecsSinceEpoch() < $datetime_base)
                            {
                                dt.addDays(1);
                            }
                            $datetime_base = dt.toMSecsSinceEpoch();

                            // CONSTRUCT: _time
                            final->setTime($datetime_base);
                        }
                    }
                }
            }
        }
    }
}

/**
 * AIM_CUSTOM: Try to Comprehend an AIM Datestamp
 */
QDate Aim::interpretDate(QString input)
{
    QDate attempt;
    // Try: "Monday, August 08, 2010"
    attempt = QDate::fromString(input, "dddd, MMMM dd, yyyy");
    // Try: "Monday, August 8, 2010"
    if (!attempt.isValid())
        attempt = QDate::fromString(input, "dddd, MMMM d, yyyy");
    // Try: "08 August 2010"
    if (!attempt.isValid())
        attempt = QDate::fromString(input, "dd MMMM yyyy");
    // Try: "8 August 2010"
    if (!attempt.isValid())
        attempt = QDate::fromString(input, "d MMMM yyyy");

    return attempt;
}

/**
 * AIM_CUSTOM: Try to Comprehend an AIM Timestamp
 */
QTime Aim::interpretTime(QString input)
{
    QTime attempt;
    // Try: "1:23:45 AM"
    attempt = QTime::fromString(input, "h:mm:ss AP");
    // Try: "1:23:45 am"
    if (!attempt.isValid())
        attempt = QTime::fromString(input, "h:mm:ss ap");
    // Try: "01:23:45 AM"
    if (!attempt.isValid())
        attempt = QTime::fromString(input, "hh:mm:ss AP");
    // Try: "01:23:45 AM"
    if (!attempt.isValid())
        attempt = QTime::fromString(input, "hh:mm:ss ap");
    // Try: "1:23:45"
    if (!attempt.isValid())
        attempt = QTime::fromString(input, "h:mm:ss");
    // Try: "01:23:45"
    if (!attempt.isValid())
        attempt = QTime::fromString(input, "hh:mm:ss");
    // Try: "01:23:45.678"
    if (!attempt.isValid())
        attempt = QTime::fromString(input, "hh:mm:ss.zzz");

    return attempt;
}

/**
 * Convert all HTML entities to their applicable characters
 * @todo Port from PHP (and steal its entity table >:D )
 * @warning This feature is incomplete.
 */
QString Aim::html_entity_decode(QString input)
{
    input.replace("&#160;", " ");
    input.replace("&apos;", "'");

    return input;
}

/**
 * Generate Log from Standardized Log
 */
QVariant Aim::generate(StdFormat *$log)
{
    // TODO
}

/**
 * Process "From" Request
 */
StdFormat* Aim::from(QHash<QString, QVariant> data)
{
    // Step 1/3: Fetch the data.
    QMap<QString, QVariant> list;
    if (data["files"].isNull())
        list = Helper::files_get_contents(data["path"].toString());
    else
        list = data["files"].toMap();
    QMap<QString, QVariant>::const_iterator i = list.constBegin();

    // Step 2/3: Process the data.
    int c = 0;
    while (i != list.constEnd())
    {
        QVariant $raw_item = (i.value());

        /// "%PATHUPTOAIMLOGGER%/AIMLogger/your_username/IM Logs/buddy_username.html"
        QString unames_proc = i.key();

        // Guess the account and buddy (not accurate)
        accountGuess.clear();
        buddyGuess.clear();
        if (unames_proc.endsWith(".html"))
        {
            unames_proc = QDir::fromNativeSeparators(unames_proc);
            unames_proc.remove(unames_proc.length() - 4);
            QStringList unames_split = unames_proc.split("/");
            // Buddy Username
            buddyGuess = unames_split.takeLast();
            buddyGuess.remove(buddyGuess.length() - 10);
            if (buddyGuess.startsWith("conf-"))
                buddyGuess = "_group";
            // Cruft
            unames_split.pop_back();
            // Account Username
            accountGuess = unames_split.takeLast();
            accountGuess.remove(accountGuess.length() - 10);

            this->load($raw_item);
        }

        c++;
        updateProgress((40 * c / list.count()) + 10, "Interpreted " + QVariant(c).toString() + "/" + QVariant(list.count()).toString() + " files...");
        i ++;
    }final->applyAuto();qDebug()<<final->log;

    // Step 3/3: Submit the Log2Log-standardized chat log array.
    emit finished();
    return this->final;
}

/**
 * Process "To" Request
 */
void Aim::to(StdFormat* $log)
{
    // Count Log Entries
    updateProgress(50, "Counting Log Entries...");
    $log->resetPointer();
    total = 0;
    while ($log->nextEntry())
        {
            total ++;
            updateProgress(50, "Counting Log Entries... ("+QVariant(total).toString()+" found so far)");
        }

    data = this->generate($log);
    emit finished();
}
