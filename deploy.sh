COMPOSE=docker-compose.yml
set -e

git checkout sna_project
git pull
set +e
docker-compose -f $COMPOSE down
set -e
docker-compose -f $COMPOSE up --build -d
