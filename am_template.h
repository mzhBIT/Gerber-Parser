#ifndef AM_TEMPLATE_H
#define AM_TEMPLATE_H

#include <QStringList>

class am_template
{

    QString name;                // Macro template name
    QStringList all_data_blocks; // An array of block data strings describing the macro template

public:

    am_template(const QString name_of_macro, const QStringList data_blocks);
    QString get_name();
    QStringList get_data_blocks();

};

#endif // AM_TEMPLATE_H
