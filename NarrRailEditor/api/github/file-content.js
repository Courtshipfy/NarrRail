import { githubFetchJson, requireGithubToken } from "../auth/_github.js";

function normalizePath(path) {
  return String(path || "")
    .trim()
    .replace(/^\/+/, "");
}

async function mapWithConcurrency(items, limit, mapper) {
  const results = new Array(items.length);
  let cursor = 0;

  async function worker() {
    while (cursor < items.length) {
      const index = cursor;
      cursor += 1;
      results[index] = await mapper(items[index], index);
    }
  }

  const workerCount = Math.max(1, Math.min(limit, items.length));
  await Promise.all(Array.from({ length: workerCount }, () => worker()));
  return results;
}

async function fetchLatestCommitDateForPath({
  owner,
  repo,
  branch,
  path,
  accessToken,
}) {
  const commitsUrl = new URL(
    `https://api.github.com/repos/${owner}/${repo}/commits`,
  );
  commitsUrl.searchParams.set("sha", branch);
  commitsUrl.searchParams.set("path", path);
  commitsUrl.searchParams.set("per_page", "1");

  try {
    const commits = await githubFetchJson(commitsUrl.toString(), accessToken);
    const first = Array.isArray(commits) ? commits[0] : null;
    return (
      first?.commit?.author?.date || first?.commit?.committer?.date || null
    );
  } catch (error) {
    if (error?.status === 404 || error?.status === 409) {
      return null;
    }
    throw error;
  }
}

async function fetchContentTextByPath({
  owner,
  repo,
  branch,
  path,
  accessToken,
}) {
  const contentUrl = new URL(
    `https://api.github.com/repos/${owner}/${repo}/contents/${encodeURIComponent(path)}`,
  );
  contentUrl.searchParams.set("ref", branch);

  const file = await githubFetchJson(contentUrl.toString(), accessToken);
  const encoded = String(file?.content || "").replace(/\n/g, "");
  const text = Buffer.from(encoded, "base64").toString("utf-8");

  return {
    sha: file?.sha || "",
    size: file?.size || 0,
    content: text,
  };
}

function countYamlArrayItemsByTopLevelKey(yamlText, key) {
  const lines = String(yamlText || "").split(/\r?\n/);
  const keyRegex = new RegExp(`^${key}:\\s*(.*)$`);

  let start = -1;
  let tail = "";

  for (let i = 0; i < lines.length; i += 1) {
    const match = lines[i].match(keyRegex);
    if (match) {
      start = i;
      tail = String(match[1] || "").trim();
      break;
    }
  }

  if (start < 0) return null;
  if (tail === "[]") return 0;

  let count = 0;
  for (let i = start + 1; i < lines.length; i += 1) {
    const line = lines[i];

    if (!line.trim()) continue;
    if (/^\s*#/.test(line)) continue;

    if (/^[^\s][^:]*:\s*/.test(line)) {
      break;
    }

    if (/^\s*-\s+/.test(line)) {
      count += 1;
    }
  }

  return count;
}

function deriveNarrRailCounts(yamlText) {
  const nodeCount = countYamlArrayItemsByTopLevelKey(yamlText, "nodes");
  const edgeCount = countYamlArrayItemsByTopLevelKey(yamlText, "edges");
  return {
    nodeCount: Number.isFinite(nodeCount) ? nodeCount : null,
    edgeCount: Number.isFinite(edgeCount) ? edgeCount : null,
  };
}

function unquoteYamlScalar(value) {
  const text = String(value || "")
    .replace(/\s+#.*$/, "")
    .trim();
  if (!text) return "";
  if (
    (text.startsWith('"') && text.endsWith('"')) ||
    (text.startsWith("'") && text.endsWith("'"))
  ) {
    return text.slice(1, -1).trim();
  }
  return text.replace(/\s+#.*$/, "").trim();
}

function extractMetaScalar(yamlText, key) {
  const lines = String(yamlText || "").split(/\r?\n/);
  let inMeta = false;
  const keyRegex = new RegExp(`^\\s{2,}${key}\\s*:\\s*(.*)$`);

  for (const line of lines) {
    if (!inMeta) {
      if (/^meta\s*:\s*$/.test(line)) {
        inMeta = true;
      }
      continue;
    }

    if (/^[^\s][^:]*:\s*/.test(line)) {
      break;
    }

    const match = line.match(keyRegex);
    if (!match) continue;
    return unquoteYamlScalar(match[1]);
  }

  return "";
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

      const candidates = tree
        .filter((item) => item?.type === "blob")
        .map((item) => ({
          path: item.path,
          size: item.size || 0,
        }))
        .filter((item) => {
          const lower = item.path.toLowerCase();
          const isStoryScript =
            lower.endsWith(".nrstory") || lower.endsWith(".nrrail");
          const inRoot = rootPath ? item.path.startsWith(rootPath) : true;
          return isStoryScript && inRoot;
        });

      const files = await mapWithConcurrency(candidates, 6, async (item) => {
        const [updatedAt, contentInfo] = await Promise.all([
          fetchLatestCommitDateForPath({
            owner,
            repo,
            branch,
            path: item.path,
            accessToken,
          }),
          fetchContentTextByPath({
            owner,
            repo,
            branch,
            path: item.path,
            accessToken,
          }).catch(() => null),
        ]);

        const extension = item.path.toLowerCase().endsWith(".nrrail")
          ? ".nrrail"
          : ".nrstory";
        const counts = contentInfo
          ? deriveNarrRailCounts(contentInfo.content)
          : { nodeCount: null, edgeCount: null };
        const fileStem = (item.path.split("/").pop() || "").replace(
          /\.(nrstory|nrrail)$/i,
          "",
        );
        const storyId =
          extension === ".nrstory" && contentInfo
            ? extractMetaScalar(contentInfo.content, "storyId") || fileStem
            : fileStem;
        const railId =
          extension === ".nrrail"
            ? contentInfo
              ? extractMetaScalar(contentInfo.content, "railId") || fileStem
              : fileStem
            : undefined;

        return {
          id: item.path,
          path: item.path,
          fileName: item.path.split("/").pop(),
          extension,
          storyId,
          railId,
          size: item.size,
          updatedAt,
          nodeCount: counts.nodeCount,
          edgeCount: counts.edgeCount,
          tags: ["GitHub"],
          source: "github",
          owner,
          repo,
          branch,
        };
      });

      res.status(200).json({ files });
      return;
    }

    if (!path) {
      res.status(400).json({ error: "path is required for content mode" });
      return;
    }

    const file = await fetchContentTextByPath({
      owner,
      repo,
      branch,
      path,
      accessToken,
    });

    res.status(200).json({
      owner,
      repo,
      branch,
      path,
      sha: file.sha,
      content: file.content,
      size: file.size,
    });
  } catch (error) {
    const status = error.status || 500;
    res.status(status).json({ error: error.message || "Failed to read file" });
  }
}
