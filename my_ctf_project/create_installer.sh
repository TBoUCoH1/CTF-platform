#!/bin/bash
set -e

INSTALLER_NAME="CTF_Platform_Setup.run"
PAYLOAD_DIR="payload_temp"

echo "=========================================="
echo "🚀 Создание финального установщика"
echo "=========================================="

./build_appimage.sh

echo "🐳 Сборка Backend..."
docker-compose build

rm -rf "$PAYLOAD_DIR"
mkdir -p "$PAYLOAD_DIR/images"
mkdir -p "$PAYLOAD_DIR/config/db"

echo "💾 Экспорт Docker образов..."
docker save ctf-platform-backend:latest | gzip > "$PAYLOAD_DIR/images/backend.tar.gz"
docker save ctf-platform-web:latest | gzip > "$PAYLOAD_DIR/images/web.tar.gz"

cp CTFPlatform.AppImage "$PAYLOAD_DIR/"
cp docker-compose.yml "$PAYLOAD_DIR/config/"
cp db/init.sql "$PAYLOAD_DIR/config/db/"

cat > "$PAYLOAD_DIR/install.sh" << 'EOF_INSTALL'
#!/bin/bash
set -e
INSTALL_DIR="$HOME/.local/share/ctf-platform"
BIN_DIR="$HOME/.local/bin"

echo "🎯 Установка CTF Platform..."

# Проверки
if ! command -v docker &> /dev/null; then
    echo "❌ Docker не найден! Установите docker.io и docker-compose."
    exit 1
fi

mkdir -p "$INSTALL_DIR" "$BIN_DIR"

# Копирование
cp CTFPlatform.AppImage "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/CTFPlatform.AppImage"
cp -r config/* "$INSTALL_DIR/"

# Загрузка образов
echo "🐳 Загрузка компонентов..."
docker load < images/backend.tar.gz
docker load < images/web.tar.gz

# Launcher
cat > "$BIN_DIR/ctf-platform" << 'LAUNCHER'
#!/bin/bash
APP_DIR="$HOME/.local/share/ctf-platform"
if ! docker ps --format '{{.Names}}' | grep -q ctf_backend; then
    echo "🚀 Запуск сервера..."
    cd "$APP_DIR" && docker-compose up -d
    sleep 2
fi
"$APP_DIR/CTFPlatform.AppImage" "$@"
LAUNCHER
chmod +x "$BIN_DIR/ctf-platform"

# Ярлык
cat > "$HOME/.local/share/applications/ctf-platform.desktop" << DESKTOP
[Desktop Entry]
Type=Application
Name=CTF Platform
Exec=$BIN_DIR/ctf-platform
Icon=security-high
Terminal=false
Categories=Education;
DESKTOP

echo "✅ Готово! Запускайте 'CTF Platform' из меню."
EOF_INSTALL

chmod +x "$PAYLOAD_DIR/install.sh"

if ! command -v makeself &> /dev/null; then
    echo "⚠️ Устанавливаю makeself..."
    sudo apt-get install makeself -y
fi

makeself --gzip "$PAYLOAD_DIR" "$INSTALLER_NAME" "CTF Platform Installer" "./install.sh"

rm -rf "$PAYLOAD_DIR" CTFPlatform.AppImage

echo ""
echo "🎉 Файл готов: $INSTALLER_NAME"
