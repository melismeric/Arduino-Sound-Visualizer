# Arduino-Sound-Visualizer
Sound interactive led strip coded with Arduino Nano and Sound Sensor

In my undergraduate years I learned programming Arduino Boards. After I graduated I decided to implement a music visualizer at home using a led strip, Arduino board, and a sound detector. I used Arduino Nano and a Sparkfun sound detector. The sound sensor takes the music as input and retrieves the volume. The RGB led strip is an addressable one, which I soldered to add to the circuit. The circuit plan is similar to the one below.

In the code with the sound sensor, we keep a record of the volume, and when there is a higher volume than the average or the highest loudness, we store it. There are two visual effects I used. In the first one, we see the rainbow colors at the center, and as the sound increases, the LEDs turn from the middle to the right and left. The other visual effect is more complex. The LEDs light after one other creating a dancing effect. The speed and brightness get higher if the volume gets higher. If the volume gets lower, the LEDs fade out. If there is a "bump" in the sound, it is visualized too.

For the future work, I am planning to add a function to detect the mood of sound (possibly using Openframeworks) and select the colors according to it. 

Resource: https://learn.sparkfun.com/tutorials/interactive-led-music-visualizer/all
