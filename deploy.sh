COMPOSE=docker-compose.yml
set -e

git checkout sna_project
git pull
sudo docker compose -f $COMPOSE down
sudo docker compose -f $COMPOSE up --build -d
