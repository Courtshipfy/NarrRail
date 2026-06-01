import { getSessionFromRequest } from "./_session.js";

export default async function handler(req, res) {
  try {
    const session = getSessionFromRequest(req);
    if (!session) {
      res.status(200).json({ authenticated: false, user: null });
      return;
    }

    res.status(200).json({
      authenticated: true,
      user: session.user,
      provider: session.provider,
      issuedAt: session.issuedAt,
    });
  } catch (error) {
    res.status(500).json({ error: error.message || "Failed to fetch auth state" });
  }
}
