import crypto from "node:crypto";

export function getRequiredEnv(name) {
  const value = process.env[name];
  if (!value) throw new Error(`${name} is not configured`);
  return value;
}

export function getAppBaseUrl(req) {
  if (process.env.APP_BASE_URL) return process.env.APP_BASE_URL;
  const proto = req.headers["x-forwarded-proto"] || "https";
  const host = req.headers.host;
  return `${proto}://${host}`;
}

export function getFrontendRedirectUrl(req) {
  return process.env.FRONTEND_URL || getAppBaseUrl(req);
}

export function createState() {
  return crypto.randomBytes(24).toString("hex");
}

export function buildSetStateCookie(state) {
  return `nr_oauth_state=${state}; Path=/; HttpOnly; Secure; SameSite=Lax; Max-Age=600`;
}

export function buildClearStateCookie() {
  return `nr_oauth_state=; Path=/; HttpOnly; Secure; SameSite=Lax; Max-Age=0`;
}

export function parseStateFromCookie(req) {
  const cookie = req.headers.cookie || "";
  const parts = cookie.split(";").map((v) => v.trim());
  for (const item of parts) {
    if (item.startsWith("nr_oauth_state=")) {
      return item.slice("nr_oauth_state=".length);
    }
  }
  return "";
}
