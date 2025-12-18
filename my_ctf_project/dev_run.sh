#!/bin/bash
set -e

echo "🚀 Запуск режима разработки..."

# 1. Запускаем Backend и БД
echo "🐳 Поднимаем Docker сервисы..."
docker-compose up -d

# 2. Компилируем и запускаем Frontend
echo "🔨 Компиляция Frontend..."
mkdir -p frontend/build
cd frontend/build
cmake ..
make -j$(nproc)

echo "▶️ Запуск приложения..."
# Передаем переменные для корректной темы
export GTK_THEME=Adwaita:dark
./CTFApp
