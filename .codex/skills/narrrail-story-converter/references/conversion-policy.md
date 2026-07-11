# Conversion Policy

## General

- Infer the simplest valid story flow.
- Prefer preserving source order over inventing structure.
- Use `Narrator` for scene/action prose unless a project profile says otherwise.
- Do not invent variables, conditions, timers, events, affection flags, or hidden
  state unless explicitly present or requested.
- If the source includes writer notes, second-run hints, comments, or uncertain
  text, put them in conversion notes instead of the main `.nrstory`.

## Manuscript Interpretation

- Speaker + line -> `Dialogue`.
- Consecutive same-speaker lines -> one `MultiDialogue`.
- Scene/action text -> `Narrator` dialogue.
- Explicit alternatives, numbered choices, or "player can choose" language ->
  `Choice`.
- "Both routes return/merge" language -> connect branches to the same later node.
- Missing explicit ending -> add one `End` node at the end of the main path.

## Ambiguity

When ambiguous, choose the conservative option and record a review note.

Examples:

- Unclear speaker -> `Narrator`, note the source location.
- Possible second-playthrough content -> omit from main path and note it.
- Pure author comment -> omit from `.nrstory`, note it.
- Unclear branch target -> create a descriptive target only if the source implies
  a clear branch; otherwise keep linear and note ambiguity.

## Review Notes Contract

When a review note is needed, write it to `conversion-notes.md` using the shared
contract in `Docs/spec/CONVERSION_REVIEW_CONTRACT.md` when working inside the
NarrRail repository.

Each review item should include:

- stable item ID
- severity: `error`, `warning`, or `review`
- issue type, such as `ambiguous-source`, `omitted-content`, or
  `needs-confirmation`
- source location, such as file, sheet, row, line, section, or excerpt
- generated target when available, such as `.nrstory` path, node ID, edge ID, or
  field
- suggested action for the human reviewer

If the user asks for an Import Package or Project Review Queue-ready output,
also emit optional `conversion-review.json` with the same item IDs and
vocabulary. JSON review metadata is a structured mirror of the Markdown notes;
it does not replace `conversion-notes.md`.

## ID Naming

- Prefix story node IDs with `N_`.
- Use stable ASCII IDs when possible.
- For generated linear nodes, use zero-padded numbers:
  `N_TrainDraft_001`, `N_TrainDraft_002`.
- Avoid changing existing IDs during revisions unless necessary.
