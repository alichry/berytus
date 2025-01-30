FROM node:21-alpine

ENV HOST=0.0.0.0
ENV PORT=8080
EXPOSE $PORT

WORKDIR /app

COPY package.json .
COPY package-lock.json .
COPY tsconfig.json .
COPY tailwind.config.mjs .
COPY astro.config.mjs .
COPY run-server.mjs .
COPY src ./src
COPY public ./public
COPY data/migrations ./data/migrations

RUN npm install
RUN npm run astro telemetry disable
RUN npm run build

CMD [ "npm", "run", "start" ]