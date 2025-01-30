FROM redgate/flyway:10.4.1

COPY ./data/migrations /sql

ENTRYPOINT sh -c 'FLYWAY_PASSWORD="$(cat "$DB_PASSWORD_FILE")" flyway migrate' && sleep 300
# ENTRYPOINT [ "flyway", "migrate" ]