#!/bin/bash

echo "🗑️  Uninstalling CTF Platform..."

# Остановить контейнеры
if [ -d "$HOME/.local/share/ctf-platform" ]; then
    cd "$HOME/.local/share/ctf-platform"
    docker-compose down 2>/dev/null
    docker volume prune -f 2>/dev/null
fi

# Удалить файлы
rm -rf "$HOME/.local/share/ctf-platform"
rm -f "$HOME/.local/bin/ctf-platform"
rm -f "$HOME/.local/share/applications/ctf-platform.desktop"

update-desktop-database ~/.local/share/applications/ 2>/dev/null || true

echo "✅ CTF Platform has been uninstalled"
