#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

char blue = 31, green = 0, red = 31;
unsigned char step = 0;

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}


// axis zero for col, axis 1 for row

short drawPos[2] = {40,screenHeight/2}, controlPos[2] = {40, (screenHeight/2)-1};
short rowVelocity = 0, rowLimits[2] = {1, screenHeight};
short rowAccel = 1, jumpHeight = 7;

void
erase_bird(int col, int row)
{
  fillRectangle(col-3, row-3, 7, 10, COLOR_CYAN);
  fillRectangle(col-6,row-3, 4, 7, COLOR_CYAN);
  fillRectangle(col-8, row+1, 3, 3, COLOR_CYAN);
  drawPixel(col-7, row, COLOR_CYAN);
  fillRectangle(col+2, row, 4, 4, COLOR_CYAN);
}

void
draw_bird(int col, int row)
{
  //OUTLINE
  fillRectangle(col-3, row-3, 7, 10, COLOR_BLACK);
  fillRectangle(col-6,row-3, 4, 7, COLOR_BLACK);
  fillRectangle(col-8, row+1, 3, 3, COLOR_BLACK);
  drawPixel(col-7, row, COLOR_BLACK);
  fillRectangle(col+2, row, 4, 4, COLOR_BLACK);

  //BODY
  fillRectangle(col-2, row-2, 5, 8, COLOR_YELLOW);
  fillRectangle(col-5,row-2, 3, 5, COLOR_YELLOW);
  drawPixel(col-6, row+1, COLOR_YELLOW);
  drawPixel(col-6, row+2, COLOR_YELLOW);
  drawPixel(col-7, row+2, COLOR_YELLOW);

  //BEAK
  fillRectangle(col+3, row+1, 2, 2, COLOR_ORANGE_RED);

  //EYE
  drawPixel(col+1, row+4, COLOR_BLACK);
}

short pipePos[3] = {screenWidth, screenHeight/2, 40}, pipeControl[3] = {screenWidth+2, screenHeight/2, 40};
short pipeVelocity = 2, pipeLimits[3] = {0, 25, 50};

void
erase_pipe(int col, int row, int offset)
{
  int botRow = row - offset/2;
  int topRow = row + offset/2;

  fillRectangle(col-12, 0, 25, botRow+6, COLOR_CYAN);
  fillRectangle(col-12, topRow-5, 25, screenHeight, COLOR_CYAN);
}

void
draw_pipe(int col, int row, int offset)
{
  int botRow = row - offset/2;

  //OUTLINE
  fillRectangle(col-12, botRow-5, 25, 11, COLOR_BLACK);
  fillRectangle(col-9, 0, 19, botRow-5, COLOR_BLACK);

  //MAIN LAYER
  fillRectangle(col-11, botRow-4, 23, 9, COLOR_GREEN);
  fillRectangle(col-8, 0, 17, botRow-7, COLOR_GREEN);
  
  //DETAILS
  fillRectangle(col-8, botRow-7, 17, 2, COLOR_DARK_GREEN);
  fillRectangle(col-9, botRow-3, 1, 7, COLOR_WHITE);
  fillRectangle(col-6, 0, 1, botRow-9, COLOR_WHITE);

  //MIRROR
  
  int topRow = row + offset/2;
  
  //OUTLINE
  fillRectangle(col-12, topRow-5, 25, 11, COLOR_BLACK);
  fillRectangle(col-9, topRow+5, 19, screenHeight, COLOR_BLACK);

  //MAIN LAYER
  fillRectangle(col-11, topRow-4, 23, 9, COLOR_GREEN);
  fillRectangle(col-8, topRow+7, 17, screenHeight, COLOR_GREEN);
  
  //DETAILS
  fillRectangle(col-8, topRow+6, 17, 1, COLOR_DARK_GREEN);
  fillRectangle(col-9, topRow-3, 1, 7, COLOR_WHITE);
  fillRectangle(col-6, topRow+8, 1, screenHeight, COLOR_WHITE);
}


void
screen_update_bird()
{
  for (char axis = 0; axis < 2; axis ++) 
    if (drawPos[axis] != controlPos[axis]) /* position changed? */
      goto redraw;
  return;			/* nothing to do */
 redraw:
  erase_bird(drawPos[0], drawPos[1]); /* erase */
  for (char axis = 0; axis < 2; axis ++) 
    drawPos[axis] = controlPos[axis];
  draw_bird(drawPos[0], drawPos[1]); /* draw */
}

void
screen_update_pipe()
{
  for (char i = 0; i < 3; i++)
    if (pipePos[i] != pipeControl[i])
      goto redrawPipe;
  return;
redrawPipe:
  erase_pipe(pipePos[0], pipePos[1], pipePos[2]);
  for (char i = 0; i < 3; i++)
    pipePos[i] = pipeControl[i];
  draw_pipe(pipePos[0], pipePos[1], pipePos[2]);
}
  
void update_screen();
static int birdCount = 0;
static int pipeCount = 0;
static int randint = 0;

void wdt_c_handler()
{
  randint++;
  birdCount ++;
  pipeCount++;

  if (pipeCount >= 45) {
    short oldPipePos = pipeControl[0];
    short newPipePos = oldPipePos - 3*pipeVelocity;
    if (newPipePos > pipeLimits[0])
    {
      pipeControl[0] = newPipePos;
    }
    else
    {
      pipeControl[0] = screenWidth;
      pipeControl[1] = screenHeight/2 + (randint % 80 - 40);
      pipeControl[2] = 60 - randint % 15;
      pipeVelocity = 6 - randint % 4;
    }

    pipeCount = 0;
  }

  if (birdCount >= 15) 		/* 10/sec */
  {
    short oldRow = controlPos[1];
    rowVelocity -= rowAccel;
    short newRow = oldRow + rowVelocity;

    if (newRow <= rowLimits[0] || newRow >= rowLimits[1])
      rowVelocity = -rowVelocity;
    else
      controlPos[1] = newRow;

    update_screen();

    birdCount = 0;
  }

  if (switches & SW1)
  {
    switches &= ~SW1;
    rowVelocity = jumpHeight;
    randint -= (randint >> 2) + (randint % 300); //equation to somewhat randomize
  }
}

void main()
{
  configureClocks();
  lcd_init();
  clearScreen(COLOR_CYAN);
  switch_init();

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x18);	              /**< GIE (enable interrupts) */
}

    
void
update_screen()
{
  screen_update_bird();
  screen_update_pipe();
}
   


void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
