# Split Unreal consumer into its own repository

NarrRail is being repositioned as a script creation and authoring-assistance product for Narrative Creators, so the Unreal Engine plugin and sample host should move to a separate `NarrRail-Unreal-Plugin` repository. The main repository remains the owner of the `.nrstory` / `.nrrail` story format, authoring-side validation, export semantics, NarrRailEditor, and Script Conversion workflow; the Unreal repository becomes a Story Consumer that declares which story format versions it supports.

The split should use a clean copy rather than a history-preserving extraction, because the new repository needs a clearer consumer-facing shape and the source history remains available in the original repository. Migration should proceed by freezing the story format contract first, then creating the Unreal repository, then updating the main repository positioning, and finally removing old Unreal-specific coupling from the main repo.
