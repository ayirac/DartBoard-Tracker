# DartBoard Tracker :dart:
Determines the score of a dart on a dartboard, allowing two players to play different games with eachother. Utilzies the opencv c++ library.
<p align="center">
  <img src="https://github.com/ayirac/DartBoard-Tracker/blob/master/301-progression.gif">
  <sub>A game of 301 between two players without doubling in</sub>
</p>
## Features :sparkles:
- Straightforward calibration
- Profile system that allows for calibration settings to be saved
- Keybinds for maniuplating state
- Works with different lighting circumstances
- Fixed score games such as 301 for two players, includes options for doubling in/out.
- Simple UI utilizied to keep track of the current scores

## Calibration :wrench:
### Positioning Equipment (lights, cameras)
Only one camera is required and it should be positioned to the left and away from the well-lit Dartboard. Lighting is important to dampen the effects of shadows (resulting in higher dart accuracy), Dartboard light rings are perfect but placed floodlights work as well. A square corkboard behind the dartboard is required as well.

### Steps
* Trackbars are utilizied to allow the user to fine tune the calibration for each specific stage. The calibration keybind 'space' will progress the calibration stages.
1. The first step is isolating the Dartboard corkboard by determining its corner points and the warp factors needed to achieve a perfect circle.
2. The second step is isolating the Dartboard so future calculations don't need to account for the cork.
3. The third step is to identify the inner/outer boundary circles for bullseye, double, and triple. Resulting in six boundaries.
4. The fourth step is to identify the segment lines for [1..20]
5. The fifth and final step is entering the specific game/rules that a player wishes to play via CLI
6. Finally, an empty Dartboard is recorded for 50 frames to generate a background image using MOG2, which is used for forground segmentation during the dart detection phase.
<p align="center">
  <img src="https://github.com/ayirac/DartBoard-Tracker/blob/master/fix.png">
  <sub>Isolating the corkboard</sub>
</p>
<p align="center">
  <img src="https://github.com/ayirac/DartBoard-Tracker/blob/master/calib-fix.gif" width="300" height="300">
</p>
<p align="center">
  <sub>Calibration pipeline for locating bullseyes, triples, doubles, and lines</sub>
</p>

# Motivation
I enjoy throwing darts and creating an automatic scorer for various games sounded like a good project to tackle. After doing preliminary research into different ways of achieivng this, the opencv library had the tools I required. Putting together the project in a rough draft wasn't too difficult however, I changed implementations for specific steps to make the program as robust as possible which took a lot of time. Overall, I learned a lot about OOP, openCV, manipulating images, and software development.
