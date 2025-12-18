-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Таблица задач
CREATE TABLE IF NOT EXISTS tasks (
    id SERIAL PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    description TEXT,
    hint TEXT,
    flag VARCHAR(100) NOT NULL,
    points INT NOT NULL,
    category VARCHAR(50) DEFAULT 'General',
    display_order INT DEFAULT 0
);

-- Таблица решений
CREATE TABLE IF NOT EXISTS solved_tasks (
    user_id INT REFERENCES users(id),
    task_id INT REFERENCES tasks(id),
    solved_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, task_id)
);

-- ============================================================
-- CTF ЗАДАНИЯ (22 задания)
-- ============================================================
INSERT INTO tasks (title, description, hint, flag, points, category) VALUES

-- ЛЕГКИЕ (50-100 баллов)
('Первые шаги',
'Добро пожаловать в CTF! Флаг находится прямо перед вами.

Формат флага: CTF{текст}

Ответ: welcome_to_ctf_2025',
'Флаг написан прямо в описании! Оберните ответ в формат CTF{...}',
'CTF{welcome_to_ctf_2025}', 50, 'Intro'),

('Base64 декодирование',
'Мы перехватили сообщение:

YmFzZTY0X2lzX2Vhc3k=

Похоже, оно закодировано в Base64. Декодируйте его и оберните в формат CTF{...}',
'Используйте CyberChef (gchq.github.io/CyberChef) или команду: echo "YmFzZTY0X2lzX2Vhc3k=" | base64 -d',
'CTF{base64_is_easy}', 75, 'Crypto'),

('ROT13 шифр',
'Античный шифр Цезаря со сдвигом 13:

ebg13_vf_fvzcyr

Расшифруйте и оберните в формат CTF{...}',
'ROT13 - каждая буква заменяется буквой через 13 позиций. A→N, B→O и т.д. Используйте rot13.com',
'CTF{rot13_is_simple}', 75, 'Crypto'),

('Инспектор кода',
'На веб-странице есть скрытый комментарий в HTML!

🌐 Откройте: http://localhost:5000/challenge/inspector

Нажмите F12 или Ctrl+U чтобы посмотреть исходный код страницы.',
'F12 откроет DevTools. Ищите HTML-комментарий вида <!-- ... -->',
'CTF{inspect_the_source}', 100, 'Web'),

('Hex декодирование',
'Найден файл с hex строкой:

6865785f746f5f6173636969

Это hex представление ASCII текста. Декодируйте и оберните в CTF{...}',
'В hex каждые 2 символа = 1 ASCII символ. 68=h, 65=e, 78=x...
Используйте CyberChef "From Hex" или: echo "6865785f746f5f6173636969" | xxd -r -p',
'CTF{hex_to_ascii}', 100, 'Crypto'),

('ASCII коды',
'Перехвачена последовательность ASCII кодов:

97 115 99 105 105 95 99 111 100 101 115

Преобразуйте каждое число в символ и оберните в CTF{...}',
'Это десятичные ASCII коды. 97=a, 115=s, 99=c...
В ASCII таблице или Python: chr(97)',
'CTF{ascii_codes}', 100, 'Crypto'),

('URL декодирование',
'Декодируйте URL-encoded строку:

🌐 Откройте: http://localhost:5000/challenge/urldecode

url%5Fdecode%5Fme

Оберните результат в формат CTF{...}',
'URL encoding: %5F это символ _. Используйте URL decoder онлайн',
'CTF{url_decode_me}', 75, 'Web'),

('Бинарный код',
'Сообщение в двоичной системе:

01100010 01101001 01101110 01100001 01110010 01111001

Переведите каждый байт в ASCII и оберните результат "binary" в CTF{...}',
'Каждые 8 бит = 1 ASCII символ. 01100010 = 98 = "b". CyberChef "From Binary"',
'CTF{binary}', 100, 'Crypto'),

('Morse код',
'Перехвачен сигнал:

-- --- .-. ... .

Декодируйте азбуку Морзе и оберните результат "morse" в CTF{...}',
'-- = M, --- = O, .-. = R, ... = S, . = E. Используйте morsedecoder.com',
'CTF{morse}', 100, 'Crypto'),

('Обратная строка',
'Строка выглядит странно:

esrever_gnirts

Попробуйте прочитать её задом наперёд. Оберните в CTF{...}',
'Переверните строку. В Python: text[::-1]',
'CTF{string_reverse}', 75, 'Crypto'),

-- СРЕДНИЕ (125-200 баллов)
('SQL Injection базовый',
'Найдена уязвимая форма логина!

🌐 Откройте: http://localhost:5000/challenge/sql

SQL запрос: SELECT * FROM users WHERE username=''USER'' AND password=''PASS''

Войдите как admin без знания пароля!',
'SQL комментарий в SQL это --. Попробуйте username: admin''--',
'CTF{sql_injection_basic}', 150, 'Web'),

('Caesar brute force',
'Шифр Цезаря с неизвестным сдвигом:

fdhvduflskhueuxwh

Сдвиг от 1 до 26. Попробуйте все варианты! Оберните результат в CTF{...}',
'Перебирайте сдвиги от 1 до 25. ROT3 → caesar... CyberChef ROT13 Brute Force',
'CTF{caesar_cipher_brute}', 150, 'Crypto'),

('Cookie manipulation',
'На сайте установлен cookie role=user.

🌐 Откройте: http://localhost:5000/challenge/cookie

Измените cookie на role=admin чтобы получить доступ!',
'DevTools (F12) → Console → выполните:
document.cookie = "role=admin; path=/"; location.reload();',
'CTF{cookie_manipulation}', 150, 'Web'),

('XOR шифр',
'Данные зашифрованы XOR с ключом "KEY". Hex ciphertext:

332a2b1426303b2d3c39

Расшифруйте и оберните в CTF{...}',
'XOR: ciphertext ⊕ KEY = plaintext. CyberChef: 1) From Hex 2) XOR (key="KEY", UTF8)',
'CTF{xor_cipher}', 175, 'Crypto'),

('MD5 rainbow table',
'Найден MD5 хеш:

5f4dcc3b5aa765d61d8327deb882cf99

Используйте rainbow table для взлома! Оберните результат в CTF{...}',
'Используйте crackstation.net, md5decrypt.net или hashcat. Это популярный MD5!',
'CTF{password}', 175, 'Crypto'),

('Directory Traversal',
'Найден file viewer API:

🌐 Откройте: http://localhost:5000/challenge/files

GET /challenge/files?name=document.txt

Получите доступ к flag.txt используя path traversal!',
'Попробуйте ?name=flag.txt или ?name=../flag.txt',
'CTF{directory_traversal}', 175, 'Web'),

('Frequency analysis',
'Substitution cipher (каждая буква заменена другой):

ugsakfapereakofdv

Используйте частотный анализ. Результат: frequencyanalysis, оберните в CTF{...}',
'В английском самая частая буква - E. Используйте quipqiup.com',
'CTF{frequency_analysis}', 150, 'Crypto'),

('RGB стеганография',
'В изображении спрятано сообщение в RGB значениях:

Pixel 1: R=115, G=116, B=101
Pixel 2: R=103, G=111, B=95
Pixel 3: R=114, G=103, B=98

Конвертируйте RGB в ASCII и оберните результат в CTF{...}',
'R, G, B - это ASCII коды. R=115=s, G=116=t, B=101=e...',
'CTF{stego_rgb}', 125, 'Stego'),

-- СЛОЖНЫЕ (250-300 баллов)
('Vigenère cipher',
'Шифр Виженера (полиалфавитная подстановка):

xqvlrvtmryetmms...

Ключ: 6 букв. Подсказка: начинается с C.
Результат: vigenerecracked, оберните в CTF{...}',
'1. Определите длину ключа (6)
2. Частотный анализ для каждой позиции
3. Ключ: CIPHER. Используйте dcode.fr/vigenere-cipher',
'CTF{vigenere_cracked}', 250, 'Crypto'),

('RSA слабый модуль',
'RSA шифрование с маленьким модулем n=3233, e=17.

📥 Скачайте файл с зашифрованным сообщением через кнопку "Скачать файл"

Факторизуйте n и расшифруйте! Результат в ASCII, оберните в CTF{...}',
'1. Факторизуйте n=3233 (p=61, q=53)
2. φ(n)=(p-1)*(q-1)=3120
3. Найдите d: d*e ≡ 1 (mod φ) → d=2753
4. m = c^d mod n для каждого числа
5. Конвертируйте в ASCII',
'CTF{rsa_weak}', 300, 'Crypto'),

('Polyglot файл',
'📥 Скачайте suspicious.png через кнопку "Скачать файл"

Файл выглядит как PNG, но внутри спрятан архив!

Извлеките скрытые данные. Формат флага: CTF{...}',
'1. Polyglot = PNG + ZIP
2. PNG начинается с 89 50 4E 47
3. ZIP начинается с 50 4B 03 04
4. Попробуйте: unzip suspicious.png или 7z x suspicious.png или binwalk -e suspicious.png
5. Проверьте: file image.png → PNG, затем unzip -l image.png',
'CTF{polyglot_file_format}', 300, 'Forensics');

-- Индексы для производительности
CREATE INDEX IF NOT EXISTS idx_solved_tasks_user_id ON solved_tasks(user_id);
CREATE INDEX IF NOT EXISTS idx_solved_tasks_task_id ON solved_tasks(task_id);
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);