/**
 * @author igordcard
 * @author Deltik
 */
#ifndef HELPERS_H
#define HELPERS_H

#include <QDialog>
#include <QLayoutItem>
#include <QGridLayout>
#include <QMap>
#include "log2log.h"

class Helper
{
public:
    Helper();

public:
    static void hide(QGridLayout *ql);
    static void hide(QVBoxLayout *ql);
    static void show(QGridLayout *ql);
    static void show(QVBoxLayout *ql);
    static void convertMode(Ui::Log2Log *parent);
    static void mainMode(Ui::Log2Log *parent);
    static void hideWebItems(QLayout *ql1, QLayout *ql2);
    static void hidePathItems(QLayout *ql, QWidget *qw);
    static void showWebItems(QLayout *ql1, QLayout *ql2);
    static void showPathItems(QLayout *ql, QWidget *qw);
    QString whatPrecision(int index);
    QString whatTrinary(int bit);
    /* PHP Function Ports */
    static void dummy();
    static QMap<QString, QVariant> files_get_contents(QString directory_path);
    static QList<QList<QString> > timezone_abbreviations_list();
};

#endif // HELPERS_H
