# Spec-Kit Workflow

NarrRail uses GitHub Issues as the source of truth for work selection, and official Spec Kit project scaffolding for larger execution specs. The repository is initialized with the GitHub Spec Kit `specify` CLI using the Codex skill integration.

Reference: GitHub Spec Kit is the upstream "Spec-Driven Development" toolkit at <https://github.com/github/spec-kit>.

## Local CLI

The project was initialized with Spec Kit `0.12.11`.

Recommended local install:

```bash
brew install uv
uv tool install specify-cli --from git+https://github.com/github/spec-kit.git@v0.12.11
specify version
```

The official project files live in:

- `.specify/`: Spec Kit scripts, templates, workflow metadata, and NarrRail constitution memory.
- `.agents/skills/speckit-*`: Codex skill integration for Spec Kit commands.
- `specs/`: feature specs generated or maintained for large execution issues.

Do not place credentials or private agent state under `.agents/`. Only the `speckit-*` skills are intended to be tracked.

## When To Use This

Use a spec before implementation when an issue has any of these traits:

- It spans more than one surface, such as core + UI, storage + review, or docs + repo migration.
- It introduces or changes a file contract, schema, import/export behavior, or cross-repository compatibility rule.
- It has multiple user-visible states, navigation paths, or failure modes.
- It will likely split into several implementation tasks.
- It is one of the already identified large execution issues, such as Import Package Review, Project Dashboard, Project Review Queue UI, Project Preview, Structured Script View editor, or the Unreal consumer split.

Implement directly from the issue when the work is small and concrete:

- A focused docs update.
- A single-module refactor with clear acceptance criteria.
- A narrow bug fix.
- A test-only improvement.
- A Wayfinder ticket whose output is a decision rather than executable product behavior.

Use `wayfinder` when the question is still foggy. Use a spec when the destination is decided and the remaining work is execution design.

Before planning a large feature, make sure `.specify/memory/constitution.md` still reflects the current NarrRail product direction.

## Issue To Spec Mapping

Each large execution issue gets one feature folder:

```text
specs/
  0059-import-package-review/
    spec.md
    plan.md
    tasks.md
    checklist.md
```

Use the four-digit GitHub issue number plus a short kebab-case name. The issue body should link to the feature folder once it exists.

If a feature already has a Figma file, research note, ADR, or prototype, link to it from `spec.md` rather than duplicating the content.

## Workflow

1. **Select the GitHub issue.**
   Read the issue body, comments, linked Wayfinder decision, ADRs, and domain terms before creating the spec.

2. **Create the feature folder.**
   Prefer the official Spec Kit skills, such as `$speckit-specify`, `$speckit-plan`, and `$speckit-tasks`. For existing GitHub issues, pass or persist `SPECIFY_FEATURE_DIRECTORY=specs/<four-digit-issue-number>-<short-name>` so the folder stays aligned with the issue number. When creating folders manually, use the same convention.

3. **Write `spec.md`.**
   Capture user-facing behavior, scope, non-goals, domain terms, acceptance criteria, and open questions.

4. **Write `plan.md`.**
   Identify affected modules, data flow, integration seams, tests, rollout order, and risks.

5. **Write `tasks.md`.**
   Break the plan into agent-sized implementation tasks with dependencies. A task should be small enough to commit and verify independently.

6. **Write `checklist.md`.**
   Turn the issue acceptance criteria and plan risks into a verification checklist.

7. **Then implement.**
   Use the `implement` skill against the issue plus its spec folder. Keep code changes aligned with the spec. If implementation reveals a product decision, update the spec or create a follow-up issue rather than burying the decision in code.

## Relationship To Existing Skills

- `wayfinder`: creates and resolves decision maps. It should produce issues, not detailed implementation specs, unless the map explicitly says execution is in scope.
- `to-spec`: can synthesize a spec from an already-settled conversation or issue.
- `to-tickets`: can break a settled plan or spec into smaller issues.
- `$speckit-specify`, `$speckit-plan`, `$speckit-tasks`, `$speckit-implement`: official Spec Kit flow for larger execution issues.
- `implement`: executes an issue or task. For large issues, read the corresponding `specs/<four-digit-issue-number>-<short-name>/` folder first.
- `code-review`: review implementation against both the issue and the spec folder.

## What Not To Do

- Do not force tiny bug fixes through a spec folder.
- Do not use specs to reopen settled Wayfinder decisions without saying so explicitly.
- Do not duplicate long issue histories. Link to issues, ADRs, Figma files, and research artifacts.
- Do not treat a spec as code ownership. The GitHub issue remains the tracker item; the spec is the execution contract.
