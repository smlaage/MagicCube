// Zauberwürfel-Testprogramm
// Version 0.7 - 21-09-2021

const bool debug = true;

// Anschluss-Pins
const int col_edge_out = 2, col_edge_s2 = 3, col_edge_s3 = 4;
const int col_corner_out = 5, col_corner_s2 = 6, col_corner_s3 = 7;
const int m_enable[] =   {22, 23, 24, 25, 26, 27};
const int m_step[] = {28, 29, 30, 31, 32, 33};
const int m_dir[] =    {34, 35, 36, 37, 38, 39};
const int bt_left[] =  {40, 41, 42, 43, 44, 45};
const int bt_right[] = {46, 47, 48, 49, 50, 51};

// Konstanten
const int mot_delay = 3;    // Motor-Geschwindigkeit
const int mot_turn = 100;   // Anzahl Schritte für eine 90 Grad Drehung
const int average_cnt = 20; // Anzahl der Werte füpr eine Durchschnittsberechnung
const char col_str[7][10] = {"yellow", "white", "orange", "red", "blue", "green", "undefined"};


//----------------------------------------------------------------
// Dreht einen Motor. Parameter:
// m -> Nummer des Motors (0 ... 5)
// cnt -> Anzahl der Schritte (100 entspricht 90 Grad)
// dir -> Laufrichtung links oder rechts (false oder true)
void run_motor(int m, int cnt, bool dir) {
  digitalWrite(m_dir[m], dir);        // Richtung setzen
  digitalWrite(m_enable[m], LOW);     // Motor einschalten
  for (int i = 0; i < cnt; ++i) {     // den Motor mit der Anzahl Schritte laufen lassen
    digitalWrite(m_step[m], HIGH);
    delay(mot_delay);
    digitalWrite(m_step[m], LOW);
    delay(mot_delay);
  }
  digitalWrite(m_enable[m], HIGH);     // Motor ausschalten
}

//----------------------------------------------------------------
// Liest den Farbsensor "Edge" aus und lädt das Ergebnis in die drei Variablen 
void get_col_edge(long *red, long *blue, long *green, long *white) {
  *red = 0; *blue = 0; *green = 0; *white = 0;
  // Der Sensor wird 20 mal gelesen. Dann werden die Mittelwerte berechnet
  for (int i = 0; i < 20; ++i) {
    digitalWrite(col_edge_s2, LOW);
    digitalWrite(col_edge_s3, LOW);
    delay(1);
    *red += pulseIn(col_edge_out, LOW, 10000);
    digitalWrite(col_edge_s3, HIGH);
    delay(1);
    *blue += pulseIn(col_edge_out, LOW, 10000);
    digitalWrite(col_edge_s2, HIGH);
    delay(1);
    *green += pulseIn(col_edge_out, LOW, 10000);
    digitalWrite(col_edge_s3, LOW);
    delay(1);
    *white += pulseIn(col_edge_out, LOW, 10000);
  }
  *white = (int) (*white / 20);
  *red = (int) (*red / 20);
  *blue = (int) (*blue / 20);
  *green = (int) (*green / 20);
}

//----------------------------------------------------------------
// Liest den Farbsensor "Corner" aus und lädt das Ergebnis in die drei Variablen 
void get_col_corner(long *red, long *blue, long *green, long *white) { 
  *red = 0; *blue = 0; *green = 0, *white=0;
  // Der Sensor wird 20 mal gelesen. Dann werden die Mittelwerte berechnet
  for (int i = 0; i < 20; ++i) {
    digitalWrite(col_corner_s2, LOW);
    digitalWrite(col_corner_s3, LOW);
    delay(1);
    *red += pulseIn(col_corner_out, LOW, 10000);
    digitalWrite(col_corner_s3, HIGH);
    delay(1);
    *blue += pulseIn(col_corner_out, LOW, 10000);
    digitalWrite(col_corner_s2, HIGH);
    delay(1);
    *green += pulseIn(col_corner_out, LOW, 10000);
    digitalWrite(col_corner_s3, LOW);
    delay(1);
    *white += pulseIn(col_corner_out, LOW, 10000);
  }
  *red = (int) (*red / 20);
  *blue = (int) (*blue / 20);
  *green = (int) (*green / 20);
  *white = (int) (*white / 20);
}


//----------------------------------------------------------------
// Decode color edge
// Diese Funktion findet die Farbe. Der Ausgabewert ist die Nummer der 
// gefundenen Farbe (0 ... 5) oder 6, wenn keine passende Farbe gefunden 
// werden konnte.
int decode_col_edge(void) {
  // Tabelle mit den Grenzwerten
  // red_min, red_max, blue_min, blue_max, green_min, green_max, white_min, white_max  
  const int col_tab[6][8] = {
    220, 310,    405, 875,     340,  415,    23,  65,   // yellow
    260, 365,    370, 470,     400,  520,    30,  55,   // white
    160, 210,    400, 910,     430, 1010,    25,  60,   // orange
    130, 190,    305, 490,     320,  490,    30, 150,   // red
    176, 205,    270, 315,     305,  390,    36, 160,   // blue
    200, 285,    350, 540,     280,  330,    45, 100,   // green
  };
  long red, blue, green, white;
  int red_norm, blue_norm, green_norm;
  double denominator;
  int i;

  // get the color values
  get_col_edge(&red, &blue, &green, &white);

  // caculate normalized color values
  denominator = 0.02 * white * white + 5.0 * white;
  red_norm  = (int) ((red   * 1000.0) / denominator);
  blue_norm  = (int) ((blue  * 1000.0) / denominator);
  green_norm = (int) ((green * 1000.0) / denominator);
  
  // find matches
  for (i = 0; i < 6; ++i) {
    if ((red_norm   >= col_tab[i][0]) && (red_norm   <= col_tab[i][1]) &&
        (blue_norm  >= col_tab[i][2]) && (blue_norm  <= col_tab[i][3]) &&
        (green_norm >= col_tab[i][4]) && (green_norm <= col_tab[i][5]) &&
        (white      >= col_tab[i][6]) && (white      <= col_tab[i][7])) {
          break;
    }
  }

  if (debug && (i > 5)) {
    Serial.println();
    Serial.print("=====> Undefined Edge ");
    Serial.print("R "); print_dec(red_norm); Serial.print(",");
    Serial.print("  B "); print_dec(blue_norm); Serial.print(",");
    Serial.print("  G "); print_dec(green_norm);  Serial.print(",");
    Serial.print("  W "); print_dec(white); 
    Serial.println(" =================");
  }

  return i;
}

//----------------------------------------------------------------
// Decode color corner
// Diese Funktion findet die Farbe. Input sind die Sensor-Werte
// für rot, blau, grün und weiß.
// Der Ausgabewert ist die Nummer der gefundenen Farbe (0 ... 5) oder 6, wenn 
// keine passende Farbe gefunden werden konnte.
 
int decode_col_corner(void) {
  // Tabelle mit den Grenzwerten
  // red_min, red_max, blue_min, blue_max, green_min, green_max, white_min, white_max  
  const int col_tab[6][8] = {
    225, 300,    425,  870,   340,  415,    25,  65,   // yellow
    240, 365,    370,  505,   390,  540,    28,  60,   // white
    170, 255,    380,  915,   400, 1010,    25,  65,   // orange
    125, 185,    310,  490,   320,  500,    40, 165,   // red
    160, 190,    255,  309,   300,  390,    40, 180,   // blue
    200, 270,    320,  550,   220,  320,    35, 115,   // green
  };
  long red, blue, green, white;
  int red_norm, blue_norm, green_norm;
  double denominator; 
  int i;

  // get the color values
  get_col_corner(&red, &blue, &green, &white);

  // caculate normalized color values
  denominator = 0.02 * white * white + 5.0 * white;
  red_norm   = (int) ((red   * 1000.0) / denominator);
  blue_norm  = (int) ((blue  * 1000.0) / denominator);
  green_norm = (int) ((green * 1000.0) / denominator);
 
  // find matches
  for (i = 0; i < 6; ++i) {
    if ((red_norm   >= col_tab[i][0]) && (red_norm   <= col_tab[i][1]) &&
        (blue_norm  >= col_tab[i][2]) && (blue_norm  <= col_tab[i][3]) &&
        (green_norm >= col_tab[i][4]) && (green_norm <= col_tab[i][5]) &&
        (white      >= col_tab[i][6]) && (white      <= col_tab[i][7])) {
          break;
    }
  }

  if (debug && (i > 5)) {
    Serial.println();
    Serial.print("=====> Undefined Corner ");
    Serial.print("R "); print_dec(red_norm); Serial.print(",");
    Serial.print("  B "); print_dec(blue_norm); Serial.print(",");
    Serial.print("  G "); print_dec(green_norm);  Serial.print(",");
    Serial.print("  W "); print_dec(white); 
    Serial.println(" =================");
  }
  
  return i;
}


//----------------------------------------------------------------
// Gibt eine Zahl mit führenden Leerzeichen (rechtsbündig) auf dem seriellen Monitor aus
void print_dec(int i) {
  bool minus = false;
  if (i < 0) {
    minus = true;
    i = -i;
  }
  if (i < 1000) Serial.print(' ');
  if (i < 100) Serial.print(' ');
  if (i < 10) Serial.print(' ');
  if (minus) Serial.print('-');
  else Serial.print(' ');
  Serial.print(i);
}

//----------------------------------------------------------------
// Fragt die Tasten ab und dreht den Motor, wenn eine Taste gedrückt wurde.
bool check_buttons(void) {
  bool button_pressed = false;
  
  for (int i = 0; i < 6; ++i) {
    if (digitalRead(bt_left[i]) == LOW) {       // eine gedrückte Taste bedeutet "LOW"
      Serial.print("Motor ");                   // Motordrehung auf dem seriellen Monitor anzeigen
      Serial.print(i);
      Serial.print(" links ... ");
      run_motor(i, mot_turn, LOW);              // Motor drehen lassen
      Serial.println("fertig");
      button_pressed = true;
      while (digitalRead(bt_left[i]) == LOW);   // warte, bis der Taster wieder losgelassen wurde
    }
    if (digitalRead(bt_right[i]) == LOW) {      // dasselbe mit der anderen Tastenreihe
      Serial.print("Motor "); 
      Serial.print(i);
      Serial.print(" rechts ... ");
      run_motor(i, mot_turn, HIGH);
      Serial.println("fertig");
      button_pressed = true;
      while (digitalRead(bt_right[i]) == LOW);  // warte, bis der Taster wieder losgelassen wurde
    }
  }
  return button_pressed;
}

//----------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Zauberwürfel Test-Programm Version 0.7");
  // set I/O pins
  pinMode(col_edge_out, INPUT_PULLUP);
  pinMode(col_edge_s2, OUTPUT);
  pinMode(col_edge_s3, OUTPUT);
  pinMode(col_corner_out, INPUT);
  pinMode(col_corner_s2, OUTPUT);
  pinMode(col_corner_s3, OUTPUT);  
  for (int i = 0; i < 6; ++i) {
    pinMode(m_enable[i], OUTPUT);
    pinMode(m_step[i], OUTPUT);
    pinMode(m_dir[i], OUTPUT);
    pinMode(bt_left[i], INPUT_PULLUP);
    pinMode(bt_right[i], INPUT_PULLUP);
  }
  // disable the drives
  for (int i = 0; i < 6; ++i) {       
    digitalWrite(m_enable[i], HIGH);
    digitalWrite(m_step[i], LOW);
    digitalWrite(m_dir[i], LOW);
  }

}

//----------------------------------------------------------------
// Das ist das Hauptprogramm, von dem alles aufgerufen wird
//----------------------------------------------------------------
void loop() {
  int color;

  Serial.println();  
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 10000; ++j) {
      if (check_buttons()) break; 
    };
    Serial.print("E: ");
    color = decode_col_edge();
    if (color > 5) while(1);
    Serial.print(col_str[color]); Serial.print("  ");  
    for (int j = 0; j < 10000; ++j) {
      if (check_buttons()) break; 
    };
    Serial.print("C: ");
    color = decode_col_corner();
    if (color > 5) while(1);
    Serial.print(col_str[color]); Serial.print("  "); 
  }
  run_motor(3, 100, LOW);              // Motor drehen lassen
  run_motor(4, 100, HIGH);             // Motor drehen lassen
  run_motor(5, 100, LOW);
}
