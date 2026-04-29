import {
  buildSetStateCookie,
  createState,
  getAppBaseUrl,
  getRequiredEnv,
} from "./_oauth.js";

export default async function handler(req, res) {
  try {
    const clientId = getRequiredEnv("GITHUB_CLIENT_ID");
    const appBaseUrl = getAppBaseUrl(req);
    const redirectUri =
      process.env.GITHUB_REDIRECT_URI ||
      `${appBaseUrl}/api/auth/github-callback`;

    const state = createState();

    const url = new URL("https://github.com/login/oauth/authorize");
    url.searchParams.set("client_id", clientId);
    url.searchParams.set("redirect_uri", redirectUri);
    url.searchParams.set("scope", "read:user user:email repo");
    url.searchParams.set("state", state);

    res.setHeader("Set-Cookie", buildSetStateCookie(state));
    res.writeHead(302, { Location: url.toString() });
    res.end();
  } catch (error) {
    res.status(500).json({ error: error.message || "OAuth start failed" });
  }
}
