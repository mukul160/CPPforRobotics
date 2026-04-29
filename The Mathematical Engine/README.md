## Summary of this Chapter

This chapter was all about learning the fundamental math behind robotics. I now know how to set up vectors, and use them to calculate the dot product and the cross product; including their physical significance. 

I think the concept of operator overloading was new? 
Haven't tried that before. 

And yeah, this code here?

```c
    //The Dot Product Formula: (x1 * x2) + (y1 * y2)
    double dot(const Vector2D& other) const {
        return (x * other.x) + (y * other.y);
    }
```

- I now understand what the 'const' means in this context.
- I calculated the angle between 2 vectors. That's bound to be useful later.

No notes on the rotation matrix. 
Just a bit of caution on this point:

```c
 struct Pose2D {
    Vector2D position;
    double theta; //Orientation in Degrees

    //The 'Transform' - the bread and butter of robotics navigation
    Vector2D transformToGlobal(const Vector2D& localPoint) const {
        // Step 1: Rotate the local point by the robot's current heading
        Vector2D rotatedPoint = localPoint.rotated(theta);

        //Step 2: Translate (add) the robot's current position
        return position + rotatedPoint;
    }
 };
 ```

 During the transform: always rotate the argument first, then translate it. Not the other way around. This is because we want any object's reference frame to be aligned with the global frame before we can answer how close or far it is from the global origin.

 ---
