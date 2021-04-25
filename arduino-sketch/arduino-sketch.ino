
// pin numbers
#define LCD_CS 14
#define LCD_WR 15
#define LCD_DAT 16

/* datasheet says command and write address are sent MSB-first but data is sent LSB-first. wtf....
 * here I reversed the bits of each command so a function sending LSB first could send them */
#define LCD_CMD_PREFIX(x) ((x)<<3|1) /* command begins with bits 101 */
#define LCD_CMD_BIAS_OFF LCD_CMD_PREFIX(0x40)
#define LCD_CMD_BIAS_ON LCD_CMD_PREFIX(0xc0)
#define LCD_CMD_BIAS_COM(ab,c) LCD_CMD_PREFIX(4|(ab)<<4|(c)<<7)
#define LCD_CMD_OSC_ON LCD_CMD_PREFIX(0x80) // turn on system oscillator
#define LCD_CMD_OSC_RC256K LCD_CMD_PREFIX(0x18) // select on-chip 256 kHz RC osc
#define LCD_CMD_OSC_XTAL32K LCD_CMD_PREFIX(0x28) // external 32 kHz crystal
#define LCD_CMD_OSC_EXT256K LCD_CMD_PREFIX(0x38) // external 256 kHz
#define LCD_CMD_TIMER_EN LCD_CMD_PREFIX(0x60) // enable time base output
#define LCD_CMD_NORMAL_MODE LCD_CMD_PREFIX(0xc7) // normal mode (as opposed to test mode)
#define LCD_CMD_TIMEBASE_WDT(tt) LCD_CMD_PREFIX(5|((tt)&1)<<7|((tt)&2)<<5|((tt)&4)<<3)
#define LCD_CMD_CLEAR_WDT LCD_CMD_PREFIX(0x70)

#define AB_2COMMONS 0
#define AB_3COMMONS 2
#define AB_4COMMONS 1
#define C_BIAS_HALF 0
#define C_BIAS_THIRD 1

/* LCD_CMD_BIAS_COM(ab,c):
 * c=0: 1/2 bias option
 * c=1: 1/3 bias option
 * ab=00: 2 commons option
 * ab=01: 3 commons option
 * ab=10: 4 commons option
 */

static uint8_t lcd_mem[4] = {0};
static void lcd_cmd(uint16_t cmd);
static void lcd_refresh(void);

#define SEGDP (1<<3)
#define LCD_VFLIP 0

#if LCD_VFLIP
// rotate the segments 180 degrees for upside-down mounted lcd
#define SEG(a,b,c,d,e,f,g) (a<<7|b<<2|c<<1|d<<0|e<<4|f<<6|g<<5)
#else
#define SEG(a,b,c,d,e,f,g) (a<<0|b<<4|c<<6|d<<7|e<<2|f<<1|g<<5)
#endif

static uint8_t lcd_num(uint8_t i)
{
  static const uint8_t num[10] = {
    SEG(1,1,1,1,1,1,0), //0
    SEG(0,1,1,0,0,0,0), //1
    SEG(1,1,0,1,1,0,1), //2
    SEG(1,1,1,1,0,0,1), //3
    SEG(0,1,1,0,0,1,1), //4
    SEG(1,0,1,1,0,1,1), //5
    SEG(1,0,1,1,1,1,1), //6
    SEG(1,1,1,0,0,0,0), //7
    SEG(1,1,1,1,1,1,1), //8
    SEG(1,1,1,1,0,1,1), //9
  };
  return i < 10 ? num[i] : 0;
}

// could probably communicate faster than 8 kHz
#define lcd_delay() _delay_us(1e6/8000)

static void lcd_cs_low(void) {
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_DAT, OUTPUT);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_CS, LOW);
  lcd_delay();
}

static void lcd_cs_high(void) {
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_CS, HIGH);
  lcd_delay();
}

/* LSB first */
static void lcd_bitbang(uint16_t bits, uint8_t n)
{
  // bits clocked in at rising edge of WR
  while (n) {
    uint8_t databit = bits & 1u;
    digitalWrite(LCD_WR, LOW);
    digitalWrite(LCD_DAT, databit ? HIGH : LOW);
    lcd_delay();

    n -= 1;
    bits >>= 1;
    digitalWrite(LCD_WR, HIGH);
    lcd_delay();
  }
}

static void lcd_cmd(uint16_t cmd)
{
  lcd_cs_low();
  lcd_bitbang(cmd, 3 + 9);
  lcd_cs_high();
}

static void lcd_refresh(void)
{
  uint16_t *src = (uint16_t*) lcd_mem;
  lcd_cs_low();
  lcd_bitbang(5u, 3 + 6); // begin write command at address=0
  lcd_bitbang(src[0], 16);
  lcd_bitbang(src[1], 16);
  lcd_cs_high();
}

static void lcd_init(void)
{
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_DAT, OUTPUT);
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  _delay_ms(50);
  lcd_cmd(LCD_CMD_NORMAL_MODE); // normal, not test mode
  lcd_cmd(LCD_CMD_OSC_ON); // enable oscillator
  lcd_cmd(LCD_CMD_OSC_RC256K); // want the internal oscillator
  lcd_cmd(LCD_CMD_BIAS_ON);
  lcd_cmd(LCD_CMD_BIAS_COM(AB_4COMMONS, C_BIAS_HALF));
  lcd_cmd(LCD_CMD_CLEAR_WDT);
  // datasheet recommends explicitly initializing all settings, TODO
}

void setup() {
  lcd_init();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  static uint8_t num = 0;
  
  digitalWrite(LED_BUILTIN, HIGH);
  
  lcd_mem[0] = lcd_mem[1] = lcd_mem[2] = lcd_mem[3] = lcd_num(num);
  lcd_mem[0] |= SEGDP; //semicolon
  lcd_refresh();

  digitalWrite(LED_BUILTIN, LOW);
  _delay_ms(500);
  
  if (++num == 10) num = 0;
}
