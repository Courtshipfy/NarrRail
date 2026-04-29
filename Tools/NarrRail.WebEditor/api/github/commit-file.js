import { githubFetchJson, requireGithubToken } from "../auth/_github.js";

function toBase64(input) {
  return Buffer.from(String(input || ""), "utf-8").toString("base64");
}

export default async function handler(req, res) {
  try {
    if (req.method !== "POST") {
      res.setHeader("Allow", "POST");
      res.status(405).json({ error: "Method not allowed" });
      return;
    }

    const { accessToken } = requireGithubToken(req);

    const { owner, repo, branch = "main", path, content, message, sha } = req.body || {};

    if (!owner || !repo || !path) {
      res.status(400).json({ error: "owner, repo, path are required" });
      return;
    }

    const url = `https://api.github.com/repos/${owner}/${repo}/contents/${encodeURIComponent(path)}`;

    const payload = {
      message: message || `chore(webeditor): update ${path}`,
      content: toBase64(content || ""),
      branch,
      ...(sha ? { sha } : {}),
    };

    const data = await githubFetchJson(url, accessToken, {
      method: "PUT",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(payload),
    });

    res.status(200).json({
      ok: true,
      commit: {
        sha: data?.commit?.sha || "",
        message: data?.commit?.message || "",
        url: data?.commit?.html_url || "",
      },
      content: {
        path: data?.content?.path || path,
        sha: data?.content?.sha || "",
      },
    });
  } catch (error) {
    const status = error.status || 500;
    res.status(status).json({ error: error.message || "Failed to commit file" });
  }
}
