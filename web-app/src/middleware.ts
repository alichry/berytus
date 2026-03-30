import { defineMiddleware } from "astro:middleware";
import { handleRequest } from "./backend/middlewares/e2ee.js";

export const onRequest = defineMiddleware(async (context, next) => {
    await handleRequest(context);
    return next();
});