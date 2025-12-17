package main

import (
	"database/sql"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
	"github.com/joho/godotenv"
	_ "github.com/lib/pq"
	"golang.org/x/crypto/bcrypt"
)

var db *sql.DB
var jwtSecret []byte

// ============== ИНИЦИАЛИЗАЦИЯ ==============
func init() {
	godotenv.Load()

	secretKey := os.Getenv("JWT_SECRET")
	if secretKey == "" {
		log.Fatal("JWT_SECRET environment variable is not set")
	}

	if len(secretKey) < 32 {
		log.Println("⚠️ WARNING: JWT_SECRET is too short! Use at least 32 characters.")
	}

	jwtSecret = []byte(secretKey)

	var err error
	connStr := fmt.Sprintf(
		"user=%s password=%s host=%s port=%s dbname=%s sslmode=disable",
		os.Getenv("DB_USER"),
		os.Getenv("DB_PASSWORD"),
		os.Getenv("DB_HOST"),
		os.Getenv("DB_PORT"),
		os.Getenv("DB_NAME"),
	)

	db, err = sql.Open("postgres", connStr)
	if err != nil {
		log.Fatal("Cannot connect to database:", err)
	}

	if err = db.Ping(); err != nil {
		log.Fatal("Database ping failed:", err)
	}

	db.SetMaxOpenConns(25)
	db.SetMaxIdleConns(5)
	db.SetConnMaxLifetime(5 * time.Minute)

	log.Println("✅ Database connected successfully!")
}

// ============== ВАЛИДАЦИЯ ПАРОЛЯ ==============
func validatePassword(password string) error {
	if len(password) < 8 {
		return fmt.Errorf("Пароль должен быть минимум 8 символов")
	}

	hasDigit := regexp.MustCompile(`[0-9]`).MatchString(password)
	if !hasDigit {
		return fmt.Errorf("Пароль должен содержать хотя бы одну цифру")
	}

	hasLetter := regexp.MustCompile(`[a-zA-Z]`).MatchString(password)
	if !hasLetter {
		return fmt.Errorf("Пароль должен содержать хотя бы одну букву")
	}

	return nil
}

// ============== ВАЛИДАЦИЯ USERNAME ==============
func validateUsername(username string) error {
	if len(username) < 3 {
		return fmt.Errorf("Имя пользователя должно быть минимум 3 символа")
	}

	if len(username) > 50 {
		return fmt.Errorf("Имя пользователя не должно превышать 50 символов")
	}

	validUsername := regexp.MustCompile(`^[a-zA-Z0-9_]+$`).MatchString(username)
	if !validUsername {
		return fmt.Errorf("Имя пользователя может содержать только буквы, цифры и подчеркивание")
	}

	return nil
}

// ============== РЕГИСТРАЦИЯ ==============
func register(c *gin.Context) {
	var req struct {
		Username string `json:"username" binding:"required"`
		Password string `json:"password" binding:"required"`
	}

	if err := c.BindJSON(&req); err != nil {
		c.JSON(400, gin.H{"error": "Неверный формат запроса"})
		return
	}

	if err := validateUsername(req.Username); err != nil {
		c.JSON(400, gin.H{"error": err.Error()})
		return
	}

	if err := validatePassword(req.Password); err != nil {
		c.JSON(400, gin.H{"error": err.Error()})
		return
	}

	var exists bool
	err := db.QueryRow("SELECT EXISTS(SELECT 1 FROM users WHERE username = $1)", req.Username).Scan(&exists)
	if err == nil && exists {
		log.Printf("⚠️ Registration attempt for existing user: %s from IP: %s", req.Username, c.ClientIP())
		c.JSON(409, gin.H{"error": "Пользователь уже существует"})
		return
	}

	hashedPassword, err := bcrypt.GenerateFromPassword([]byte(req.Password), 12)
	if err != nil {
		log.Println("Error hashing password:", err)
		c.JSON(500, gin.H{"error": "Ошибка сервера"})
		return
	}

	_, err = db.Exec(
		"INSERT INTO users (username, password_hash, created_at) VALUES ($1, $2, NOW())",
		req.Username,
		string(hashedPassword),
	)

	if err != nil {
		log.Println("Error inserting user:", err)
		c.JSON(500, gin.H{"error": "Ошибка базы данных"})
		return
	}

	log.Printf("✅ New user registered: %s from IP: %s", req.Username, c.ClientIP())
	c.JSON(200, gin.H{"message": "Регистрация успешна"})
}

// ============== ЛОГИН ==============
func login(c *gin.Context) {
	var req struct {
		Username string `json:"username" binding:"required"`
		Password string `json:"password" binding:"required"`
	}

	if err := c.BindJSON(&req); err != nil {
		c.JSON(400, gin.H{"error": "Неверный формат запроса"})
		return
	}

	var userID int
	var passwordHash string
	err := db.QueryRow(
		"SELECT id, password_hash FROM users WHERE username = $1",
		req.Username,
	).Scan(&userID, &passwordHash)

	if err != nil {
		log.Printf("⚠️ Failed login attempt for non-existent user: %s from IP: %s", req.Username, c.ClientIP())
		c.JSON(401, gin.H{"error": "Неверное Имя пользователя или Пароль"})
		return
	}

	err = bcrypt.CompareHashAndPassword([]byte(passwordHash), []byte(req.Password))
	if err != nil {
		log.Printf("⚠️ Failed login attempt (wrong password) for user: %s from IP: %s", req.Username, c.ClientIP())
		c.JSON(401, gin.H{"error": "Неверное Имя пользователя или Пароль"})
		return
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, jwt.MapClaims{
		"user_id":  userID,
		"username": req.Username,
		"exp":      time.Now().Add(24 * time.Hour).Unix(),
		"iat":      time.Now().Unix(),
	})

	tokenString, err := token.SignedString(jwtSecret)
	if err != nil {
		log.Println("Error creating token:", err)
		c.JSON(500, gin.H{"error": "Ошибка создания токена"})
		return
	}

	log.Printf("✅ Successful login for user: %s from IP: %s", req.Username, c.ClientIP())
	c.JSON(200, gin.H{"token": tokenString})
}

// ============== MIDDLEWARE ==============
func authMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		authHeader := c.GetHeader("Authorization")
		if authHeader == "" {
			c.JSON(401, gin.H{"error": "Отсутствует токен авторизации"})
			c.Abort()
			return
		}

		token, err := jwt.Parse(authHeader, func(token *jwt.Token) (any, error) {
			if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, fmt.Errorf("неожиданный метод подписи")
			}
			return jwtSecret, nil
		})

		if err != nil || !token.Valid {
			log.Printf("⚠️ Invalid token attempt from IP: %s", c.ClientIP())
			c.JSON(401, gin.H{"error": "Невалидный токен"})
			c.Abort()
			return
		}

		claims, ok := token.Claims.(jwt.MapClaims)
		if !ok {
			c.JSON(401, gin.H{"error": "Невалидные claims токена"})
			c.Abort()
			return
		}

		c.Set("claims", &claims)
		c.Next()
	}
}

// ============== ПОЛУЧИТЬ ЗАДАЧИ ==============
func getTasks(c *gin.Context) {
	claims := c.MustGet("claims").(*jwt.MapClaims)
	userID := int((*claims)["user_id"].(float64))

	rows, err := db.Query(`
		SELECT id, title, description, hint, points,
		EXISTS(SELECT 1 FROM solved_tasks WHERE user_id = $1 AND task_id = tasks.id) as is_solved
		FROM tasks
		ORDER BY id ASC
	`, userID)

	if err != nil {
		log.Println("Database error:", err)
		c.JSON(500, gin.H{"error": "Ошибка базы данных"})
		return
	}
	defer rows.Close()

	type Task struct {
		ID          int    `json:"id"`
		Title       string `json:"title"`
		Description string `json:"description"`
		Hint        string `json:"hint"`
		Points      int    `json:"points"`
		IsSolved    bool   `json:"is_solved"`
	}

	tasks := make([]Task, 0, 20)
	for rows.Next() {
		var task Task
		if err := rows.Scan(&task.ID, &task.Title, &task.Description, &task.Hint, &task.Points, &task.IsSolved); err != nil {
			continue
		}
		tasks = append(tasks, task)
	}

	if err := rows.Err(); err != nil {
		c.JSON(500, gin.H{"error": "Ошибка чтения задач"})
		return
	}

	if tasks == nil {
		tasks = []Task{}
	}

	c.JSON(200, tasks)
}

// ============== СДАТЬ ФЛАГ ==============
func submitFlag(c *gin.Context) {
	claims := c.MustGet("claims").(*jwt.MapClaims)
	userID := int((*claims)["user_id"].(float64))
	username := (*claims)["username"].(string)

	var req struct {
		TaskID int    `json:"task_id" binding:"required"`
		Flag   string `json:"flag" binding:"required"`
	}

	if err := c.BindJSON(&req); err != nil {
		c.JSON(400, gin.H{"error": "Неверный формат запроса"})
		return
	}

	var correctFlag string
	var points int
	err := db.QueryRow(
		"SELECT flag, points FROM tasks WHERE id = $1",
		req.TaskID,
	).Scan(&correctFlag, &points)

	if err != nil {
		c.JSON(404, gin.H{"error": "Задание не найдено"})
		return
	}

	if req.Flag != correctFlag {
		log.Printf("⚠️ Wrong flag submitted by user: %s for task: %d from IP: %s", username, req.TaskID, c.ClientIP())
		c.JSON(200, gin.H{"result": "wrong"})
		return
	}

	var alreadySolved bool
	err = db.QueryRow(
		"SELECT EXISTS(SELECT 1 FROM solved_tasks WHERE user_id = $1 AND task_id = $2)",
		userID,
		req.TaskID,
	).Scan(&alreadySolved)

	if alreadySolved {
		c.JSON(200, gin.H{"result": "already_solved", "points": 0})
		return
	}

	_, err = db.Exec(
		"INSERT INTO solved_tasks (user_id, task_id, solved_at) VALUES ($1, $2, NOW())",
		userID,
		req.TaskID,
	)

	if err != nil {
		log.Println("Error saving solution:", err)
		c.JSON(500, gin.H{"error": "Ошибка сохранения решения"})
		return
	}

	log.Printf("✅ Task %d solved by user: %s (%d points) from IP: %s", req.TaskID, username, points, c.ClientIP())
	c.JSON(200, gin.H{"result": "correct", "points": points})
}

// ============== ТАБЛИЦА ЛИДЕРОВ ==============
func getLeaderboard(c *gin.Context) {
	rows, err := db.Query(`
		SELECT u.username, COALESCE(SUM(t.points), 0) as score, COUNT(st.task_id) as solved_count
		FROM users u
		LEFT JOIN solved_tasks st ON u.id = st.user_id
		LEFT JOIN tasks t ON st.task_id = t.id
		GROUP BY u.id, u.username
		ORDER BY score DESC, u.username ASC
		LIMIT 20
	`)

	if err != nil {
		log.Println("Database error:", err)
		c.JSON(500, gin.H{"error": "Ошибка базы данных"})
		return
	}
	defer rows.Close()

	type Leader struct {
		Username    string `json:"username"`
		Score       int    `json:"score"`
		SolvedCount int    `json:"solved_count"`
	}

	leaders := make([]Leader, 0, 20)
	for rows.Next() {
		var leader Leader
		if err := rows.Scan(&leader.Username, &leader.Score, &leader.SolvedCount); err != nil {
			log.Println("Scan error:", err)
			continue
		}
		leaders = append(leaders, leader)
	}

	if leaders == nil {
		leaders = []Leader{}
	}

	c.JSON(200, leaders)
}

// ============== ИСТОРИЯ РЕШЕНИЙ ==============
func getHistory(c *gin.Context) {
	claims := c.MustGet("claims").(*jwt.MapClaims)
	username := (*claims)["username"].(string)

	rows, err := db.Query(`
		SELECT t.id, t.title, t.points, st.solved_at
		FROM solved_tasks st
		JOIN users u ON st.user_id = u.id
		JOIN tasks t ON st.task_id = t.id
		WHERE u.username = $1
		ORDER BY st.solved_at DESC
	`, username)

	if err != nil {
		log.Println("Database error:", err)
		c.JSON(500, gin.H{"error": "Ошибка базы данных"})
		return
	}
	defer rows.Close()

	type HistoryEntry struct {
		TaskID    int    `json:"task_id"`
		TaskTitle string `json:"task_title"`
		Points    int    `json:"points"`
		SolvedAt  string `json:"solved_at"`
	}

	history := make([]HistoryEntry, 0, 50)
	for rows.Next() {
		var entry HistoryEntry
		if err := rows.Scan(&entry.TaskID, &entry.TaskTitle, &entry.Points, &entry.SolvedAt); err != nil {
			log.Println("Scan error:", err)
			continue
		}
		history = append(history, entry)
	}

	if history == nil {
		history = []HistoryEntry{}
	}

	c.JSON(200, history)
}

// ============== СКАЧИВАНИЕ ОТЧЕТА ==============
func downloadReport(c *gin.Context) {
	claims := c.MustGet("claims").(*jwt.MapClaims)
	username := (*claims)["username"].(string)

	rows, err := db.Query(`
		SELECT t.title, t.points, st.solved_at
		FROM solved_tasks st
		JOIN users u ON st.user_id = u.id
		JOIN tasks t ON st.task_id = t.id
		WHERE u.username = $1
		ORDER BY st.solved_at ASC
	`, username)

	if err != nil {
		log.Println("Database error:", err)
		c.JSON(500, gin.H{"error": "Ошибка базы данных"})
		return
	}
	defer rows.Close()

	var csvBuilder strings.Builder
	csvBuilder.WriteString("Задача,Очки,Дата решения\n")

	totalPoints := 0
	for rows.Next() {
		var title string
		var points int
		var solvedAt string
		if err := rows.Scan(&title, &points, &solvedAt); err != nil {
			log.Println("Scan error:", err)
			continue
		}
		totalPoints += points
		title = fmt.Sprintf(`"%s"`, title)
		csvBuilder.WriteString(fmt.Sprintf("%s,%d,%s\n", title, points, solvedAt))
	}

	csvBuilder.WriteString(fmt.Sprintf("\nИтого,%d,\n", totalPoints))

	c.Header("Content-Disposition", fmt.Sprintf("attachment; filename=ctf_report_%s.csv", username))
	c.Header("Content-Type", "text/csv; charset=utf-8")
	c.String(200, csvBuilder.String())
}

// ============== СКАЧИВАНИЕ ФАЙЛОВ ЗАДАНИЙ ==============
func downloadFile(c *gin.Context) {
	taskIDStr := c.Param("id")
	taskID, err := strconv.Atoi(taskIDStr)
	if err != nil {
		c.JSON(400, gin.H{"error": "Неверный ID задания"})
		return
	}

	fileMap := map[int]struct {
		filename    string
		contentType string
	}{
		20: {"rsa_challenge.txt", "text/plain"},
		21: {"suspicious.png", "image/png"},
	}

	fileInfo, exists := fileMap[taskID]
	if !exists {
		c.JSON(404, gin.H{"error": "Для этого задания нет файла"})
		return
	}

	possiblePaths := []string{
		filepath.Join("files", fileInfo.filename),
		filepath.Join("backend", "files", fileInfo.filename),
		filepath.Join("..", "files", fileInfo.filename),
	}

	var data []byte
	for _, path := range possiblePaths {
		if _, err := os.Stat(path); err == nil {
			data, err = os.ReadFile(path)
			if err == nil {
				log.Printf("✅ File found at: %s", path)
				break
			}
		}
	}

	if data == nil {
		log.Printf("❌ File not found. Tried paths: %v", possiblePaths)
		c.JSON(500, gin.H{"error": "Файл не найден на сервере"})
		return
	}

	c.Header("Content-Disposition", fmt.Sprintf("attachment; filename=%s", fileInfo.filename))
	c.Data(200, fileInfo.contentType, data)
}

// ============== ПРОФИЛЬ ==============
func getProfileEnhanced(c *gin.Context) {
	claims := c.MustGet("claims").(*jwt.MapClaims)
	username := (*claims)["username"].(string)
	userID := int((*claims)["user_id"].(float64))

	var score, solvedCount, totalTasks int
	var registrationDate string

	err := db.QueryRow(`
		SELECT COALESCE(SUM(t.points), 0) as score, COUNT(st.task_id) as solved_count
		FROM solved_tasks st
		LEFT JOIN tasks t ON st.task_id = t.id
		WHERE st.user_id = $1
	`, userID).Scan(&score, &solvedCount)

	if err != nil {
		log.Println("Error getting score:", err)
	}

	err = db.QueryRow("SELECT COUNT(*) FROM tasks").Scan(&totalTasks)
	if err != nil {
		log.Println("Error getting total tasks:", err)
	}

	err = db.QueryRow("SELECT created_at FROM users WHERE id = $1", userID).Scan(&registrationDate)
	if err != nil {
		log.Println("Error getting registration date:", err)
		registrationDate = "Unknown"
	}

	level := 1 + solvedCount/3
	if level > 10 {
		level = 10
	}

	c.JSON(200, gin.H{
		"username":          username,
		"score":             score,
		"level":             level,
		"solved_count":      solvedCount,
		"total_tasks":       totalTasks,
		"registration_date": registrationDate,
		"progress":          float64(solvedCount) / float64(totalTasks) * 100,
	})
}

// ============== MAIN ==============
func main() {
	defer func() {
		if err := db.Close(); err != nil {
			log.Println("Error closing database:", err)
		}
	}()

	gin.SetMode(gin.ReleaseMode)
	router := gin.Default()

	// CORS
	router.Use(func(c *gin.Context) {
		origin := c.Request.Header.Get("Origin")

		allowedOrigins := []string{
			"http://localhost:3000",
			"http://localhost:8080",
			"http://127.0.0.1:3000",
		}

		allowed := false
		for _, allowedOrigin := range allowedOrigins {
			if origin == allowedOrigin {
				allowed = true
				break
			}
		}

		if allowed {
			c.Writer.Header().Set("Access-Control-Allow-Origin", origin)
			c.Writer.Header().Set("Access-Control-Allow-Credentials", "true")
		}

		c.Writer.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		c.Writer.Header().Set("Access-Control-Allow-Headers", "Content-Type, Authorization")

		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}

		c.Next()
	})

	// Открытые маршруты
	router.POST("/api/auth/register", register)
	router.POST("/api/auth/login", login)

	// Защищенные маршруты
	api := router.Group("/api")
	api.Use(authMiddleware())
	{
		api.GET("/tasks", getTasks)
		api.POST("/submit", submitFlag)
		api.GET("/profile", getProfileEnhanced)
		api.GET("/leaderboard", getLeaderboard)
		api.GET("/history", getHistory)
		api.GET("/report", downloadReport)
		api.GET("/download/:id", downloadFile)
	}

	// Health check
	router.GET("/health", func(c *gin.Context) {
		c.JSON(200, gin.H{"status": "ok"})
	})

	log.Println("🔒 Server running on :8080 with enhanced security")
	router.Run(":8080")
}
