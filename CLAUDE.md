# CLAUDE.md

## Project Instructions

- `PROJECT.md` defines the current project scope and completion criteria.
- Consult `PROJECT.md` when planning work or making decisions that affect project scope.
- You may suggest ideas outside the current scope, but do not implement them without explicit user approval.

## Feature Specifications

A feature specification defines what behavior is required and how completion can be verified.

When defining or reviewing a feature specification:

- Include only the detail needed to remove meaningful ambiguity.
- Adjust the level of detail to the feature's complexity, ambiguity, interactions, and verification needs.
- Focus on required behavior and constraints rather than implementation details.
- Keep implementation choices open when they can reasonably be decided during implementation planning.
- Do not mechanically fill a fixed template when a simpler specification is sufficient.

A specification should answer the following questions where relevant:

- **Intent** — Why does the feature exist?
- **Behavior** — What must the feature do?
- **Boundary** — What is and is not the responsibility of the feature?
- **Verification** — How can we determine that the required behavior is complete?

These are guiding questions, not mandatory document sections.

Classes, APIs, files, architecture, implementation sequence, and task breakdown belong in implementation planning unless they are explicit requirements.

## Implementation Planning

An implementation plan defines how the current codebase should change to satisfy an approved feature specification.

A good implementation plan should be:

- **Grounded** — Inspect the relevant existing code first and base the plan on the actual codebase rather than assumptions.
- **Minimal** — Design only what is needed for the current requirements. Do not implement speculative future requirements.
- **Executable** — The plan should lead to a practical implementation sequence and a way to verify the result.

When planning, consider the following questions where relevant:

- **Current State** — How does the related code currently work?
- **Change** — What needs to be added, removed, or changed?
- **Design** — Are there meaningful design decisions or trade-offs?
- **Sequence** — In what order should the changes be implemented?
- **Verification** — How will the implementation be verified?
- **Risks / Unknowns** — Are there unresolved issues that affect the plan?

These are guiding questions, not a required template.

Document alternatives and trade-offs only when the choice meaningfully affects behavior, structure, maintainability, or scope.

Break the plan into small implementation tasks after the implementation approach is understood. The number and size of tasks should match the complexity of the feature.

## Implementation Conventions

- Implement gameplay logic in C++.
- Expose asset references and tuning values as editable properties, and set them in Blueprint subclasses.

## Task Execution

- Inspect the relevant existing code before making changes. Do not guess how the current implementation works.
- Work within the current task scope.
- If implementation reveals that the task boundary or implementation plan should change, explain why and adjust the plan with the user before expanding the work.
- After implementation, verify the change using the strongest practical method available, such as compilation, tests, or running the relevant behavior in Unreal Editor.