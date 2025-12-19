#!/bin/bash
set -e

echo "🚀 Запуск режима разработки..."

echo "🐳 Поднимаем Docker сервисы..."
docker-compose up -d
echo "🔨 Компиляция Frontend..."
mkdir -p frontend/build
cd frontend/build
cmake ..
make -j$(nproc)

echo "▶️ Запуск приложения..."
export GTK_THEME=Adwaita:dark
./CTFApp
