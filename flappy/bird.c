
#include <msp430.h>
#include "lcddraw.h"
#include "lcdutils.h"
#include "bird.h"
/*
 * draw bird
 */

void drawBird() {
  // clear old bird space, draw new bird, make new bird pos old bird pos
  if (oldY) {
    fillRectangle(BIRD_X-1, oldY-1, 3, 3, COLOR_CYAN);
  }
  fillRectangle(BIRD_X-1, birdY-1, 3, 3, COLOR_YELLOW);
  oldY = birdY;
}

void updateBirdPos() {
  birdY += acceleration; 
  drawBird();
}

  
  
  

