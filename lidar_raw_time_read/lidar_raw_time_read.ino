/* File: lidar_raw_time_read.ino
 * Name: Kelvin Lu
 * Desc: Continously calculates the distance measured 
 *       from an lidar sensor based on OSLRF-01
 */
#define ZERO_OUT_PIN   (A0) // Laser reading
#define SIGNAL_OUT_PIN (A1) // Photodetector reading
#define SYNC_PIN       (2)  // Sq. wave reference signal from control

#define REF_DIST_CM    (1833) // Reference distance of sensor in cm
#define SENSOR_THRESH  (50)   // ~250 mV analog threshold for signal

unsigned long sync_period = 0; // Sync square wave period
unsigned long sync_start  = 0; // Time when each measurement started
unsigned long distance    = 0;

void setup() {
  /* Set sync pin as input 
   * Begin serial comm
   * Time the period of the sync sq wave
   */
  pinMode(SYNC_PIN, INPUT);
  Serial.begin(115200);
  sync_period = get_sync_period(SYNC_PIN);
}

void loop() {
  /* Wait for falling edge of sync sq wave
   * to trigger distance calculation then display
   * over serial
   */
  while (!digitalRead(SYNC_PIN)) {}
  while ( digitalRead(SYNC_PIN)) {}
  sync_start = micros();

  distance = get_distance(ZERO_OUT_PIN, SIGNAL_OUT_PIN, sync_start);

  Serial.println(distance);
}

/* Calculates the period of the sync signal by
 * measuring the time between successive falling edges
 * 
 * @param  sync_pin    Pin on micro for the sync signal
 * 
 * @return sync_period Time in us of sync signal period
 */
unsigned long get_sync_period(int sync_pin) {
  unsigned long first_falling_edge  = 0;
  unsigned long second_falling_edge = 0;

  while (!digitalRead(sync_pin)) {} 
  while ( digitalRead(sync_pin)) {}
  first_falling_edge = micros();

  while (!digitalRead(sync_pin)) {}
  while ( digitalRead(sync_pin)) {}
  second_falling_edge = micros();

  return (second_falling_edge - first_falling_edge);
}

/* Calculates the distance from sensor based on time
 * laser is fired and photodetector retrives signal
 * 
 * 1.) Sp - period of sync signal falling edge to falling edge
 * 2.) Zt - time of laser pulse from falling edge of sync
 * 3.) Rt - time of detector signal from falling edge of sync
 * 
 * SENSOR_THRESH is roughly ~250mV to make sure actual signal
 *               and not just noise
 * 
 * @param  zero_pin   Pin on micro where laser pulse is captured
 * @param  signal_pin Pin on micro where detector signal is captured
 * @param  sync_start Time in us when measurement started
 * 
 * @return distance  ((Rt - Zt) / Sp) * 18.33 m
 */
unsigned long get_distance(int zero_pin, int signal_pin, int sync_start) {
  unsigned long laser_out_t = 0;
  unsigned long detector_t  = 0;
  unsigned long tof         = 0;

  while (analogRead(zero_pin) < SENSOR_THRESH) {}
  laser_out_t = micros() - sync_start;

  while (analogRead(signal_pin) < SENSOR_THRESH) {}
  detector_t = micros() - sync_start;

  tof = ((laser_out_t - detector_t) / sync_period);

  return (tof * REF_DIST_CM);
}
