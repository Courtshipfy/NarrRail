import { githubFetchJson, requireGithubToken } from "../auth/_github.js";

export default async function handler(req, res) {
  try {
    if (req.method !== "POST") {
      res.setHeader("Allow", "POST");
      res.status(405).json({ error: "Method not allowed" });
      return;
    }

    const { accessToken } = requireGithubToken(req);

    const {
      owner,
      repo,
      branch = "main",
      path,
      sha,
      message,
    } = req.body || {};

    if (!owner || !repo || !path || !sha) {
      res
        .status(400)
        .json({ error: "owner, repo, path and sha are required" });
      return;
    }

    const url = `https://api.github.com/repos/${owner}/${repo}/contents/${encodeURIComponent(path)}`;

    const payload = {
      message: message || `chore(webeditor): delete ${path}`,
      sha,
      branch,
    };

    const data = await githubFetchJson(url, accessToken, {
      method: "DELETE",
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
      },
    });
  } catch (error) {
    const status = error.status || 500;
    res.status(status).json({
      error: error.message || "Failed to delete file",
      details: error.payload || undefined,
    });
  }
}
