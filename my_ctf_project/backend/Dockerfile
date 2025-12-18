# --- ЭТАП 1: СБОРКА ---
FROM golang:1.21-alpine AS builder
WORKDIR /app
COPY go.mod go.sum ./
RUN go mod download
COPY . .
# -s -w удаляет таблицу символов и отладочную инфу (усложняет реверс-инжиниринг и уменьшает размер)
RUN go build -ldflags "-s -w" -o server main.go

# --- ЭТАП 2: ФИНАЛЬНЫЙ ОБРАЗ ---
FROM alpine:latest
WORKDIR /app
# Копируем ТОЛЬКО бинарник
COPY --from=builder /app/server .
# Копируем файлы заданий (если есть)
COPY files/ ./files/

EXPOSE 8080
CMD ["./server"]
