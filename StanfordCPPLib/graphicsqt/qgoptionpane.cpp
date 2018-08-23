/*
 * File: qgoptionpane.cpp
 * ----------------------
 *
 * This code is largely copied from goptionpane.cpp and modified to use
 * Qt's QMessageBox and QInputDialog classes.
 *
 * @version 2018/06/28
 * - initial version
 */

#ifdef SPL_QT_GUI
#include "qgoptionpane.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QWidget>
#include "error.h"
#include "qgthread.h"
#include "strlib.h"

QGOptionPane::QGOptionPane() {
    // empty
}

QGOptionPane::ConfirmResult QGOptionPane::showConfirmDialog(const std::string& message,
                                                            const std::string& title,
                                                            ConfirmType type) {
    return showConfirmDialog(/* parent */ nullptr, message, title, type);
}

QGOptionPane::ConfirmResult QGOptionPane::showConfirmDialog(QWidget* parent,
                                                            const std::string& message,
                                                            const std::string& title,
                                                            ConfirmType type) {
    if (type != QGOptionPane::ConfirmType::YES_NO
            && type != QGOptionPane::ConfirmType::YES_NO_CANCEL
            && type != QGOptionPane::ConfirmType::OK_CANCEL) {
        error("QGOptionPane::showConfirmDialog: Illegal dialog type");
    }
    std::string titleToUse = title.empty() ? std::string("Select an option") : title;

    // convert our enum types to Qt's button enum type
    QMessageBox::StandardButtons buttons;
    QMessageBox::StandardButton defaultButton = QMessageBox::Cancel;
    if (type == QGOptionPane::ConfirmType::YES_NO) {
        buttons = QMessageBox::Yes | QMessageBox::No;
        defaultButton = QMessageBox::No;
    } else if (type == QGOptionPane::ConfirmType::YES_NO_CANCEL) {
        buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;
    } else if (type == QGOptionPane::ConfirmType::OK_CANCEL) {
        buttons = QMessageBox::Ok | QMessageBox::Cancel;
    }

    QGOptionPane::ConfirmResult confirmResult = QGOptionPane::CANCEL;
    QGThread::runOnQtGuiThread([parent, titleToUse, message, buttons, defaultButton, &confirmResult]() {
        int dialogResult = QMessageBox::question(parent,
                QString::fromStdString(titleToUse),
                QString::fromStdString(message),
                buttons,
                defaultButton);
        switch (dialogResult) {
            case QMessageBox::Yes:
                confirmResult = QGOptionPane::ConfirmResult::YES;
                break;
            case QMessageBox::No:
                confirmResult = QGOptionPane::ConfirmResult::NO;
                break;
            case QMessageBox::Cancel:
            default:
                confirmResult = QGOptionPane::ConfirmResult::CANCEL;
                break;
        }
    });
    return confirmResult;
}

std::string QGOptionPane::showInputDialog(const std::string& message,
                                          const std::string& title,
                                          const std::string& initialValue) {
    return showInputDialog(/* parent */ nullptr, message, title, initialValue);
}

std::string QGOptionPane::showInputDialog(QWidget* parent,
                                          const std::string& message,
                                          const std::string& title,
                                          const std::string& initialValue) {
    std::string titleToUse = title.empty() ? std::string("Type a value") : title;
    std::string result = "";
    QGThread::runOnQtGuiThread([parent, titleToUse, message, initialValue, &result]() {
        result = QInputDialog::getText(parent,
                QString::fromStdString(titleToUse),
                QString::fromStdString(message),
                QLineEdit::Normal,
                QString::fromStdString(initialValue)).toStdString();
    });
    return result;
}

void QGOptionPane::showMessageDialog(const std::string& message,
                                     const std::string& title,
                                     MessageType type) {
    showMessageDialog(/* parent */ nullptr, message, title, type);
}

void QGOptionPane::showMessageDialog(QWidget* parent,
                                     const std::string& message,
                                     const std::string& title,
                                     MessageType type) {
    if (type != QGOptionPane::MessageType::PLAIN
            && type != QGOptionPane::MessageType::INFORMATION
            && type != QGOptionPane::MessageType::ERROR
            && type != QGOptionPane::MessageType::WARNING
            && type != QGOptionPane::MessageType::QUESTION
            && type != QGOptionPane::MessageType::ABOUT) {
        error("QGOptionPane::showMessageDialog: Illegal dialog type");
    }
    std::string titleToUse = title.empty() ? std::string("Message") : title;

    QGThread::runOnQtGuiThread([parent, message, titleToUse, type]() {
        if (type == QGOptionPane::MessageType::PLAIN
                || type == QGOptionPane::MessageType::INFORMATION
                || type == QGOptionPane::MessageType::QUESTION) {
            QMessageBox::information(parent, QString::fromStdString(titleToUse), QString::fromStdString(message));
        } else if (type == QGOptionPane::MessageType::WARNING) {
            QMessageBox::warning(parent, QString::fromStdString(titleToUse), QString::fromStdString(message));
        } else if (type == QGOptionPane::MessageType::ERROR) {
            QMessageBox::critical(parent, QString::fromStdString(titleToUse), QString::fromStdString(message));
        } else if (type == QGOptionPane::MessageType::ABOUT) {
            QMessageBox::about(parent, QString::fromStdString(titleToUse), QString::fromStdString(message));
        }
    });
}

std::string QGOptionPane::showOptionDialog(const std::string& message,
                                           const Vector<std::string>& options,
                                           const std::string& title,
                                           const std::string& initiallySelected) {
    return showOptionDialog(/* parent */ nullptr, message, options, title, initiallySelected);
}

std::string QGOptionPane::showOptionDialog(QWidget* parent,
                                           const std::string& message,
                                           const Vector<std::string>& options,
                                           const std::string& title,
                                           const std::string& initiallySelected) {
    std::string titleToUse = title.empty() ? std::string("Select an option") : title;
    QMessageBox box;
    if (parent) {
        box.setParent(parent);
    }
    box.setText(QString::fromStdString(message));
    box.setWindowTitle(QString::fromStdString(titleToUse));
    box.setAttribute(Qt::WA_QuitOnClose, false);

    for (std::string option : options) {
        box.addButton(QString::fromStdString(option), QMessageBox::ActionRole);
    }
    if (!initiallySelected.empty()) {
        // TODO: dunno how to set initially selected button properly
        // box.setDefaultButton(QString::fromStdString(initiallySelected));
    }

    std::string result = "";
    QGThread::runOnQtGuiThread([&box, &options, &result]() {
        int index = box.exec();
        if (index == QGOptionPane::InternalResult::CLOSED_OPTION
                || index < 0 || index >= options.size()) {
            result = "";
        } else {
            result = options[index];
        }
    });
    return result;
}

void QGOptionPane::showTextFileDialog(const std::string& message,
                                      const std::string& title,
                                      int /* rows */,
                                      int /* cols */) {
    std::string titleToUse = title.empty() ? std::string("Text file contents") : title;
    showMessageDialog(message, titleToUse);
    // TODO
    error("QGOptionPane::showTextFileDialog: not implemented");
}
void QGOptionPane::showTextFileDialog(QWidget* parent,
                                      const std::string& message,
                                      const std::string& title,
                                      int /* rows */,
                                      int /* cols */) {
    std::string titleToUse = title.empty() ? std::string("Text file contents") : title;
    showMessageDialog(parent, message, titleToUse);
    // TODO
    error("QGOptionPane::showTextFileDialog: not implemented");
}

#endif // SPL_QT_GUI
