/************************************************************************************
 * Copyright (C) 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>                *
 *                                                                                  *
 * This program is free software; you can redistribute it and/or                    *
 * modify it under the terms of the GNU General Public License                      *
 * as published by the Free Software Foundation; either version 2                   *
 * of the License, or (at your option) any later version.                           *
 *                                                                                  *
 * This program is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 * GNU General Public License for more details.                                     *
 *                                                                                  *
 * You should have received a copy of the GNU General Public License                *
 * along with this program; if not, write to the Free Software                      *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 ************************************************************************************/

#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QStandardPaths>
#include <QUrl>
#include <QDebug>

#include <purposewidgets/menu.h>
#include <purpose/alternativesmodel.h>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QScopedPointer<Purpose::Menu> menu(new Purpose::Menu);
    Purpose::AlternativesModel* model = menu->model();

    const QJsonObject input = QJsonObject {
        { QStringLiteral("urls"), QJsonArray {QStringLiteral("http://kde.org")} },
        { QStringLiteral("mimeType"), QStringLiteral("dummy/thing") }
    };
    model->setInputData(input);
    model->setPluginType(QStringLiteral("Export"));
    menu->reload();
    menu->exec();

    QObject::connect(menu.data(), &Purpose::Menu::finished, menu.data(), [](const QJsonObject &output, int error, const QString &errorMessage) {
        if (error != 0) {
            qDebug() << "job failed with error" << errorMessage;
        }
        qDebug() << "output:" << output;
    });

    return app.exec();
}
