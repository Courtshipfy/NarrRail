import { githubFetchJson, requireGithubToken } from "../auth/_github.js";

export default async function handler(req, res) {
  try {
    if (req.method !== "GET") {
      res.setHeader("Allow", "GET");
      res.status(405).json({ error: "Method not allowed" });
      return;
    }

    const { accessToken } = requireGithubToken(req);

    const url = new URL("https://api.github.com/user/repos");
    url.searchParams.set("per_page", "100");
    url.searchParams.set("sort", "updated");
    url.searchParams.set("direction", "desc");

    const repos = await githubFetchJson(url.toString(), accessToken);

    const mapped = (Array.isArray(repos) ? repos : []).map((r) => ({
      id: String(r.id),
      name: r.name,
      fullName: r.full_name,
      owner: r.owner?.login || "",
      defaultBranch: r.default_branch || "main",
      private: !!r.private,
      updatedAt: r.updated_at,
    }));

    res.status(200).json({ repos: mapped });
  } catch (error) {
    const status = error.status || 500;
    res.status(status).json({ error: error.message || "Failed to list repos" });
  }
}
