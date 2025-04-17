#ifndef bird_included
#define bird_included

#define GRAVITY 5
#define BIRD_X 20

extern int birdY;
extern int oldY;
extern int acceleration;

void drawBird();
void updateBirdPos();

#endif
