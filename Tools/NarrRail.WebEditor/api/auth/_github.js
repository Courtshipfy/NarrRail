import { getSessionFromRequest } from "./_session.js";

export function requireGithubToken(req) {
  const session = getSessionFromRequest(req);
  if (!session?.accessToken) {
    const err = new Error("Unauthorized");
    err.status = 401;
    throw err;
  }

  return {
    accessToken: session.accessToken,
    user: session.user,
  };
}

export async function githubFetchJson(url, accessToken, init = {}) {
  const response = await fetch(url, {
    ...init,
    headers: {
      Accept: "application/vnd.github+json",
      Authorization: `Bearer ${accessToken}`,
      "User-Agent": "NarrRail-WebEditor",
      ...(init.headers || {}),
    },
  });

  const data = await response.json();

  if (!response.ok) {
    const message = data?.message || `GitHub API failed (${response.status})`;
    const err = new Error(message);
    err.status = response.status;
    err.payload = data;
    throw err;
  }

  return data;
}
