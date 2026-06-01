import crypto from "node:crypto";

const COOKIE_NAME = "nr_session";

function getSessionSecret() {
  const secret = process.env.SESSION_SECRET;
  if (!secret) throw new Error("SESSION_SECRET is not configured");
  return secret;
}

function base64url(input) {
  return Buffer.from(input).toString("base64url");
}

function sign(payloadBase64) {
  return crypto
    .createHmac("sha256", getSessionSecret())
    .update(payloadBase64)
    .digest("base64url");
}

export function createSessionCookieValue(sessionObj) {
  const payloadBase64 = base64url(JSON.stringify(sessionObj));
  const signature = sign(payloadBase64);
  return `${payloadBase64}.${signature}`;
}

export function parseCookies(req) {
  const cookie = req.headers.cookie || "";
  const entries = cookie
    .split(";")
    .map((v) => v.trim())
    .filter(Boolean)
    .map((v) => {
      const idx = v.indexOf("=");
      if (idx <= 0) return [v, ""];
      return [v.slice(0, idx), v.slice(idx + 1)];
    });
  return Object.fromEntries(entries);
}

export function verifySessionCookieValue(rawValue) {
  if (!rawValue || typeof rawValue !== "string") return null;

  const parts = rawValue.split(".");
  if (parts.length !== 2) return null;

  const [payloadBase64, signature] = parts;
  const expected = sign(payloadBase64);

  const signatureBuf = Buffer.from(signature);
  const expectedBuf = Buffer.from(expected);
  if (signatureBuf.length !== expectedBuf.length) return null;

  const valid = crypto.timingSafeEqual(signatureBuf, expectedBuf);
  if (!valid) return null;

  try {
    const json = Buffer.from(payloadBase64, "base64url").toString("utf-8");
    const data = JSON.parse(json);
    if (!data?.user?.id || !data?.issuedAt) return null;
    return data;
  } catch {
    return null;
  }
}

export function buildSetSessionCookie(value, maxAgeSec = 60 * 60 * 24 * 7) {
  return `${COOKIE_NAME}=${value}; Path=/; HttpOnly; Secure; SameSite=Lax; Max-Age=${maxAgeSec}`;
}

export function buildClearSessionCookie() {
  return `${COOKIE_NAME}=; Path=/; HttpOnly; Secure; SameSite=Lax; Max-Age=0`;
}

export function getSessionFromRequest(req) {
  const cookies = parseCookies(req);
  return verifySessionCookieValue(cookies[COOKIE_NAME]);
}

export const SESSION_COOKIE_NAME = COOKIE_NAME;
