# Feature Specification: Time-Travel Puzzle Game

**Feature Branch**: `001-build-a-time`
**Created**: September 25, 2025
**Status**: Draft
**Input**: User description: "Build a time-travel puzzle game where players manipulate historical events to prevent a dystopian future. Use a dynamic time map with branching timelines, nodes (e.g., 1066 Battle of Hastings), and color-coded Causality Links (green=stable, red=high-risk). Include a Paradox Meter (0-100, triggers Time Wraiths at 60/90) to track stability. Time Wraiths block nodes or revert changes when meter spikes. Players solve era-specific puzzles (e.g., deciphering texts, sabotaging war machines), manage limited time energy, and explore branches for an ideal timeline. Nodes unlock new events; Paradox Meter increases with changes (+5 minor, +20 major); Wraiths countered by stabilizing nodes or puzzles. Ensure one puzzle per era. Clarify: Time map visualizes branches with draggable nodes; Causality Links animate stability; Paradox Meter as visual orb (cracks at high values); Wraiths as era-specific sprites (e.g., ghostly warrior). Time energy limits interventions. Branches unlock clues for ideal timeline. Use Vite with vanilla HTML, CSS, JavaScript. Store timeline data in local SQLite database. Time map as interactive canvas with Web API drag-and-drop for nodes, animated Causality Links. Paradox Meter as dynamic UI element. Time Wraiths as CSS-animated sprites. Support real-time updates for meter and drag-and-drop. Break down tasks and generate actionable task list. Implement per plan. Notes: Use Vite, vanilla HTML/CSS/JS, SQLite. Time map as canvas; Wraiths as sprites. Focus on puzzles, strategy, narrative. Track progress with pull requests (gh pr create)."

---

## Execution Flow (main)
```
1. Parse user description from Input
   → If empty: ERROR "No feature description provided"
2. Extract key concepts from description
   → Identify: actors, actions, data, constraints
3. For each unclear aspect:
   → Mark with [NEEDS CLARIFICATION: specific question]
4. Fill User Scenarios & Testing section
   → If no clear user flow: ERROR "Cannot determine user scenarios"
5. Generate Functional Requirements
   → Each requirement must be testable
   → Mark ambiguous requirements
6. Identify Key Entities (if data involved)
7. Run Review Checklist
   → If any [NEEDS CLARIFICATION]: WARN "Spec has uncertainties"
   → If implementation details found: ERROR "Remove tech details"
8. Return: SUCCESS (spec ready for planning)
```

---

## Key Concepts
- Actors: Player, Time Wraiths
- Actions: Manipulate historical events, solve puzzles, manage time energy, stabilize nodes, counter Wraiths
- Data: Timeline nodes, Causality Links, Paradox Meter, time energy, puzzle states, timeline branches
- Constraints: Limited time energy, Paradox Meter thresholds, one puzzle per era, Wraiths block/revert changes, stability management

## User Scenarios & Testing
- Player explores a time map with draggable nodes representing historical events
- Player manipulates nodes to change history, observing Causality Links (color-coded for stability)
- Paradox Meter increases with changes; at thresholds (60/90), Time Wraiths appear and block/revert changes
- Player solves era-specific puzzles to stabilize nodes and counter Wraiths
- Player manages limited time energy to optimize interventions
- Player explores branches to discover clues and achieve an ideal timeline
- Each era presents a unique puzzle; solving unlocks new events
- Test: Simulate timeline changes, verify Paradox Meter and Wraith behavior, ensure puzzles are solvable and unique per era

## Functional Requirements
- Dynamic time map with draggable, branching nodes
- Color-coded Causality Links (green=stable, red=high-risk) with animation
- Paradox Meter (0-100) as a visual orb, cracks at high values
- Time Wraiths appear at meter 60/90, block nodes or revert changes, visualized as era-specific sprites
- One puzzle per era, each puzzle must be solvable and era-appropriate
- Limited time energy resource, decrements with interventions
- Nodes unlock new events when solved/stabilized
- Paradox Meter increases: +5 (minor), +20 (major) changes
- Wraiths can be countered by stabilizing nodes or solving puzzles
- Branches unlock clues for ideal timeline
- All requirements must be testable via user interaction and state changes

## Key Entities
- Timeline Node: id, era, event, stability, unlocked, puzzle
- Causality Link: source, target, stability, color
- Paradox Meter: value, visual state
- Time Wraith: era, active, blocked nodes
- Time Energy: current, max
- Puzzle: era, type, solved

## Review Checklist
- [ ] All user flows are described and testable
- [ ] All requirements are unambiguous and testable
- [ ] No implementation/tech details in requirements
- [ ] All ambiguities are marked with [NEEDS CLARIFICATION]
- [ ] Key entities are identified
- [ ] Spec is ready for planning

---

## Ambiguities / Needs Clarification
- [NEEDS CLARIFICATION: What is the minimum/maximum number of eras and nodes per timeline?]
- [NEEDS CLARIFICATION: How is "time energy" replenished, if at all?]
- [NEEDS CLARIFICATION: What constitutes a "minor" vs. "major" change?]
- [NEEDS CLARIFICATION: Are there win/lose conditions beyond achieving the ideal timeline or Paradox Meter maxing out?]
- [NEEDS CLARIFICATION: How are clues presented to the player when branches are explored?]
