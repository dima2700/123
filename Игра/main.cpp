#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <sstream>
#include <cmath>

// Режимы игры
enum class GameMode {
    Menu,
    Quiz,
    DragAndDrop
};

// Структура для вопроса викторины
struct Question {
    std::string question;
    std::vector<std::string> answers;
    int correctAnswer;
    
    Question(const std::string& q, const std::vector<std::string>& ans, int correct)
        : question(q), answers(ans), correctAnswer(correct) {}
};

// Структура для задания на перетаскивание
struct DragTask {
    std::string country;
    std::string capital;
    sf::Vector2f countryPos;
    sf::Vector2f capitalStartPos;
    bool matched;
    
    DragTask(const std::string& c, const std::string& cap, float x1, float y1, float x2, float y2)
        : country(c), capital(cap), countryPos(x1, y1), capitalStartPos(x2, y2), matched(false) {}
};

// Класс игры
class GeographyGame {
private:
    sf::RenderWindow window;
    
    // Режим игры
    GameMode currentMode;
    
    // Викторина
    std::vector<Question> questions;
    int currentQuestionIndex;
    int quizScore;
    int selectedAnswer;
    bool answered;
    bool gameOver;
    
    // Перетаскивание
    std::vector<DragTask> dragTasks;
    int currentDragTask;
    int dragScore;
    int draggedIndex;
    bool isDragging;
    sf::Vector2f dragOffset;
    bool dragGameOver;
    
    // Общие элементы
    sf::Font font;
    sf::Text scoreText;
    sf::Text gameOverText;
    sf::Text restartText;
    sf::RectangleShape restartButton;
    sf::Clock clock;
    float timeLimit;
    sf::Text timerText;
    
    // Викторина UI
    sf::Text questionText;
    std::vector<sf::Text> answerTexts;
    std::vector<sf::RectangleShape> answerButtons;
    
    // Перетаскивание UI
    std::vector<sf::RectangleShape> countryBoxes;
    std::vector<sf::RectangleShape> capitalBoxes;
    std::vector<sf::Text> countryTexts;
    std::vector<sf::Text> capitalTexts;
    std::vector<sf::RectangleShape> dropZones;
    
    // Меню
    sf::Text menuTitle;
    sf::RectangleShape quizButton;
    sf::RectangleShape dragButton;
    sf::Text quizButtonText;
    sf::Text dragButtonText;
    
    // Цвета
    sf::Color backgroundColor;
    sf::Color buttonColor;
    sf::Color buttonHoverColor;
    sf::Color correctColor;
    sf::Color wrongColor;
    sf::Color textColor;
    sf::Color dragColor;
    sf::Color dropZoneColor;
    
public:
    GeographyGame() : window(sf::VideoMode(sf::Vector2u(1200, 800)), "Geography Game", sf::Style::Close),
                      currentMode(GameMode::Menu),
                      questionText(font, ""),
                      answerTexts(4, sf::Text(font, "")),
                      scoreText(font, ""),
                      gameOverText(font, ""),
                      restartText(font, ""),
                      timerText(font, ""),
                      menuTitle(font, ""),
                      quizButtonText(font, ""),
                      dragButtonText(font, ""),
                      currentQuestionIndex(0), quizScore(0), selectedAnswer(-1),
                      answered(false), gameOver(false), timeLimit(30.0f),
                      currentDragTask(0), dragScore(0), draggedIndex(-1),
                      isDragging(false), dragGameOver(false) {
        
        // Загрузка шрифта
        bool fontLoaded = false;
        #ifdef __APPLE__
            const char* fontPaths[] = {
                "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
                "/System/Library/Fonts/Supplemental/Arial.ttf",
                "/Library/Fonts/Arial Unicode.ttf",
                "/Library/Fonts/Arial.ttf"
            };
            for (const char* path : fontPaths) {
                if (font.openFromFile(path)) {
                    if (font.hasGlyph(0x0410)) {
                        fontLoaded = true;
                        break;
                    }
                }
            }
        #else
            const char* fontPaths[] = {
                "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                "C:/Windows/Fonts/arial.ttf"
            };
            for (const char* path : fontPaths) {
                if (font.openFromFile(path)) {
                    fontLoaded = true;
                    break;
                }
            }
        #endif
        
        if (!fontLoaded) {
            std::cerr << "ОШИБКА: Не удалось загрузить шрифт!" << std::endl;
            return;
        }
        
        // Инициализация шрифтов для всех текстов
        questionText.setFont(font);
        for (int i = 0; i < 4; i++) {
            answerTexts[i].setFont(font);
        }
        scoreText.setFont(font);
        gameOverText.setFont(font);
        restartText.setFont(font);
        timerText.setFont(font);
        menuTitle.setFont(font);
        quizButtonText.setFont(font);
        dragButtonText.setFont(font);
        
        // Цвета
        backgroundColor = sf::Color(240, 248, 255);
        buttonColor = sf::Color(70, 130, 180);
        buttonHoverColor = sf::Color(100, 149, 237);
        correctColor = sf::Color(50, 205, 50);
        wrongColor = sf::Color(220, 20, 60);
        textColor = sf::Color(25, 25, 112);
        dragColor = sf::Color(255, 165, 0);
        dropZoneColor = sf::Color(200, 200, 200);
        
        initializeQuestions();
        initializeDragTasks();
        setupMenu();
    }
    
    void initializeQuestions() {
        questions.push_back(Question("Какая столица Франции?", {"Лондон", "Париж", "Берлин", "Мадрид"}, 1));
        questions.push_back(Question("Какая самая большая страна в мире?", {"Китай", "США", "Россия", "Канада"}, 2));
        questions.push_back(Question("Какая столица Японии?", {"Сеул", "Пекин", "Токио", "Бангкок"}, 2));
        questions.push_back(Question("Какая река самая длинная в мире?", {"Амазонка", "Нил", "Янцзы", "Миссисипи"}, 1));
        questions.push_back(Question("Какая самая высокая гора в мире?", {"К2", "Эверест", "Килиманджаро", "Монблан"}, 1));
        questions.push_back(Question("Какая столица Австралии?", {"Сидней", "Мельбурн", "Канберра", "Брисбен"}, 2));
        questions.push_back(Question("Какая столица Бразилии?", {"Рио-де-Жанейро", "Сан-Паулу", "Бразилиа", "Сальвадор"}, 2));
        questions.push_back(Question("Какая столица Египта?", {"Александрия", "Каир", "Гиза", "Луксор"}, 1));
        questions.push_back(Question("Какая столица Канады?", {"Торонто", "Ванкувер", "Оттава", "Монреаль"}, 2));
        questions.push_back(Question("Какая столица Индии?", {"Мумбаи", "Дели", "Калькутта", "Бангалор"}, 1));
    }
    
    void initializeDragTasks() {
        dragTasks.clear();
        // Инициализация заданий на перетаскивание: страна -> столица
        // НЕ перемешиваем здесь - это сделаем в setupDragAndDrop, перемешивая только позиции
        dragTasks.push_back(DragTask("Франция", "Париж", 200, 200, 800, 200));
        dragTasks.push_back(DragTask("Россия", "Москва", 200, 300, 800, 300));
        dragTasks.push_back(DragTask("Япония", "Токио", 200, 400, 800, 400));
        dragTasks.push_back(DragTask("Бразилия", "Бразилиа", 200, 500, 800, 500));
        dragTasks.push_back(DragTask("Египет", "Каир", 200, 600, 800, 600));
    }
    
    void setupMenu() {
        std::string titleStr = "География для школьников";
        menuTitle.setString(sf::String::fromUtf8(titleStr.begin(), titleStr.end()));
        menuTitle.setCharacterSize(48);
        menuTitle.setFillColor(textColor);
        menuTitle.setStyle(sf::Text::Bold);
        menuTitle.setPosition(sf::Vector2f(300, 150));
        
        quizButton.setSize(sf::Vector2f(400, 80));
        quizButton.setPosition(sf::Vector2f(400, 350));
        quizButton.setFillColor(buttonColor);
        quizButton.setOutlineColor(sf::Color::Black);
        quizButton.setOutlineThickness(3);
        
        std::string quizStr = "Викторина";
        quizButtonText.setString(sf::String::fromUtf8(quizStr.begin(), quizStr.end()));
        quizButtonText.setCharacterSize(32);
        quizButtonText.setFillColor(sf::Color::White);
        quizButtonText.setPosition(sf::Vector2f(520, 370));
        
        dragButton.setSize(sf::Vector2f(400, 80));
        dragButton.setPosition(sf::Vector2f(400, 470));
        dragButton.setFillColor(buttonColor);
        dragButton.setOutlineColor(sf::Color::Black);
        dragButton.setOutlineThickness(3);
        
        std::string dragStr = "Перетаскивание столиц";
        dragButtonText.setString(sf::String::fromUtf8(dragStr.begin(), dragStr.end()));
        dragButtonText.setCharacterSize(32);
        dragButtonText.setFillColor(sf::Color::White);
        dragButtonText.setPosition(sf::Vector2f(420, 490));
    }
    
    void setupQuiz() {
        questionText.setCharacterSize(32);
        questionText.setFillColor(textColor);
        questionText.setStyle(sf::Text::Bold);
        questionText.setPosition(sf::Vector2f(50, 50));
        
        answerButtons.resize(4);
        for (int i = 0; i < 4; i++) {
            answerButtons[i].setSize(sf::Vector2f(1100, 80));
            answerButtons[i].setPosition(sf::Vector2f(50, 200 + i * 100));
            answerButtons[i].setFillColor(buttonColor);
            answerButtons[i].setOutlineColor(sf::Color::Black);
            answerButtons[i].setOutlineThickness(2);
            
            answerTexts[i].setCharacterSize(24);
            answerTexts[i].setFillColor(sf::Color::White);
            answerTexts[i].setPosition(sf::Vector2f(70, 220 + i * 100));
        }
        
        scoreText.setCharacterSize(28);
        scoreText.setFillColor(textColor);
        scoreText.setStyle(sf::Text::Bold);
        scoreText.setPosition(sf::Vector2f(50, 700));
        
        timerText.setCharacterSize(28);
        timerText.setFillColor(textColor);
        timerText.setStyle(sf::Text::Bold);
        timerText.setPosition(sf::Vector2f(900, 700));
        
        std::string gameOverStr = "Игра окончена!";
        gameOverText.setString(sf::String::fromUtf8(gameOverStr.begin(), gameOverStr.end()));
        gameOverText.setCharacterSize(48);
        gameOverText.setFillColor(textColor);
        gameOverText.setStyle(sf::Text::Bold);
        gameOverText.setPosition(sf::Vector2f(400, 250));
        
        restartButton.setSize(sf::Vector2f(300, 60));
        restartButton.setPosition(sf::Vector2f(450, 350));
        restartButton.setFillColor(buttonColor);
        restartButton.setOutlineColor(sf::Color::Black);
        restartButton.setOutlineThickness(2);
        
        std::string restartStr = "Играть снова";
        restartText.setString(sf::String::fromUtf8(restartStr.begin(), restartStr.end()));
        restartText.setCharacterSize(28);
        restartText.setFillColor(sf::Color::White);
        restartText.setPosition(sf::Vector2f(500, 360));
        
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(questions.begin(), questions.end(), g);
        updateQuestion();
    }
    
    void setupDragAndDrop() {
        countryBoxes.resize(dragTasks.size());
        capitalBoxes.resize(dragTasks.size());
        countryTexts.clear();
        capitalTexts.clear();
        countryTexts.reserve(dragTasks.size());
        capitalTexts.reserve(dragTasks.size());
        dropZones.resize(dragTasks.size());
        
        // Собираем начальные позиции столиц для перемешивания
        std::vector<sf::Vector2f> capitalPositions;
        for (size_t i = 0; i < dragTasks.size(); i++) {
            capitalPositions.push_back(dragTasks[i].capitalStartPos);
        }
        
        // Перемешиваем только позиции столиц, сохраняя соответствие индекс-столица
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(capitalPositions.begin(), capitalPositions.end(), g);
        
        for (size_t i = 0; i < dragTasks.size(); i++) {
            // Боксы для стран (слева) - остаются на своих местах
            countryBoxes[i].setSize(sf::Vector2f(250, 60));
            countryBoxes[i].setPosition(dragTasks[i].countryPos);
            countryBoxes[i].setFillColor(buttonColor);
            countryBoxes[i].setOutlineColor(sf::Color::Black);
            countryBoxes[i].setOutlineThickness(2);
            
            sf::Text countryText(font, sf::String::fromUtf8(dragTasks[i].country.begin(), dragTasks[i].country.end()));
            countryText.setCharacterSize(24);
            countryText.setFillColor(sf::Color::White);
            countryText.setPosition(sf::Vector2f(dragTasks[i].countryPos.x + 10, dragTasks[i].countryPos.y + 15));
            countryTexts.push_back(countryText);
            
            // Боксы для столиц (справа, перемешанные позиции)
            // Важно: индекс i всегда соответствует dragTasks[i].capital
            dragTasks[i].capitalStartPos = capitalPositions[i];
            capitalBoxes[i].setSize(sf::Vector2f(250, 60));
            capitalBoxes[i].setPosition(capitalPositions[i]);
            capitalBoxes[i].setFillColor(dragColor);
            capitalBoxes[i].setOutlineColor(sf::Color::Black);
            capitalBoxes[i].setOutlineThickness(2);
            
            sf::Text capitalText(font, sf::String::fromUtf8(dragTasks[i].capital.begin(), dragTasks[i].capital.end()));
            capitalText.setCharacterSize(24);
            capitalText.setFillColor(sf::Color::White);
            capitalText.setPosition(sf::Vector2f(capitalPositions[i].x + 10, capitalPositions[i].y + 15));
            capitalTexts.push_back(capitalText);
            
            // Зоны для сброса (рядом со странами)
            dropZones[i].setSize(sf::Vector2f(250, 60));
            dropZones[i].setPosition(sf::Vector2f(dragTasks[i].countryPos.x + 300, dragTasks[i].countryPos.y));
            dropZones[i].setFillColor(sf::Color::Transparent);
            dropZones[i].setOutlineColor(dropZoneColor);
            dropZones[i].setOutlineThickness(2);
        }
        
        scoreText.setCharacterSize(28);
        scoreText.setFillColor(textColor);
        scoreText.setStyle(sf::Text::Bold);
        scoreText.setPosition(sf::Vector2f(50, 50));
        
        std::string gameOverStr = "Все столицы сопоставлены!";
        gameOverText.setString(sf::String::fromUtf8(gameOverStr.begin(), gameOverStr.end()));
        gameOverText.setCharacterSize(48);
        gameOverText.setFillColor(textColor);
        gameOverText.setStyle(sf::Text::Bold);
        gameOverText.setPosition(sf::Vector2f(300, 300));
        
        restartButton.setSize(sf::Vector2f(300, 60));
        restartButton.setPosition(sf::Vector2f(450, 400));
        restartButton.setFillColor(buttonColor);
        restartButton.setOutlineColor(sf::Color::Black);
        restartButton.setOutlineThickness(2);
        
        std::string restartStr = "Играть снова";
        restartText.setString(sf::String::fromUtf8(restartStr.begin(), restartStr.end()));
        restartText.setCharacterSize(28);
        restartText.setFillColor(sf::Color::White);
        restartText.setPosition(sf::Vector2f(500, 410));
    }
    
    void updateQuestion() {
        if (currentQuestionIndex < questions.size()) {
            questionText.setString(sf::String::fromUtf8(
                questions[currentQuestionIndex].question.begin(),
                questions[currentQuestionIndex].question.end()
            ));
            
            for (int i = 0; i < 4; i++) {
                answerTexts[i].setString(sf::String::fromUtf8(
                    questions[currentQuestionIndex].answers[i].begin(),
                    questions[currentQuestionIndex].answers[i].end()
                ));
            }
            
            answered = false;
            selectedAnswer = -1;
            clock.restart();
        } else {
            gameOver = true;
        }
    }
    
    void handleQuizAnswer(int answerIndex) {
        if (answered || gameOver) return;
        
        selectedAnswer = answerIndex;
        answered = true;
        
        if (answerIndex == questions[currentQuestionIndex].correctAnswer) {
            quizScore += 10;
            answerButtons[answerIndex].setFillColor(correctColor);
        } else {
            answerButtons[answerIndex].setFillColor(wrongColor);
            answerButtons[questions[currentQuestionIndex].correctAnswer].setFillColor(correctColor);
        }
    }
    
    void updateQuiz() {
        if (gameOver) return;
        
        float elapsed = clock.getElapsedTime().asSeconds();
        float remaining = timeLimit - elapsed;
        
        if (remaining <= 0 && !answered) {
            handleQuizAnswer(-1);
        }
        
        std::ostringstream timerStream;
        timerStream << "Время: " << (int)remaining;
        std::string timerStr = timerStream.str();
        timerText.setString(sf::String::fromUtf8(timerStr.begin(), timerStr.end()));
        
        if (remaining < 10) {
            timerText.setFillColor(wrongColor);
        } else {
            timerText.setFillColor(textColor);
        }
        
        std::ostringstream scoreStream;
        scoreStream << "Счет: " << quizScore;
        std::string scoreStr = scoreStream.str();
        scoreText.setString(sf::String::fromUtf8(scoreStr.begin(), scoreStr.end()));
        
        if (answered) {
            static sf::Clock delayClock;
            if (delayClock.getElapsedTime().asSeconds() > 2.0f) {
                delayClock.restart();
                currentQuestionIndex++;
                
                for (int i = 0; i < 4; i++) {
                    answerButtons[i].setFillColor(buttonColor);
                }
                
                updateQuestion();
            }
        }
    }
    
    void shuffleUnmatchedCapitals() {
        // Собираем индексы и позиции несопоставленных столиц
        std::vector<std::pair<size_t, sf::Vector2f>> unmatched;
        for (size_t i = 0; i < dragTasks.size(); i++) {
            if (!dragTasks[i].matched) {
                unmatched.push_back({i, capitalBoxes[i].getPosition()});
            }
        }
        
        if (unmatched.size() < 2) return; // Нечего перемешивать
        
        // Перемешиваем позиции (но сохраняем соответствие индексов)
        std::random_device rd;
        std::mt19937 g(rd());
        std::vector<sf::Vector2f> positions;
        for (const auto& pair : unmatched) {
            positions.push_back(pair.second);
        }
        std::shuffle(positions.begin(), positions.end(), g);
        
        // Применяем новые позиции, сохраняя соответствие индекс-столица
        for (size_t j = 0; j < unmatched.size(); j++) {
            size_t idx = unmatched[j].first;
            dragTasks[idx].capitalStartPos = positions[j];
            capitalBoxes[idx].setPosition(positions[j]);
            capitalTexts[idx].setPosition(sf::Vector2f(positions[j].x + 10, positions[j].y + 15));
        }
    }
    
    void updateDragAndDrop() {
        std::ostringstream scoreStream;
        scoreStream << "Счет: " << dragScore << " / " << dragTasks.size();
        std::string scoreStr = scoreStream.str();
        scoreText.setString(sf::String::fromUtf8(scoreStr.begin(), scoreStr.end()));
        
        // Проверяем, все ли сопоставлено
        bool allMatched = true;
        for (const auto& task : dragTasks) {
            if (!task.matched) {
                allMatched = false;
                break;
            }
        }
        
        if (allMatched && !dragGameOver) {
            dragGameOver = true;
        }
        
        // Периодически перемешиваем несопоставленные столицы
        static sf::Clock shuffleClock;
        if (shuffleClock.getElapsedTime().asSeconds() > 3.0f && !allMatched) {
            shuffleUnmatchedCapitals();
            shuffleClock.restart();
        }
    }
    
    void handleMenuClick(int x, int y) {
        if (x >= 400 && x <= 800 && y >= 350 && y <= 430) {
            currentMode = GameMode::Quiz;
            currentQuestionIndex = 0;
            quizScore = 0;
            gameOver = false;
            setupQuiz();
        } else if (x >= 400 && x <= 800 && y >= 470 && y <= 550) {
            currentMode = GameMode::DragAndDrop;
            dragScore = 0;
            dragGameOver = false;
            isDragging = false;
            draggedIndex = -1;
            initializeDragTasks();
            setupDragAndDrop();
        }
    }
    
    void handleQuizClick(int x, int y) {
        if (gameOver) {
            if (x >= 450 && x <= 750 && y >= 350 && y <= 410) {
                currentQuestionIndex = 0;
                quizScore = 0;
                gameOver = false;
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(questions.begin(), questions.end(), g);
                updateQuestion();
            }
            return;
        }
        
        if (answered) return;
        
        for (int i = 0; i < 4; i++) {
            if (x >= 50 && x <= 1150 && y >= 200 + i * 100 && y <= 280 + i * 100) {
                handleQuizAnswer(i);
                break;
            }
        }
    }
    
    void handleDragClick(int x, int y) {
        if (dragGameOver) {
            if (x >= 450 && x <= 750 && y >= 400 && y <= 460) {
                dragScore = 0;
                dragGameOver = false;
                isDragging = false;
                draggedIndex = -1;
                initializeDragTasks();
                setupDragAndDrop();
            }
            return;
        }
        
        // Проверяем клик по столицам (справа)
        for (size_t i = 0; i < capitalBoxes.size(); i++) {
            if (!dragTasks[i].matched && 
                x >= capitalBoxes[i].getPosition().x && 
                x <= capitalBoxes[i].getPosition().x + capitalBoxes[i].getSize().x &&
                y >= capitalBoxes[i].getPosition().y && 
                y <= capitalBoxes[i].getPosition().y + capitalBoxes[i].getSize().y) {
                isDragging = true;
                draggedIndex = i;
                dragOffset.x = x - capitalBoxes[i].getPosition().x;
                dragOffset.y = y - capitalBoxes[i].getPosition().y;
                break;
            }
        }
    }
    
    void handleDragRelease(int x, int y) {
        if (!isDragging || draggedIndex == -1) return;
        
        // Проверяем, попали ли в зону сброса
        for (size_t i = 0; i < dropZones.size(); i++) {
            if (x >= dropZones[i].getPosition().x && 
                x <= dropZones[i].getPosition().x + dropZones[i].getSize().x &&
                y >= dropZones[i].getPosition().y && 
                y <= dropZones[i].getPosition().y + dropZones[i].getSize().y) {
                
                // Проверяем правильность сопоставления
                // Столица с индексом draggedIndex должна соответствовать стране с индексом i
                // Это правильно, потому что индекс всегда соответствует содержимому
                if (dragTasks[draggedIndex].capital == dragTasks[i].capital && 
                    dragTasks[draggedIndex].country == dragTasks[i].country &&
                    !dragTasks[i].matched) {
                    dragTasks[draggedIndex].matched = true;
                    dragTasks[i].matched = true; // Помечаем страну как сопоставленную
                    capitalBoxes[draggedIndex].setPosition(dropZones[i].getPosition());
                    capitalTexts[draggedIndex].setPosition(sf::Vector2f(
                        dropZones[i].getPosition().x + 10,
                        dropZones[i].getPosition().y + 15
                    ));
                    capitalBoxes[draggedIndex].setFillColor(correctColor);
                    dragScore++;
                    
                    // Перемешиваем оставшиеся столицы после правильного ответа
                    shuffleUnmatchedCapitals();
                } else {
                    // Возвращаем на место
                    capitalBoxes[draggedIndex].setPosition(dragTasks[draggedIndex].capitalStartPos);
                    capitalTexts[draggedIndex].setPosition(sf::Vector2f(
                        dragTasks[draggedIndex].capitalStartPos.x + 10,
                        dragTasks[draggedIndex].capitalStartPos.y + 15
                    ));
                }
                break;
            }
        }
        
        // Если не попали в зону, возвращаем на место
        if (!dragTasks[draggedIndex].matched) {
            capitalBoxes[draggedIndex].setPosition(dragTasks[draggedIndex].capitalStartPos);
            capitalTexts[draggedIndex].setPosition(sf::Vector2f(
                dragTasks[draggedIndex].capitalStartPos.x + 10,
                dragTasks[draggedIndex].capitalStartPos.y + 15
            ));
        }
        
        isDragging = false;
        draggedIndex = -1;
    }
    
    void handleMouseMove(int x, int y) {
        if (currentMode == GameMode::Quiz && !gameOver && !answered) {
            for (int i = 0; i < 4; i++) {
                if (x >= 50 && x <= 1150 && y >= 200 + i * 100 && y <= 280 + i * 100) {
                    answerButtons[i].setFillColor(buttonHoverColor);
                } else {
                    answerButtons[i].setFillColor(buttonColor);
                }
            }
        } else if (currentMode == GameMode::DragAndDrop && isDragging && draggedIndex != -1) {
            capitalBoxes[draggedIndex].setPosition(sf::Vector2f(x - dragOffset.x, y - dragOffset.y));
            capitalTexts[draggedIndex].setPosition(sf::Vector2f(
                x - dragOffset.x + 10,
                y - dragOffset.y + 15
            ));
        }
    }
    
    void drawMenu() {
        window.clear(backgroundColor);
        window.draw(menuTitle);
        window.draw(quizButton);
        window.draw(quizButtonText);
        window.draw(dragButton);
        window.draw(dragButtonText);
        window.display();
    }
    
    void drawQuiz() {
        window.clear(backgroundColor);
        
        if (gameOver) {
            window.draw(gameOverText);
            
            std::ostringstream finalScore;
            finalScore << "Ваш счет: " << quizScore << " из " << (questions.size() * 10);
            std::string finalScoreStr = finalScore.str();
            sf::Text finalScoreText(font, sf::String::fromUtf8(finalScoreStr.begin(), finalScoreStr.end()));
            finalScoreText.setCharacterSize(36);
            finalScoreText.setFillColor(textColor);
            finalScoreText.setPosition(sf::Vector2f(400, 300));
            window.draw(finalScoreText);
            
            window.draw(restartButton);
            window.draw(restartText);
        } else {
            window.draw(questionText);
            window.draw(scoreText);
            window.draw(timerText);
            
            for (int i = 0; i < 4; i++) {
                window.draw(answerButtons[i]);
                window.draw(answerTexts[i]);
            }
        }
        
        window.display();
    }
    
    void drawDragAndDrop() {
        window.clear(backgroundColor);
        
        if (dragGameOver) {
            window.draw(gameOverText);
            
            std::ostringstream finalScore;
            finalScore << "Вы набрали: " << dragScore << " из " << dragTasks.size() << " очков!";
            std::string finalScoreStr = finalScore.str();
            sf::Text finalScoreText(font, sf::String::fromUtf8(finalScoreStr.begin(), finalScoreStr.end()));
            finalScoreText.setCharacterSize(36);
            finalScoreText.setFillColor(textColor);
            finalScoreText.setPosition(sf::Vector2f(400, 350));
            window.draw(finalScoreText);
            
            window.draw(restartButton);
            window.draw(restartText);
        } else {
            window.draw(scoreText);
            
            // Рисуем инструкцию
            std::string instructionStr = "Перетащите столицы к соответствующим странам";
            sf::Text instruction(font, sf::String::fromUtf8(instructionStr.begin(), instructionStr.end()));
            instruction.setCharacterSize(24);
            instruction.setFillColor(textColor);
            instruction.setPosition(sf::Vector2f(300, 100));
            window.draw(instruction);
            
            // Рисуем страны и зоны сброса (только несопоставленные)
            for (size_t i = 0; i < countryBoxes.size(); i++) {
                if (!dragTasks[i].matched) {
                    window.draw(countryBoxes[i]);
                    window.draw(countryTexts[i]);
                    window.draw(dropZones[i]);
                }
            }
            
            // Рисуем столицы (только несопоставленные, которые можно перетаскивать)
            for (size_t i = 0; i < capitalBoxes.size(); i++) {
                if (!dragTasks[i].matched) {
                    window.draw(capitalBoxes[i]);
                    window.draw(capitalTexts[i]);
                }
            }
            
            // Рисуем правильно сопоставленные столицы (в зонах сброса)
            for (size_t i = 0; i < capitalBoxes.size(); i++) {
                if (dragTasks[i].matched && capitalBoxes[i].getFillColor() == correctColor) {
                    window.draw(capitalBoxes[i]);
                    window.draw(capitalTexts[i]);
                }
            }
        }
        
        window.display();
    }
    
    void run() {
        if (!window.isOpen()) {
            std::cerr << "Ошибка: Окно не было создано!" << std::endl;
            return;
        }
        
        sf::Clock frameClock;
        while (window.isOpen()) {
            while (auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                    break;
                } else if (auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mousePressed->button == sf::Mouse::Button::Left) {
                        if (currentMode == GameMode::Menu) {
                            handleMenuClick(mousePressed->position.x, mousePressed->position.y);
                        } else if (currentMode == GameMode::Quiz) {
                            handleQuizClick(mousePressed->position.x, mousePressed->position.y);
                        } else if (currentMode == GameMode::DragAndDrop) {
                            handleDragClick(mousePressed->position.x, mousePressed->position.y);
                        }
                    }
                } else if (auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                    if (mouseReleased->button == sf::Mouse::Button::Left && currentMode == GameMode::DragAndDrop) {
                        handleDragRelease(mouseReleased->position.x, mouseReleased->position.y);
                    }
                } else if (auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                    handleMouseMove(mouseMoved->position.x, mouseMoved->position.y);
                }
            }
            
            if (!window.isOpen()) break;
            
            if (currentMode == GameMode::Quiz) {
                updateQuiz();
                drawQuiz();
            } else if (currentMode == GameMode::DragAndDrop) {
                updateDragAndDrop();
                drawDragAndDrop();
            } else {
                drawMenu();
            }
            
            sf::Time elapsed = frameClock.restart();
            if (elapsed.asMilliseconds() < 16) {
                sf::sleep(sf::milliseconds(16 - elapsed.asMilliseconds()));
            }
        }
    }
};

int main() {
    GeographyGame game;
    game.run();
    return 0;
}
