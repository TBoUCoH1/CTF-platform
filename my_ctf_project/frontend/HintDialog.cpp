#include "HintDialog.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>

HintDialog::HintDialog(const QString &title, const QString &message, HintDialog::Type type, QWidget *parent)
    : QDialog(parent) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setMinimumWidth(400);
    setMaximumWidth(600);

    QString borderColor, iconBg, btnColor, iconText;
    if (type == Success) {
        borderColor = "#22C55E";
        iconBg = "rgba(34, 197, 94, 0.15)";
        btnColor = "#22C55E";
        iconText = "✓";
    } else if (type == Error) {
        borderColor = "#EF4444";
        iconBg = "rgba(239, 68, 68, 0.15)";
        btnColor = "#EF4444";
        iconText = "✕";
    } else {
        borderColor = "#3B82F6";
        iconBg = "rgba(59, 130, 246, 0.15)";
        btnColor = "#3B82F6";
        iconText = "ℹ";
    }

    QWidget *container = new QWidget(this);
    container->setStyleSheet(QString("background-color: #1E293B; border: 1px solid %1; border-radius: 16px;").arg(borderColor));

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0,0,0,120));
    shadow->setOffset(0, 8);
    container->setGraphicsEffect(shadow);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addWidget(container);

    QVBoxLayout *contentLayout = new QVBoxLayout(container);
    contentLayout->setContentsMargins(28, 24, 28, 24);
    contentLayout->setSpacing(16);

    QHBoxLayout *header = new QHBoxLayout();
    QLabel *icon = new QLabel(iconText);
    icon->setFixedSize(44, 44);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet(QString("background-color: %1; color: %2; border-radius: 12px; font-size: 20px; border: none;").arg(iconBg, borderColor));

    QLabel *titleLbl = new QLabel(title);
    titleLbl->setStyleSheet("color: white; font-size: 20px; font-weight: bold; border: none;");

    QPushButton *closeBtn = new QPushButton("×");
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { background: transparent; color: #64748B; border: none; font-size: 24px; } QPushButton:hover { color: #EF4444; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    header->addWidget(icon);
    header->addWidget(titleLbl, 1);
    header->addWidget(closeBtn);

    QLabel *msgLbl = new QLabel(message);
    msgLbl->setWordWrap(true);
    msgLbl->setStyleSheet("color: #94A3B8; font-size: 15px; border: none; background: transparent;");
    msgLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(msgLbl);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMaximumHeight(400);
    scrollArea->setMinimumHeight(80);

    scrollArea->setStyleSheet(QString(R"(
        QScrollArea {
            border: none;
            background-color: transparent;
        }

        QScrollBar:vertical {
            border: none;
            background: #0F172A;
            width: 8px;
            border-radius: 4px;
            margin: 0px;
        }

        QScrollBar::handle:vertical {
            background-color: #334155;
            border-radius: 4px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: %1;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            border: none;
            background: none;
            height: 0px;
        }
    )").arg(borderColor));

    QPushButton *okBtn = new QPushButton("OK");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet(QString("background-color: %1; color: white; border: none; border-radius: 10px; padding: 12px; font-weight: bold;").arg(btnColor));
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);

    contentLayout->addLayout(header);
    contentLayout->addWidget(scrollArea);
    contentLayout->addWidget(okBtn);
}

void HintDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void HintDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void HintDialog::showInfo(const QString &t, const QString &m, QWidget *p) { HintDialog(t, m, Info, p).exec(); }
void HintDialog::showSuccess(const QString &t, const QString &m, QWidget *p) { HintDialog(t, m, Success, p).exec(); }
void HintDialog::showError(const QString &title, const QString &message, QWidget *parent) {
    qDebug() << "HintDialog::showError called:" << title << message;
    HintDialog dialog(title, message, Error, parent);
    dialog.exec();
}
void HintDialog::showHint(const QString &t, const QString &m, QWidget *p) { HintDialog(t, m, Info, p).exec(); }
