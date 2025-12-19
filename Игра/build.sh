#!/bin/bash

# Скрипт для сборки игры по географии

echo "Сборка игры по географии..."

# Создание директории build, если её нет
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Запуск CMake
cmake ..

# Компиляция
make

echo ""
echo "Сборка завершена!"
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Для запуска игры выполните:"
    echo "  open build/GeographyGame.app"
    echo "или"
    echo "  ./build/GeographyGame.app/Contents/MacOS/GeographyGame"
else
    echo "Для запуска игры выполните: ./build/GeographyGame"
fi

