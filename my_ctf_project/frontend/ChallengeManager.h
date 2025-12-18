#ifndef CHALLENGEMANAGER_H
#define CHALLENGEMANAGER_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QKeyEvent>

class CTFFlagInput : public QLineEdit {
    Q_OBJECT
public:
    explicit CTFFlagInput(QWidget *parent = nullptr) : QLineEdit(parent) {
        // Разрешаем виджету получать события Tab
        setFocusPolicy(Qt::StrongFocus);
    }
    
protected:
    bool event(QEvent *event) override {
        // Перехватываем Tab ДО того, как его обработает система навигации
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Tab && !keyEvent->modifiers()) {
                QString currentText = text();
                
                // Если поле пустое или только начало CTF
                if (currentText.isEmpty() || 
                    (currentText.startsWith("C") && currentText.length() < 5)) {
                    setText("CTF{}");
                    // Устанавливаем курсор между скобками
                    setCursorPosition(4);
                    return true; // Событие обработано, не передаем дальше
                }
            }
        }
        
        return QLineEdit::event(event);
    }
};

class ChallengeManager : public QObject {
    Q_OBJECT

public:
    explicit ChallengeManager(QNetworkAccessManager *netManager, const QString &serverUrl, QObject *parent = nullptr);

    // Настройка UI страницы заданий
    QWidget* setupChallengesPage();

    // Загрузка заданий с сервера
    void loadTasks(const QString &authToken);

    // Обновление UI на основе данных задания
    void updateTaskDetails(int taskId);

    // Получение виджета страницы
    QWidget* getChallengesPage() const { return challengesPage; }

    // Установка токена авторизации
    void setAuthToken(const QString &token) { authToken = token; }

    // Обработка событий карточек задач
    bool handleCardEvent(QObject *obj, QEvent *event);

    // Установка event filter на карточки
    void installCardEventFilters(QObject *filterObject);

signals:
    void taskSubmitSuccess(int points);
    void taskSubmitError(const QString &title, const QString &message);
    void taskSubmitAlreadySolved(const QString &title, const QString &message);
    void showErrorDialog(const QString &title, const QString &message);
    void showSuccessDialog(const QString &title, const QString &message);
    void cardsReady();

private slots:
    void onTasksResponse(QNetworkReply *reply);
    void onSubmitResponse(QNetworkReply *reply);
    void onSubmitFlag();
    void onTaskSelected(int taskId);
    void onHintClicked();
    void onDownloadFile();
    void onDownloadFinished(QNetworkReply *reply);

private:
    QWidget* createTaskCard(const QString &title, const QString &difficulty, int progress, int taskId, bool solved);
    QTextEdit *copyableDataEdit;
    QString extractCopyableData(const QString &description);
    void refreshTaskList();

    // Сетевой менеджер
    QNetworkAccessManager *manager;
    QString SERVER_URL;
    QString authToken;

    // UI элементы
    QWidget *challengesPage;
    QVBoxLayout *taskListLayout;
    QWidget *taskListContainer;
    QScrollArea *taskScrollArea;
    QLabel *taskTitleLabel;
    QLabel *taskDescriptionLabel;
    QTextEdit *codeSnippetEdit;
    QLineEdit *flagInput;
    QPushButton *submitBtn;
    QPushButton *hintBtn;

    // Данные
    QJsonArray tasksData;
    int selectedTaskId;
    QWidget *selectedCard;
    QNetworkReply *currentTasksReply = nullptr;
    QMap<int, QString> taskHints;
    QPushButton *downloadBtn;

    // Константы цветов (должны совпадать с CTFApp)
    const QString BG_DARK = "#0a0e17";
    const QString BG_CARD = "#111827";
    const QString ACCENT_BLUE = "#3b82f6";
    const QString ACCENT_CYAN = "#22d3ee";
    const QString TEXT_PRIMARY = "#ffffff";
    const QString TEXT_SECONDARY = "#94a3b8";
    const QString BORDER_COLOR = "#1e293b";
};

#endif // CHALLENGEMANAGER_H
