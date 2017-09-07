/*
  This Arduino program functions as a poor-man's air quality detector. It uses
  the Sainsmart TGS2620, which measures the levels of various hazardous gases
  in the air. It measures these using a circuit that lowers its resistance in
  the presence of these gases. Thus, by taking a ratio of the measured
  resistance and the calibrated normal resistance, we can get a rough estimate
  of the levels of a particular hazardous gas. There are graphs shown in the
  datasheet for calculating parts-per-million for a given gas, but they depend
  on temperature and humidity, and the exact curves are not given. So, we
  output a ratio. However, it very clearly is working, as you can see the
  effect if you put the sensor over a glass of wine wine and see the values go
  notably down as the sensor detects ethanol.
*/

/* The LED to blink. */
const int led = 13;

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
}

/*
 The load resistor applied to sensor(-) in series with GND.
 Note that this is hard-coded for my particular setup,
 so you should change it for a different setup.
*/
const double rl = 44.2*1000;

/* The input current. */
const double vc = 5.0;

/* The LED blink period. */
double period = 1000.0;

void toggle_led()
{
  static int led_state = LOW;

  if (led_state == LOW) {
    led_state = HIGH;
  }
  else {
    led_state = LOW;
  }

  digitalWrite(led, led_state);
}

double calc_ohms(
  unsigned int sample_count,
  unsigned int sample_delay,
  boolean verbose)
{
  unsigned int i;
  unsigned int now;
  unsigned long last_blink;

  unsigned int sum;
  double vout;

  sum = 0;
  last_blink = millis();
  for (i = 0; i < sample_count; ++i) {
    sum += analogRead(A0);
    delay(sample_delay);
    if (verbose) {
      Serial.print(".");
    }

    now = millis();
    if (now - last_blink >= period) {
      toggle_led();
      last_blink = now;
    }
  }

  /*
     Scale the ADC 10-bit values (0-1023) to the voltage vc and compute an
     average.
  */
  vout = ((double)sum) * vc/1023 / ((double)sample_count);
  /* Use the formula from the datasheet for rs. */
  return vc*rl/vout - rl;
}

void loop()
{
  double r0;
  double ratio;
  double rs;

  /* Calibration. */
  Serial.println("Calibrating, keep the air quality clean!");
  r0 = calc_ohms(20, 1000, true);
  Serial.print("\nCalibration: base resistance r0 = ");
  Serial.print(r0, 1);
  Serial.println(" ohms");

  while (true) {
    rs = calc_ohms(500, 1, false);
    ratio = rs/r0;

    Serial.print("rs (current resistance, ohms): ");
    Serial.println(rs, 3);
    Serial.print("ratio rs/r0 (lower == more substance detected): ");
    Serial.println(ratio, 3);

    /*
       Make blink frequency be at ratio/10 Hz, so that a lower ratio (more substance)
       means faster blinking.
     */
     period = 1000.0/10.0 * ratio;
  }
}
