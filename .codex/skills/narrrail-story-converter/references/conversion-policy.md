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

## ID Naming

- Prefix story node IDs with `N_`.
- Use stable ASCII IDs when possible.
- For generated linear nodes, use zero-padded numbers:
  `N_TrainDraft_001`, `N_TrainDraft_002`.
- Avoid changing existing IDs during revisions unless necessary.

