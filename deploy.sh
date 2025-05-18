COMPOSE=docker-compose.yml

mkdir -p Stickcher
cd Stickcher
if [ -d .git ]; then
    git checkout sna_project
    git pull
    docker-compose -f $COMPOSE down
else
    git clone -b sna_project https://github.com/Makcal/Stickcher.git .
fi
docker-compose -f $COMPOSE up --build -d
