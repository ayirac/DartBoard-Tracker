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
#### Isolating the corkboard
<p align="center">
  <img src="https://github.com/ayirac/DartBoard-Tracker/blob/master/fix.png">
</p>

#### Calibration pipeline for locating bullseyes, triples, doubles, and lines

<p align="center">
  <img src="https://github.com/ayirac/DartBoard-Tracker/blob/master/calib-fix.gif" width="300" height="300">
</p>

# Challenges Faced
I learned opencv specifically for this project which took a lot of reading and a few small programs to familarize myself with the library. Laying out the steps needed for the project was easy, but implemntation quickly grew complicated as different methods was availble. Originally, a camera was aimed dead center at the bullseye which made is VERY annoying for players to throw darts. To remedy this, I applied a perspective transform using the 4 corners of a square corkboard to warp the dartboard as if a camera is looking dead-on with it no matter where the camera is. After this, 7 circles (bullseyes, doubles) are detected on the board which is easy with the perfect transformation. I experimented with a semi-automatic calibraiton in which the user doesn't have to adjust any parameters, but it was rough and didn't work as I envisioned so it was canned. 

Detecting the segments (1, 20, 7, etc) was done with this cool method of detecting lines on the dartboard, using the parameters to filter out any incorrect lines. The lines are then merged with their partners into a data structure called a Segment. When a dart is thrown all the segments are iterated through and the two line equations in each segment are checked to see if a point lands between the lines using inequalities. This was a fairly heavy mathametical problem I'm very happy with how the results turned out.
<p align="center">
  <img src="https://github.com/ayirac/DartBoard-Tracker/blob/master/graph.png" width="40%">
</p>

The final challenge I faced was detecting the visible tip of the dart with the highest accuracy/precison possible. I originally used the absdiff function to get the difference between an empty-board calibration image and a board with a dart embedded in it. This would often return far too many shadows and anomalies that were difficult to filter out so I researched for solutions and found the MOG2 Background Subtraction technique. By recording a large amount of frames, all those frames can be used to create a background model that has limited shadows or anomalies. I applied some post-processing to the foreground detected whenever a dart thrown, and it turned out very accurate with only an issue happening due to shadows once in a few play test games.

# Motivation
I enjoy throwing darts and creating an automatic scorer for various games sounded like a good project to tackle. After doing preliminary research into different ways of achieivng this, the opencv library had the tools I required. Putting together the project in a rough draft wasn't too difficult however, I changed implementations for specific steps to make the program as robust as possible which took a lot of time. Overall, I learned a lot about OOP, openCV, manipulating images, and software development.
