import { buildClearSessionCookie } from "./_session.js";

export default async function handler(req, res) {
  try {
    if (req.method !== "POST") {
      res.setHeader("Allow", "POST");
      res.status(405).json({ error: "Method not allowed" });
      return;
    }

    res.setHeader("Set-Cookie", buildClearSessionCookie());
    res.status(200).json({ ok: true });
  } catch (error) {
    res.status(500).json({ error: error.message || "Logout failed" });
  }
}
