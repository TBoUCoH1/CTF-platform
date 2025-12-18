#ifndef LEADERBOARDSTYLE_H
#define LEADERBOARDSTYLE_H

#include <QString>

// QSS стили для таблицы лидеров в стиле "Striped List" с темной темой
const QString LEADERBOARD_STYLE = R"(
    QTableWidget {
        background-color: #0F172A;
        border: none;
        gridline-color: transparent;
        alternate-background-color: #1E293B;
    }
    
    QTableWidget::item {
        background-color: #0F172A;
        border: none;
        border-bottom: 1px solid #1E293B;
        padding: 10px 12px;
        height: 40px;
    }
    
    QTableWidget::item:alternate {
        background-color: #1E293B;
    }
    
    QTableWidget::item:selected {
        background-color: #3B82F6;
        color: white;
    }
    
    QHeaderView::section {
        background-color: transparent;
        color: #94A3B8;
        padding: 8px 12px;
        border: none;
        font-weight: 600;
        font-size: 12px;
        text-transform: uppercase;
    }
    
    QTableWidget QTableCornerButton::section {
        background-color: transparent;
        border: none;
    }
)";

// Цвета для топ-3 участников
const QString GOLD_COLOR = "#FFD700";
const QString SILVER_COLOR = "#C0C0C0";
const QString BRONZE_COLOR = "#CD7F32";

#endif // LEADERBOARDSTYLE_H