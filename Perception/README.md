## Summary of Chapter 4 (Perception):

### Experiment 1:
This is a low pass filter -- it weights new data against the old one. Helps in reducing jitter.

### Experiment 2:
This is a moving average filter. Uses a deque data structure. 

### Experiment 3:
This is a deadband. Not much else to say.

### Experiment 4:
This is a median filter. We define a window of measurements (ideally odd numbered), then create a copy, sort that copy, and extract the median. 

### Experiment 5:
Pretty much just a low-pass filter, but with two inputs instead of one. 
---
