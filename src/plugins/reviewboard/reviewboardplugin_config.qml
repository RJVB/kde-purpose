/*
 Copyright 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

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

import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import org.kde.purpose.reviewboard 1.0

ColumnLayout {
    id: root
    property alias server: serverField.text
    property alias username: usernameField.text
    property alias password: passwordField.text
    property alias repository: repositoriesCombo.currentText
    property string updateRR: ""
    property string baseDir
    property string reviewboardrc
    property variant extraData: rcfile.extraData

    ReviewboardRC {
        id: rcfile
        path: root.reviewboardrc
    }

    Label { text: i18n("Server:") }
    TextField {
        id: serverField
        Layout.fillWidth: true
        text: rcfile.server
    }
    Label { text: i18n("Username:") }
    TextField {
        id: usernameField
        Layout.fillWidth: true
    }
    Label { text: i18n("Password:") }
    TextField {
        id: passwordField
        echoMode: TextInput.Password
        Layout.fillWidth: true
    }

    Label { text: i18n("Repository:") }
    ComboBox {
        id: repositoriesCombo
        Layout.fillWidth: true
        textRole: "display"
        model: RepositoriesModel {
            server: serverField.text
            onRepositoriesChanged: {
                repositoriesCombo.currentIndex = findRepository(rcfile.repository);
            }
        }
    }

    Item {
        Layout.fillWidth: true
        height: update.height

        CheckBox {
            anchors.centerIn: parent
            id: update
            text: i18n("Update Review:")
            enabled: updateRRCombo.count > 0
            onCheckedChanged: {
                if (!checked)
                    root.updateRR = ""
            }
        }
    }
    ComboBox {
        id: updateRRCombo
        Layout.fillWidth: true
        enabled: update.checked
        textRole: "display"
        model: ReviewsListModel {
            id: reviewsList
            server: root.server
            repository: root.repository
            username: root.username
            status: "pending"
        }
        onCurrentIndexChanged: {
            if (currentIndex>0 && enabled)
                root.updateRR = reviewsList.get(currentIndex, "toolTip")
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
}