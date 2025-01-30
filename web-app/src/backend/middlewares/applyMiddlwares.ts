import type { Application } from 'express';
import morgan from 'morgan';

export function applyMiddlewares(app: Application) {
    app.set("trust proxy", true);
    app.set('x-powered-by', false);
    app.use(morgan('[:date[clf]] :remote-addr :method ":url" HTTP/:http-version :status ":referrer" ":user-agent"'));
}