[![CI - C++ Linting](https://github.com/Makcal/Stickcher/actions/workflows/cpp-lint.yml/badge.svg?branch=sna_project)](https://github.com/Makcal/Stickcher/actions/workflows/cpp-lint.yml)
[![GitHub Stars](https://img.shields.io/github/stars/Makcal/Stickcher)](https://github.com/Makcal/Stickcher/stargazers)
[![License](https://img.shields.io/github/license/Makcal/Stickcher)](https://github.com/Makcal/Stickcher/blob/main/LICENSE)

# Stickcher
Searching Telegram stickers is now easier than ever! Index stickers by tags and query them inline!

## Features

- **Telegram Bot Interface** (C++):
  - User-friendly interaction
  - Image upload and processing
  - Sticker search results

- **Parser Service** (Python):
  - OCR text extraction from images
  - Sticker metadata processing
  - Scalable microservice architecture

- **Infrastructure**:
  - Docker containerization
  - PostgreSQL database
  - Nginx load balancing
  - Horizontal scaling for parser instances

- **DevOps Implementation**:
  - GitHub Actions CI/CD pipelines:
    - Code linting for bot
    - Deployment automation
  - Comprehensive logging
  - Automated backups

## Prerequisites

- Docker 27.5+
- Telegram Bot API token

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/Makcal/Stickcher.git
   cd Stickcher
   ```

2. Set up environment variables:
   ```bash
   cp .env.example .env
   # Edit .env with your configuration
   ```

3. Build and start containers:
   ```bash
   docker compose up --build
   ```

## Configuration

Edit the `.env` file with your specific settings:

```ini
BOT_TOKEN=number:smth

DB_USER=postgres
DB_PASSWORD=postgres
DB_NAME=stickcher

SIMILARITY_THRESHOLD=0.80
ASSOCIATION_LIMIT=10
```

## CI/CD Pipeline

The project includes GitHub Actions workflows for:

- **Linting**: Code quality checks for both C++ and Python
- **Deployment**: Automated container builds and deployment

View workflows in `.github/workflows/`

## License

Distributed under the MIT License. See `LICENSE` for more information.
