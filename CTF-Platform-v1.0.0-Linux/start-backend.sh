#!/bin/bash
echo "🐳 Starting CTF Platform Backend..."
cd "$(dirname "$0")"
docker-compose up -d
sleep 3
docker-compose ps
echo ""
echo "✅ Backend started!"
echo "🌐 Web: http://localhost:5000"
echo "🔌 API: http://localhost:8080"
