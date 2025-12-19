#ifndef HINTDIALOG_H
#define HINTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>

class HintDialog : public QDialog {
    Q_OBJECT
public:
    enum Type { Info, Success, Error };
    explicit HintDialog(const QString &title, const QString &msg, Type type, QWidget *parent = nullptr);
    static void showInfo(const QString &t, const QString &m, QWidget *p);
    static void showSuccess(const QString &t, const QString &m, QWidget *p);
    static void showError(const QString &t, const QString &m, QWidget *p);
    static void showHint(const QString &t, const QString &m, QWidget *p);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint dragPosition;
};
#endif