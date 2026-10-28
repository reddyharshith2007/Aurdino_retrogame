//Import modules
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
// --- Pins ---
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_MISO 19
#define TFT_LED  15    
#define JOY_VRX  34   
#define JOY_VRY  35   
#define JOY_SW   32   
#define BTN_A    33
#define BTN_B    25
int menuIndex = 0;  // 0 = Dino, 1 = Snake, 2 = Pong
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
enum GameState { STATE_MENU, STATE_DINO, STATE_SNAKE, STATE_PONG };
GameState currentState = STATE_MENU;
// Bitmaps
const unsigned char dino_bmp[] PROGMEM = { 0x00, 0x07, 0xFE, 0x00, 0x05, 0xFF, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xF0, 0x00, 0x07, 0xFF, 0x00, 0x07, 0xE0, 0x00, 0x07, 0xE0, 0x00, 0x07, 0xE0, 0x40, 0x07, 0xC0, 0x40, 0x07, 0xC0, 0x60, 0x1F, 0xC0, 0x70, 0x3F, 0x80, 0x78, 0x7F, 0x00, 0x7C, 0xFF, 0x00, 0x7F, 0xFF, 0x00, 0x3F, 0xFE, 0x00, 0x0F, 0xFE, 0x00, 0x07, 0xFC, 0x00, 0x03, 0xF8, 0x00, 0x01, 0xD0, 0x00, 0x01, 0x90, 0x00, 0x01, 0x90, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00 };
const unsigned char cactus_bmp[] PROGMEM = { 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x21, 0x84, 0x21, 0x84, 0x21, 0x84, 0x21, 0x84, 0x21, 0x8C, 0x21, 0xCC, 0x31, 0xCC, 0x3F, 0xCC, 0x1F, 0xCC, 0x01, 0xFC, 0x01, 0xF8, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80 };
//Game constanta
#define GROUND_Y 200
#define DINO_WIDTH 24
#define DINO_HEIGHT 24
#define CACTUS_WIDTH 16
#define CACTUS_HEIGHT 24
#define CELL_SIZE 10
#define GRID_W 32   
#define GRID_H 24   
#define MAX_LENGTH 100
#define SCORE_BAR_HEIGHT 20
// --- Pong Variables ---
int paddleY, oldPaddleY;
int ballX, ballY, oldBallX, oldBallY;
int ballVX, ballVY;
int pongScore;
bool pongGameOver;
unsigned long lastPongUpdate;
//pong variables
int snakeX[MAX_LENGTH], snakeY[MAX_LENGTH];
int snakeLength;
int foodX, foodY;
int dirX, dirY;
bool snakeGameOver;
unsigned long lastMoveTime;
int snakeScore;
float dinoY, dinoVY, cactusX, cactusSpeed;
bool isJumping, gameOver;
int score;
unsigned long lastActionTime = 0; 

void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  tft.begin();
  tft.setRotation(1);
  showMainMenu();
}

void loop() {
  if (currentState == STATE_MENU) {
    updateMenu();
  } else if (currentState == STATE_DINO) {
    updateDino();
  } else if (currentState == STATE_SNAKE) {
    updateSnake();
  }
  if(currentState == STATE_PONG){
    updatePong();
  }
}

void showMainMenu() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(60, 40);
  tft.print("ESP32 ARCADE");
  tft.setTextSize(2);
  // Dino
  tft.setCursor(70, 100);
  tft.setTextColor(menuIndex == 0 ? ILI9341_CYAN : ILI9341_WHITE);
  tft.print("1. Dino Game");
  // Snake
  tft.setCursor(70, 130);
  tft.setTextColor(menuIndex == 1 ? ILI9341_CYAN : ILI9341_WHITE);
  tft.print("2. Snake Game");
  // Pong
  tft.setCursor(70, 160);
  tft.setTextColor(menuIndex == 2 ? ILI9341_CYAN : ILI9341_WHITE);
  tft.print("3. Pong Game");
  tft.setCursor(70, 200);
  tft.setTextColor(ILI9341_GREEN);
  tft.print("Use Joystick + A");
}

void updateMenu() {
  int yVal = analogRead(JOY_VRY);
  // Move selection up/down
  if (yVal < 1500 && millis() - lastActionTime > 300) {
    menuIndex = (menuIndex - 1 + 3) % 3;  // wrap around
    lastActionTime = millis();
    showMainMenu();
  }
  else if (yVal > 3000 && millis() - lastActionTime > 300) {
    menuIndex = (menuIndex + 1) % 3;  // wrap around
    lastActionTime = millis();
    showMainMenu();
  }
  // Confirm selection with Button A
  if (digitalRead(BTN_A) == LOW && millis() - lastActionTime > 300) {
    lastActionTime = millis();
    if (menuIndex == 0) {
      resetGame();
      currentState = STATE_DINO;
    } else if (menuIndex == 1) {
      resetSnake();
      currentState = STATE_SNAKE;
    } else if (menuIndex == 2) {
      resetPong();
      currentState = STATE_PONG;
    }
  }
}

void updateDino() {
  // 1. Exit Logic (Non-blocking)
  if (digitalRead(BTN_B) == LOW && millis() - lastActionTime > 500) {
    Serial.println("Exiting...");
    lastActionTime = millis();
    currentState = STATE_MENU;
    showMainMenu();
    return;
  }
  // Game Over Logic
  if (gameOver) {
    tft.setCursor(100, 100);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print("GAME OVER");
    // Restart game with A
    if (digitalRead(BTN_A) == LOW && millis() - lastActionTime > 500) {
       lastActionTime = millis();
       resetGame(); 
    }
    // Exit to menu with B (NEW!)
    if (digitalRead(BTN_B) == HIGH && millis() - lastActionTime > 500) {
       Serial.println("Exiting from Game Over...");
       lastActionTime = millis();
       currentState = STATE_MENU;
       showMainMenu();
    }
    return;   // end frame
  }
  // --- Normal gameplay logic continues here ---
  bool jumpInput = (digitalRead(BTN_A) == LOW || digitalRead(JOY_SW) == LOW || analogRead(JOY_VRY) < 1500);
  if (jumpInput && !isJumping) {
    dinoVY = -8.0;
    isJumping = true;
  }
  // 4. Physics & Movement
  float oldDinoY = dinoY;
  float oldCactusX = cactusX;
  dinoVY += 0.55; // Gravity
  dinoY += dinoVY;
  if (dinoY >= GROUND_Y - DINO_HEIGHT) {
    dinoY = GROUND_Y - DINO_HEIGHT;
    dinoVY = 0;
    isJumping = false;
  }
  cactusX -= cactusSpeed;
  if (cactusX < -CACTUS_WIDTH) {
    // Clear the ghost cactus when it resets
    tft.fillRect(0, GROUND_Y - CACTUS_HEIGHT, CACTUS_WIDTH + 10, CACTUS_HEIGHT, ILI9341_BLACK);
    cactusX = 320;
    score++;
    cactusSpeed = min(cactusSpeed + 0.15, 12.0); 
  }
  // 5. Collision Check
  if (checkCollision()) {
    gameOver = true;
    Serial.println("Hit!");
    return;
  }
  // 6. Optimized Rendering (Erase only what moved)
  // Erase Dino's previous position if it moved
  if ((int)oldDinoY != (int)dinoY) {
    tft.fillRect(30, (int)oldDinoY, DINO_WIDTH, DINO_HEIGHT, ILI9341_BLACK);
  }
  tft.drawBitmap(30, (int)dinoY, dino_bmp, DINO_WIDTH, DINO_HEIGHT, ILI9341_WHITE, ILI9341_BLACK);
  // Erase Cactus trail
  int trail = (int)oldCactusX - (int)cactusX;
  if (trail > 0) {
    tft.fillRect((int)cactusX + CACTUS_WIDTH, GROUND_Y - CACTUS_HEIGHT, trail + 2, CACTUS_HEIGHT, ILI9341_BLACK);
  }
  tft.drawBitmap((int)cactusX, GROUND_Y - CACTUS_HEIGHT, cactus_bmp, CACTUS_WIDTH, CACTUS_HEIGHT, ILI9341_GREEN, ILI9341_BLACK);
  // 7. UI Update
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print("Score: "); tft.print(score);
  tft.drawLine(0, GROUND_Y, 320, GROUND_Y, ILI9341_WHITE);
  delay(10); // Framerate control
}
bool checkCollision() {
  // Dino is fixed at X=30. Cactus is at X=cactusX.
  // Hitbox margins (m) make it more forgiving
  int m = 4; 
  bool overlapX = (30 + m < cactusX + CACTUS_WIDTH - m) && (30 + DINO_WIDTH - m > cactusX + m);
  bool overlapY = (dinoY + m < GROUND_Y) && (dinoY + DINO_HEIGHT - m > GROUND_Y - CACTUS_HEIGHT + m);
  return overlapX && overlapY;
}

void resetGame() {
  tft.fillScreen(ILI9341_BLACK);
  dinoY = GROUND_Y - DINO_HEIGHT;
  dinoVY = 0;
  isJumping = false;
  cactusX = 320;
  cactusSpeed = 4.5;
  score = 0;
  gameOver = false;
  tft.drawLine(0, GROUND_Y, 320, GROUND_Y, ILI9341_WHITE);
  lastActionTime = millis();
}

// --- Snake Game Functions ---
void resetSnake() {
  tft.fillScreen(ILI9341_BLACK);
  // Draw top boundary bar
  tft.fillRect(0, 0, 320, SCORE_BAR_HEIGHT, ILI9341_BLUE);
  snakeLength = 3;
  snakeX[0] = GRID_W/2; snakeY[0] = GRID_H/2;
  snakeX[1] = snakeX[0]-1; snakeY[1] = snakeY[0];
  snakeX[2] = snakeX[1]-1; snakeY[2] = snakeY[1];
  dirX = 1; dirY = 0;
  // Food must spawn below the score bar
  foodX = random(0, GRID_W);
  foodY = random(SCORE_BAR_HEIGHT/CELL_SIZE, GRID_H);
  snakeGameOver = false;
  snakeScore = 0;
  lastMoveTime = millis();
}

void updateSnake() {
  // Exit to menu
  if (digitalRead(BTN_A) == LOW && millis() - lastActionTime > 500) {
    resetSnake();          // <-- restart Snake
    lastActionTime = millis();
}
  if (snakeGameOver) {
    // Show Game Over once
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(60, 100);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.print("GAME OVER");
    tft.setCursor(40, 130);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("Score: "); tft.print(snakeScore);
    delay(1500); // short pause so player sees the message
    // Exit back to menu automatically
    currentState = STATE_MENU;
    showMainMenu();
    return;
}
  // --- Direction from joystick ---
  int xVal = analogRead(JOY_VRX);
  int yVal = analogRead(JOY_VRY);
  if (xVal < 1500 && dirX != 1) { dirX = -1; dirY = 0; }
  else if (xVal > 3000 && dirX != -1) { dirX = 1; dirY = 0; }
  else if (yVal < 1500 && dirY != 1) { dirX = 0; dirY = -1; }
  else if (yVal > 3000 && dirY != -1) { dirX = 0; dirY = 1; }
  // --- Move snake every 150ms ---
  if (millis() - lastMoveTime > 150) {
    lastMoveTime = millis();
    // Shift body
    for (int i = snakeLength-1; i > 0; i--) {
      snakeX[i] = snakeX[i-1];
      snakeY[i] = snakeY[i-1];
    }
    // Move head
    snakeX[0] += dirX;
    snakeY[0] += dirY;
    // Check collisions
    // Check collisions with walls
    if (snakeX[0] < 0 || snakeX[0] >= GRID_W || snakeY[0] < SCORE_BAR_HEIGHT/CELL_SIZE || snakeY[0] >= GRID_H) {
      snakeGameOver = true;
    }
    for (int i = 1; i < snakeLength; i++) {
      if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) snakeGameOver = true;
    }
    // Eat food
    if (snakeX[0] == foodX && snakeY[0] == foodY) {
      if (snakeLength < MAX_LENGTH) snakeLength++;
      snakeScore++;
      foodX = random(0, GRID_W);
      foodX = random(0, GRID_W);
      foodY = random(SCORE_BAR_HEIGHT / CELL_SIZE, GRID_H);
    }
    // --- Render optimized ---
      // Erase tail (last segment)
      tft.fillRect(snakeX[snakeLength-1]*CELL_SIZE, snakeY[snakeLength-1]*CELL_SIZE,
                CELL_SIZE, CELL_SIZE, ILI9341_BLACK);
      // Draw head
      tft.fillRect(snakeX[0]*CELL_SIZE, snakeY[0]*CELL_SIZE,
                CELL_SIZE, CELL_SIZE, ILI9341_GREEN);
      // Draw food (always redraw to avoid erasure)
      tft.fillRect(foodX*CELL_SIZE, foodY*CELL_SIZE, CELL_SIZE, CELL_SIZE, ILI9341_RED);
      // Draw score at top
      // Clear score bar area only
        tft.fillRect(0,0,320,SCORE_BAR_HEIGHT,ILI9341_BLUE);
        tft.setCursor(5, 5);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.print("Score: "); tft.print(snakeScore);
  }
}

void resetPong() {
  tft.fillScreen(ILI9341_BLACK);
  // Draw score bar
  tft.fillRect(0, 0, 320, SCORE_BAR_HEIGHT, ILI9341_BLUE);
  paddleY = 120;
  ballX = 160; 
  ballY = 120;
  // --- RANDOM INITIAL MOVEMENT ---
  // ballVX: Random speed between 3 and 5 (Always positive = moving RIGHT)
  ballVX = random(2,4); 
  // ballVY: Random speed between 2 and 4
  ballVY = random(1,3);
  // 50% chance to make the ball go UP or DOWN initially
  if (random(0, 2) == 0) {
    ballVY *= -1;
  }
  pongScore = 0;
  pongGameOver = false;
  lastPongUpdate = millis();
  // Draw initial state
  tft.fillRect(10, paddleY, 10, 40, ILI9341_WHITE);
  tft.fillRect(ballX, ballY, 8, 8, ILI9341_YELLOW);
  tft.setCursor(200, 10);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLUE);
  tft.print("Score: 0");
}

void updatePong() {
  // Exit to menu
  if (digitalRead(BTN_B) == LOW && millis() - lastActionTime > 500) {
    lastActionTime = millis(); // Reset timer
    currentState = STATE_MENU;
    showMainMenu();
    return;
  }
  if (pongGameOver) {
    // Only draw the Game Over text once to prevent flickering
    tft.setCursor(100, 100);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.print("GAME OVER");
    // Check for Restart (Button A)
    // Removed the strict lastActionTime check or reduced it for faster restart
    if (digitalRead(BTN_A) == LOW) {
      delay(200); // Simple debounce
      lastActionTime = millis(); 
      resetPong();
    }
    return;
  }
  // --- Game Loop ---
  if (millis() - lastPongUpdate > 15) {
    lastPongUpdate = millis();
    int oldPaddleY = paddleY;
    int oldBallX = ballX;
    int oldBallY = ballY;
    // Joystick Logic
    int yVal = analogRead(JOY_VRY);
    if (yVal < 1500) paddleY -= 4;
    else if (yVal > 3000) paddleY += 4;
    paddleY = constrain(paddleY, 20, 180);
    // Ball Logic
    ballX += ballVX;
    ballY += ballVY;
    if (ballY <= 20 || ballY >= 232) ballVY = -ballVY;
    if (ballX <= 20 && ballX >= 10 && ballY >= paddleY && ballY <= paddleY + 40) {
      if (ballVX < 0) { // Only bounce if moving towards the paddle
        ballVX = -ballVX;      // Reverse direction
        ballX = 21;            // MANUALLY MOVE BALL OUTSIDE HITBOX
        pongScore++;
        // Update score text on the blue bar
        tft.setCursor(200, 10);
        tft.setTextColor(ILI9341_CYAN, ILI9341_BLUE);
        tft.print("Score: "); 
        tft.print(pongScore);
      }
    }
    if (ballX < 0) {
        pongGameOver = true;
        lastActionTime = millis(); // Set this so we don't accidentally restart instantly
    }
    if (ballX >= 310) ballVX = -ballVX;
    // --- Rendering ---
    // Update Paddle (only if it moved)
    if (oldPaddleY != paddleY) {
      tft.fillRect(10, oldPaddleY, 10, 40, ILI9341_BLACK);
      tft.fillRect(10, paddleY, 10, 40, ILI9341_WHITE);
    }
    // Update Ball
    tft.fillRect(oldBallX, oldBallY, 8, 8, ILI9341_BLACK);
    tft.fillRect(ballX, ballY, 8, 8, ILI9341_YELLOW);
  }
}