#include "CTFApp.h"
#include "HintDialog.h"
#include "ChallengeManager.h"
#include <QProcess>
#include <QHeaderView>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QSpacerItem>
#include <QKeyEvent>
#include <QScreen>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QDir>
#include <QFile>

const QString BG_DARK = "#0a0e17";
const QString BG_CARD = "#111827";
const QString BG_SIDEBAR = "#0d1320";
const QString ACCENT_BLUE = "#3b82f6";
const QString ACCENT_CYAN = "#22d3ee";
const QString TEXT_PRIMARY = "#ffffff";
const QString TEXT_SECONDARY = "#94a3b8";
const QString BORDER_COLOR = "#1e293b";

CTFApp::CTFApp(QWidget *parent)
    : QWidget(parent), currentScore(0), 
      dashScoreLabel(nullptr), dashSolvedLabel(nullptr), dashRankLabel(nullptr) {
 
    setWindowFlags(Qt::Window);

    setMinimumSize(1000, 600);
    resize(1280, 800);
    
    manager = new QNetworkAccessManager(this);
    
    challengeManager = new ChallengeManager(manager, SERVER_URL, this);

    connect(challengeManager, &ChallengeManager::taskSubmitSuccess,
        this, &CTFApp::onChallengeSubmitSuccess);
    connect(challengeManager, &ChallengeManager::showErrorDialog, this, &CTFApp::showError);
    connect(challengeManager, &ChallengeManager::showSuccessDialog, this, &CTFApp::showSuccess);
    connect(challengeManager, &ChallengeManager::cardsReady, this, [this]() {
        challengeManager->installCardEventFilters(this);
    });

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    
    mainStack = new QStackedWidget(this);
    
    setupLoginPage();
    setupDashboardPage();
    applyDarkTheme();
    
    mainStack->addWidget(loginPage);
    mainStack->addWidget(dashboardPage);
    mainStack->setCurrentWidget(loginPage);
    
    rootLayout->addWidget(mainStack, 1);
    
    setWindowTitle("CTF Platform");
    resize(1200, 800);
}

void CTFApp::onChallengeSubmitSuccess(int points) {
    currentScore += points;
    loadProfile();
}

CTFApp::~CTFApp() = default;

void CTFApp::applyDarkTheme() {
    setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            color: %2;
            font-family: 'Segoe UI', 'SF Pro Display', Arial, sans-serif;
            font-size: 14px;
        }
        QLineEdit {
            padding: 12px 16px;
            border: 1px solid %3;
            border-radius: 8px;
            background-color: %4;
            color: %2;
            font-size: 14px;
        }
        QLineEdit:focus {
            border: 1px solid %5;
        }
        QLineEdit::placeholder {
            color: %6;
        }
        QPushButton {
            padding: 12px 24px;
            border: none;
            border-radius: 8px;
            font-weight: 600;
            font-size: 14px;
        }
        QPushButton#primaryBtn {
            background-color: %5;
            color: white;
        }
        QPushButton#primaryBtn:hover {
            background-color: #2563eb;
        }
        QPushButton#secondaryBtn {
            background-color: transparent;
            border: 1px solid %3;
            color: %6;
        }
        QPushButton#secondaryBtn:hover {
            background-color: %4;
        }
        QTableWidget {
            background-color: %4;
            border: 1px solid %3;
            border-radius: 8px;
            gridline-color: %3;
        }
        QTableWidget::item {
            padding: 12px;
            border-bottom: 1px solid %3;
        }
        QHeaderView::section {
            background-color: %1;
            color: %6;
            padding: 12px;
            border: none;
            border-bottom: 1px solid %3;
            font-weight: 600;
        }
        QScrollArea {
            border: none;
            background-color: transparent;
        }
        /* Глобальный скроллбар */
        QScrollBar:vertical {
            border: none;
            background: #0F172A;
            width: 8px;
            margin: 0px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: #334155;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %5;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            border: none;
            background: none;
            height: 0px;
        }
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
        /* Горизонтальный скроллбар */
        QScrollBar:horizontal {
            border: none;
            background: #0F172A;
            height: 8px;
            margin: 0px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal {
            background-color: #334155;
            border-radius: 4px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: %5;
        }
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            border: none;
            background: none;
            width: 0px;
        }
        QScrollBar::add-page:horizontal,
        QScrollBar::sub-page:horizontal {
            background: none;
        }
        QTextEdit {
            background-color: #1a1f2e;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 16px;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 13px;
            color: %7;
        }
        QProgressBar {
            border: none;
            border-radius: 4px;
            background-color: %3;
            height: 6px;
            text-align: center;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %5, stop:1 %7);
            border-radius: 4px;
        }
    )").arg(BG_DARK, TEXT_PRIMARY, BORDER_COLOR, BG_CARD, ACCENT_BLUE, TEXT_SECONDARY, ACCENT_CYAN));
}

void CTFApp::setupLoginPage() {
    loginPage = new QWidget();
    loginPage->setStyleSheet(QString("background-color: %1;").arg(BG_DARK));
    
    QVBoxLayout *mainLayout = new QVBoxLayout(loginPage);
    mainLayout->setAlignment(Qt::AlignCenter);

    QWidget *formContainer = new QWidget();
    formContainer->setFixedWidth(400);
    formContainer->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border-radius: 16px;
            border: 1px solid %2;
        }
    )").arg(BG_CARD, BORDER_COLOR));
    
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(40, 40, 40, 40);
    formLayout->setSpacing(20);

    QLabel *logoLabel = new QLabel("🎯 CTF");
    logoLabel->setStyleSheet(QString(R"(
        font-size: 48px;
        font-weight: bold;
        color: %1;
        background: transparent;
        border: none;
    )").arg(ACCENT_BLUE));
    logoLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *subtitleLabel = new QLabel("Capture The Flag Platform");
    subtitleLabel->setStyleSheet(QString("color: %1; font-size: 14px; background: transparent; border: none;").arg(TEXT_SECONDARY));
    subtitleLabel->setAlignment(Qt::AlignCenter);
    QString inputStyle = QString(R"(
        QLineEdit {
            padding: 12px 16px;
            border: 2px solid %1;
            border-radius: 8px;
            background-color: %2;
            color: %3;
            font-size: 14px;
        }
        QLineEdit:hover {
            border: 2px solid rgba(59, 130, 246, 0.5);
            background-color: rgba(59, 130, 246, 0.05);
        }
        QLineEdit:focus {
            border: 2px solid %4;
            background-color: rgba(59, 130, 246, 0.1);
        }
    )").arg(BORDER_COLOR, BG_CARD, TEXT_PRIMARY, ACCENT_BLUE);

    usernameLineEdit = new QLineEdit();
    usernameLineEdit->setPlaceholderText("Username");
    usernameLineEdit->setMinimumHeight(48);
    usernameLineEdit->setStyleSheet(inputStyle);
    
    passwordLineEdit = new QLineEdit();
    passwordLineEdit->setPlaceholderText("Password");
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    passwordLineEdit->setMinimumHeight(48);
    passwordLineEdit->setStyleSheet(inputStyle);

    QString primaryBtnStyle = QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 8px;
            font-weight: 600;
            font-size: 14px;
            padding: 12px 24px;
        }
        QPushButton:hover {
            background-color: #2563eb;
        }
        QPushButton:pressed {
            background-color: #1d4ed8;
        }
    )").arg(ACCENT_BLUE);
    
    QString secondaryBtnStyle = QString(R"(
        QPushButton {
            background-color: transparent;
            border: 2px solid %1;
            border-radius: 8px;
            color: %2;
            font-weight: 600;
            font-size: 14px;
            padding: 12px 24px;
        }
        QPushButton:hover {
            background-color: rgba(59, 130, 246, 0.1);
            border: 2px solid %3;
            color: %4;
        }
        QPushButton:pressed {
            background-color: rgba(59, 130, 246, 0.2);
            border: 2px solid %3;
        }
    )").arg(BORDER_COLOR, TEXT_SECONDARY, ACCENT_BLUE, TEXT_PRIMARY);

    QPushButton *loginButton = new QPushButton("Sign In");
    loginButton->setMinimumHeight(48);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setStyleSheet(primaryBtnStyle);
    
    QPushButton *registerButton = new QPushButton("Create Account");
    registerButton->setMinimumHeight(48);
    registerButton->setCursor(Qt::PointingHandCursor);
    registerButton->setStyleSheet(secondaryBtnStyle);
    
    formLayout->addWidget(logoLabel);
    formLayout->addWidget(subtitleLabel);
    formLayout->addSpacing(20);
    formLayout->addWidget(usernameLineEdit);
    formLayout->addWidget(passwordLineEdit);
    formLayout->addSpacing(10);
    formLayout->addWidget(loginButton);
    formLayout->addWidget(registerButton);
    
    mainLayout->addWidget(formContainer);

    connect(usernameLineEdit, &QLineEdit::returnPressed, this, &CTFApp::onLoginButtonClicked);
    connect(passwordLineEdit, &QLineEdit::returnPressed, this, &CTFApp::onLoginButtonClicked);
    connect(loginButton, &QPushButton::clicked, this, &CTFApp::onLoginButtonClicked);
    connect(registerButton, &QPushButton::clicked, this, &CTFApp::onRegisterButtonClicked);
}

void CTFApp::setupDashboardPage() {
    dashboardPage = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(dashboardPage);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    setupSidebar();
    
    contentStack = new QStackedWidget();
    
    setupHomePage();
    setupLeaderboardPage();
    setupProfilePage();
    QWidget *challengesPage = challengeManager->setupChallengesPage();

    contentStack->addWidget(homePage);
    contentStack->addWidget(challengesPage);
    contentStack->addWidget(leaderboardPage);
    contentStack->addWidget(profilePage);
    
    layout->addWidget(sidebar);
    layout->addWidget(contentStack, 1);
    
    contentStack->setCurrentWidget(challengesPage);
}

void CTFApp::setupSidebar() {
    sidebar = new QWidget();
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet(QString(R"(
        QWidget#sidebar {
            background-color: %1;
            border-right: 1px solid %2;
        }
    )").arg(BG_SIDEBAR, BORDER_COLOR));
    sidebar->setObjectName("sidebar");
    
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);
    
    QWidget *logoSection = new QWidget();
    logoSection->setFixedHeight(64);
    logoSection->setStyleSheet(QString("background-color: transparent; border-bottom: 1px solid %1;").arg(BORDER_COLOR));
    
    QHBoxLayout *logoLayout = new QHBoxLayout(logoSection);
    logoLayout->setContentsMargins(16, 0, 16, 0);
    
    QLabel *logoIcon = new QLabel(QString::fromUtf8("🚩"));
    logoIcon->setStyleSheet("font-size: 22px; background: transparent;");
    
    QLabel *logoText = new QLabel("CTF Platform");
    logoText->setStyleSheet(QString(R"(
        font-size: 16px;
        font-weight: 700;
        color: %1;
        background: transparent;
        letter-spacing: 0.5px;
    )").arg(TEXT_PRIMARY));
    
    logoLayout->addWidget(logoIcon);
    logoLayout->addSpacing(10);
    logoLayout->addWidget(logoText);
    logoLayout->addStretch();
    
    QWidget *navSection = new QWidget();
    navSection->setStyleSheet("background: transparent;");
    QVBoxLayout *navLayout = new QVBoxLayout(navSection);
    navLayout->setContentsMargins(8, 16, 8, 16);
    navLayout->setSpacing(4);

    QString navBtnStyle = QString(R"(
        QPushButton {
            background-color: transparent;
            border: none;
            border-left: 3px solid transparent;
            border-radius: 0px;
            padding: 14px 16px;
            color: %1;
            font-size: 14px;
            font-weight: 500;
            text-align: left;
        }
        QPushButton:hover:!checked {
            background-color: #1E293B;
            color: %2;
            border: none;
            border-left: 3px solid transparent;
        }
        QPushButton:checked {
            background-color: #1E293B;
            color: %2;
            border: none;
            border-left: 3px solid %3;
        }
    )").arg(TEXT_SECONDARY, TEXT_PRIMARY, ACCENT_BLUE);
    
    homeBtn = new QPushButton(QString::fromUtf8("🏠 Главная"));
    homeBtn->setCheckable(true);
    homeBtn->setStyleSheet(navBtnStyle);
    homeBtn->setFixedHeight(48);
    homeBtn->setCursor(Qt::PointingHandCursor);

    challengesBtn = new QPushButton(QString::fromUtf8("🎯 Задания"));
    challengesBtn->setCheckable(true);
    challengesBtn->setChecked(true);
    challengesBtn->setStyleSheet(navBtnStyle);
    challengesBtn->setFixedHeight(48);
    challengesBtn->setCursor(Qt::PointingHandCursor);

    leaderboardBtn = new QPushButton(QString::fromUtf8("🏆 Рейтинг"));
    leaderboardBtn->setCheckable(true);
    leaderboardBtn->setStyleSheet(navBtnStyle);
    leaderboardBtn->setFixedHeight(48);
    leaderboardBtn->setCursor(Qt::PointingHandCursor);

    profileBtn = new QPushButton(QString::fromUtf8("👤 Профиль"));
    profileBtn->setCheckable(true);
    profileBtn->setStyleSheet(navBtnStyle);
    profileBtn->setFixedHeight(48);
    profileBtn->setCursor(Qt::PointingHandCursor);
    
    QLabel *sectionLabel = new QLabel("НАВИГАЦИЯ");
    sectionLabel->setStyleSheet(QString(R"(
        color: %1;
        font-size: 10px;
        font-weight: 600;
        letter-spacing: 1px;
        padding: 8px 16px;
        background: transparent;
    )").arg(TEXT_SECONDARY));
    
    navLayout->addWidget(sectionLabel);
    navLayout->addWidget(homeBtn);
    navLayout->addWidget(challengesBtn);
    navLayout->addWidget(leaderboardBtn);
    navLayout->addWidget(profileBtn);
    navLayout->addStretch();
    
    QWidget *bottomSection = new QWidget();
    bottomSection->setStyleSheet(QString("background: transparent; border-top: 1px solid %1;").arg(BORDER_COLOR));
    bottomSection->setFixedHeight(50);
    
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomSection);
    bottomLayout->setContentsMargins(16, 0, 16, 0);
    
    QLabel *versionLabel = new QLabel("v1.0.0");
    versionLabel->setStyleSheet(QString("color: %1; font-size: 11px; background: transparent;").arg(TEXT_SECONDARY));
    bottomLayout->addWidget(versionLabel);
    bottomLayout->addStretch();
    
    sidebarLayout->addWidget(logoSection);
    sidebarLayout->addWidget(navSection, 1);
    sidebarLayout->addWidget(bottomSection);
    
    connect(homeBtn, &QPushButton::clicked, this, &CTFApp::showHomePage);
    connect(challengesBtn, &QPushButton::clicked, this, &CTFApp::showChallengesPage);
    connect(leaderboardBtn, &QPushButton::clicked, this, &CTFApp::showLeaderboardPage);
    connect(profileBtn, &QPushButton::clicked, this, &CTFApp::showProfilePage);
}

void CTFApp::setupHomePage() {
    homePage = new QWidget();
    homePage->setStyleSheet(QString("background-color: %1;").arg(BG_DARK));
    
    QVBoxLayout *layout = new QVBoxLayout(homePage);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(24);

    QLabel *welcomeLabel = new QLabel("Главный экран");
    welcomeLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(TEXT_PRIMARY));
    layout->addWidget(welcomeLabel);

    QHBoxLayout *statsRow = new QHBoxLayout();
    statsRow->setSpacing(20);

    auto createStatCard = [this](const QString &icon, const QString &value, const QString &label, const QString &accentColor, QLabel **outValueLabel) -> QFrame* {
        QFrame *card = new QFrame();
        card->setObjectName("statCard");
        card->setStyleSheet(QString(R"(
            QFrame#statCard {
                background-color: %1;
                border: 1px solid %2;
                border-radius: 16px;
                padding: 20px;
            }
            QFrame#statCard:hover {
                border-color: %3;
            }
        )").arg(BG_CARD, BORDER_COLOR, accentColor));
        card->setMinimumHeight(120);
        
        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(20, 16, 20, 16);
        cardLayout->setSpacing(16);

        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet(QString("font-size: 36px; background: transparent; color: %1;").arg(accentColor));
        iconLabel->setFixedSize(56, 56);
        iconLabel->setAlignment(Qt::AlignCenter);
        
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(4);
        
        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(QString("font-size: 32px; font-weight: bold; color: %1; background: transparent;").arg(accentColor));
        *outValueLabel = valueLabel;
        
        QLabel *labelText = new QLabel(label);
        labelText->setStyleSheet(QString("font-size: 13px; color: %1; background: transparent;").arg(TEXT_SECONDARY));
        
        textLayout->addWidget(valueLabel);
        textLayout->addWidget(labelText);
        
        cardLayout->addWidget(iconLabel);
        cardLayout->addLayout(textLayout, 1);
        
        return card;
    };
    
    QFrame *scoreCard = createStatCard(QString::fromUtf8("⭐"), "0", "Всего очков", ACCENT_BLUE, &dashScoreLabel);        // Синий
    QFrame *solvedCard = createStatCard(QString::fromUtf8("✅"), "0", "Решено заданий", "#22c55e", &dashSolvedLabel);   // Зеленый
    QFrame *rankCard = createStatCard(QString::fromUtf8("🏆"), "--", "Рейтинг", "#f59e0b", &dashRankLabel);           // Оранжевый
    
    statsRow->addWidget(scoreCard);
    statsRow->addWidget(solvedCard);
    statsRow->addWidget(rankCard);
    
    layout->addLayout(statsRow);
    
    QHBoxLayout *mainArea = new QHBoxLayout();
    mainArea->setSpacing(24);
    
    QFrame *recommendedCard = new QFrame();
    recommendedCard->setObjectName("recommendedCard");
    recommendedCard->setStyleSheet(QString(R"(
        QFrame#recommendedCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 16px;
        }
        QFrame#recommendedCard:hover {
            border-color: %3;
        }
    )").arg(BG_CARD, BORDER_COLOR, ACCENT_BLUE));
    
    QVBoxLayout *recLayout = new QVBoxLayout(recommendedCard);
    recLayout->setContentsMargins(24, 24, 24, 24);
    recLayout->setSpacing(16);
    
    QLabel *recHeader = new QLabel(QString::fromUtf8("🎯 Рекомендуем"));
    recHeader->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1; background: transparent;").arg(TEXT_SECONDARY));
    
    QLabel *recTitle = new QLabel("Начни изучать основы CTF!");
    recTitle->setStyleSheet(QString("font-size: 22px; font-weight: bold; color: %1; background: transparent;").arg(TEXT_PRIMARY));
    
    QLabel *recDesc = new QLabel("Открой для себя мир Capture The Flag. Решай задачи, изучай новые техники и прокачивай свои навыки в информационной безопасности.");
    recDesc->setStyleSheet(QString("font-size: 14px; color: %1; line-height: 1.5; background: transparent;").arg(TEXT_SECONDARY));
    recDesc->setWordWrap(true);
    
    QHBoxLayout *badgeRow = new QHBoxLayout();
    
    QLabel *diffBadge = new QLabel("Начальный этап");
    diffBadge->setStyleSheet(QString(R"(
        background-color: rgba(34, 197, 94, 0.2);
        color: #22c55e;
        padding: 6px 14px;
        border-radius: 12px;
        font-size: 12px;
        font-weight: 600;
    )"));
    diffBadge->setMinimumWidth(120);
    diffBadge->setAlignment(Qt::AlignCenter);
    
    QLabel *startBadge = new QLabel("🚀 Старт");
    startBadge->setStyleSheet(QString(R"(
        background-color: rgba(245, 158, 11, 0.2);
        color: #f59e0b;
        padding: 6px 14px;
        border-radius: 12px;
        font-size: 12px;
        font-weight: 600;
    )"));
    startBadge->setMinimumWidth(80);
    startBadge->setAlignment(Qt::AlignCenter);
    
    badgeRow->addWidget(diffBadge);
    badgeRow->addSpacing(12);
    badgeRow->addWidget(startBadge);
    badgeRow->addStretch();
    
    QPushButton *startBtn = new QPushButton("Начать");
    startBtn->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 14px 28px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #2563eb;
        }
        QPushButton:pressed {
            background-color: #1d4ed8;
        }
    )").arg(ACCENT_BLUE));
    startBtn->setCursor(Qt::PointingHandCursor);
    startBtn->setFixedWidth(120);
    
    connect(startBtn, &QPushButton::clicked, this, &CTFApp::showChallengesPage);
    
    recLayout->addWidget(recHeader);
    recLayout->addWidget(recTitle);
    recLayout->addWidget(recDesc);
    recLayout->addSpacing(8);
    recLayout->addLayout(badgeRow);
    recLayout->addStretch();
    recLayout->addWidget(startBtn);
    
    QFrame *toolsCard = new QFrame();
    toolsCard->setObjectName("toolsCard");
    toolsCard->setStyleSheet(QString(R"(
        QFrame#toolsCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 16px;
        }
    )").arg(BG_CARD, BORDER_COLOR));
    
    QVBoxLayout *toolsLayout = new QVBoxLayout(toolsCard);
    toolsLayout->setContentsMargins(24, 24, 24, 24);
    toolsLayout->setSpacing(16);
    
    QLabel *toolsHeader = new QLabel(QString::fromUtf8("🛠️  Полезные инструменты"));
    toolsHeader->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1; background: transparent;").arg(TEXT_SECONDARY));
    
    QGridLayout *toolsGrid = new QGridLayout();
    toolsGrid->setSpacing(12);
    
    auto createToolBtn = [this](const QString &icon, const QString &name, const QString &url) -> QPushButton* {
        QPushButton *btn = new QPushButton(icon + "  " + name);
        btn->setStyleSheet(QString(R"(
            QPushButton {
                background-color: %1;
                border: 1px solid %2;
                border-radius: 12px;
                padding: 16px;
                color: %3;
                font-size: 13px;
                font-weight: 500;
                text-align: left;
            }
            QPushButton:hover {
                background-color: #1E293B;
                border-color: %4;
                color: %5;
            }
        )").arg(BG_DARK, BORDER_COLOR, TEXT_SECONDARY, ACCENT_BLUE, TEXT_PRIMARY));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(56);
        btn->setProperty("url", url);
        return btn;
    };
    
    QPushButton *cyberChefBtn = createToolBtn(QString::fromUtf8("🍳"), "CyberChef", "https://gchq.github.io/CyberChef/");
    QPushButton *revShellBtn = createToolBtn(QString::fromUtf8("🐚"), "RevShells", "https://www.revshells.com/");
    QPushButton *gtfobinsBtn = createToolBtn(QString::fromUtf8("🐧"), "GTFOBins", "https://gtfobins.github.io/");
    QPushButton *crackStationBtn = createToolBtn(QString::fromUtf8("🔓"), "CrackStation", "https://crackstation.net/");
    QPushButton *dcodeBtn = createToolBtn(QString::fromUtf8("🔐"), "dCode", "https://www.dcode.fr/en");
    QPushButton *webhookBtn = createToolBtn(QString::fromUtf8("🌐"), "Webhook.site", "https://webhook.site/");
    
    auto openUrl = [](const QString &url) {
    	QDesktopServices::openUrl(QUrl(url));
	};
    
	connect(cyberChefBtn, &QPushButton::clicked, []() { 
	    QDesktopServices::openUrl(QUrl("https://gchq.github.io/CyberChef/")); 
	});
	connect(revShellBtn, &QPushButton::clicked, []() { 
	    QDesktopServices::openUrl(QUrl("https://www.revshells.com/")); 
	});
	connect(gtfobinsBtn, &QPushButton::clicked, []() { 
	    QDesktopServices::openUrl(QUrl("https://gtfobins.github.io/")); 
	});
	connect(crackStationBtn, &QPushButton::clicked, []() { 
	    QDesktopServices::openUrl(QUrl("https://crackstation.net/")); 
	});
	connect(dcodeBtn, &QPushButton::clicked, []() { 
	    QDesktopServices::openUrl(QUrl("https://www.dcode.fr/en")); 
	});
	connect(webhookBtn, &QPushButton::clicked, []() { 
	    QDesktopServices::openUrl(QUrl("https://webhook.site/")); 
	});
    
    toolsGrid->addWidget(cyberChefBtn, 0, 0);
    toolsGrid->addWidget(revShellBtn, 0, 1);
    toolsGrid->addWidget(gtfobinsBtn, 1, 0);
    toolsGrid->addWidget(crackStationBtn, 1, 1);
    toolsGrid->addWidget(dcodeBtn, 2, 0);
    toolsGrid->addWidget(webhookBtn, 2, 1);
    
    toolsLayout->addWidget(toolsHeader);
    toolsLayout->addLayout(toolsGrid);
    toolsLayout->addStretch();
    
    mainArea->addWidget(recommendedCard, 3);
    mainArea->addWidget(toolsCard, 2);
    
    layout->addLayout(mainArea, 1);
}

void CTFApp::setupLeaderboardPage() {
    leaderboardPage = new QWidget();
    leaderboardPage->setStyleSheet(QString("background-color: %1;").arg(BG_DARK));
    
    QVBoxLayout *layout = new QVBoxLayout(leaderboardPage);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(20);
    
    QLabel *titleLabel = new QLabel("Таблица лидеров");
    titleLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(TEXT_PRIMARY));
    layout->addWidget(titleLabel);
    
    QWidget *tableContainer = new QWidget();
    tableContainer->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 16px;
        }
    )").arg(BG_CARD, BORDER_COLOR));
    
    QVBoxLayout *containerLayout = new QVBoxLayout(tableContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    
    QWidget *tableHeader = new QWidget();
    tableHeader->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *headerLayout = new QHBoxLayout(tableHeader);
    headerLayout->setContentsMargins(24, 16, 24, 16);
    
    QLabel *headerTitle = new QLabel(QString::fromUtf8("🏆 Топ игроков"));
    headerTitle->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1; background: transparent;").arg(TEXT_PRIMARY));
    
    headerLayout->addWidget(headerTitle);
    headerLayout->addStretch();
    
    leaderboardTable = new QTableWidget();
    leaderboardTable->setColumnCount(4);
    leaderboardTable->setHorizontalHeaderLabels({"Ранг", "Игрок", "Очки", "Решено"});
    leaderboardTable->horizontalHeader()->setStretchLastSection(true);
    leaderboardTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    leaderboardTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    leaderboardTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    leaderboardTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    leaderboardTable->setColumnWidth(0, 120);
    leaderboardTable->setColumnWidth(2, 150);
    leaderboardTable->verticalHeader()->setVisible(false);
    leaderboardTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    leaderboardTable->setSelectionMode(QAbstractItemView::SingleSelection);
    leaderboardTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leaderboardTable->setFocusPolicy(Qt::NoFocus);
    leaderboardTable->setShowGrid(false);
    
    leaderboardTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    leaderboardTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leaderboardTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

    leaderboardTable->setStyleSheet(QString(R"(
        QTableWidget {
            background-color: transparent;
            border: none;
            border-top: 1px solid %1;
            border-radius: 0px;
            gridline-color: transparent;
            outline: none;
        }
        
        QHeaderView::section {
            background-color: %2;
            color: %3;
            padding: 12px 16px;
            border: none;
            border-bottom: 1px solid %1;
            font-weight: 600;
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        QTableWidget::item {
            padding: 10px 16px;
            border: none;
            border-bottom: 1px solid %1;
            font-size: 13px;
        }
        
        QTableWidget::item:selected {
            background-color: rgba(59, 130, 246, 0.1);
        }
        
        QTableWidget::item:alternate {
            background-color: rgba(255, 255, 255, 0.02);
        }
        
        QTableWidget::item:hover {
            background-color: rgba(59, 130, 246, 0.05);
        }
    )").arg(BORDER_COLOR, BG_CARD, TEXT_SECONDARY));
    
    leaderboardTable->setAlternatingRowColors(true);
    
    containerLayout->addWidget(tableHeader);
    containerLayout->addWidget(leaderboardTable);
    
    layout->addWidget(tableContainer, 1);
}

void CTFApp::setupProfilePage() {
    profilePage = new QWidget();
    profilePage->setStyleSheet(QString("background-color: %1;").arg(BG_DARK));
    
    QVBoxLayout *layout = new QVBoxLayout(profilePage);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(24);
    
    QFrame *profileCard = new QFrame();
    profileCard->setObjectName("profileCard");
    profileCard->setStyleSheet(QString(R"(
        QFrame#profileCard {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 20px;
        }
    )").arg(BG_CARD, BORDER_COLOR));
    
    QVBoxLayout *cardLayout = new QVBoxLayout(profileCard);
    cardLayout->setContentsMargins(32, 32, 32, 32);
    cardLayout->setSpacing(24);
    
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(20);
    
    QLabel *avatar = new QLabel(QString::fromUtf8("👤"));
    avatar->setFixedSize(80, 80);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(QString(R"(
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2);
        border-radius: 40px;
        font-size: 36px;
    )").arg(ACCENT_BLUE, ACCENT_CYAN));
    
    QVBoxLayout *nameLayout = new QVBoxLayout();
    nameLayout->setSpacing(4);
    
    profileNameLabel = new QLabel("Username");
    profileNameLabel->setStyleSheet(QString(R"(
        font-size: 26px;
        font-weight: bold;
        color: %1;
        background: transparent;
    )").arg(TEXT_PRIMARY));
    
    QLabel *titleLabel = new QLabel(QString::fromUtf8("🎮 CTF Player"));
    titleLabel->setStyleSheet(QString("font-size: 14px; color: %1; background: transparent;").arg(TEXT_SECONDARY));
    
    nameLayout->addWidget(profileNameLabel);
    nameLayout->addWidget(titleLabel);
    
    QPushButton *logoutBtn = new QPushButton(QString::fromUtf8("🚪 Выйти"));
    logoutBtn->setStyleSheet(QString(R"(
        QPushButton {
            background-color: rgba(239, 68, 68, 0.15);
            color: #ef4444;
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 10px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: rgba(239, 68, 68, 0.25);
            border-color: #ef4444;
        }
    )"));
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setFixedHeight(40);
    connect(logoutBtn, &QPushButton::clicked, this, &CTFApp::onLogoutClicked);
    QPushButton *reportBtn = new QPushButton(QString::fromUtf8("📄 Отчет"));
    reportBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(59, 130, 246, 0.15);
            color: #3b82f6;
            border: 1px solid rgba(59, 130, 246, 0.3);
            border-radius: 10px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: rgba(59, 130, 246, 0.25);
            border-color: #3b82f6;
        }
    )");
    reportBtn->setCursor(Qt::PointingHandCursor);
    reportBtn->setFixedHeight(40);

    connect(reportBtn, &QPushButton::clicked, this, [this]() {
        QNetworkRequest request(QUrl(SERVER_URL + "/api/report"));
        request.setRawHeader("Authorization", authToken.toUtf8());
        
        QNetworkReply *reply = manager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QString savePath = QFileDialog::getSaveFileName(this, "Сохранить отчет", QDir::homePath() + "/ctf_report.csv", "CSV Files (*.csv)");
                if (!savePath.isEmpty()) {
                    QFile file(savePath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(reply->readAll());
                        file.close();
                        showSuccess("Успех", "Отчет сохранен!");
                    }
                }
            } else {
                showError("Ошибка", "Не удалось скачать отчет");
            }
            reply->deleteLater();
        });
    });

    headerLayout->addWidget(avatar);
    headerLayout->addLayout(nameLayout, 1);
    headerLayout->addWidget(logoutBtn, 0, Qt::AlignTop);
    headerLayout->addWidget(reportBtn);
    
    QHBoxLayout *statsRow = new QHBoxLayout();
    statsRow->setSpacing(16);
    auto createStatCard = [this](const QString &icon, const QString &value, 
    const QString &label, QLabel **outValueLabel) -> QFrame* {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString(R"(
            QFrame {
                background-color: %1;
                border: 1px solid %2;
                border-radius: 14px;
                padding: 16px;
            }
        )").arg(BG_DARK, BORDER_COLOR));
        card->setMinimumHeight(100);
        
        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setContentsMargins(16, 12, 16, 12);
        layout->setSpacing(8);
        layout->setAlignment(Qt::AlignCenter);
        
        int fixedWidth = 120;

        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet(QString("font-size: 24px; background-color: %1; border-radius: 10px; padding: 8px;").arg(BG_CARD));
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setFixedSize(fixedWidth, 48);
        
        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1; background-color: %2; border-radius: 10px; padding: 8px;").arg(TEXT_PRIMARY, BG_CARD));
        valueLabel->setAlignment(Qt::AlignCenter);
        valueLabel->setFixedSize(fixedWidth, 50);
        *outValueLabel = valueLabel;

        QLabel *labelText = new QLabel(label);
        labelText->setStyleSheet(QString("font-size: 12px; color: %1; background-color: %2; border-radius: 10px; padding: 6px;").arg(TEXT_SECONDARY, BG_CARD));
        labelText->setAlignment(Qt::AlignCenter);
        labelText->setFixedSize(fixedWidth, 30);
        
        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(labelText);
        
        return card;
    };
    
    QFrame *scoreCard = createStatCard(QString::fromUtf8("⭐"), "0", "ОЧКИ", &profileScoreLabel);
    QFrame *rankCard = createStatCard(QString::fromUtf8("🏆"), "--", "РАНГ", &profileLevelLabel);
    QFrame *solvedCard = createStatCard(QString::fromUtf8("✅"), "0", "РЕШЕНО", &profileSolvedLabel);
    
    statsRow->addWidget(scoreCard);
    statsRow->addWidget(rankCard);
    statsRow->addWidget(solvedCard);
    
    QFrame *progressSection = new QFrame();
    progressSection->setStyleSheet(QString(R"(
        QFrame {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 14px;
        }
    )").arg(BG_DARK, BORDER_COLOR));

    QVBoxLayout *progressLayout = new QVBoxLayout(progressSection);
    progressLayout->setContentsMargins(20, 16, 20, 16);
    progressLayout->setSpacing(12);

    QHBoxLayout *progressHeader = new QHBoxLayout();

    QLabel *xpLabel = new QLabel(QString::fromUtf8("⚡ Прогресс выполнения"));
    xpLabel->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1; background: transparent; border: none;").arg(TEXT_PRIMARY));

    QLabel *xpValueLabel = new QLabel("0% (0/0)");
    xpValueLabel->setObjectName("xpValueLabel");
    xpValueLabel->setStyleSheet(QString("font-size: 13px; color: %1; background: transparent; border: none;").arg(TEXT_SECONDARY));
    profileProgressPercentLabel = xpValueLabel;

    progressHeader->addWidget(xpLabel);
    progressHeader->addStretch();
    progressHeader->addWidget(xpValueLabel);

    profileProgressBar = new QProgressBar();
    profileProgressBar->setMinimum(0);
    profileProgressBar->setMaximum(100);
    profileProgressBar->setValue(0);
    profileProgressBar->setTextVisible(false);
    profileProgressBar->setFixedHeight(10);
    profileProgressBar->setStyleSheet(QString(R"(
        QProgressBar {
            border: none;
            border-radius: 5px;
            background-color: %1;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %2, stop:1 %3);
            border-radius: 5px;
        }
    )").arg(BORDER_COLOR, ACCENT_BLUE, ACCENT_CYAN));

    progressLayout->addLayout(progressHeader);
    progressLayout->addWidget(profileProgressBar);

    cardLayout->addLayout(headerLayout);
    cardLayout->addLayout(statsRow);
    cardLayout->addWidget(progressSection);

    layout->addWidget(profileCard);
    layout->addStretch();
}

void CTFApp::updateNavButtons(QPushButton *active) {
    homeBtn->setChecked(homeBtn == active);
    challengesBtn->setChecked(challengesBtn == active);
    leaderboardBtn->setChecked(leaderboardBtn == active);
    profileBtn->setChecked(profileBtn == active);
}

void CTFApp::showHomePage() {
    updateNavButtons(homeBtn);
    contentStack->setCurrentWidget(homePage);
}

void CTFApp::showChallengesPage() {
    updateNavButtons(challengesBtn);
    contentStack->setCurrentWidget(challengeManager->getChallengesPage());
    challengeManager->setAuthToken(authToken);
    
    static bool tasksLoaded = false;
    if (!tasksLoaded) {
        challengeManager->loadTasks(authToken);
        tasksLoaded = true;
    } else {
        challengeManager->installCardEventFilters(this);
    }
}


void CTFApp::showLeaderboardPage() {
    updateNavButtons(leaderboardBtn);
    contentStack->setCurrentWidget(leaderboardPage);
    loadLeaderboard();
}

void CTFApp::showProfilePage() {
    updateNavButtons(profileBtn);
    contentStack->setCurrentWidget(profilePage);
    loadProfile();
}

void CTFApp::onLoginButtonClicked() {
    QString username = usernameLineEdit->text().trimmed();
    QString password = passwordLineEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        showError("Ошибка", "Введите логин и пароль");
        return;
    }
    
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;
    
    QNetworkRequest request(QUrl(SERVER_URL + "/api/auth/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = manager->post(request, QJsonDocument(json).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, username]() {
        QByteArray responseData = reply->readAll();
        QJsonObject jsonObj = QJsonDocument::fromJson(responseData).object();
        
        if (reply->error() == QNetworkReply::NoError && jsonObj.contains("token")) {
            authToken = jsonObj["token"].toString();
            currentUsername = username;
            mainStack->setCurrentWidget(dashboardPage);
            challengeManager->setAuthToken(authToken);
            challengeManager->loadTasks(authToken);
            loadProfile();
        } else {
            QString errorMsg = jsonObj.contains("error") ? jsonObj["error"].toString() : "Ошибка входа";
            showError("Ошибка", errorMsg);
        }
        reply->deleteLater();
    });
}

void CTFApp::onRegisterButtonClicked() {
    QString username = usernameLineEdit->text().trimmed();
    QString password = passwordLineEdit->text();
    
    if (username.length() < 3 || password.length() < 3) {
        showError("Ошибка", "Логин и пароль должны быть не менее 3 символов");
        return;
    }
    
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;
    
    QNetworkRequest request(QUrl(SERVER_URL + "/api/auth/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = manager->post(request, QJsonDocument(json).toJson());
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            showSuccess("Успех", "Регистрация успешна! Войдите в систему.");
        } else {
            QByteArray responseData = reply->readAll();
            QJsonObject jsonObj = QJsonDocument::fromJson(responseData).object();
            QString errorText = jsonObj.contains("error") ? jsonObj["error"].toString() : "Ошибка регистрации";
            showError("Ошибка", errorText);
        }
        reply->deleteLater();
    });
}

void CTFApp::onLogoutClicked() {
    authToken.clear();
    currentUsername.clear();
    currentScore = 0;
    usernameLineEdit->clear();
    passwordLineEdit->clear();
    mainStack->setCurrentWidget(loginPage);
}

void CTFApp::loadLeaderboard() {
    QNetworkRequest request(QUrl(SERVER_URL + "/api/leaderboard"));
    request.setRawHeader("Authorization", authToken.toUtf8());
    QNetworkReply *reply = manager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonArray data = QJsonDocument::fromJson(reply->readAll()).array();
            leaderboardTable->setRowCount(0);
            
            int row = 0;
            for (const auto &item : data) {
                QJsonObject obj = item.toObject();
                leaderboardTable->insertRow(row);
                QLabel *rankLabel = new QLabel();
                rankLabel->setAlignment(Qt::AlignCenter);
                QString rankHtml;
                
                if (row == 0) {
                    rankHtml = "<span style='color: #FFD700; font-weight: bold; font-size: 13px;'>🥇 1</span>";
                } else if (row == 1) {
                    rankHtml = "<span style='color: #C0C0C0; font-weight: bold; font-size: 13px;'>🥈 2</span>";
                } else if (row == 2) {
                    rankHtml = "<span style='color: #CD7F32; font-weight: bold; font-size: 13px;'>🥉 3</span>";
                } else {
                    rankHtml = QString("<span style='color: %1; font-weight: bold; font-size: 13px;'>%2</span>")
                        .arg(TEXT_PRIMARY).arg(row + 1);
                }
                
                rankLabel->setText(rankHtml);
                rankLabel->setStyleSheet("background: transparent; border: none; padding: 0px;");
                leaderboardTable->setCellWidget(row, 0, rankLabel);

                QTableWidgetItem *playerItem = new QTableWidgetItem(obj["username"].toString());
                playerItem->setFont(QFont("Segoe UI", 13));
                playerItem->setTextAlignment(Qt::AlignCenter);
                playerItem->setForeground(QColor(TEXT_PRIMARY));

                QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(obj["score"].toInt()));
                scoreItem->setTextAlignment(Qt::AlignCenter);
                scoreItem->setFont(QFont("Consolas", 13, QFont::Bold));
                scoreItem->setForeground(QColor(TEXT_PRIMARY));

                QTableWidgetItem *solvedItem = new QTableWidgetItem(QString::number(obj["solved_count"].toInt()));
                solvedItem->setTextAlignment(Qt::AlignCenter);
                solvedItem->setFont(QFont("Consolas", 13));
                solvedItem->setForeground(QColor(TEXT_PRIMARY));
                
                leaderboardTable->setItem(row, 1, playerItem);
                leaderboardTable->setItem(row, 2, scoreItem);
                leaderboardTable->setItem(row, 3, solvedItem);
                
                row++;
            }

            for (int i = 0; i < leaderboardTable->rowCount(); i++) {
                leaderboardTable->setRowHeight(i, 48);
            }
        }
        reply->deleteLater();
    });
}



void CTFApp::onLeaderboardResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        leaderboardTable->setRowCount(0);
        QJsonArray leaders = QJsonDocument::fromJson(reply->readAll()).array();
        
        int row = 0;
        for (const auto &leader : leaders) {
            QJsonObject obj = leader.toObject();
            leaderboardTable->insertRow(row);
            
            QString rankText = QString("#%1").arg(row + 1);
            if (row == 0) rankText = "🥇 " + rankText;
            else if (row == 1) rankText = "🥈 " + rankText;
            else if (row == 2) rankText = "🥉 " + rankText;
            
            leaderboardTable->setItem(row, 0, new QTableWidgetItem(rankText));
            leaderboardTable->setItem(row, 1, new QTableWidgetItem(obj["username"].toString()));
            leaderboardTable->setItem(row, 2, new QTableWidgetItem(QString::number(obj["score"].toInt())));
            leaderboardTable->setItem(row, 3, new QTableWidgetItem(QString::number(obj["solved_count"].toInt())));
            
            row++;
        }
    }
    reply->deleteLater();
}

void CTFApp::loadProfile() {
    QNetworkRequest request(QUrl(SERVER_URL + "/api/profile"));
    request.setRawHeader("Authorization", authToken.toUtf8());
    
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onProfileResponse(reply);
    });
}

void CTFApp::onProfileResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Profile response:" << data;
        
        QJsonObject profile = QJsonDocument::fromJson(data).object();
        
        currentScore = profile["score"].toInt();
        int rank = profile["rank"].toInt();
        int level = profile["level"].toInt();
        int solved = profile["solved_count"].toInt();
        int total = profile["total_tasks"].toInt();
        
        qDebug() << "Score:" << currentScore << "Rank:" << rank << "Level:" << level << "Solved:" << solved;
        
        int progressPercent = total > 0 ? (solved * 100 / total) : 0;

        int displayRank = (rank > 0) ? rank : level;
        
        profileNameLabel->setText(currentUsername);
        profileScoreLabel->setText(QString::number(currentScore));
        profileLevelLabel->setText(displayRank > 0 ? QString::number(displayRank) : "--");
        profileSolvedLabel->setText(QString::number(solved));
        
        profileProgressBar->setValue(progressPercent);
        profileProgressPercentLabel->setText(QString("%1% (%2/%3)").arg(progressPercent).arg(solved).arg(total));

        if (dashScoreLabel) dashScoreLabel->setText(QString::number(currentScore));
        if (dashSolvedLabel) dashSolvedLabel->setText(QString::number(solved));
        if (dashRankLabel) dashRankLabel->setText(displayRank > 0 ? QString::number(displayRank) : "--");
    }
    reply->deleteLater();
}


void CTFApp::showError(const QString &title, const QString &message) {
    qDebug() << "Show error:" << title << message;
    HintDialog::showError(title, message, this);
}

void CTFApp::showSuccess(const QString &title, const QString &message) {
    HintDialog::showSuccess(title, message, this);
}

bool CTFApp::eventFilter(QObject *obj, QEvent *event) {
    if (!obj) {
        return QWidget::eventFilter(obj, event);
    }
    
    QWidget *card = qobject_cast<QWidget*>(obj);
    if (card && card->property("taskId").isValid()) {
        if (!card->isHidden() && card->parent()) {
            return challengeManager->handleCardEvent(obj, event);
        }
    }
    return QWidget::eventFilter(obj, event);
}

void CTFApp::openUrl(const QString &url) {
    QDesktopServices::openUrl(QUrl(url));
}

