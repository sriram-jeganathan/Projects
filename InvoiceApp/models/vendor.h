#ifndef VENDOR_H
#define VENDOR_H

#include <QString>

struct Vendor {
    int id = -1;
    QString name;
    QString gst;
    QString phone;
    QString email;
    QString address;
    QString taxDetails;
    QString createdAt;
};

#endif // VENDOR_H
