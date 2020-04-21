/*
 * STMPE610 Resistive Touchscreen driver for SPI
 * 
 */

// ==================================================
// Constants
//

//
// Touchscreen pins
//
const int ts_cs_pin = 32;

//
// Calibration constants - these work with the displays I have tried, but you might need to adjust
//
#define STMPE610_X_MIN       160
#define STMPE610_Y_MIN       230
#define STMPE610_X_MAX       3800
#define STMPE610_Y_MAX       3800
#define STMPE610_XY_SWAP     true
#define STMPE610_X_INV       false
#define STMPE610_Y_INV       true



//
// STMPE610 Registers
//
/** 16-bit Chip Version **/
#define STMPE_CHIP_ID 0x00

/** Reset Control **/
#define STMPE_SYS_CTRL1 0x03
#define STMPE_SYS_CTRL1_RESET 0x02

/** Clock Contrl **/
#define STMPE_SYS_CTRL2 0x04

/** SPI Config **/
#define STMPE_SPI_CFG 0x08
#define STMPE_SPI_CFG_MODE0 0x00
#define STMPE_SPI_CFG_MODE1 0x01
#define STMPE_SPI_CFG_MODE2 0x02
#define STMPE_SPI_CFG_MODE3 0x03
#define STMPE_SPI_CFG_AA 0x04

/** Touchscreen controller setup **/
#define STMPE_TSC_CTRL 0x40
#define STMPE_TSC_CTRL_EN 0x01
#define STMPE_TSC_CTRL_XYZ 0x00
#define STMPE_TSC_CTRL_XY 0x02
#define STEMP_TSC_CTRL_TRACK_0 0x00
#define STEMP_TSC_CTRL_TRACK_4 0x10
#define STEMP_TSC_CTRL_TRACK_8 0x20
#define STEMP_TSC_CTRL_TRACK_16 0x30
#define STEMP_TSC_CTRL_TRACK_32 0x40
#define STEMP_TSC_CTRL_TRACK_64 0x50
#define STEMP_TSC_CTRL_TRACK_92 0x60
#define STEMP_TSC_CTRL_TRACK_127 0x70
#define STMPE_TSC_TOUCHED 0x80

/** Interrupt control **/
#define STMPE_INT_CTRL 0x09
#define STMPE_INT_CTRL_POL_HIGH 0x04
#define STMPE_INT_CTRL_POL_LOW 0x00
#define STMPE_INT_CTRL_EDGE 0x02
#define STMPE_INT_CTRL_LEVEL 0x00
#define STMPE_INT_CTRL_ENABLE 0x01
#define STMPE_INT_CTRL_DISABLE 0x00

/** Interrupt enable **/
#define STMPE_INT_EN 0x0A
#define STMPE_INT_EN_TOUCHDET 0x01
#define STMPE_INT_EN_FIFOTH 0x02
#define STMPE_INT_EN_FIFOOF 0x04
#define STMPE_INT_EN_FIFOFULL 0x08
#define STMPE_INT_EN_FIFOEMPTY 0x10
#define STMPE_INT_EN_ADC 0x40
#define STMPE_INT_EN_GPIO 0x80

/** Interrupt status **/
#define STMPE_INT_STA 0x0B
#define STMPE_INT_STA_TOUCHDET 0x01

/** ADC control **/
#define STMPE_ADC_CTRL1 0x20
#define STMPE_ADC_CTRL1_INT   0x00
#define STMPE_ADC_CTRL1_EXT   0x02
#define STMPE_ADC_CTRL1_12BIT 0x08
#define STMPE_ADC_CTRL1_10BIT 0x00
#define STMPE_ADC_CTRL1_36CLK 0x00
#define STMPE_ADC_CTRL1_44CLK 0x10
#define STMPE_ADC_CTRL1_56CLK 0x20
#define STMPE_ADC_CTRL1_64CLK 0x30
#define STMPE_ADC_CTRL1_80CLK 0x40
#define STMPE_ADC_CTRL1_96CLK 0x50
#define STMPE_ADC_CTRL1_124CLK 0x60

/** ADC control **/
#define STMPE_ADC_CTRL2 0x21
#define STMPE_ADC_CTRL2_1_625MHZ 0x00
#define STMPE_ADC_CTRL2_3_25MHZ 0x01
#define STMPE_ADC_CTRL2_6_5MHZ 0x02

/** Touchscreen controller configuration **/
#define STMPE_TSC_CFG 0x41
#define STMPE_TSC_CFG_1SAMPLE 0x00
#define STMPE_TSC_CFG_2SAMPLE 0x40
#define STMPE_TSC_CFG_4SAMPLE 0x80
#define STMPE_TSC_CFG_8SAMPLE 0xC0
#define STMPE_TSC_CFG_DELAY_10US 0x00
#define STMPE_TSC_CFG_DELAY_50US 0x08
#define STMPE_TSC_CFG_DELAY_100US 0x10
#define STMPE_TSC_CFG_DELAY_500US 0x18
#define STMPE_TSC_CFG_DELAY_1MS 0x20
#define STMPE_TSC_CFG_DELAY_5MS 0x28
#define STMPE_TSC_CFG_DELAY_10MS 0x30
#define STMPE_TSC_CFG_DELAY_50MS 0x38
#define STMPE_TSC_CFG_SETTLE_10US 0x00
#define STMPE_TSC_CFG_SETTLE_100US 0x01
#define STMPE_TSC_CFG_SETTLE_500US 0x02
#define STMPE_TSC_CFG_SETTLE_1MS 0x03
#define STMPE_TSC_CFG_SETTLE_5MS 0x04
#define STMPE_TSC_CFG_SETTLE_10MS 0x05
#define STMPE_TSC_CFG_SETTLE_50MS 0x06
#define STMPE_TSC_CFG_SETTLE_100MS 0x07

/** FIFO level to generate interrupt **/
#define STMPE_FIFO_TH 0x4A

/** Current filled level of FIFO **/
#define STMPE_FIFO_SIZE 0x4C

/** Current status of FIFO **/
#define STMPE_FIFO_STA 0x4B
#define STMPE_FIFO_STA_RESET 0x01
#define STMPE_FIFO_STA_OFLOW 0x80
#define STMPE_FIFO_STA_FULL 0x40
#define STMPE_FIFO_STA_EMPTY 0x20
#define STMPE_FIFO_STA_THTRIG 0x10

/** Touchscreen controller drive I **/
#define STMPE_TSC_I_DRIVE 0x58
#define STMPE_TSC_I_DRIVE_20MA 0x00
#define STMPE_TSC_I_DRIVE_50MA 0x01

/** Data port for TSC data address **/
#define STMPE_TSC_DATA_X 0x4D
#define STMPE_TSC_DATA_Y 0x4F
#define STMPE_TSC_DATA_Z 0x51
#define STMPE_TSC_FRACTION_Z 0x56

/** GPIO **/
#define STMPE_GPIO_SET_PIN 0x10
#define STMPE_GPIO_CLR_PIN 0x11
#define STMPE_GPIO_DIR 0x13
#define STMPE_GPIO_ALT_FUNCT 0x17

//
// Touchscreen SPI parameters
//
#define TS_SPI_SETTINGS SPISettings(1000000, MSBFIRST, SPI_MODE1)



// ==================================================
// littlevgl integration
//
bool littlevgl_tp_read(lv_indev_drv_t * indev_driver, lv_indev_data_t *data)
{
  static int16_t last_x = 0;
  static int16_t last_y = 0;
  bool valid = true;
  int c = 0;
  int16_t x = 0;
  int16_t y = 0;
  uint8_t z;

  if ((_ts_read_8bit_reg(STMPE_TSC_CTRL) & STMPE_TSC_TOUCHED) == STMPE_TSC_TOUCHED) {
    // Making sure that we read all data and return the latest point
    while (!_ts_buffer_empty()) {
      _ts_read_data(&x, &y, &z);
      c++;
    }
    
    if (c > 0) {
      _ts_adjust_data(&x, &y);
      last_x = x;
      last_y = y;
    }
    //Serial.printf("%d %d\n", x, y);
    z = _ts_read_8bit_reg(STMPE_INT_STA);  // Clear interrupts
    z = _ts_read_8bit_reg(STMPE_FIFO_STA);
    if ((z & STMPE_FIFO_STA_OFLOW) == STMPE_FIFO_STA_OFLOW) {
      // Clear the FIFO if we discover an overflow
      _ts_write_8bit_reg(STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
      _ts_write_8bit_reg(STMPE_FIFO_STA, 0); // unreset
    }
  }
    
  if (c == 0) {
    x = last_x;
    y = last_y;
    valid = false;
  }

  data->point.x = (int16_t) x;
  data->point.y = (int16_t) y;
  data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;

  return false;
}



// ==================================================
// API Code
//
void ts_begin()
{
  // Initialize the hardware
  pinMode(ts_cs_pin, OUTPUT);
  digitalWrite(ts_cs_pin, HIGH);

  stmpe610_init();
}



// ==================================================
// Internal Module Code
//
void stmpe610_init(void)
{
  uint8_t u8;
  uint16_t u16;
  
  // Get the initial SPI configuration
  //u8 = read_8bit_reg(STMPE_SPI_CFG);
  //ESP_LOGI(TAG, "SPI_CFG = 0x%x", u8);
  
    // Attempt a software reset
  _ts_write_8bit_reg(STMPE_SYS_CTRL1, STMPE_SYS_CTRL1_RESET);
  vTaskDelay(10 / portTICK_RATE_MS);
  
  // Reset the SPI configuration, making sure auto-increment is set
  u8 = _ts_read_8bit_reg(STMPE_SPI_CFG);
  _ts_write_8bit_reg(STMPE_SPI_CFG, u8 | STMPE_SPI_CFG_AA);
  u8 = _ts_read_8bit_reg(STMPE_SPI_CFG);
  
  // Verify SPI communication
  u16 = _ts_read_16bit_reg(STMPE_CHIP_ID);
  if (u16 != 0x811) {
    Serial.printf("TS: Incorrect version: 0x%x\n", u16);
  }

  _ts_write_8bit_reg(STMPE_SYS_CTRL2, 0x00); // Disable clocks
  _ts_write_8bit_reg(STMPE_TSC_CTRL, 0);     // Disable to allow writing
  
  _ts_write_8bit_reg(STMPE_TSC_CTRL,
                   STEMP_TSC_CTRL_TRACK_0 | 
                   STMPE_TSC_CTRL_XYZ |
                   STMPE_TSC_CTRL_EN);
  
  _ts_write_8bit_reg(STMPE_TSC_CFG, STMPE_TSC_CFG_4SAMPLE |
                   STMPE_TSC_CFG_DELAY_1MS |
                   STMPE_TSC_CFG_SETTLE_1MS);
                   
  _ts_write_8bit_reg(STMPE_TSC_FRACTION_Z, 0x7);
  _ts_write_8bit_reg(STMPE_TSC_I_DRIVE, STMPE_TSC_I_DRIVE_50MA);
  
  _ts_write_8bit_reg(STMPE_SYS_CTRL2, 0x04); // GPIO clock off, TSC clock on, ADC clock on
  
  _ts_write_8bit_reg(STMPE_ADC_CTRL1, STMPE_ADC_CTRL1_12BIT | STMPE_ADC_CTRL1_80CLK);
  _ts_write_8bit_reg(STMPE_ADC_CTRL2, STMPE_ADC_CTRL2_3_25MHZ);
  
  _ts_write_8bit_reg(STMPE_GPIO_ALT_FUNCT, 0x00);  // Disable GPIO

  _ts_write_8bit_reg(STMPE_FIFO_TH, 1);                      // Set FIFO threshold
  _ts_write_8bit_reg(STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);  // Assert FIFO reset
  _ts_write_8bit_reg(STMPE_FIFO_STA, 0);                     // Deassert FIFO reset
  
  _ts_write_8bit_reg(STMPE_INT_EN, 0x00);  // No interrupts
  _ts_write_8bit_reg(STMPE_INT_STA, 0xFF); // reset all ints
}


void _ts_write_8bit_reg(uint8_t reg, uint8_t val)
{
  SPI.beginTransaction(TS_SPI_SETTINGS);
  digitalWrite(ts_cs_pin, LOW);
  SPI.transfer(reg);
  SPI.transfer(val);
  digitalWrite(ts_cs_pin, HIGH);
  SPI.endTransaction(); 
}


uint16_t _ts_read_16bit_reg(uint8_t reg)
{
  uint8_t data_recv[2];
  
  SPI.beginTransaction(TS_SPI_SETTINGS);
  digitalWrite(ts_cs_pin, LOW);
  SPI.transfer(0x80 | reg); // Read
  data_recv[0] = SPI.transfer(0x80 | (reg+1));
  data_recv[1] = SPI.transfer(0);
  digitalWrite(ts_cs_pin, HIGH);
  SPI.endTransaction(); 
  
  return data_recv[0] << 8 | data_recv[1];
}


uint8_t _ts_read_8bit_reg(uint8_t reg)
{ 
  uint8_t data_recv;
  
  SPI.beginTransaction(TS_SPI_SETTINGS);
  digitalWrite(ts_cs_pin, LOW);
  SPI.transfer(0x80 | reg); // Read
  SPI.transfer(0);
  data_recv = SPI.transfer(0);
  digitalWrite(ts_cs_pin, HIGH);
  SPI.endTransaction(); 
  
  return data_recv;
}


void _ts_read_data(int16_t *x, int16_t *y, uint8_t *z)
{
  *x = _ts_read_16bit_reg(STMPE_TSC_DATA_X);
  *y = _ts_read_16bit_reg(STMPE_TSC_DATA_Y);
  *z = _ts_read_8bit_reg(STMPE_TSC_DATA_Z);
}


bool _ts_buffer_empty()
{
  return ((_ts_read_8bit_reg(STMPE_FIFO_STA) & STMPE_FIFO_STA_EMPTY) == STMPE_FIFO_STA_EMPTY);
}


void _ts_adjust_data(int16_t * x, int16_t * y)
{
#if STMPE610_XY_SWAP != 0
    int16_t swap_tmp;
    swap_tmp = *x;
    *x = *y;
    *y = swap_tmp;
#endif

    if((*x) > STMPE610_X_MIN)(*x) -= STMPE610_X_MIN;
    else(*x) = 0;

    if((*y) > STMPE610_Y_MIN)(*y) -= STMPE610_Y_MIN;
    else(*y) = 0;

    (*x) = (uint32_t)((uint32_t)(*x) * LV_HOR_RES) /
           (STMPE610_X_MAX - STMPE610_X_MIN);

    (*y) = (uint32_t)((uint32_t)(*y) * LV_VER_RES) /
           (STMPE610_Y_MAX - STMPE610_Y_MIN);

#if STMPE610_X_INV != 0
    (*x) =  LV_HOR_RES - (*x);
#endif

#if STMPE610_Y_INV != 0
    (*y) =  LV_VER_RES - (*y);
#endif

}
