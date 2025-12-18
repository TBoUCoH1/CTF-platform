# ЭТАП 1: Сборка
FROM golang:1.21-alpine AS builder
WORKDIR /app
COPY main.go .
RUN mkdir -p challenge_files
RUN go build -ldflags "-s -w" -o web-server main.go

# ЭТАП 2: Финальный образ
FROM alpine:latest
WORKDIR /app
COPY --from=builder /app/web-server .
# Создаем папку, если нужна
RUN mkdir -p challenge_files

EXPOSE 5000
CMD ["./web-server"]
