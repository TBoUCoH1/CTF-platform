#!/bin/bash
echo "🛑 Stopping CTF Platform Backend..."
cd "$(dirname "$0")"
docker-compose down
echo "✅ Backend stopped!"
