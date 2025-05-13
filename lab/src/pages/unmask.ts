import type { APIRoute } from "astro";

export const POST: APIRoute = async ({ params, request }) => {
  return new Response(
    request.body,
    {
        headers: {
            'content-type': request.headers.get('content-type')
        }
    }
);
}