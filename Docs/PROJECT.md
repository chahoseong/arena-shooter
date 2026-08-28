# Project

## Purpose

이 프로젝트는 Unreal Engine으로 하나의 게임을 처음부터 끝까지 완성하면서, 게임 개발의 전체 사이클을 경험하기 위한 학습 프로젝트다.

## Game Overview

플레이어가 하나의 아레나에서 이동과 원거리 공격을 사용해 적과 전투하는 싱글플레이 3인칭 아레나 슈터다. 각 Wave의 적을 모두 처치하면 다음 Wave로 진행하며, 마지막 Wave를 클리어하면 승리한다.

## Core Loop

```text
Move & Shoot
→ Defeat Enemies
→ Clear Wave
→ Advance
→ Repeat
```

## Scope

### Player

- Third-person movement and camera
- Jump
- Aim
- Basic ranged attack
- Dash
- Health, hit, and death

### Enemy

- One melee enemy type
- Detect and target the player
- Move toward the player
- Basic melee attack
- Health, hit, and death

### Game Flow

- Three waves
- Spawn enemies for each wave
- Track remaining enemies
- Advance to the next wave when all enemies are defeated
- Victory after clearing Wave 3
- Defeat when the player dies
- Restart after Victory or Defeat

### UI / Feedback

- Crosshair
- Player health
- Current wave
- Victory / Defeat state
- Restart prompt
- Basic feedback for attacks, hits, and deaths

## Non Goals

This project does not include:

- progression or economy systems such as equipment, inventory, loot, leveling, or shops;
- additional content such as multiple weapons, enemy types, bosses, or levels;
- multiplayer, dedicated servers, or online systems;
- additional game systems that are not required by the defined core loop, such as save/load, quests, or procedural generation;
- Gameplay Ability System (GAS).

## Definition of Done

The project is complete when:

- all features defined in `Scope` are implemented;
- the game can be played from start to Victory or Defeat and restarted;
- there are no known bugs that block or break the core game loop;
- the project builds successfully and a packaged game can run outside the Unreal Editor;
- the playable game has been profiled at least once to inspect its runtime performance.