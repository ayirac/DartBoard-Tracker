<h1>DartBoard Tracker</h1>
Tracks thrown darts at a physical dartboard and records the hit segment's ID & multiplier. A game can be selected for two players to play against, or someone can practice throwing by themselves.
GIF showing the game<<

<h2>Features :sparkles:</h2>
- Straightforward calibration
- Profile system that allows for calibration settings to be saved
- Keybinds for maniuplating state
  * Works with different lighting circumstances
  * Fixed score games such as 301 for two players, includes options for doubling in/out.
  * Simple UI utilizied to keep track of Game score/status
test
<h2>Calibration :wrench:</h2>
Only one camera is required and it should be positioned to the left and away from the well-lit Dartboard. Lighting is important to dampen the effects of shadows (resulting in higher dart accuracy), Dartboard light rings are perfect but placed floodlights work as well. A square corkboard behind the dartboard is required as well.

*Trackbars are utilizied to allow the user to fine tune the calibration for each specific stage. The calibration keybind 'space' will progress the calibration stages.
*The first step is isolating the Dartboard corkboard and determining its corner points and the warp factors needed to achieve a perfect circle. After the calibration keybind is pressed, a perspective transformation is applied to crop the corkboard rectangle.
*The second step is isolating the Dartboard so future calculations don't need to account for the cork.
*The third step is to identify the inner/outer boundary circles for bullseye, double, and triple.
*The fourth step is to identify the segment lines for [1..20]
*The fifth and final step is entering the specific game/rules that a player wishes to play.
*Finally, a empty Dartboard is recorded for 50 frames to generate a background image using MOG2, which is used for forground segmentation during the dart detection phase.

<h2>Playing a Game :dart:</h2>
A FixedScore game of 301 was chosen.

<h2>Motivation :running:</h2>
..
..
