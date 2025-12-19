#ifndef CTFAPP_H
#define CTFAPP_H

#include <QWidget>
#include <QString>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QProgressBar>
#include <QPoint>
#include <QRect>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMouseEvent>
#include "ChallengeManager.h"

class CTFApp : public QWidget {
    Q_OBJECT

public:
    explicit CTFApp(QWidget *parent = nullptr);
    ~CTFApp();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    
private slots:
    void onLoginButtonClicked();
    void onRegisterButtonClicked();
    void onLogoutClicked();
    void showHomePage();
    void showChallengesPage();
    void showLeaderboardPage();
    void showProfilePage();
    void onChallengeSubmitSuccess(int points);

private:
    void loadActivityHistory();
    void onHistoryResponse(QNetworkReply *reply);
    void openUrl(const QString &url);

    void setupLoginPage();
    void setupDashboardPage();
    void setupSidebar();
    void setupHomePage();
    void setupLeaderboardPage();
    void setupProfilePage();

    void applyDarkTheme();

    void loadLeaderboard();
    void loadProfile();
    void updateNavButtons(QPushButton *active);

    void showError(const QString &title, const QString &message);
    void showSuccess(const QString &title, const QString &message);
    void onLeaderboardResponse(QNetworkReply *reply);
    void onProfileResponse(QNetworkReply *reply);

    void highlightTopRankings();

    QNetworkAccessManager *manager;

    QStackedWidget *mainStack;
    QStackedWidget *contentStack;

    QWidget *loginPage;
    QWidget *dashboardPage;
    QWidget *sidebar;
    QWidget *homePage;
    QWidget *leaderboardPage;
    QWidget *profilePage;

    QPushButton *homeBtn;
    QPushButton *challengesBtn;
    QPushButton *leaderboardBtn;
    QPushButton *profileBtn;

    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;

    QTableWidget *leaderboardTable;

    QLabel *dashScoreLabel;
    QLabel *dashSolvedLabel;
    QLabel *dashRankLabel;

    QLabel *profileNameLabel;
    QLabel *profileScoreLabel;
    QLabel *profileLevelLabel;
    QLabel *profileSolvedLabel;
    QProgressBar *profileProgressBar;
    QLabel *profileProgressPercentLabel;

    QString authToken;
    QString currentUsername;
    int currentScore;

    ChallengeManager *challengeManager;

    const QString SERVER_URL = "http://localhost:8080";
};

#endif
