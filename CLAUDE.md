# CLAUDE.md

## Project Instructions

- `PROJECT.md` defines the current project scope and completion criteria.
- Consult `PROJECT.md` when planning work or making decisions that affect project scope.
- You may suggest ideas outside the current scope, but do not implement them without explicit user approval.

## Feature Specifications

A feature specification defines the required behavior of a feature and how its completion can be verified.

When defining or reviewing a feature specification:

- Include only the detail needed to remove meaningful ambiguity.
- Adjust the level of detail to the feature's complexity, ambiguity, interactions, and verification needs.
- Focus on required behavior and constraints rather than implementation details.
- Do not introduce implementation architecture, classes, APIs, files, or task breakdowns unless they are themselves explicit requirements.
- Use additional sections such as constraints, boundaries, edge cases, or out-of-scope items only when they materially clarify the feature.
- Do not mechanically fill a fixed template when a simpler specification is sufficient.

A specification should make the following clear where relevant:

- **Intent** — Why does the feature exist?
- **Behavior** — What must the feature do?
- **Boundary** — What is and is not the responsibility of this feature?
- **Verification** — How can we determine that the required behavior is complete?

These are questions the specification should answer, not mandatory document sections.

Implementation choices that can reasonably be decided later should remain open until implementation planning.

## Task Execution

- Inspect the relevant existing code before making changes. Do not guess how the current implementation works.
- Work within the current task scope.
- If implementation reveals that the task boundary or implementation plan should change, explain why and adjust the plan with the user before expanding the work.
- After implementation, verify the change using the strongest practical method available, such as compilation, tests, or running the relevant behavior in Unreal Editor.