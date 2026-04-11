import { defineMiddleware } from "astro:middleware";
import { handleRequest, handleResponse } from "./backend/middlewares/e2ee.js";

export const onRequest = defineMiddleware(async (context, next) => {
    await handleRequest(context);
    const resp = await next();
    return await handleResponse(context, resp);
});