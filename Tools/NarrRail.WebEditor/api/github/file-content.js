import { githubFetchJson, requireGithubToken } from "../auth/_github.js";

function normalizePath(path) {
  return String(path || "")
    .trim()
    .replace(/^\/+/, "");
}

export default async function handler(req, res) {
  try {
    if (req.method !== "GET") {
      res.setHeader("Allow", "GET");
      res.status(405).json({ error: "Method not allowed" });
      return;
    }

    const { accessToken } = requireGithubToken(req);
    const owner = String(req.query?.owner || "").trim();
    const repo = String(req.query?.repo || "").trim();
    const branch = String(req.query?.branch || "").trim() || "main";
    const mode = String(req.query?.mode || "content").trim();
    const path = normalizePath(req.query?.path || "");

    if (!owner || !repo) {
      res.status(400).json({ error: "owner and repo are required" });
      return;
    }

    if (mode === "list") {
      const rootPath = path || "";
      const url = new URL(
        `https://api.github.com/repos/${owner}/${repo}/git/trees/${encodeURIComponent(branch)}?recursive=1`,
      );
      let tree = [];
      try {
        const treeData = await githubFetchJson(url.toString(), accessToken);
        tree = Array.isArray(treeData?.tree) ? treeData.tree : [];
      } catch (error) {
        if (error?.status === 409) {
          res.status(200).json({ files: [] });
          return;
        }
        throw error;
      }

      const files = tree
        .filter((item) => item?.type === "blob")
        .map((item) => ({
          path: item.path,
          size: item.size || 0,
        }))
        .filter((item) => {
          const lower = item.path.toLowerCase();
          const isYaml =
            lower.endsWith(".narrrail.yaml") ||
            lower.endsWith(".narrrail.yml") ||
            lower.endsWith("global-config.narrrail.yaml") ||
            lower.endsWith("global-config.narrrail.yml");
          const inRoot = rootPath ? item.path.startsWith(rootPath) : true;
          return isYaml && inRoot;
        })
        .map((item) => ({
          id: item.path,
          path: item.path,
          fileName: item.path.split("/").pop(),
          extension: item.path.toLowerCase().endsWith(".yml")
            ? ".yml"
            : ".yaml",
          storyId: (item.path.split("/").pop() || "").replace(
            /\.narrrail\.ya?ml$/i,
            "",
          ),
          size: item.size,
          updatedAt: null,
          tags: ["GitHub"],
          source: "github",
          owner,
          repo,
          branch,
        }));

      res.status(200).json({ files });
      return;
    }

    if (!path) {
      res.status(400).json({ error: "path is required for content mode" });
      return;
    }

    const contentUrl = new URL(
      `https://api.github.com/repos/${owner}/${repo}/contents/${encodeURIComponent(path)}`,
    );
    contentUrl.searchParams.set("ref", branch);

    const file = await githubFetchJson(contentUrl.toString(), accessToken);
    const encoded = String(file?.content || "").replace(/\n/g, "");
    const text = Buffer.from(encoded, "base64").toString("utf-8");

    res.status(200).json({
      owner,
      repo,
      branch,
      path,
      sha: file?.sha || "",
      content: text,
      size: file?.size || 0,
    });
  } catch (error) {
    const status = error.status || 500;
    res.status(status).json({ error: error.message || "Failed to read file" });
  }
}
