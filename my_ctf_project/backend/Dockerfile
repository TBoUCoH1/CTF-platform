# СБОРКА
FROM golang:1.21-alpine AS builder
WORKDIR /app
COPY go.mod go.sum ./
RUN go mod download
COPY . .
RUN go build -ldflags "-s -w" -o server main.go

# ФИНАЛЬНЫЙ ОБРАЗ
FROM alpine:latest
WORKDIR /app
COPY --from=builder /app/server .
COPY files/ ./files/

EXPOSE 8080
CMD ["./server"]
