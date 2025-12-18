#include "ChallengeManager.h"
#include "HintDialog.h"
#include <QStandardPaths>
#include <QFile>
#include <QProcess>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QFrame>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>
#include <QDir>
#include <QFileDialog>

ChallengeManager::ChallengeManager(QNetworkAccessManager *netManager, const QString &serverUrl, QObject *parent)
    : QObject(parent), manager(netManager), SERVER_URL(serverUrl), selectedTaskId(-1), selectedCard(nullptr) {
    challengesPage = nullptr;
}

QWidget* ChallengeManager::setupChallengesPage() {
    challengesPage = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(challengesPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === Левая панель - список задач ===
    QWidget *leftPanel = new QWidget();
    leftPanel->setFixedWidth(380);
    leftPanel->setStyleSheet(QString("background-color: %1;").arg(BG_DARK));

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 30, 20, 30);
    leftLayout->setSpacing(16);

    QLabel *challengesTitle = new QLabel("Задания");
    challengesTitle->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(TEXT_PRIMARY));

    // Скролл для задач
    taskScrollArea = new QScrollArea();
    taskScrollArea->setWidgetResizable(true);
    taskScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    taskScrollArea->setStyleSheet(QString(R"(
        QScrollArea {
            border: none;
            background-color: transparent;
        }
        QScrollBar:vertical {
            border: none;
            background: transparent;
            width: 6px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background-color: #334155;
            border-radius: 3px;
            min-height: 20px;
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
    )").arg(ACCENT_BLUE));

    taskListContainer = new QWidget();
    taskListLayout = new QVBoxLayout(taskListContainer);
    taskListLayout->setContentsMargins(0, 0, 12, 0);
    taskListLayout->setSpacing(12);
    taskListLayout->setAlignment(Qt::AlignTop);

    taskScrollArea->setWidget(taskListContainer);

    leftLayout->addWidget(challengesTitle);
    leftLayout->addWidget(taskScrollArea, 1);

    // === Вертикальный разделитель ===
    QWidget *divider = new QWidget();
    divider->setFixedWidth(1);
    divider->setStyleSheet(QString("background-color: %1;").arg(BORDER_COLOR));

    // === Правая панель - информация о задаче ===
    QWidget *rightPanel = new QWidget();
    rightPanel->setStyleSheet(QString("background-color: %1;").arg(BG_DARK));

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(30, 30, 30, 30);
    rightLayout->setSpacing(0);

    // Карточка с информацией о задаче
    QWidget *taskInfoCard = new QWidget();
    taskInfoCard->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 16px;
        }
    )").arg(BG_CARD, BORDER_COLOR));

    QVBoxLayout *taskInfoLayout = new QVBoxLayout(taskInfoCard);
    taskInfoLayout->setContentsMargins(30, 30, 30, 30);
    taskInfoLayout->setSpacing(16);

    // Task Info label
    QLabel *taskInfoLabel = new QLabel("Информация о задании");
    taskInfoLabel->setStyleSheet(QString("font-size: 13px; color: %1; background: transparent; border: none;").arg(TEXT_SECONDARY));

    // Заголовок задачи с кнопкой подсказки
    QHBoxLayout *titleRow = new QHBoxLayout();

    taskTitleLabel = new QLabel("Выберите задание");
    taskTitleLabel->setStyleSheet(QString("font-size: 26px; font-weight: bold; color: %1; background: transparent; border: none;").arg(TEXT_PRIMARY));
    taskTitleLabel->setWordWrap(true);

    // Кнопка подсказки с иконкой лампочки
    hintBtn = new QPushButton();
    QLabel *hintIcon = new QLabel("💡");
    hintIcon->setStyleSheet("font-size: 16px; background: transparent; border: none;");
    hintIcon->setAlignment(Qt::AlignCenter);

    QHBoxLayout *hintBtnLayout = new QHBoxLayout(hintBtn);
    hintBtnLayout->setContentsMargins(0, 0, 0, 0);
    hintBtnLayout->addWidget(hintIcon);

    hintBtn->setToolTip("Get a hint");
    hintBtn->setFixedSize(40, 40);
    hintBtn->setStyleSheet(QString(R"(
        QPushButton {
            background-color: rgba(59, 130, 246, 0.1);
            border: 1px solid %1;
            border-radius: 10px;
        }
        QPushButton:hover {
            background-color: rgba(59, 130, 246, 0.25);
            border-color: %2;
        }
    )").arg(BORDER_COLOR, ACCENT_BLUE));
    hintBtn->setCursor(Qt::PointingHandCursor);

    titleRow->addWidget(taskTitleLabel, 1);
    titleRow->addWidget(hintBtn);

    taskDescriptionLabel = new QLabel("Выберите задание из списка, чтобы увидеть его описание.");
    taskDescriptionLabel->setStyleSheet(QString("font-size: 15px; color: %1; line-height: 1.5; background: transparent; border: none;").arg(TEXT_SECONDARY));
    taskDescriptionLabel->setWordWrap(true);

    // Код сниппет
    codeSnippetEdit = new QTextEdit();
    codeSnippetEdit->setReadOnly(true);
    codeSnippetEdit->setMaximumHeight(180);
    codeSnippetEdit->setStyleSheet(QString(R"(
        QTextEdit {
            background-color: #1a1f2e;
            border: 1px solid %1;
            border-radius: 8px;
            padding: 16px;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 13px;
            color: %2;
        })").arg(BORDER_COLOR, ACCENT_CYAN));
    codeSnippetEdit->setPlaceholderText("Код появится здесь...");
    codeSnippetEdit->hide();
    
    // Поле для копируемых данных
    copyableDataEdit = new QTextEdit();
    copyableDataEdit->setReadOnly(true);
    copyableDataEdit->setMaximumHeight(100);
    copyableDataEdit->setStyleSheet(QString(R"(
        QTextEdit {
            background-color: #1e293b;
            border: 1px solid %1;
            border-radius: 8px;
            padding: 12px;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 14px;
            color: %2;
            selection-background-color: %3;
        }
    )").arg(BORDER_COLOR, TEXT_PRIMARY, ACCENT_BLUE));
    copyableDataEdit->setPlaceholderText("Данные для копирования появятся здесь...");
    copyableDataEdit->hide();

    // Ввод флага - полностью слитный компонент (Input Group)
    QWidget *flagContainer = new QWidget();
    flagContainer->setStyleSheet("background: transparent;");

    QHBoxLayout *flagLayout = new QHBoxLayout(flagContainer);
    flagLayout->setContentsMargins(0, 0, 0, 0);
    flagLayout->setSpacing(0);

    flagInput = new CTFFlagInput();
    flagInput->setPlaceholderText("CTF{...} (Нажмите Tab для автозаполнения)");
    flagInput->setFixedHeight(50);
    flagInput->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: %1;
            border: 1px solid %2;
            border-right: none;
            border-top-left-radius: 12px;
            border-bottom-left-radius: 12px;
            border-top-right-radius: 0px;
            border-bottom-right-radius: 0px;
            padding: 14px 20px;
            font-size: 14px;
            color: %3;
        }
        QLineEdit:focus {
            border-color: %4;
            background-color: rgba(59, 130, 246, 0.05);
        }
    )").arg(BG_CARD, BORDER_COLOR, TEXT_PRIMARY, ACCENT_BLUE));

    submitBtn = new QPushButton("Отправить →");
    submitBtn->setFixedHeight(50);
    submitBtn->setFixedWidth(140);
    submitBtn->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-top-left-radius: 0px;
            border-bottom-left-radius: 0px;
            border-top-right-radius: 12px;
            border-bottom-right-radius: 12px;
            font-weight: 600;
            font-size: 14px;
            padding: 14px 20px;
            margin: 0px;
        }
        QPushButton:hover {
            background-color: #2563eb;
        }
        QPushButton:pressed {
            background-color: #1d4ed8;
        }
    )").arg(ACCENT_BLUE));
    submitBtn->setCursor(Qt::PointingHandCursor);

    downloadBtn = new QPushButton("📥 Скачать файл");
    downloadBtn->setFixedHeight(50);
    downloadBtn->setFixedWidth(180);
    downloadBtn->setStyleSheet(QString(R"(
        QPushButton {
            background-color: #10b981;
            color: white;
            border: none;
            border-radius: 12px;
            font-weight: 600;
            font-size: 14px;
            padding: 14px 20px;
            margin-left: 10px;
        }
        QPushButton:hover {
            background-color: #059669;
        }
        QPushButton:pressed {
            background-color: #047857;
        }
    )"));
    downloadBtn->setCursor(Qt::PointingHandCursor);
    downloadBtn->hide();  // Скрыта по умолчанию

    flagLayout->addWidget(flagInput, 1);
    flagLayout->addWidget(submitBtn);
    flagLayout->addWidget(downloadBtn);

    // Enter для отправки
    connect(flagInput, &QLineEdit::returnPressed, this, &ChallengeManager::onSubmitFlag);

    // Сборка taskInfoCard
    taskInfoLayout->addWidget(taskInfoLabel);
    taskInfoLayout->addLayout(titleRow);
    taskInfoLayout->addWidget(taskDescriptionLabel);
    taskInfoLayout->addSpacing(10);
    taskInfoLayout->addWidget(codeSnippetEdit);
    taskInfoLayout->addWidget(copyableDataEdit);
    taskInfoLayout->addStretch();
    taskInfoLayout->addWidget(flagContainer);

    rightLayout->addWidget(taskInfoCard, 1);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(divider);
    mainLayout->addWidget(rightPanel, 1);

    connect(submitBtn, &QPushButton::clicked, this, &ChallengeManager::onSubmitFlag);
    connect(hintBtn, &QPushButton::clicked, this, &ChallengeManager::onHintClicked);
    connect(downloadBtn, &QPushButton::clicked, this, &ChallengeManager::onDownloadFile);

    return challengesPage;
}

void ChallengeManager::loadTasks(const QString &token) {
    // ОТМЕНЯЕМ ПРЕДЫДУЩИЙ ЗАПРОС
    if (currentTasksReply) {
        currentTasksReply->abort();
        currentTasksReply->deleteLater();
        currentTasksReply = nullptr;
    }
    
    authToken = token;
    QNetworkRequest request(QUrl(SERVER_URL + "/api/tasks"));
    request.setRawHeader("Authorization", authToken.toUtf8());
    
    currentTasksReply = manager->get(request);
    connect(currentTasksReply, &QNetworkReply::finished, this, [this]() {
        onTasksResponse(currentTasksReply);
        currentTasksReply = nullptr;
    });
}


void ChallengeManager::onTasksResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        tasksData = QJsonDocument::fromJson(reply->readAll()).array();
        refreshTaskList();
    }
    reply->deleteLater();
}

void ChallengeManager::refreshTaskList() {
    
    // Сбрасываем selectedCard перед удалением
    selectedCard = nullptr;
    
    // Очищаем список безопасно
    QLayoutItem *item;
    while ((item = taskListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    taskHints.clear();
    
    for (const auto &task : tasksData) {
        QJsonObject obj = task.toObject();
        QString title = obj["title"].toString();
        int points = obj["points"].toInt();
        int taskId = obj["id"].toInt();
        bool solved = obj["is_solved"].toBool();
        
        QString hint = obj["hint"].toString();
        if (!hint.isEmpty()) {
            taskHints[taskId] = hint;
        }
        
        QString difficulty = "Easy";
        if (points >= 100) difficulty = "Medium";
        if (points >= 200) difficulty = "Hard";
        
        int progress = solved ? 100 : 0;
        QWidget *card = createTaskCard(title, difficulty, progress, taskId, solved);
        card->setProperty("taskId", taskId);
        taskListLayout->addWidget(card);
    }
    
    qDebug() << "=== LOADING TASKS ===";
    for (const auto &task : tasksData) {
        QJsonObject obj = task.toObject();
        int taskId = obj["id"].toInt();
        QString hint = obj["hint"].toString();
        
        qDebug() << "Task" << taskId << "has hint:" << (!hint.isEmpty()) << "length:" << hint.length();
        
        if (!hint.isEmpty()) {
            taskHints[taskId] = hint;
        }
    }
    qDebug() << "Total hints loaded:" << taskHints.size();

    emit cardsReady();
}

QWidget* ChallengeManager::createTaskCard(const QString &title, const QString &difficulty, int progress, int taskId, bool solved) {
    QWidget *card = new QWidget();
    card->setObjectName("taskCard");
    card->setCursor(Qt::PointingHandCursor);
    card->setFixedHeight(110);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setStyleSheet(QString(R"(
QWidget#taskCard {
    background-color: %1;
    border: 1px solid %2;
    border-radius: 12px;
}
QWidget#taskCard:hover {
    border: 1px solid %3;
}
QWidget#taskCard * {
    background: transparent;
    border: none;
}
)").arg(BG_CARD, BORDER_COLOR, ACCENT_BLUE));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 12, 16, 12);
    cardLayout->setSpacing(6);

    // Заголовок и Progress
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setWordWrap(true);
    titleLabel->setMaximumHeight(40);
    titleLabel->setStyleSheet(QString("font-size: 15px; font-weight: 600; color: %1;").arg(TEXT_PRIMARY));

    QLabel *progressLabel = new QLabel("Прогресс");
    progressLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(TEXT_SECONDARY));

    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(progressLabel);

    // Difficulty и статус
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    
    QString difficultyRu = difficulty;
    if (difficulty == "Easy") difficultyRu = "Легко";
    else if (difficulty == "Medium") difficultyRu = "Средне";
    else if (difficulty == "Hard") difficultyRu = "Сложно";
    
    QLabel *diffLabel = new QLabel("Сложность: " + difficultyRu);
    diffLabel->setStyleSheet(QString("font-size: 12px; color: %1;").arg(TEXT_SECONDARY));

    QLabel *statusLabel = new QLabel(solved ? "✓" : "");
    statusLabel->setStyleSheet(QString("color: #22c55e; font-size: 16px;"));

    bottomLayout->addWidget(diffLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(statusLabel);

    // Progress bar - только визуал
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(solved ? 100 : (progress > 0 ? progress : 0));
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(6);
    progressBar->setStyleSheet(QString(R"(
QProgressBar {
    border: none;
    border-radius: 3px;
    background-color: %1;
}
QProgressBar::chunk {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %2, stop:1 %3);
    border-radius: 3px;
}
)").arg(BORDER_COLOR, ACCENT_BLUE, ACCENT_CYAN));

    cardLayout->addLayout(headerLayout);
    cardLayout->addLayout(bottomLayout);
    cardLayout->addWidget(progressBar);

    // Установка taskId
    card->setProperty("taskId", taskId);

    return card;
}

void ChallengeManager::onTaskSelected(int taskId) {
    selectedTaskId = taskId;
    if (taskId == 20 || taskId == 21) {
        downloadBtn->show();
    } else {
        downloadBtn->hide();
    }
    for (const auto &task : tasksData) {
        QJsonObject obj = task.toObject();
        if (obj["id"].toInt() == taskId) {
            taskTitleLabel->setText(obj["title"].toString());
            QString desc = obj["description"].toString();
            taskDescriptionLabel->setText(desc);

            // Извлекаем данные для копирования
            QString copyableData = extractCopyableData(desc);

            if (!copyableData.isEmpty()) {
                copyableDataEdit->setPlainText(copyableData);
                copyableDataEdit->show();
            } else {
                copyableDataEdit->hide();
            }

            // Показываем код если есть
            if (desc.contains("```")) {
                codeSnippetEdit->show();
                codeSnippetEdit->setText("// Analyze the challenge\n//Find the flag in format CTF{...}");
            } else {
                codeSnippetEdit->hide();
            }

            break;
        }
    }
}

void ChallengeManager::updateTaskDetails(int taskId) {
    onTaskSelected(taskId);
}

void ChallengeManager::onSubmitFlag() {
    if (selectedTaskId <= 0) {
        emit showErrorDialog("Ошибка", "Сначала выберите задание");
        return;
    }

    QString flag = flagInput->text().trimmed();
    if (flag.isEmpty()) {
        emit showErrorDialog("Ошибка", "Введите флаг");
        return;
    }

    qDebug() << "Submitting flag:" << flag << "for task:" << selectedTaskId;

    QJsonObject json;
    json["task_id"] = selectedTaskId;
    json["flag"] = flag;

    QNetworkRequest request(QUrl(SERVER_URL + "/api/submit"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", authToken.toUtf8());

    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSubmitResponse(reply);
    });
}

void ChallengeManager::onSubmitResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QJsonObject resp = QJsonDocument::fromJson(reply->readAll()).object();
        
        if (resp["result"].toString() == "correct") {
            int points = resp["points"].toInt();
            flagInput->clear();
            emit showSuccessDialog("Правильно!", QString("Вы получили %1 очков!").arg(points)); 
            emit taskSubmitSuccess(points);
            loadTasks(authToken);
        } else if (resp["result"].toString() == "already_solved") {
            emit showErrorDialog("Уже решено", "Вы уже решили это задание");
        } else {
            emit showErrorDialog("Неверный флаг", "Попробуйте снова!");
        }
    } else {
        emit showErrorDialog("Ошибка", reply->errorString());
    }
    reply->deleteLater();
}

void ChallengeManager::onHintClicked() {
    if (selectedTaskId <= 0) {
        HintDialog::showInfo("Подсказка", 
            "Сначала выберите задание из списка!", 
            challengesPage);
        return;
    }
    
    // Получаем подсказку для выбранного задания
    QString hint = taskHints.value(selectedTaskId, "");
    
    if (hint.isEmpty()) {
        HintDialog::showInfo("Подсказка", 
            "Для этого задания пока нет подсказок. Попробуйте решить самостоятельно!", 
            challengesPage);
    } else {
        // Находим название задания
        QString taskTitle = "";
        for (const auto &task : tasksData) {
            QJsonObject obj = task.toObject();
            if (obj["id"].toInt() == selectedTaskId) {
                taskTitle = obj["title"].toString();
                break;
            }
        }
        
        HintDialog::showInfo(
            QString("💡 Подсказка: %1").arg(taskTitle), 
            hint, 
            challengesPage
        );
    }
}

bool ChallengeManager::handleCardEvent(QObject *obj, QEvent *event) {
    // Клик на карточке задачи
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *card = qobject_cast<QWidget*>(obj);
        if (card && card->property("taskId").isValid()) {
            int taskId = card->property("taskId").toInt();

            // Сбрасываем стиль предыдущей выбранной карточки
            if (selectedCard && selectedCard != card) {
                selectedCard->setStyleSheet(QString(R"(
                    QWidget#taskCard {
                        background-color: %1;
                        border: 1px solid %2;
                        border-radius: 12px;
                    }
                    QWidget#taskCard:hover {
                        border: 1px solid %3;
                    }
                    QWidget#taskCard * {
                        background: transparent;
                        border: none;
                    }
                    )").arg(BG_CARD, BORDER_COLOR, ACCENT_BLUE));
            }

            // Устанавливаем стиль выбранной карточки
            card->setStyleSheet(QString(R"(
                QWidget#taskCard {
                    background-color: %1;
                    border: 2px solid %2;
                    border-radius: 12px;
                }
                QWidget#taskCard * {
                    background: transparent;
                    border: none;
                }
                )").arg(BG_CARD, ACCENT_BLUE));
            selectedCard = card;
            onTaskSelected(taskId);
            return true;
        }
    }
    return false;
}

void ChallengeManager::installCardEventFilters(QObject *filterObject) {
    QList<QWidget*> cards = taskListContainer->findChildren<QWidget*>("taskCard");
    for (QWidget *card : cards) {
        card->installEventFilter(filterObject);
    }
}

QString ChallengeManager::extractCopyableData(const QString &description) {
    QStringList lines = description.split('\n');
    QString result;
    bool nextLineIsCopyable = false;

    for (int i = 0; i < lines.size(); i++) {
        QString trimmed = lines[i].trimmed();

        // Пропускаем пустые строки
        if (trimmed.isEmpty()) {
            continue;
        }

        // === ПРИОРИТЕТ 1: URL АДРЕСА ===
        // Прямые URL (http, https, localhost)
        if (trimmed.startsWith("http://") || trimmed.startsWith("https://") ||
            trimmed.startsWith("localhost") || trimmed.startsWith("127.0.0.1")) {
            result += trimmed + "\n";
            continue;
        }

        // URL в середине строки
        QRegularExpression urlPattern(R"((https?://[^\s]+|localhost:[0-9]+[^\s]*|127\.0\.0\.1:[0-9]+[^\s]*))");
        QRegularExpressionMatch urlMatch = urlPattern.match(trimmed);
        if (urlMatch.hasMatch()) {
            result += urlMatch.captured(1) + "\n";
            continue;
        }

        // === ПРИОРИТЕТ 2: ДАННЫЕ ПОСЛЕ МЕТОК ===
        // Проверяем заголовки с двоеточием
        if (trimmed.contains("Ссылка:", Qt::CaseInsensitive) ||
            trimmed.contains("URL:", Qt::CaseInsensitive) ||
            trimmed.contains("Адрес:", Qt::CaseInsensitive) ||
            trimmed.contains("Payload:", Qt::CaseInsensitive) ||
            trimmed.contains("Ответ:", Qt::CaseInsensitive) ||
            trimmed.contains("Используйте:", Qt::CaseInsensitive)) {
            
            int colonPos = trimmed.indexOf(':');
            if (colonPos != -1 && colonPos < trimmed.length() - 1) {
                QString afterColon = trimmed.mid(colonPos + 1).trimmed();
                if (!afterColon.isEmpty() && !afterColon.endsWith(':')) {
                    result += afterColon + "\n";
                } else {
                    // Данные на следующей строке
                    nextLineIsCopyable = true;
                }
            }
            continue;
        }

        // Если предыдущая строка была заголовком
        if (nextLineIsCopyable && !trimmed.isEmpty()) {
            result += trimmed + "\n";
            nextLineIsCopyable = false;
            continue;
        }

        // Пропускаем заголовки, заканчивающиеся на ":"
        if (trimmed.endsWith(':') && trimmed.length() < 50) {
            nextLineIsCopyable = true;
            continue;
        }

        // === ПРИОРИТЕТ 3: КРИПТОГРАФИЧЕСКИЕ ДАННЫЕ ===
        // Hex данные (20+ символов)
        if (QRegularExpression("^[0-9a-fA-F]{20,}$").match(trimmed).hasMatch()) {
            result += trimmed + "\n";
            continue;
        }

        // Base64
        if (QRegularExpression("^[A-Za-z0-9+/]{16,}=*$").match(trimmed).hasMatch()) {
            result += trimmed + "\n";
            continue;
        }

        // Binary данные
        if (QRegularExpression(R"(^[01\s]{20,}$)").match(trimmed).hasMatch()) {
            result += trimmed + "\n";
            continue;
        }

        // Morse код
        if (QRegularExpression(R"(^[\-.\s]{10,}$)").match(trimmed).hasMatch()) {
            result += trimmed + "\n";
            continue;
        }

        // ASCII коды
        if (QRegularExpression(R"(^(\d+\s+){3,}\d+$)").match(trimmed).hasMatch()) {
            result += trimmed + "\n";
            continue;
        }

        // === ПРИОРИТЕТ 4: СПЕЦИАЛЬНЫЕ ФОРМАТЫ ===
        // JSON массив
        if (trimmed.startsWith("[") && trimmed.endsWith("]") && trimmed.contains(",")) {
            result += trimmed + "\n";
            continue;
        }

        // JWT токен (eyJ...)
        if (trimmed.startsWith("eyJ") && trimmed.contains(".")) {
            result += trimmed + "\n";
            continue;
        }

        // MD5/SHA хеши
        if (QRegularExpression("^[0-9a-f]{32}$").match(trimmed).hasMatch() ||
            QRegularExpression("^[0-9a-f]{40,}$").match(trimmed).hasMatch()) {
            result += trimmed + "\n";
            continue;
        }

        // === ПРИОРИТЕТ 5: SQL И ИНЪЕКЦИИ ===
        // SQL запросы
        if (QRegularExpression("SELECT.*FROM|WHERE.*=|INSERT INTO|UPDATE.*SET",
                              QRegularExpression::CaseInsensitiveOption).match(trimmed).hasMatch()) {
            result += trimmed + "\n";
            continue;
        }

        // SQL/LDAP инъекции с кавычками
        if ((trimmed.contains("'") || trimmed.contains("--") || trimmed.contains(")(")) &&
            trimmed.length() > 5 && trimmed.length() < 100) {
            result += trimmed + "\n";
            continue;
        }

        // Python/Command injection
        if (trimmed.contains("__import__") || trimmed.contains("os.system") ||
            trimmed.contains("eval(")) {
            result += trimmed + "\n";
            continue;
        }

        // === ПРИОРИТЕТ 6: XML/XXE ===
        if (trimmed.contains("<!ENTITY") || trimmed.contains("<!DOCTYPE") ||
            trimmed.contains("SYSTEM") || trimmed.contains("file://")) {
            result += trimmed + "\n";
            continue;
        }

        // Зашифрованный текст (только буквы и подчеркивания, 10+ символов)
        if (QRegularExpression("^[a-z_]{10,}$").match(trimmed).hasMatch()) {
            result += trimmed + "\n";
            continue;
        }
    }

    return result.trimmed();
}

void ChallengeManager::onDownloadFile() {
    if (selectedTaskId <= 0) {
        emit showErrorDialog("Ошибка", "Сначала выберите задание");
        return;
    }

    qDebug() << "Downloading file for task ID:" << selectedTaskId;

    QNetworkRequest request(QUrl(SERVER_URL + "/api/download/" + QString::number(selectedTaskId)));
    request.setRawHeader("Authorization", authToken.toUtf8());

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray fileData = reply->readAll();
            
            // Получаем имя файла из заголовка Content-Disposition
            QString filename = "downloaded_file";
            QString disposition = reply->rawHeader("Content-Disposition");
            if (disposition.contains("filename=")) {
                int start = disposition.indexOf("filename=") + 9;
                filename = disposition.mid(start).trimmed();
                filename.remove('"');
            }

            // Сохраняем файл
            QString savePath = QFileDialog::getSaveFileName(
                challengesPage,
                "Сохранить файл",
                QDir::homePath() + "/" + filename,
                "All Files (*.*)"
            );

            if (!savePath.isEmpty()) {
                QFile file(savePath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(fileData);
                    file.close();
                    emit showSuccessDialog("Успех", "Файл сохранён: " + savePath);
                } else {
                    emit showErrorDialog("Ошибка", "Не удалось сохранить файл");
                }
            }
        } else {
            QByteArray errorData = reply->readAll();
            QString errorMsg = QString::fromUtf8(errorData);
            qDebug() << "Download error:" << reply->error() << errorMsg;
            emit showErrorDialog("Ошибка", "Для этого задания нет файла");
        }
        reply->deleteLater();
    });
}

void ChallengeManager::onDownloadFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString filename = "taskfile.txt";
        QString disposition = reply->rawHeader("Content-Disposition");
        if (!disposition.isEmpty()) {
            int pos = disposition.indexOf("filename=");
            if (pos != -1) {
                filename = disposition.mid(pos + 9).trimmed();
                filename.remove('"');
            }
        }

        // ИСПРАВЛЕНИЕ: Проверка и создание папки загрузок
        QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        if (downloadsPath.isEmpty()) {
            downloadsPath = QDir::homePath() + "/Downloads";
        }
        
        QDir dir(downloadsPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QString filePath = downloadsPath + "/" + filename;
        QFile file(filePath);
        
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            emit showSuccessDialog("✅ Файл сохранен!", QString("📂 %1").arg(filePath));
            
            #ifdef Q_OS_WIN
            QProcess::startDetached("explorer", QStringList() << "/select," << QDir::toNativeSeparators(filePath));
            #elif defined(Q_OS_MAC)
            QProcess::execute("/usr/bin/osascript", QStringList() << "-e" << QString("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(filePath));
            QProcess::execute("/usr/bin/osascript", QStringList() << "-e" << "tell application \"Finder\" to activate");
            #else
            QDesktopServices::openUrl(QUrl::fromLocalFile(downloadsPath));
            #endif
        } else {
            emit showErrorDialog("❌ Ошибка сохранения", file.errorString());
        }
    } else {
        emit showErrorDialog("❌ Ошибка загрузки", reply->errorString());
    }
    reply->deleteLater();
}
