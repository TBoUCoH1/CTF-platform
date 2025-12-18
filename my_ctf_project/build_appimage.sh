    
#!/bin/bash
set -e

echo "📦 Сборка Frontend в AppImage..."

# 1. Скачиваем инструменты (если нет)
mkdir -p tools
cd tools
if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi
if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
fi
cd ..

# 2. Сборка Qt проекта
rm -rf frontend/build_appimage
mkdir -p frontend/build_appimage
cd frontend/build_appimage

# Важно: CMAKE_INSTALL_PREFIX=/usr обязателен для AppImage
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
make install DESTDIR=AppDir

# 3. Создаем desktop файл
cat > ../ctf-platform.desktop <<EOF
[Desktop Entry]
Type=Application
Name=CTF Platform
Exec=CTFApp
Icon=icon
Categories=Education;
EOF

# 4. Подготовка иконки (Скачиваем щит)
ICON_PATH="../../frontend/resources/icon.png"
if [ ! -f "$ICON_PATH" ]; then
    echo "⚠️ Иконка не найдена, скачиваю заглушку..."
    wget -q -O icon.png https://img.icons8.com/ios-filled/64/228BE6/security-shield-green.png
    ICON_PATH="icon.png"
fi

# 5. Упаковка
echo "🔮 Генерация AppImage..."

# Поиск qmake6
if command -v qmake6 &> /dev/null; then
    export QMAKE=$(which qmake6)
elif [ -f /usr/lib/qt6/bin/qmake ]; then
    export QMAKE=/usr/lib/qt6/bin/qmake
else
    echo "⚠️ ВНИМАНИЕ: qmake6 не найден, использую дефолтный..."
    export QMAKE=$(which qmake)
fi

../../tools/linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --plugin qt \
    --output appimage \
    --desktop-file ../ctf-platform.desktop \
    --icon-file "$ICON_PATH" \
    --executable AppDir/usr/bin/CTFApp

# Мы берем любой файл .AppImage, который появился в папке, и переименовываем его
find . -name "*.AppImage" -exec mv {} ../../CTFPlatform.AppImage \;

echo "✅ Frontend готов: CTFPlatform.AppImage"


