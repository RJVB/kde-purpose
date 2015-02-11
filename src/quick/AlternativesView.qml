/************************************************************************************
 * Copyright (C) 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>                *
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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
import org.kde.purpose 1.0

StackView {
    id: stack
    property alias pluginType: altsModel.pluginType
    property alias inputData: altsModel.inputData

    signal finished(string text)

    PurposeAlternativesModel {
        id: altsModel
    }

    initialItem: ScrollView {
        ListView {
            model: altsModel
            delegate: RowLayout {
                Label {
                    text: display
                }
                Button {
                    text: i18n("Use")
                    onClicked: {
                        var job = altsModel.createJob(index);
                        if (!job.isReady) {
                            stack.push({
                                item: shareWizardComponent,
                                properties: { job: job }
                            })
                        } else {
                            stack.push({
                                item: runningJobComponent,
                                properties: { job: job }
                            })
                            job.start()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: shareWizardComponent
        ColumnLayout {
            property alias job: wiz.job
            PurposeWizard {
                id: wiz

                Layout.fillHeight: true
                Layout.fillWidth: true
            }
            RowLayout {
                Button {
                    text: i18n("Run")
                    enabled: wiz.job && wiz.job.isReady
                    onClicked: {
                        stack.pop();
                        stack.push({
                            item: runningJobComponent,
                            properties: { job: wiz.job }
                        })
                        wiz.job.start();
                    }
                }
                Button {
                    text: i18n("Back")
                    onClicked: {
                        stack.pop();
                        wiz.cancel()
                    }
                }
            }
        }
    }
    Component {
        id: runningJobComponent
        ColumnLayout {
            id: root
            property alias job: conn.target
            Connections {
                id: conn
                onInfoMessage: {
                    info.text = rich
                }
                onResult: {
                    stack.finished(info.text)
                    stack.pop();
                }
            }
            Label {
                id: info

                Layout.fillWidth: true
                onLinkActivated: Qt.openUrlExternally(link)
            }
            ProgressBar {
                //FIXME: this is not really working yet, as QML doesn't understand ulong
                value: root.job.percent
                maximumValue: 100
                Layout.fillWidth: true
            }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
}