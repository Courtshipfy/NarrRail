# Codex Instructions

## Commit Shortcut

When the user sends exactly `commit`, `提交`, or asks to commit staged changes, treat it as the local commit workflow.

Workflow:

1. Run `git status --short`.
2. Run `git diff --cached --stat` and `git diff --cached --name-status`.
3. If there are no staged changes, stop and tell the user to stage files first or list exact files to stage.
4. If staged changes contain clearly unrelated files, ask for confirmation before committing.
5. Generate a concise Chinese commit message from the staged diff.
6. Run `git commit` using only staged changes.
7. Report the commit hash, title, and committed file summary.

Rules:

- Do not run `git add .` by default.
- Do not commit unstaged files by default.
- If the user lists exact file paths to commit, stage only those paths with `git add -- <path>`.
- Do not include unrelated local files, caches, build artifacts, or unconfirmed deletions.
- Use Chinese commit messages.
