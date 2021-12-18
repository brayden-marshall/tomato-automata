# Tomato Automata (Work in Progress)

A cellular automata simulation using SDL2 and Dear Imgui.

![App Screenshot](./screenshots/example.png)

## Todo

- Implement zooming/panning on grid
- Implement brush sizing (allow to paint/erase in larger circles at once)
- Implement ability to use different rulesets for variations of Langton's Ant
- Fix issue with UI becoming unresponsive:
    - could adjust speed so that fps stays above 60 and UI becomes responsive (prefer this for now)
    - could draw on a different thread so that main thread stays responsive
    - definitely look into optimizing render and update algorithms so speed can stay up
