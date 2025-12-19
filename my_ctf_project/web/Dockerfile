# Сборка
FROM golang:1.21-alpine AS builder
WORKDIR /app
COPY main.go .
RUN mkdir -p challenge_files
RUN go build -ldflags "-s -w" -o web-server main.go

# Финальный образ
FROM alpine:latest
WORKDIR /app
COPY --from=builder /app/web-server .
RUN mkdir -p challenge_files

EXPOSE 5000
CMD ["./web-server"]
