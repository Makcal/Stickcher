#!/bin/bash

TIMESTAMP=$(date +%Y-%m-%d_%H-%M-%S)

echo "Creating backup at $TIMESTAMP"
pg_dump -h db -U $POSTGRES_USER -d $POSTGRES_DB > /backups/backup_$TIMESTAMP.dump

