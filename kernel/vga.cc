#include "vga.h"
#include "port.h"

static uint8_t* vga_buf = (uint8_t*) 0xA0000;

VGA::VGA() {
}

uint8_t* VGA::getFrameBuffer() {
    uint8_t seg = (graphics_ctrl.readWithIndex(6)) & (3 << 2);
    if (seg == 0 << 2) return 0;
    if (seg == 1 << 2) return (uint8_t*) 0xA0000;
    if (seg == 2 << 2) return (uint8_t*) 0xB0000;
    if (seg == 3 << 2) return (uint8_t*) 0xB8000;
    Debug::panic("*** should never be here\n");
    return 0;
}

void VGA::setup(Shared<Ext2> root_fs, bool isGraphics) {
    fs = root_fs;
    initializePorts();
    if (isGraphics) {
        bg_color = 42;
        initializePalette();
        initializeGraphics();
        initializeScreen(bg_color);
    } else {
        initTextMode();
    }
    const char* str = "New Romantics";
	drawString(72, 190, str, 63);
    // const char* str = "RODEO";
    // drawString(140, 190, str, 63);
    spotify("new romantics");
    // homeScreen("320");
}

bool VGA::setPortsText(unsigned char* g_90x60_text) {

    // MISC PORT
    misc_port.write(g_90x60_text[0]);

    // SEQ PORT
    for (int i = 1; i < 6; i ++) {
        seq_port.writeWithIndex(i - 1, g_90x60_text[i]);
    }

    //DON'T BLOW IT UP
    outb(crt_ctrl_color.index_port, 0x03);
    outb(crt_ctrl_color.write_port, inb(crt_ctrl_color.write_port) | 0x80); // sets the most sig bit to 1

    outb(crt_ctrl_color.index_port, 0x11);
    outb(crt_ctrl_color.write_port, inb(crt_ctrl_color.write_port) & ~0x80); // risk it for the biscuit later

    g_90x60_text[8] |= 0x80;
    g_90x60_text[22] &= ~0x80;

    // CRTC PORT
    for (int i = 6; i < 31; i ++) {
        crt_ctrl_color.writeWithIndex(i - 6, g_90x60_text[i]);
    }

    // GRAPHICS_CTRL PORT
    for (int i = 31; i < 40; i++) {
        graphics_ctrl.writeWithIndex(i - 31, g_90x60_text[i]);
    }

    // ATTRIBUTE PORT
    for (int i = 40; i < 61; i++) {
        attribute_port.writeWithIndex(i - 40, g_90x60_text[i]);
        // val = inb(ATTRIBUTE_RESET_INDEX); // reset back to index state each time
    }
    int val = inb(ATTRIBUTE_RESET_INDEX);
    outb(attribute_port.index_port, (val | 0x20));

    return true;
}

void VGA::initializePorts() {
    attribute_port.index_port = ATTRIBUTE_INDEX_WRITE;
    attribute_port.write_port = ATTRIBUTE_INDEX_WRITE;
    attribute_port.read_port = ATTRIBUTE_READ;

    misc_port.read_port = MISC_READ;
    misc_port.write_port = MISC_WRITE;

    seq_port.index_port = SEQ_INDEX;
    seq_port.read_port = SEQ_RW;
    seq_port.write_port = SEQ_RW;

    graphics_ctrl.index_port = GRAPHICS_CTRL_INDEX;
    graphics_ctrl.read_port = GRAPHICS_CTRL_RW;
    graphics_ctrl.write_port = GRAPHICS_CTRL_RW;

    crt_ctrl_color.index_port = CRTC_COLOR_INDEX;
    crt_ctrl_color.write_port = CRTC_COLOR_WRITE;

    crt_ctrl_mono.index_port = CRTC_MONOCHROME_INDEX;
    crt_ctrl_mono.write_port = CRTC_MONOCHROME_WRITE;

    dac_mask_port.read_port = DAC_MASK;
    dac_mask_port.write_port = DAC_MASK;
    
    color_pallete_port_write.index_port = COLOR_PALETTE_INDEX_WRITE;
    color_pallete_port_write.write_port = COLOR_PALETTE_DATA_RW;
    
    color_pallete_port_read.index_port = COLOR_PALETTE_INDEX_READ;
    color_pallete_port_read.write_port = COLOR_PALETTE_DATA_RW;
}

void VGA::initializePalette() {
    dac_mask_port.write(0xFF);
    uint8_t num_colors = 64;
    outb(0x3C8, 0);
    for (uint16_t i = 0; i < num_colors*3; i++) {
        outb(0x03C9, palette[i] << 2); // only lower 6 bits used
    }
    // uint8_t num_colors = 256;
    // dac_mask_port.write(0xFF);
    // outb(0x3C8, 0);
    // for (uint16_t i = 0; i < 256*3; i++) {
    //     outb(0x03C9, palette[i]); // only lower 6 bits used
    // }
}

void VGA::initializeGraphics() {
    length = 200;
    width = 320;
    unsigned char g_320x200x256[] =
    {
    /* MISC */
        0x63,
    /* SEQ */
        0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28,	0x40, 0x96, 0xB9, 0xA3,
        0xFF,
    /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
    /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00,	0x00
    };
    setPortsText(g_320x200x256);

}

uint8_t VGA::getColor(uint8_t r, uint8_t g, uint8_t b) {

    const int COLOR_RANGES[] = {0, 85, 170, 255};
    int r_idx = 0, g_idx = 0, b_idx = 0;
    for (int i = 1; i < 4; i++) {
        if (r >= COLOR_RANGES[i-1] && r <= COLOR_RANGES[i]) {
            int below_dist = r - COLOR_RANGES[i-1];
            int above_dist = COLOR_RANGES[i] - r;
            r_idx = below_dist < above_dist ? i-1 : i;
        }
        if (g >= COLOR_RANGES[i-1] && g <= COLOR_RANGES[i]) {
            int below_dist = g - COLOR_RANGES[i-1];
            int above_dist = COLOR_RANGES[i] - g;
            g_idx = below_dist < above_dist ? i-1 : i;
        }
        if (b >= COLOR_RANGES[i-1] && b <= COLOR_RANGES[i]) {
            int below_dist = b - COLOR_RANGES[i-1];
            int above_dist = COLOR_RANGES[i] - b;
            b_idx = below_dist < above_dist ? i-1 : i;
        }
    }
    // Compute the closest 6-bit color value based on the color ranges
    uint8_t r_scaled = COLOR_RANGES[r_idx];
    uint8_t g_scaled = COLOR_RANGES[g_idx];
    uint8_t b_scaled = COLOR_RANGES[b_idx];
    for (uint8_t i = 0; i < 64*3; i+=3) {
        if (r_scaled == palette[i] && g_scaled == palette[i+1] && b_scaled == palette[i+2]) 
            return (i/3);
     }
     Debug::PANIC("NEVER CATCH ME!");
     return 1;
}

void VGA::progressBarInit() {
    drawLine(110, 115, 210, 115, 63);
}


void VGA::playingSong() {
    while (playing) {
        if ((Pit::jiffies - last_jif) / 1000 > 0) {
            last_jif = Pit::jiffies;
            elapsed_time ++;
            uint32_t min = elapsed_time / 60;
            uint32_t sec = elapsed_time % 60;
            char* str = new char[4];
            drawRectangle(78, 136, 86, 144, bg_color, true);
            str[0] = (char) (min + ((uint8_t) 48));
            str[1] = ':';
            str[2] = (char) (sec / 10 + ((uint8_t) 48));
            str[3] = (char) (sec % 10 + ((uint8_t) 48));
            drawString(78, 136, (const char*) str, 63);
            delete[] str;
            if (elapsed_time % 2 == 0) {
                drawLine(110 + (elapsed_time / 2), 140, 110 + (elapsed_time / 2) + 1, 140, 0);
            }
        }
    }
}

void VGA::initializeScreen(uint8_t color) {
    for (uint32_t i = 0; i < width * length; i ++) {
        vga_buf[i] = color;
    }
}

void VGA::putPixel(uint16_t x, uint16_t y, uint8_t color) {
    uint32_t index = x + y*320;
    char* col = (char*) &color;
    memcpy((char*) (index + vga_buf), col, 1);
    // vga_buf[index] = color;
    
}

void VGA::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color) {
    if (y1 == y2) {
        for (uint16_t i = x1; i <= x2; i++)
            putPixel(i, y1, color);
        return;
    }

    if (x1 == x2) {
        for (uint16_t i = y1; i <= y2; i++) {
            putPixel(x1, i, color);
        }
        return;
    }
}

void VGA::drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, bool fill) {
    if (fill){
        for (uint16_t y = y1; y < y2; y++)
            drawLine(x1, y, x2, y, color);
    } else {
        drawLine(x1, y1, x2, y1, color);
        drawLine(x2, y1, x2, y2, color);
        drawLine(x1, y1, x1, y2, color);
        drawLine(x1, y2, x2, y2, color);
    }
}

void VGA::drawTriangle(uint16_t x1, uint16_t y1, uint16_t length, uint8_t color) {
    while (length > 0) {
        drawLine(x1, y1, x1, y1+length, color);
        length -=2;
        x1++;
        y1++;
    }

}

void VGA::drawCircle(int centerX, int centerY, int radius, uint8_t color) {
    int x = 0;
    int y = radius;
    int d = 5 - 4 * radius;

    while (x <= y) {
        putPixel(centerX + x, centerY + y, color);
        putPixel(centerX + y, centerY + x, color);
        putPixel(centerX - x, centerY + y, color);
        putPixel(centerX - y, centerY + x, color);
        putPixel(centerX + x, centerY - y, color);
        putPixel(centerX + y, centerY - x, color);
        putPixel(centerX - x, centerY - y, color);
        putPixel(centerX - y, centerY - x, color);

        if (d < 0) {
            d += 8 * x + 12;
        }
        else {
            d += 8 * (x - y) + 20;
            y--;
        }
        x++;
    }
}

void VGA::useTextMode(char* buf, uint32_t size) {
    char* text_buf = (char*) 0xB8000;
    for (uint32_t i = 0; i < size * 2; i += 2) {
        text_buf[i] = buf[i / 2];
        text_buf[i + 1] = 0x0F; // just normal white characters
    }
}

void VGA::initTextMode() {
    outb(CRTC_COLOR_INDEX, 0x00); 
    outb(CRTC_COLOR_WRITE, 0x67); 

    outb(CRTC_COLOR_INDEX, 0x01);
    outb(CRTC_COLOR_WRITE, 0x00); 
    outb(CRTC_COLOR_WRITE, 0x01); 

    outb(CRTC_COLOR_INDEX, 0x03); 
    outb(CRTC_COLOR_WRITE, 0x00); 

    outb(CRTC_COLOR_INDEX, 0x04); 
    outb(CRTC_COLOR_WRITE, 0x00); 
    outb(CRTC_COLOR_WRITE, 0x01); 
    outb(CRTC_COLOR_WRITE, 0x02); 

    outb(CRTC_COLOR_INDEX, 0x06);
    outb(CRTC_COLOR_WRITE, 0x0E);

    outb(CRTC_COLOR_INDEX, 0x0E);
    outb(CRTC_COLOR_WRITE, 0x05);

    outb(CRTC_COLOR_INDEX, 0x0F);
    outb(CRTC_COLOR_WRITE, 0x00);

    outb(CRTC_COLOR_INDEX, 0x10);
    outb(CRTC_COLOR_WRITE, 0x05);

    outb(CRTC_COLOR_INDEX, 0x11);
    outb(CRTC_COLOR_WRITE, 0x00);

    outb(CRTC_COLOR_INDEX, 0x14);
    outb(CRTC_COLOR_WRITE, 0x00);

    // Clear screen and reset cursor
    unsigned char* video_memory_text = (unsigned char*)0xB8000;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory_text[i] = ' ';
        video_memory_text[i + 1] = 0x07;
    }
    outb(CRTC_COLOR_INDEX, 0x0F);
    outb(CRTC_COLOR_WRITE, 0x00);
    outb(CRTC_COLOR_INDEX, 0x0E);
    outb(CRTC_COLOR_WRITE, 0x00);
}

void VGA::drawChar(int x, int y, char c, uint8_t color) {
    unsigned char* bitmap = vga_font[(int) c];

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (bitmap[row] & (1 << col)) {
                putPixel(x + col, y + row, color);
            }
        }
    }
}

// Define a function to render a string at a given position
void VGA::drawString(int x, int y, const char* str, uint8_t color) {
    // Loop through each character in the string and render it
    int offset = 0;
    while (*str) {
        drawChar(x + offset, y, *str, color);
        offset += 8; // Advance to the next character position
        str++; // Move to the next character in the string
    }
}

void VGA::homeScreen(const char* name) {
    Shared<Node> bmp = fs->find(fs->root, "320");
    char* rgb = bmp->read_bmp(bmp);
    place_bmp(0, 200, 320, 200, rgb);
}

void VGA::place_bmp(uint32_t x, uint32_t ending_y, uint32_t pic_width, uint32_t pic_length, char* rgb_buf) {
    uint32_t size = pic_length * pic_width * 3;
    uint32_t start_x = x;
    uint32_t y = ending_y;
    for (uint32_t i = 0; i < size; i += 3) {
        if (i % (pic_width * 3) == 0) {
            x = start_x;
            y -= 1;
        }
        uint8_t r = rgb_buf[i];
        uint8_t g = rgb_buf[i+1];
        uint8_t b = rgb_buf[i+2];
        uint8_t color = getColor(r, g, b);
        putPixel(x, y, color);
        x ++;
    }
}

void VGA::spotify(const char* name) {
    drawRectangle(width/2 - 35, length/3 - 35, width/2 + 35, length/3 + 35, BLACK, false);

    Shared<Node> png = fs->find(fs->root, name);

    char* pixels = png->read_bmp(png);

    uint32_t starting_x = width/2 - 35;
    uint32_t starting_y = length/3 + 36;
    place_bmp(starting_x, starting_y, 70, 70, pixels);
    delete pixels;
    play_pause();
}

void VGA::play_pause() {
    Debug::printf("Should Pause\n");
    uint32_t center_x = 160;
    uint32_t center_y = 150;
    uint32_t radius = 15;
    uint8_t color = 25;
    if (!playing) {
        color = 49;
        drawCircle(center_x, center_y, radius, color);
        drawTriangle(center_x-4, center_y-10, 20, bg_color);
        drawRectangle(center_x - 8, center_y - 9, center_x - 3, center_y + 9, 63, true);
        drawRectangle(center_x + 3, center_y - 9, center_x + 8, center_y + 9, 63, true);    
        playing = 1;    
    } else {
        drawRectangle(center_x + 3, center_y - 9, center_x + 8, center_y + 9, bg_color, true);
        drawRectangle(center_x - 8, center_y - 9, center_x - 3, center_y + 9, bg_color, true);
        drawCircle(center_x, center_y, radius, color);
        drawTriangle(center_x-4, center_y-10, 20, 63);
        playing = 0;
    }
}