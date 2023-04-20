#include "vga.h"
#include "kb.h"

class Tct {
    VGA* vga;
    char board[3][3];
    uint32_t turnNum;
    const uint8_t BLACK = 0;
    uint8_t cursor[2];
    const int DIRECTIONS [8][2] = { {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1} };
    
    Tct(VGA* v) : vga(v) {
        for (int i = 0; i < 3; i ++) {
            for (int j = 0; j < 3; j ++) {
                board[i][j] = '.';
            }
        }
        turnNum = 1;
        vga->initializeScreen(63);
        vga->drawRectangle(115, 55, 205, 145, BLACK, false);
        vga->drawLine(145, 55, 145, 145, BLACK);
        vga->drawLine(185, 55, 185, 145, BLACK);
        vga->drawLine(115, 85, 205, 85, BLACK);
        vga->drawLine(115, 115, 205, 115, BLACK);
    }

    void playTurn() {
        cursor[0] = 0; 
        cursor[1] = 0;
        if (turnNum % 2 == 1) {
            // playPlus();
        } else {
            // playCircle();
        }
        checkWin();
        turnNum ++;
    }

    void playPlus();

    void playCircle();

    void checkWin() {
        for (int i = 0; i < 8; i ++) {
            inBounds(cursor[0], cursor[1], i);
            
        }
    }

    bool inBounds(int r, int c, int d) {
        if (r + DIRECTIONS[d][0] > 2 || r + DIRECTIONS[d][0] < 0) return false;
        if (c + DIRECTIONS[d][1] > 2 || c + DIRECTIONS[d][1] < 0) return false;
        return true;
    }
    
    
};