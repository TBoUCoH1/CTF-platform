package main

import (
	"fmt"
	"html/template"
	"log"
	"net/http"
	"os"
	"strings"
)

func inspectorHandler(w http.ResponseWriter, r *http.Request) {
	html := `<!DOCTYPE html>
<html>
<head>
    <title>Секретная страница</title>
    <style>
        body { font-family: Arial; background: #0a0e17; color: #fff; padding: 50px; }
        .container { max-width: 600px; margin: 0 auto; }
        h1 { color: #3b82f6; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔍 Задание: Инспектор кода</h1>
        <p>Где-то на этой странице спрятан флаг...</p>
        <p>Нажмите F12 или Ctrl+U чтобы посмотреть исходный код!</p>
        
        <!-- Флаг: CTF{inspect_the_source} -->
        
        <div style="display:none;">Здесь ничего нет</div>
    </div>
</body>
</html>`
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprint(w, html)
}

func urlDecodeHandler(w http.ResponseWriter, r *http.Request) {
	html := `<!DOCTYPE html>
<html>
<head>
    <title>URL Decode</title>
    <style>
        body { font-family: Arial; background: #0a0e17; color: #fff; padding: 50px; }
        code { background: #1e293b; padding: 10px; display: block; margin: 20px 0; }
    </style>
</head>
<body>
    <h1>🔗 Задание: URL декодирование</h1>
    <p>Декодируйте эту строку:</p>
    <code>url%5Fdecode%5Fme</code>
    <p>Оберните результат в формат CTF{...}</p>
</body>
</html>`
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprint(w, html)
}

func sqlHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method == "GET" {
		html := `<!DOCTYPE html>
<html>
<head>
    <title>SQL Login</title>
    <meta charset="utf-8">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: #0a0e17; 
            color: #fff; 
            padding: 50px; 
            margin: 0;
        }
        .container {
            max-width: 500px;
            margin: 0 auto;
        }
        h1 {
            color: #3b82f6;
            margin-bottom: 20px;
        }
        p {
            color: #cbd5e1;
            margin-bottom: 25px;
            line-height: 1.5;
        }
        form {
            background: #111827;
            padding: 30px;
            border-radius: 12px;
            border: 1px solid #1e293b;
            margin-bottom: 25px;
        }
        input {
            width: 100%;
            padding: 12px;
            margin: 10px 0;
            font-size: 16px;
            background: #1e293b;
            border: 1px solid #334155;
            border-radius: 8px;
            color: #fff;
            box-sizing: border-box;
        }
        input:focus {
            outline: none;
            border-color: #3b82f6;
        }
        button {
            width: 100%;
            padding: 12px;
            margin-top: 15px;
            font-size: 16px;
            font-weight: 600;
            background: #3b82f6;
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: background 0.2s;
        }
        button:hover {
            background: #2563eb;
        }
        .hint {
            background: #1e293b;
            padding: 20px;
            border-radius: 8px;
            border: 1px solid #334155;
            margin-top: 20px;
        }
        .hint p {
            color: #64748b;
            font-size: 13px;
            margin: 0 0 10px 0;
        }
        code {
            background: #0f172a;
            padding: 8px 12px;
            display: block;
            border-radius: 6px;
            font-family: 'Courier New', monospace;
            font-size: 13px;
            color: #60a5fa;
            margin-top: 10px;
            overflow-x: auto;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔐 SQL Injection Challenge</h1>
        <p>Попробуйте войти как admin без знания пароля!</p>
        
        <form method="POST">
            <input type="text" name="username" placeholder="Username" required>
            <input type="password" name="password" placeholder="Password" required>
            <button type="submit">Login</button>
        </form>

        <div class="hint">
            <p>💡 Подсказка: SQL запрос выглядит так:</p>
            <code>SELECT * FROM users WHERE username='user' AND password='pass'</code>
        </div>
    </div>
</body>
</html>`
		w.Header().Set("Content-Type", "text/html; charset=utf-8")
		fmt.Fprint(w, html)
		return
	}

	r.ParseForm()
	username := r.FormValue("username")
	password := r.FormValue("password")

	if strings.Contains(username, "'") || strings.Contains(username, "--") {
		if strings.Contains(strings.ToLower(username), "admin") {
			html := `<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <style>
        body { font-family: Arial; background: #0a0e17; color: #fff; padding: 50px; text-align: center; }
        h1 { color: #22c55e; font-size: 48px; margin-bottom: 20px; }
        p { font-size: 20px; margin: 15px 0; }
        strong { color: #3b82f6; }
        a { color: #3b82f6; text-decoration: none; font-weight: bold; margin-top: 30px; display: inline-block; }
        a:hover { color: #60a5fa; }
    </style>
</head>
<body>
    <h1>✅ Success!</h1>
    <p>Logged in as: <strong>admin</strong></p>
    <p>Флаг: <strong>CTF{sql_injection_basic}</strong></p>
    <a href="/challenge/sql">← Вернуться назад</a>
</body>
</html>`
			fmt.Fprint(w, html)
			return
		}
	}

	if username == "admin" && password == "super_secret_password" {
		fmt.Fprint(w, "<h1>Welcome, admin!</h1>")
		return
	}

	html := `<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <style>
        body { font-family: Arial; background: #0a0e17; color: #fff; padding: 50px; text-align: center; }
        h1 { color: #ef4444; }
        a { color: #3b82f6; text-decoration: none; font-weight: bold; margin-top: 20px; display: inline-block; }
    </style>
</head>
<body>
    <h1>❌ Invalid credentials</h1>
    <a href="/challenge/sql">Try again</a>
</body>
</html>`
	fmt.Fprint(w, html)
}

func cookieHandler(w http.ResponseWriter, r *http.Request) {
	if r.URL.Query().Get("reset") == "1" {
		http.SetCookie(w, &http.Cookie{
			Name:   "role",
			Value:  "user",
			Path:   "/",
			MaxAge: 3600,
		})
		http.Redirect(w, r, "/challenge/cookie", http.StatusFound)
		return
	}

	cookie, err := r.Cookie("role")
	role := "user"

	if err != nil {
		http.SetCookie(w, &http.Cookie{
			Name:   "role",
			Value:  "user",
			Path:   "/",
			MaxAge: 3600,
		})
	} else {
		role = cookie.Value
	}

	w.Header().Set("Content-Type", "text/html; charset=utf-8")

	if role == "admin" {
		html := `<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Admin Panel</title>
    <style>
        body { 
            font-family: Arial; 
            background: #0a0e17; 
            color: #fff; 
            padding: 50px; 
            text-align: center;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: #111827;
            padding: 40px;
            border-radius: 12px;
            border: 1px solid #1e293b;
        }
        h1 { color: #22c55e; margin-bottom: 20px; }
        p { font-size: 18px; margin: 15px 0; }
        .flag { 
            background: #1e293b; 
            padding: 15px; 
            border-radius: 8px; 
            font-family: monospace;
            font-size: 20px;
            color: #3b82f6;
            margin: 20px 0;
        }
        .reset-btn {
            display: inline-block;
            margin-top: 20px;
            padding: 12px 24px;
            background: #ef4444;
            color: white;
            text-decoration: none;
            border-radius: 8px;
            font-weight: 600;
            transition: background 0.2s;
        }
        .reset-btn:hover {
            background: #dc2626;
        }
        .info {
            color: #64748b;
            font-size: 14px;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>✅ Admin Panel</h1>
        <p>Добро пожаловать, администратор!</p>
        <p>Вы успешно получили доступ к админ-панели</p>
        
        <div class="flag">CTF{cookie_manipulation}</div>
        
        <p class="info">Текущий cookie: <code>role=admin</code></p>
        
        <a href="/challenge/cookie?reset=1" class="reset-btn">🔄 Сбросить и попробовать снова</a>
    </div>
</body>
</html>`
		fmt.Fprint(w, html)
	} else {
		html := `<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Cookie Challenge</title>
    <style>
        body { 
            font-family: Arial; 
            background: #0a0e17; 
            color: #fff; 
            padding: 50px;
        }
        .container {
            max-width: 700px;
            margin: 0 auto;
        }
        h1 { 
            color: #3b82f6; 
            margin-bottom: 20px;
        }
        .card {
            background: #111827;
            padding: 30px;
            border-radius: 12px;
            border: 1px solid #1e293b;
            margin-bottom: 20px;
        }
        .status {
            background: #1e293b;
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
        }
        code {
            background: #0f172a;
            padding: 3px 8px;
            border-radius: 4px;
            color: #60a5fa;
            font-family: monospace;
        }
        .instruction {
            background: #1e293b;
            padding: 20px;
            border-radius: 8px;
            border-left: 4px solid #3b82f6;
            margin-top: 20px;
        }
        .instruction h3 {
            color: #3b82f6;
            margin-top: 0;
        }
        .instruction ol {
            margin: 10px 0;
            padding-left: 25px;
            line-height: 1.8;
        }
        .instruction li {
            margin: 8px 0;
        }
        .code-block {
            background: #0f172a;
            padding: 12px;
            border-radius: 6px;
            margin: 10px 0;
            font-family: monospace;
            font-size: 13px;
            color: #22c55e;
            overflow-x: auto;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🍪 Cookie Manipulation</h1>
        
        <div class="card">
            <h2>Статус доступа</h2>
            <div class="status">
                <p>Ваша текущая роль: <code>user</code></p>
                <p style="color: #ef4444;">❌ Доступ запрещен. Требуется роль <code>admin</code></p>
            </div>
            
            <p style="color: #94a3b8;">Измените значение cookie <code>role</code> на <code>admin</code> чтобы получить доступ к админ-панели!</p>
        </div>

        <div class="instruction">
            <h3>💡 Как решить:</h3>
            <ol>
                <li>Нажмите <strong>F12</strong> (открыть DevTools)</li>
                <li>Перейдите на вкладку <strong>Console</strong></li>
                <li>Выполните команду:</li>
            </ol>
            <div class="code-block">document.cookie = "role=admin; path=/"; location.reload();</div>
            <p style="color: #64748b; font-size: 13px; margin-top: 15px;">
                Или используйте вкладку <strong>Application → Cookies</strong> и измените значение вручную
            </p>
        </div>
    </div>
</body>
</html>`
		fmt.Fprint(w, html)
	}
}

func filesHandler(w http.ResponseWriter, r *http.Request) {
	filename := r.URL.Query().Get("name")
	if filename == "" {
		filename = "document.txt"
	}

	os.MkdirAll("challenge_files", 0755)
	os.WriteFile("challenge_files/document.txt", []byte("This is a public document."), 0644)
	os.WriteFile("challenge_files/flag.txt", []byte("CTF{directory_traversal}"), 0644)

	if strings.Contains(filename, "..") && strings.Count(filename, "..") > 2 {
		fmt.Fprint(w, "Access denied!")
		return
	}

	content, err := os.ReadFile("challenge_files/" + filename)
	if err != nil {
		html := fmt.Sprintf(`<html>
<head><style>body { font-family: Arial; background: #0a0e17; color: #fff; padding: 50px; }</style></head>
<body>
    <h1>❌ Error</h1>
    <p>File not found: %s</p>
    <a href="/challenge/files">Back</a>
</body>
</html>`, filename)
		fmt.Fprint(w, html)
		return
	}

	html := fmt.Sprintf(`<!DOCTYPE html>
<html>
<head>
    <style>
        body { font-family: Arial; background: #0a0e17; color: #fff; padding: 50px; }
        pre { background: #1e293b; padding: 20px; }
    </style>
</head>
<body>
    <h1>📁 File Viewer</h1>
    <p>Reading: %s</p>
    <pre>%s</pre>
    <p><a href="/challenge/files?name=document.txt">document.txt</a></p>
    <p style="color: #64748b;">Попробуйте получить доступ к flag.txt...</p>
</body>
</html>`, filename, template.HTMLEscapeString(string(content)))

	fmt.Fprint(w, html)
}

func indexHandler(w http.ResponseWriter, r *http.Request) {
	html := `<!DOCTYPE html>
<html>
<head>
    <title>CTF Web Challenges</title>
    <style>
        body { font-family: Arial; background: #0a0e17; color: #fff; padding: 50px; }
        .container { max-width: 800px; margin: 0 auto; }
        h1 { color: #3b82f6; }
        .challenge { background: #111827; padding: 20px; margin: 15px 0; border-radius: 10px; }
        .challenge:hover { background: #1e293b; }
        a { color: #3b82f6; text-decoration: none; font-weight: bold; }
        a:hover { color: #60a5fa; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 CTF Web Challenges</h1>
        <p>Доступные задания:</p>
        
        <div class="challenge">
            <h3>🔍 Инспектор кода (100 pts)</h3>
            <p>Найдите скрытый флаг в HTML коде</p>
            <a href="/challenge/inspector">Открыть →</a>
        </div>
        
        <div class="challenge">
            <h3>🔗 URL декодирование (75 pts)</h3>
            <p>Декодируйте URL-encoded строку</p>
            <a href="/challenge/urldecode">Открыть →</a>
        </div>
        
        <div class="challenge">
            <h3>🔐 SQL Injection (150 pts)</h3>
            <p>Войдите как admin используя SQL injection</p>
            <a href="/challenge/sql">Открыть →</a>
        </div>
        
        <div class="challenge">
            <h3>🍪 Cookie Manipulation (150 pts)</h3>
            <p>Измените cookie для получения прав администратора</p>
            <a href="/challenge/cookie">Открыть →</a>
        </div>
        
        <div class="challenge">
            <h3>📁 Directory Traversal (175 pts)</h3>
            <p>Получите доступ к файлу flag.txt</p>
            <a href="/challenge/files">Открыть →</a>
        </div>
    </div>
</body>
</html>`
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprint(w, html)
}

func main() {
	http.HandleFunc("/", indexHandler)
	http.HandleFunc("/challenge/inspector", inspectorHandler)
	http.HandleFunc("/challenge/urldecode", urlDecodeHandler)
	http.HandleFunc("/challenge/sql", sqlHandler)
	http.HandleFunc("/challenge/cookie", cookieHandler)
	http.HandleFunc("/challenge/files", filesHandler)

	fmt.Println("CTF Web Server starting...")
	fmt.Println("http://localhost:5000")
	log.Fatal(http.ListenAndServe("0.0.0.0:5000", nil))
}
