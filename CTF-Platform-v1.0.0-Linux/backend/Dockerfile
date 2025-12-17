FROM golang:1.21-alpine

WORKDIR /app

# Установка git для зависимостей
RUN apk add --no-cache git

# Копируем go.mod и go.sum
COPY go.mod go.sum ./

# Загружаем зависимости
RUN go mod download

# Копируем исходный код
COPY main.go .

# Собираем приложение
RUN go build -o server main.go

EXPOSE 8080

CMD ["./server"]
