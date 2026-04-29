import {
  buildClearStateCookie,
  getAppBaseUrl,
  getFrontendRedirectUrl,
  getRequiredEnv,
  parseStateFromCookie,
} from "./_oauth.js";
import {
  buildSetSessionCookie,
  createSessionCookieValue,
} from "./_session.js";

async function exchangeCodeForToken({ code, redirectUri, clientId, clientSecret }) {
  const response = await fetch("https://github.com/login/oauth/access_token", {
    method: "POST",
    headers: {
      Accept: "application/json",
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      client_id: clientId,
      client_secret: clientSecret,
      code,
      redirect_uri: redirectUri,
    }),
  });

  const data = await response.json();
  if (!response.ok || !data.access_token) {
    throw new Error(data.error_description || "Failed to exchange code for token");
  }

  return data.access_token;
}

async function fetchGithubUser(accessToken) {
  const response = await fetch("https://api.github.com/user", {
    headers: {
      Authorization: `Bearer ${accessToken}`,
      Accept: "application/vnd.github+json",
      "User-Agent": "NarrRail-WebEditor",
    },
  });

  const data = await response.json();
  if (!response.ok || !data?.id) {
    throw new Error(data?.message || "Failed to fetch GitHub user");
  }

  return {
    id: String(data.id),
    login: data.login || "",
    name: data.name || "",
    avatarUrl: data.avatar_url || "",
    profileUrl: data.html_url || "",
  };
}

export default async function handler(req, res) {
  try {
    const { code, state } = req.query || {};
    const cookieState = parseStateFromCookie(req);

    if (!code || !state || !cookieState || state !== cookieState) {
      res.status(400).json({ error: "Invalid OAuth state or code" });
      return;
    }

    const clientId = getRequiredEnv("GITHUB_CLIENT_ID");
    const clientSecret = getRequiredEnv("GITHUB_CLIENT_SECRET");

    const appBaseUrl = getAppBaseUrl(req);
    const redirectUri = process.env.GITHUB_REDIRECT_URI || `${appBaseUrl}/api/auth/github-callback`;

    const accessToken = await exchangeCodeForToken({
      code,
      redirectUri,
      clientId,
      clientSecret,
    });

    const user = await fetchGithubUser(accessToken);

    const session = {
      user,
      provider: "github",
      issuedAt: new Date().toISOString(),
    };

    const sessionCookie = buildSetSessionCookie(createSessionCookieValue(session));
    const clearStateCookie = buildClearStateCookie();

    const redirectTo = new URL(getFrontendRedirectUrl(req));
    redirectTo.searchParams.set("auth", "ok");

    res.setHeader("Set-Cookie", [sessionCookie, clearStateCookie]);
    res.writeHead(302, { Location: redirectTo.toString() });
    res.end();
  } catch (error) {
    res.status(500).json({ error: error.message || "OAuth callback failed" });
  }
}
