/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

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