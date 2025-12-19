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
        setFocusPolicy(Qt::StrongFocus);
    }
    
protected:
    bool event(QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Tab && !keyEvent->modifiers()) {
                QString currentText = text();
                
                if (currentText.isEmpty() || 
                    (currentText.startsWith("C") && currentText.length() < 5)) {
                    setText("CTF{}");
                    setCursorPosition(4);
                    return true;
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

    QWidget* setupChallengesPage();

    void loadTasks(const QString &authToken);

    void updateTaskDetails(int taskId);

    QWidget* getChallengesPage() const { return challengesPage; }

    void setAuthToken(const QString &token) { authToken = token; }

    bool handleCardEvent(QObject *obj, QEvent *event);

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

    QNetworkAccessManager *manager;
    QString SERVER_URL;
    QString authToken;

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

    QJsonArray tasksData;
    int selectedTaskId;
    QWidget *selectedCard;
    QNetworkReply *currentTasksReply = nullptr;
    QMap<int, QString> taskHints;
    QPushButton *downloadBtn;

    const QString BG_DARK = "#0a0e17";
    const QString BG_CARD = "#111827";
    const QString ACCENT_BLUE = "#3b82f6";
    const QString ACCENT_CYAN = "#22d3ee";
    const QString TEXT_PRIMARY = "#ffffff";
    const QString TEXT_SECONDARY = "#94a3b8";
    const QString BORDER_COLOR = "#1e293b";
};

#endif
