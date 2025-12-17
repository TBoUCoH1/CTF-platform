#!/bin/bash

echo "=========================================="
echo "  🎯 CTF Platform Installation"
echo "=========================================="
echo ""

INSTALL_DIR="$HOME/.local/share/ctf-platform"
BIN_DIR="$HOME/.local/bin"

# Проверка зависимостей
echo "Checking dependencies..."

if ! command -v docker &> /dev/null; then
    echo "📦 Installing Docker..."
    sudo apt update
    sudo apt install -y docker.io docker-compose
    sudo systemctl enable --now docker
    sudo usermod -aG docker $USER
    echo ""
    echo "⚠️  IMPORTANT: Log out and log back in for Docker to work!"
    echo ""
    read -p "Press Enter to continue..."
fi

# Создание директорий
echo "Creating directories..."
mkdir -p "$INSTALL_DIR"
mkdir -p "$BIN_DIR"

# Копирование файлов
echo "Installing files..."
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cp -r "$SCRIPT_DIR"/* "$INSTALL_DIR/"

# Создание launcher
cat > "$BIN_DIR/ctf-platform" << 'LAUNCHER'
#!/bin/bash

INSTALL_DIR="$HOME/.local/share/ctf-platform"
cd "$INSTALL_DIR"

# Проверка и запуск Docker backend
if ! docker ps 2>/dev/null | grep -q ctf_backend; then
    echo "🐳 Starting backend services..."
    docker-compose up -d 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "❌ Failed to start backend. Try manually:"
        echo "   cd $INSTALL_DIR"
        echo "   ./start-backend.sh"
        exit 1
    fi
    echo "⏳ Waiting for backend to start..."
    sleep 5
fi

# Запуск frontend
export GTK_THEME=Adwaita:dark
export QT_QPA_PLATFORMTHEME=gtk3
./bin/CTFApp
LAUNCHER

chmod +x "$BIN_DIR/ctf-platform"

# Desktop entry
echo "Creating application shortcut..."
cat > ~/.local/share/applications/ctf-platform.desktop << DESKTOP
[Desktop Entry]
Type=Application
Name=CTF Platform
Comment=🎯 Capture The Flag Training Platform
Exec=$BIN_DIR/ctf-platform
Icon=security-high
Terminal=false
Categories=Development;Education;Security;
DESKTOP

update-desktop-database ~/.local/share/applications/ 2>/dev/null || true

echo ""
echo "=========================================="
echo "  ✅ Installation Complete!"
echo "=========================================="
echo ""
echo "🚀 How to launch:"
echo "   1. From Applications menu: 'CTF Platform'"
echo "   2. From terminal: ctf-platform"
echo ""
echo "🔧 Manual control:"
echo "   Start backend: cd $INSTALL_DIR && ./start-backend.sh"
echo "   Stop backend:  cd $INSTALL_DIR && ./stop-backend.sh"
echo "   Frontend only: cd $INSTALL_DIR && ./CTFApp"
echo ""
if ! groups | grep -q docker; then
    echo "⚠️  Docker permissions not active yet!"
    echo "   Please LOG OUT and LOG BACK IN"
    echo ""
fi
